# 大模型反向传播：工程实现与梯度计算全解

> **目标读者**：已理解链式法则，想知道 PyTorch/JAX 实际上是怎么把梯度算出来的，以及 LLM 各核心算子的梯度公式。

---

## 目录

1. [计算图与自动微分（Autograd）](#1-计算图与自动微分autograd)
2. [工程实现：从符号到数值](#2-工程实现从符号到数值)
3. [反向传播的内存与调度策略](#3-反向传播的内存与调度策略)
4. [LLM 核心算子梯度推导](#4-llm-核心算子梯度推导)
   - 4.1 Linear / MatMul
   - 4.2 Softmax
   - 4.3 Cross-Entropy Loss
   - 4.4 LayerNorm
   - 4.5 RMSNorm
   - 4.6 Attention（Scaled Dot-Product）
   - 4.7 激活函数（GeLU / SiLU / ReLU）
   - 4.8 Embedding Lookup
5. [梯度流动的完整路径示例（Transformer Block）](#5-梯度流动的完整路径示例transformer-block)
6. [工程优化技巧](#6-工程优化技巧)
7. [常见数值问题与调试](#7-常见数值问题与调试)

---

## 1. 计算图与自动微分（Autograd）

### 1.1 什么是计算图

每一次前向计算，框架都会隐式构建一张**有向无环图（DAG）**：

```
x ──► mul ──► add ──► sigmoid ──► Loss
w ──►         │
              b ──►
```

- **节点（Node）**：每个算子（op），存储前向结果和一个 `backward_fn`
- **边（Edge）**：张量依赖关系，同时记录"这个张量是哪个 op 的第几个输入"

PyTorch 中每个 `Tensor` 如果 `requires_grad=True`，就会有 `.grad_fn` 指向产生它的节点：

```python
x = torch.randn(3, requires_grad=True)
y = x * 2        # y.grad_fn = MulBackward0
z = y.sum()      # z.grad_fn = SumBackward0
z.backward()     # 触发图上的拓扑排序 + 逐节点调用 backward_fn
```

### 1.2 两种自动微分模式

| 模式 | 方向 | 适用场景 | 复杂度（参数量 P，输出量 O）|
|------|------|---------|--------------------------|
| **前向模式（Forward AD）** | 输入 → 输出，同步计算 Jacobian 列 | 输入少、输出多 | O(P) 次前向 |
| **反向模式（Reverse AD）** | 输出 → 输入，沿图反向传播 | 输入多（参数）、输出少（标量 Loss）| **1 次前向 + 1 次反向** |

深度学习参数量 >> 输出（标量 Loss），因此**永远用反向模式**。

### 1.3 反向模式的核心：VJP（Vector-Jacobian Product）

链式法则的本质是矩阵乘法。设算子 $f: \mathbb{R}^n \to \mathbb{R}^m$，其 Jacobian 为 $J \in \mathbb{R}^{m \times n}$。

反向传播传递的不是完整的 Jacobian，而是**上游梯度向量** $v \in \mathbb{R}^m$（即 $\partial L/\partial \text{output}$）与 Jacobian 的乘积：

$$\frac{\partial L}{\partial \text{input}} = v^\top J \quad \in \mathbb{R}^n$$

每个算子只需实现自己的 **`vjp`（VJP 函数）**，无需显式构造完整 Jacobian。

> **关键洞察**：框架对每个 op 注册两个函数——`forward()` 和 `backward(grad_output)`。`backward` 接收上游梯度，返回对每个输入的下游梯度。整个反向传播就是图的拓扑逆序遍历，依次调用各节点的 `backward`。

---

## 2. 工程实现：从符号到数值

### 2.1 PyTorch 的执行流程

```
z.backward()
    │
    ▼
Engine.execute_with_graph(z.grad_fn)
    │
    ├─ 拓扑排序（基于引用计数）
    │
    ├─ 初始化：z 的梯度 = 1.0（标量 Loss 对自身导数）
    │
    └─ 逆序遍历每个 Node：
           grad_inputs = node.backward(grad_outputs)
           将 grad_inputs 累加到对应 Tensor.grad
```

`grad` 是**累加**而非赋值，这是为了支持同一参数在不同位置复用（如 tied embedding）。

### 2.2 算子级别的实现（以 PyTorch C++ 为例）

对于 `y = x * w`（element-wise multiply）：

```cpp
// forward: 保存 x, w 供 backward 使用
auto result = at::mul(x, w);
ctx->save_for_backward({x, w});

// backward: 接收上游 grad_output
auto [x_saved, w_saved] = ctx->get_saved_tensors();
auto grad_x = grad_output * w_saved;   // ∂L/∂x = ∂L/∂y · w
auto grad_w = grad_output * x_saved;   // ∂L/∂w = ∂L/∂y · x
```

每个算子的 `backward` 函数就是手写的 VJP。框架内置了几百个这样的实现。

### 2.3 JAX 的不同哲学：函数变换

JAX 不构建显式计算图，而是**函数变换**：

```python
import jax

def loss_fn(params, x):
    return model(params, x).sum()

# grad() 返回一个新函数，调用时直接返回梯度
grad_fn = jax.grad(loss_fn, argnums=0)
grads = grad_fn(params, x)
```

JAX 内部用 **Jaxpr（JAX expression）** 作为中间表示，然后对 Jaxpr 做反向变换。本质上与 PyTorch 相同，只是用函数式风格封装。

### 2.4 梯度累加与 micro-batch

大模型训练时 batch 放不下，用梯度累加模拟大 batch：

```python
optimizer.zero_grad()
for i, micro_batch in enumerate(micro_batches):
    loss = model(micro_batch) / len(micro_batches)
    loss.backward()   # 每次 backward，梯度自动累加到 .grad
optimizer.step()      # 累加完再更新
```

---

## 3. 反向传播的内存与调度策略

### 3.1 激活内存问题

前向时需要保存中间激活（供 backward 使用），对于大模型这是主要内存瓶颈：

- **Transformer 1 层**的激活约占 $O(\text{seq\_len} \times \text{hidden} \times \text{batch})$
- GPT-3（96层）前向激活 ≈ 几十 GB

### 3.2 Gradient Checkpointing（重计算）

**核心思想**：前向时丢弃中间激活，backward 时重新前向计算一次。

```python
from torch.utils.checkpoint import checkpoint

# 普通前向（保存所有激活）
y = transformer_block(x)

# 使用 checkpointing（前向时丢激活，backward 时重算）
y = checkpoint(transformer_block, x)
```

**代价**：每层多一次前向计算（约增加 33% 计算量），但内存从 $O(N)$ 降到 $O(\sqrt{N})$。

### 3.3 流水线并行中的梯度调度

在流水线并行（Pipeline Parallelism）中，多个 micro-batch 交错执行：

```
时间 ─────────────────────────────►
GPU0: F0  F1  F2  F3  B3  B2  B1  B0
GPU1:     F0  F1  F2  F3  B3  B2  B1  B0
GPU2:         F0  F1  F2  F3  B3  B2  B1  B0
```

每个 GPU 只持有部分层，梯度在 bubble 时间内完成，通过 `AllReduce` 跨 GPU 同步。

---

## 4. LLM 核心算子梯度推导

> **符号约定**
> - $L$：标量 Loss
> - $\bar{X} = \partial L / \partial X$：上游传来的梯度（与 $X$ 同形状）
> - $\odot$：element-wise 乘法
> - $\text{tr}$：矩阵迹

---

### 4.1 Linear / MatMul

**前向**：$Y = XW + b$，其中 $X \in \mathbb{R}^{B \times \text{in}}$，$W \in \mathbb{R}^{\text{in} \times \text{out}}$

**反向**：

$$\boxed{\bar{X} = \bar{Y} W^\top}$$

$$\boxed{\bar{W} = X^\top \bar{Y}}$$

$$\boxed{\bar{b} = \sum_{\text{batch}} \bar{Y}}$$

**直觉**：$\bar{X}$ 需要把输出的每个梯度"分配回"输入的每个维度，权重矩阵是路由器；$\bar{W}$ 是输入与输出梯度的外积之和。

**工程细节**：实现上就是两次 `matmul`（`GEMM`），可以直接复用高度优化的 cuBLAS kernel。

---

### 4.2 Softmax

**前向**：$s_i = \frac{e^{x_i}}{\sum_j e^{x_j}}$

**反向**：设上游梯度为 $\bar{s}$，

$$\bar{x}_i = s_i \left( \bar{s}_i - \sum_j \bar{s}_j s_j \right)$$

向量形式：

$$\boxed{\bar{x} = s \odot \left(\bar{s} - \langle \bar{s}, s \rangle \right)}$$

其中 $\langle \bar{s}, s \rangle = \sum_j \bar{s}_j s_j$ 是一个标量（dot product）。

**直觉**：Softmax 输出的所有分量之和为 1（有约束），所以改变一个输入会影响所有输出。那个标量项正是在"减去共享的影响"。

**数值稳定性**：前向用 `x - max(x)` 防止 overflow，backward 无需特殊处理（因为用的是前向保存的 $s$，已经稳定）。

---

### 4.3 Cross-Entropy Loss

**前向**（配合 Softmax）：

$$L = -\log s_y = -x_y + \log \sum_j e^{x_j}$$

**反向**（对 logit $x_i$ 的梯度，已融合 softmax）：

$$\boxed{\bar{x}_i = s_i - \mathbf{1}[i = y]}$$

即：**预测概率减去 one-hot 标签**。这是最干净的梯度形式，也是为什么 LLM 训练中 loss backward 如此高效——梯度计算不需要完整的 Jacobian，一次 softmax 前向结果加减法即可。

---

### 4.4 LayerNorm

**前向**：

$$\hat{x} = \frac{x - \mu}{\sqrt{\sigma^2 + \epsilon}}, \quad y = \gamma \odot \hat{x} + \beta$$

其中 $\mu = \text{mean}(x)$，$\sigma^2 = \text{var}(x)$，$\gamma, \beta$ 是可学习参数。

**反向**（对 $x$ 的梯度，设 $N$ 为归一化维度大小）：

$$\bar{x} = \frac{\gamma}{\sqrt{\sigma^2+\epsilon}} \cdot \frac{1}{N} \left[ N\bar{y}' - \sum_j \bar{y}'_j - \hat{x} \sum_j \bar{y}'_j \hat{x}_j \right]$$

其中 $\bar{y}' = \bar{y} \odot \gamma$（即上游梯度先经过 $\gamma$ 缩放）。

$$\boxed{\bar{\gamma} = \sum_{\text{batch}} \bar{y} \odot \hat{x}}$$

$$\boxed{\bar{\beta} = \sum_{\text{batch}} \bar{y}}$$

**工程细节**：LayerNorm backward 的难点在于 $\mu$ 和 $\sigma^2$ 都依赖所有 $x_i$，所以 $\bar{x}_i$ 不是独立计算的，需要先计算两个 reduce 量（$\sum \bar{y}'$ 和 $\sum \bar{y}' \hat{x}$）再广播回去。在 CUDA kernel 中通常用 warp-level reduce 实现。

---

### 4.5 RMSNorm

**前向**：

$$y = \frac{x}{\text{RMS}(x)} \cdot \gamma, \quad \text{RMS}(x) = \sqrt{\frac{1}{N}\sum_i x_i^2 + \epsilon}$$

**反向**（比 LayerNorm 简单，无均值项）：

$$\bar{x}_i = \frac{\gamma_i}{\text{RMS}} \left( \bar{y}_i - \frac{x_i}{N \cdot \text{RMS}^2} \sum_j \bar{y}_j \gamma_j x_j \right)$$

$$\boxed{\bar{\gamma} = \sum_{\text{batch}} \bar{y} \odot \hat{x}}$$

---

### 4.6 Scaled Dot-Product Attention

**前向**：

$$A = \text{softmax}\!\left(\frac{QK^\top}{\sqrt{d_k}}\right) V$$

其中 $Q, K, V \in \mathbb{R}^{N \times d_k}$。

**反向**：设 $P = \text{softmax}(QK^\top / \sqrt{d_k})$，上游梯度为 $\bar{A}$。

**Step 1：对 V 的梯度**

$$\boxed{\bar{V} = P^\top \bar{A}}$$

**Step 2：对 P 的梯度**

$$\bar{P} = \bar{A} V^\top$$

**Step 3：Softmax 反向（对 logit $S = QK^\top/\sqrt{d_k}$ 的梯度）**

$$\bar{S}_{ij} = P_{ij}\left(\bar{P}_{ij} - \sum_k \bar{P}_{ik} P_{ik}\right)$$

（即沿 key 维度做 Softmax backward）

**Step 4：对 Q, K 的梯度**

$$\boxed{\bar{Q} = \bar{S} K / \sqrt{d_k}}$$

$$\boxed{\bar{K} = \bar{S}^\top Q / \sqrt{d_k}}$$

**FlashAttention 的特殊处理**：FlashAttention 不保存完整的 $P$ 矩阵（$O(N^2)$），只保存每行的 logsumexp（$O(N)$）。Backward 时利用 logsumexp 重算 $P$，换时间换内存。

```
保存：m_i（每行 max），l_i（每行 logsumexp）
Backward：P_{ij} = exp(s_{ij} - m_i) / l_i  ← 重算，无需缓存整矩阵
```

---

### 4.7 激活函数

#### ReLU

$$f(x) = \max(0, x)$$

$$\boxed{\bar{x} = \bar{y} \odot \mathbf{1}[x > 0]}$$

梯度在 $x > 0$ 时完全透传，$x \leq 0$ 时截断为 0（死神经元问题）。

---

#### GeLU（Gaussian Error Linear Unit）

$$f(x) = x \cdot \Phi(x), \quad \Phi(x) = \frac{1}{2}\left[1 + \text{erf}\!\left(\frac{x}{\sqrt{2}}\right)\right]$$

**近似版本**（Hendrycks & Gimpel，实际工程常用）：

$$f(x) \approx 0.5x\left[1 + \tanh\!\left(\sqrt{2/\pi}(x + 0.044715x^3)\right)\right]$$

**精确梯度**：

$$\boxed{\bar{x} = \bar{y} \cdot \left[\Phi(x) + x \cdot \phi(x)\right]}$$

其中 $\phi(x) = \frac{1}{\sqrt{2\pi}} e^{-x^2/2}$ 是标准正态 PDF。

**近似版梯度**（令 $t = \tanh(\cdot)$）：

$$\bar{x} = \bar{y} \cdot 0.5\left[(1+t) + x \cdot (1-t^2) \cdot \sqrt{2/\pi}(1 + 3 \times 0.044715 x^2)\right]$$

---

#### SiLU / Swish（LLaMA 系列使用）

$$f(x) = x \cdot \sigma(x), \quad \sigma(x) = \frac{1}{1+e^{-x}}$$

$$\boxed{\bar{x} = \bar{y} \cdot \left[\sigma(x) + x \cdot \sigma(x)(1 - \sigma(x))\right] = \bar{y} \cdot \left[\sigma(x)(1 + x(1-\sigma(x)))\right]}$$

实现时保存前向的 $\sigma(x)$ 即可，无需重算 exp。

---

#### GLU 变体（SwiGLU，用于 LLaMA FFN）

FFN 结构：$\text{FFN}(x) = \text{SiLU}(W_1 x) \odot (W_3 x)$

设 $u = W_1 x$，$v = W_3 x$，$y = \text{SiLU}(u) \odot v$，上游梯度 $\bar{y}$：

$$\bar{u} = \bar{y} \odot v \odot \text{SiLU}'(u)$$

$$\bar{v} = \bar{y} \odot \text{SiLU}(u)$$

$$\bar{W}_1 = \bar{u}^\top x, \quad \bar{W}_3 = \bar{v}^\top x$$

$$\bar{x} = W_1^\top \bar{u} + W_3^\top \bar{v}$$

---

### 4.8 Embedding Lookup

**前向**：给定 token index $i$，输出 $y = E[i]$（取 embedding 表的第 $i$ 行）。

**反向**：上游梯度 $\bar{y}$ 需要累加回 embedding 表对应行：

$$\boxed{\bar{E}[i] \mathrel{+}= \bar{y}}$$

注意是 **`+=`（稀疏累加）**，不是赋值——同一个 token 在 batch 中多次出现时梯度叠加。

工程实现用 `scatter_add`（CUDA atomic add），这是大词汇表（vocab = 128K+）时的性能瓶颈之一。

**Tied Embedding**：很多 LLM 让 input embedding 和 output projection 共享权重（$W_E = W_{\text{lm\_head}}^\top$），此时两处的梯度需要相加后一起更新。

---

## 5. 梯度流动的完整路径示例（Transformer Block）

以一个标准 Pre-Norm Transformer Block 为例，追踪梯度如何从 Loss 流回输入 $x$：

```
Forward（从上往下）:
┌─────────────────────────────────────────┐
│  x_input                                │
│     │                                   │
│  RMSNorm(x_input)  →  x_norm1          │
│     │                                   │
│  QKV projection (Linear)               │
│     │                                   │
│  Scaled Dot-Product Attention          │
│     │                                   │
│  Output projection (Linear)            │
│     │                                   │
│  x_attn = x_input + out_proj  (残差)   │
│     │                                   │
│  RMSNorm(x_attn)  →  x_norm2          │
│     │                                   │
│  FFN: SwiGLU(Linear) * Linear         │
│     │                                   │
│  x_output = x_attn + ffn_out (残差)   │
└─────────────────────────────────────────┘

Backward（梯度从 Loss 逆向流动）:
∂L/∂x_output
   │
   ├──► 残差连接分叉：直接流向 x_attn（梯度不衰减！）
   │
   └──► ∂L/∂ffn_out → FFN backward → RMSNorm backward → ∂L/∂x_attn
            │
            └──► SwiGLU backward（W_gate, W_up, W_down 各自收到梯度）

∂L/∂x_attn（两路梯度相加）
   │
   ├──► 残差：直接流向 x_input
   │
   └──► Attention backward → RMSNorm backward
            │
            └──► QKV 矩阵、O 矩阵各自收到梯度
```

**残差连接是梯度高速公路**：`x_output = x_input + f(x_input)`，对 `x_input` 求导：

$$\frac{\partial L}{\partial x_{\text{input}}} = \frac{\partial L}{\partial x_{\text{output}}} \cdot \left(1 + \frac{\partial f}{\partial x_{\text{input}}}\right)$$

那个 **`1`** 保证梯度从最后一层直接流到第一层，这是深网络可以训练的核心原因（解决梯度消失）。

---

## 6. 工程优化技巧

### 6.1 算子融合（Kernel Fusion）

将多个小算子合并为一个 CUDA kernel，减少 memory round-trip：

| 未融合 | 融合后 | 节省 |
|--------|--------|------|
| Linear + Bias + GeLU（3次读写） | FusedLinearGeLU（1次读写） | ~3× 内存带宽 |
| LayerNorm（mean + std + norm + scale，多次 pass） | 单 kernel，shared memory 内完成 reduce | ~2× 速度 |
| Softmax + Cross-Entropy | FusedCrossEntropyLoss | 避免写出完整 softmax 矩阵 |

Triton / CUDA 手写 kernel 是必要的。PyTorch 的 `torch.compile` 会自动做部分融合。

### 6.2 混合精度训练（AMP）

```
前向 & 反向：FP16 / BF16（计算快，内存省）
参数 & 梯度累加：FP32（精度保障）
梯度 scaler：防止 FP16 梯度 underflow（乘 2^N 再除回）
```

BF16（Brain Float）相比 FP16 的优势：指数位更宽（8 vs 5 bit），数值范围与 FP32 相同，溢出风险低，LLaMA / Gemma 等都用 BF16。

### 6.3 分布式训练中的梯度同步

**数据并行（DDP）**：

```
每个 GPU 独立前向 + 反向 → 各自得到梯度
AllReduce（Ring-AllReduce）：将所有 GPU 的梯度平均
每个 GPU 用平均后的梯度更新
```

AllReduce 通常与 backward 计算**重叠（overlap）**：某层 backward 结束立即触发该层梯度的 AllReduce，不等全部层 backward 完成。

**ZeRO（DeepSpeed）**：将梯度、参数、优化器状态分片到各 GPU，极端省内存。

---

## 7. 常见数值问题与调试

### 7.1 梯度消失 / 爆炸

| 症状 | 原因 | 对策 |
|------|------|------|
| 梯度趋向 0 | 激活函数饱和（sigmoid/tanh 两端）、网络过深 | 用 ReLU/SiLU、残差连接、LayerNorm |
| 梯度趋向 ∞ | 学习率过大、权重初始化不当 | 梯度裁剪（`clip_grad_norm`）、小学习率 Warmup |

```python
# 梯度裁剪（LLM 训练标配）
torch.nn.utils.clip_grad_norm_(model.parameters(), max_norm=1.0)
```

### 7.2 梯度检查（数值验证）

怀疑某算子 backward 实现有误时，用有限差分验证：

```python
from torch.autograd import gradcheck

x = torch.randn(4, requires_grad=True, dtype=torch.float64)  # 用 float64 精度高
result = gradcheck(my_func, (x,), eps=1e-6, atol=1e-4)
print(result)  # True 则实现正确
```

### 7.3 监控梯度健康

训练时建议定期记录：

```python
for name, param in model.named_parameters():
    if param.grad is not None:
        grad_norm = param.grad.norm()
        wandb.log({f"grad_norm/{name}": grad_norm})
```

关注：
- 某层梯度持续为 0 → 死神经元或梯度截断
- 某层梯度远大于其他层 → 该层可能是训练不稳定的根源

---

## 参考资料

- [PyTorch Autograd 内部机制](https://pytorch.org/docs/stable/notes/autograd.html)
- [FlashAttention 论文](https://arxiv.org/abs/2205.14135)（Backward 推导见附录）
- [Efficient Backprop（LeCun et al.）](http://yann.lecun.com/exdb/publis/pdf/lecun-98b.pdf)
- [The Matrix Calculus You Need For Deep Learning](https://explained.ai/matrix-calculus/)
- Goodfellow et al., *Deep Learning*, Chapter 6（反向传播算法）
