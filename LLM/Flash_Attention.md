# FlashAttention v1–v3 深度解析

> 让 Transformer 的 Attention 计算从 I/O 瓶颈走向算力极限的工程奇迹

---

## 目录

1. [背景：为什么 Attention 这么慢？](#background)
2. [FlashAttention v1（2022）](#v1)
3. [FlashAttention v2（2023）](#v2)
4. [FlashAttention v3（2024）](#v3)
5. [三代对比总结](#comparison)
6. [数学推导：在线 Softmax 与分块计算](#math)
7. [工程实现要点](#engineering)
8. [应用场景与局限](#applications)

---

## 1. 背景：为什么 Attention 这么慢？ {#background}

### 1.1 标准 Self-Attention 的计算与内存复杂度

标准的 Scaled Dot-Product Attention 定义为：

$$\text{Attention}(Q, K, V) = \text{softmax}\left(\frac{QK^T}{\sqrt{d_k}}\right)V$$

其中 $Q, K, V \in \mathbb{R}^{N \times d}$，$N$ 为序列长度，$d$ 为头维度。

| 操作            | 计算复杂度 | 内存复杂度   |
| --------------- | ---------- | ------------ |
| $QK^T$ 矩阵乘法 | $O(N^2 d)$ | $O(N^2)$     |
| Softmax         | $O(N^2)$   | $O(N^2)$     |
| Attention × V   | $O(N^2 d)$ | $O(Nd)$      |
| **合计**        | $O(N^2 d)$ | **$O(N^2)$** |

当 $N = 16384$（16k context），`float16` 下注意力矩阵占用 **$16384^2 \times 2 \approx 512$ MB**，完全爆显存。

### 1.2 GPU 内存层次结构

理解 FlashAttention 的核心前提是了解 GPU 的存储层次：

```
┌─────────────────────────────────────────────┐
│  Registers (~几KB, ~几十TB/s)               │
│  ┌───────────────────────────────────────┐  │
│  │  L1 Cache / Shared Memory (~几百KB)   │  │
│  │  带宽: ~几十TB/s                      │  │
│  └───────────────────────────────────────┘  │
│  L2 Cache (~几十MB)                         │
│  ┌───────────────────────────────────────┐  │
│  │  HBM (Global Memory, 几十~几百GB)     │  │
│  │  带宽: ~1-3 TB/s                      │  │
│  └───────────────────────────────────────┘  │
└─────────────────────────────────────────────┘
```

标准 Attention 的问题：频繁读写 HBM 上的 $N \times N$ 矩阵，而 HBM 带宽远小于 SRAM。

**I/O 复杂度（HBM访问次数）**：标准 Attention 为 $\Theta(N^2 d)$。

---

## 2. FlashAttention v1（2022） {#v1}

**论文**: *FlashAttention: Fast and Memory-Efficient Exact Attention with IO-Awareness*  
**作者**: Tri Dao, Daniel Y. Fu, Stefano Ermon, Atri Rudra, Christopher Ré  
**机构**: Stanford University

### 2.1 核心思想

FlashAttention v1 的核心是 **Tiling（分块）+ Online Softmax**，将 Attention 计算分解为多个小块，使每块完全在 SRAM 中完成，避免将中间结果写回 HBM。

### 2.2 在线 Softmax（Online Softmax）

经典 Softmax 需要两趟扫描：第一趟求最大值，第二趟计算指数和归一化。FA v1 利用数值稳定的在线算法，将两趟合并为一趟。

设当前已处理前 $i$ 列，维护：

- $m_i = \max(s_1, \ldots, s_i)$：局部最大值
- $\ell_i = \sum_{j=1}^{i} e^{s_j - m_i}$：局部归一化因子
- $O_i$：当前输出累积

当新增第 $i+1$ 列时，更新公式：

$$m_{i+1} = \max(m_i,\ s_{i+1})$$

$$\ell_{i+1} = e^{m_i - m_{i+1}} \cdot \ell_i + e^{s_{i+1} - m_{i+1}}$$

$$O_{i+1} = \frac{e^{m_i - m_{i+1}} \cdot \ell_i \cdot O_i + e^{s_{i+1} - m_{i+1}} \cdot v_{i+1}}{\ell_{i+1}}$$

这使得我们无需存储完整的注意力矩阵，只需维护固定大小的状态。

### 2.3 分块算法流程

```
Input:  Q, K, V ∈ R^{N×d}，块大小 B_r, B_c
Output: O ∈ R^{N×d}

将 Q 分成 T_r = ⌈N/B_r⌉ 块
将 K, V 分成 T_c = ⌈N/B_c⌉ 块

for i = 1 to T_r:                    # 外层：遍历 Q 块
    从 HBM 加载 Q_i 到 SRAM
    初始化 O_i = 0, ℓ_i = 0, m_i = -∞
    
    for j = 1 to T_c:                # 内层：遍历 K, V 块
        从 HBM 加载 K_j, V_j 到 SRAM
        
        # 在 SRAM 内完成所有计算
        S_ij = Q_i × K_j^T / √d      # (B_r × B_c)
        m̃_ij = rowmax(S_ij)
        P̃_ij = exp(S_ij - m̃_ij)
        ℓ̃_ij = rowsum(P̃_ij)
        
        # 在线更新
        m_new = max(m_i, m̃_ij)
        ℓ_new = exp(m_i - m_new)·ℓ_i + exp(m̃_ij - m_new)·ℓ̃_ij
        O_i = (ℓ_i·exp(m_i-m_new)·O_i + exp(m̃_ij-m_new)·P̃_ij·V_j) / ℓ_new
        
        更新 m_i ← m_new, ℓ_i ← ℓ_new
    
    将 O_i 写回 HBM
```

### 2.4 内存与 I/O 分析

| 指标           | 标准 Attention  | FlashAttention v1     |
| -------------- | --------------- | --------------------- |
| HBM I/O 复杂度 | $\Theta(N^2 d)$ | $\Theta(N^2 d^2 / M)$ |
| 额外内存       | $O(N^2)$        | $O(N)$                |
| 是否精确       | ✅               | ✅（数学等价）         |

其中 $M$ 为 SRAM 大小。当 $M \gg d^2$ 时，I/O 复杂度接近 $O(Nd)$。

### 2.5 反向传播

FlashAttention v1 无需存储注意力矩阵用于反向传播，而是在反向时**重新计算**（Recomputation）前向的 Softmax 统计量 $(m, \ell)$，额外存储空间仅为 $O(N)$，以少量重复计算换取大量内存节省。

### 2.6 性能表现

- 在 A100 GPU 上，序列长度 1k 时比标准 Attention **快 3×**
- 序列长度 4k 时**快 6×**
- 内存占用降低至 $O(N)$​，使 64k+ 长序列成为可能

> ## 0. 统一符号
> * $N$：序列长度
> * $d$：单头注意力维度（head_dim，常为 **64** 或 **128**）
> * $M$：GPU 片上 SRAM（共享内存）大小
> * **IO 复杂度**：读写 HBM（显存）的数据总量
> * **计算复杂度**：浮点运算次数 FLOPs
>
> ---
>
> ## 1. 标准 Attention 复杂度推导
>
> ### 1.1 计算流程
> $$\text{Attn}(Q,K,V)=\mathrm{softmax}\left(\frac{QK^\top}{\sqrt{d}}\right)V$$
>
> 1.  读入 $Q,K,V\in\mathbb{R}^{N\times d}$
> 2.  计算 $S=QK^\top\in\mathbb{R}^{N\times N}$
> 3.  对 $S$ 做 softmax，得到 $P\in\mathbb{R}^{N\times N}$
> 4.  计算输出 $O=PV\in\mathbb{R}^{N\times d}$
>
> ### 1.2 IO 复杂度（读写显存总量）
> * 读 $Q,K,V$：$3Nd$
> * 写 $S$：$N^2$
> * 读 $S$ + 写 $P$：$N^2+N^2$
> * 读 $P,V$ + 写 $O$：$N^2+Nd$
>
> 总和：
> $$\text{IO}_{\text{std}}\approx4N^2+4Nd$$
>
> 当 $N$ 很大时，$N^2\gg Nd$，所以：
> $$\boxed{\text{IO}_{\text{std}}=\Theta(N^2d)}$$
>
> ### 1.3 计算复杂度
> 矩阵乘法都是 $O(N^2d)$，softmax 可以忽略：
> $$\text{FLOPs}_{\text{std}}=\Theta(N^2d)$$
>
> ---
>
> ## 2. FlashAttention IO 复杂度推导
>
> ### 2.1 核心思路
> * 不存整张 $N\times N$ 矩阵
> * 把 $Q,K,V$ 切成小**块**，块大小 $B$ 满足：
>     $$B\cdot d\le M\Rightarrow B\approx\sqrt{\frac{M}{d}}$$
> * 只在 SRAM 里算小块矩阵乘法，算完就丢
>
> ### 2.2 IO 推导
> * 块数量：$(N/B)^2$
> * 每块读写：$Q,K,V$ 读 + $O$ 写 $\approx4Bd$
>
> 总 IO：
> $$\text{IO}_{\text{flash}}\approx\left(\frac{N}{B}\right)^2\cdot4Bd=\frac{4N^2d}{B}$$
>
> 代入 $B\approx\sqrt{M/d}$：
> $$\text{IO}_{\text{flash}}\approx\frac{4N^2d}{\sqrt{M/d}}=\frac{4N^2d^{3/2}}{\sqrt{M}}$$
>
> 更紧凑的界一般写作：
> $$\boxed{\text{IO}_{\text{flash}}=\Theta\left(\frac{N^2d^2}{M}\right)}$$
>
> ### 2.3 计算复杂度
> **完全没变！**
> $$\text{FLOPs}_{\text{flash}}=\Theta(N^2d)$$
>
> ---
>
> ## 3. 直观差距：用数字算一遍
>
> 设：
> * $N=8192$
> * $d=64$
> * $M$ = **1 MB**（SRAM很小）
>
> **标准 IO：**
> $$\text{IO}_{\text{std}}\approx N^2d=8192^2\cdot64\approx4.29\text{ GB}$$
>
> **Flash IO：**
> $$\text{IO}_{\text{flash}}\approx\frac{N^2d^2}{M}=\frac{8192^2\cdot64^2}{1024^2}\approx256\text{ MB}$$
>
> > **IO 相差约 17 倍！** 这就是速度差距的根本来源。
>
> ---
>
> ## 4. 极简代码对比（PyTorch 风格）
>
> ### 4.1 标准 Attention（爆显存、IO 大）
> ```python
> def standard_attention(Q, K, V):
>     # Q, K, V: [N, d]
>     N, d = Q.shape
>     # 1. 计算 N*N 大矩阵
>     S = Q @ K.T / d**0.5   # [N, N]
>     P = softmax(S, dim=-1) # [N, N]
>     O = P @ V              # [N, d]
>     return O
> ```
>
> **问题：**
>
> - 必须存储 `[N, N]` 矩阵
> - 大量读写 HBM
>
> ### 4.2 Flash 核心逻辑（分块、不存大矩阵）
>
> ```python
> def flash_attention_tile(Q, K, V, B):
>     N, d = Q.shape
>     O = torch.zeros_like(Q)
>     
>     # 分块循环
>     for i in range(0, N, B):
>         # 取 Q 块
>         Q_i = Q[i:i+B]
>         O_i = torch.zeros_like(Q_i)
>         
>         for j in range(0, N, B):
>             # 取 K、V 块
>             K_j = K[j:j+B]
>             V_j = V[j:j+B]
> 
>             # 小块矩阵乘法（都在 SRAM）
>             S_ij = Q_i @ K_j.T / d**0.5
>             
>             # 在线 softmax，不存全局矩阵
>             # ... 省略稳定 softmax 细节
>             
>             # 累加输出块
>             O_i += ... # 伪代码省略具体计算
> 
>         O[i:i+B] = O_i
>         
>     return O
> ```
>
> **特点：**
>
> - 全程只用到小块 `[B, B]`、`[B, d]`
> - 不生成 `[N, N]`
> - 矩阵乘法依然在用，只是变小了

---

## 3. FlashAttention v2（2023） {#v2}

**论文**: *FlashAttention-2: Faster Attention with Better Parallelism and Work Partitioning*  
**作者**: Tri Dao  
**机构**: Princeton University

### 3.1 v1 的瓶颈

v1 主要优化了 HBM I/O，但在 GPU 计算利用率上仍有不足：

1. **非矩阵乘法操作过多**：rescaling 操作（除法、乘法）占比过高，而 GPU Tensor Core 只对 GEMM 高效
2. **并行化不足**：外循环在序列维度上，内循环 warp 间存在通信
3. **work partitioning 不均衡**：warps 之间负载分配不合理

### 3.2 核心改进

#### 改进1：减少非矩阵乘法操作

v1 中每次迭代都需要对 $O_i$ 进行 rescaling（乘以 $\frac{\ell_{old}}{\ell_{new}}$），这是非 GEMM 操作。

v2 将 rescaling 推迟到最后一步，在循环中只维护**未归一化**的输出：

$$\tilde{O}_i = \sum_j e^{s_{ij} - m_i} \cdot v_j$$

最终只做一次除法：$O_i = \tilde{O}_i / \ell_i$

这将非矩阵乘法操作数减少约 **2倍**。

#### 改进2：更好的并行化策略

| 策略                 | v1      | v2      |
| -------------------- | ------- | ------- |
| 外循环               | K, V 块 | Q 块    |
| 内循环               | Q 块    | K, V 块 |
| 前向 batch/head 并行 | ✅       | ✅       |
| 前向序列并行         | ❌       | ✅       |
| 反向序列并行         | ❌       | ✅       |

v2 将 Q 放在外循环，使不同 Q 块之间无需通信，每个 thread block 独立处理一个 Q 块，**前向 pass 可在序列维度完全并行**。

#### 改进3：warp 内的 work partitioning

v1 中 warps 采用 "split-K" 策略，不同 warp 处理不同的 K 列，需要在共享内存中同步。

v2 改为 "split-Q" 策略：不同 warp 处理同一 K 块但不同的 Q 行，**消除 warp 间通信**。

```
v1 warp 策略（split-K，需要同步）：
Warp 0: Q[0:16] × K[0:32]
Warp 1: Q[0:16] × K[32:64]   # 需要 reduce
Warp 2: Q[0:16] × K[64:96]   # 需要 reduce
Warp 3: Q[0:16] × K[96:128]  # 需要 reduce

v2 warp 策略（split-Q，无需同步）：
Warp 0: Q[0:16]  × K[0:128]  # 独立
Warp 1: Q[16:32] × K[0:128]  # 独立
Warp 2: Q[32:48] × K[0:128]  # 独立
Warp 3: Q[48:64] × K[0:128]  # 独立
```

### 3.3 性能表现

- 比 FlashAttention v1 **快 2–4×**
- 在 A100 80GB SXM4 上达到 **~72% 理论 FLOPS 利用率**（前向 pass）
- 比 PyTorch 标准实现快 **5–9×**（序列越长提升越大）
- 支持 GQA（Grouped Query Attention）和 MQA（Multi-Query Attention）

### 3.4 数学等价性证明

v2 通过以下方式维持与 v1 的数学等价性：

设 $K$ 被分为 $T_c$ 块 $K_1, \ldots, K_{T_c}$，对应 $V_1, \ldots, V_{T_c}$。

对固定的 $Q_i$，累积计算：

$$m^{(j)} = \max(m^{(j-1)},\ \text{rowmax}(Q_i K_j^T / \sqrt{d}))$$

$$\ell^{(j)} = e^{m^{(j-1)} - m^{(j)}} \ell^{(j-1)} + \text{rowsum}(e^{Q_i K_j^T / \sqrt{d} - m^{(j)}})$$

$$\tilde{O}^{(j)} = e^{m^{(j-1)} - m^{(j)}} \tilde{O}^{(j-1)} + e^{Q_i K_j^T / \sqrt{d} - m^{(j)}} V_j$$

最终：$O_i = \tilde{O}^{(T_c)} / \ell^{(T_c)}$

---

## 4. FlashAttention v3（2024） {#v3}

**论文**: *FlashAttention-3: Fast and Accurate Attention with Asynchrony and Low-precision*  
**作者**: Jay Shah, Ganesh Bikshandi, Ying Zhang, Vijay Thakkar, Pradeep Ramani, Tri Dao  
**机构**: Princeton University & NVIDIA

### 4.1 新硬件特性：Hopper GPU (H100)

FA v3 专门针对 NVIDIA Hopper 架构（H100/H200）进行优化，利用三个关键新特性：

| 特性                                | 说明                                    | FA v3 利用方式         |
| ----------------------------------- | --------------------------------------- | ---------------------- |
| **TMA (Tensor Memory Accelerator)** | 专用 DMA 引擎，异步传输矩阵             | 重叠数据传输与计算     |
| **WGMMA (Warpgroup MMA)**           | 新一代矩阵乘法指令，吞吐量是 HMMA 的 2× | 替换 v2 中的 HMMA 指令 |
| **FP8 Tensor Core**                 | 支持 FP8 精度，吞吐量是 FP16 的 2×      | 低精度加速             |

### 4.2 核心技术：生产者-消费者异步流水线

FA v3 引入**软件流水线（Software Pipelining）**，将 Hopper 上的 warp 分为两组：

```
生产者 Warp (Producer):          消费者 Warp (Consumer):
━━━━━━━━━━━━━━━━━━━━━━          ━━━━━━━━━━━━━━━━━━━━━━━
TMA 异步加载 K_j, V_j     →     等待数据就绪
（使用 TMA barriers 同步）       执行 WGMMA 矩阵乘法
                                 Softmax 计算
加载 K_{j+1}, V_{j+1}   →      输出累积
（与上面的计算重叠）
```

**流水线效果**：

```
时间轴 ──────────────────────────────────────────→

无流水线:
[Load K1,V1][Compute1][Load K2,V2][Compute2][Load K3,V3][Compute3]

有流水线:
[Load K1,V1]
            [Load K2,V2][Compute1]
                        [Load K3,V3][Compute2]
                                    [Load K4,V4][Compute3]
```

数据加载延迟被有效隐藏。

### 4.3 两阶段 Attention 算法（Intra-warpgroup 流水线）

为了进一步将 WGMMA（矩阵乘法）与 Softmax 计算重叠，FA v3 采用两阶段算法：

**阶段1（Score 计算）**：
$$S_j^{(1)} = Q_i K_j^T / \sqrt{d}$$

**阶段2（异步执行 V 加权，同时计算 Softmax）**：

阶段2 的 WGMMA 计算：
$$\tilde{O}_j^{(2)} \mathrel{+}= \tilde{P}_j \cdot V_j$$

同时，利用 WGMMA 的异步延迟窗口，在等待结果期间执行：
$$m_j = \text{rowmax}(S_j^{(1)}),\quad \tilde{P}_j = e^{S_j^{(1)} - m_j},\quad \ell_j = \text{rowsum}(\tilde{P}_j)$$

### 4.4 FP8 低精度支持

FA v3 支持 FP8（E4M3/E5M2）精度，但面临两个精度挑战：

**挑战1：FP8 指数偏置与 Softmax 的兼容性**

FP8 的指数范围有限，直接计算 $e^{s_{ij}}$ 容易溢出。FA v3 使用 **incoherent processing**（非相干处理）：

对 $Q$ 和 $K$ 预乘以随机哈达玛矩阵 $H$：
$$Q' = QH,\quad K' = KH$$

这将极端值"分散"开，使 FP8 表示更稳定，同时不改变 Attention 值（因为 $H$ 是正交矩阵）。

**挑战2：矩阵乘法累加器精度**

FP8 GEMM 的累加器默认为 FP32，但 FA v3 使用 **delayed scaling**（延迟缩放）策略管理精度损失。

### 4.5 性能表现

| 场景                 | 性能              | 对比                    |
| -------------------- | ----------------- | ----------------------- |
| H100 FP16 前向       | **~740 TFLOPS**   | 达到理论峰值的 **~75%** |
| H100 FP8 前向        | **~1200 TFLOPS**  | 达到理论峰值的 ~60%     |
| vs FA v2（H100）     | **1.5–2.0×** 加速 |                         |
| vs cuDNN（因果掩码） | **~10%** 更快     |                         |

---

## 5. 三代对比总结 {#comparison}

| 维度                   | v1 (2022)       | v2 (2023)                  | v3 (2024)              |
| ---------------------- | --------------- | -------------------------- | ---------------------- |
| **核心创新**           | IO感知 + Tiling | 并行化 + work partitioning | 异步流水线 + 低精度    |
| **目标硬件**           | A100            | A100/H100                  | H100/H200 (Hopper专属) |
| **内存复杂度**         | $O(N)$          | $O(N)$                     | $O(N)$                 |
| **HBM I/O**            | $O(N^2 d^2/M)$  | $O(N^2 d^2/M)$             | $O(N^2 d^2/M)$         |
| **计算利用率**         | ~35%            | ~72%                       | ~75%                   |
| **并行策略**           | K-outer         | Q-outer                    | Q-outer + async        |
| **Softmax精度**        | 完全精确        | 完全精确                   | FP16精确/FP8近似       |
| **反向传播**           | 重计算          | 重计算（更优化）           | 重计算                 |
| **新特性支持**         | -               | GQA/MQA                    | FP8, TMA, WGMMA        |
| **相对加速（vs标准）** | ~3-6×           | ~5-9×                      | ~7-15×                 |

---

## 6. 数学推导：在线 Softmax 与分块计算 {#math}

### 6.1 数值稳定 Softmax

标准 Softmax 对向量 $x \in \mathbb{R}^N$：

$$\text{softmax}(x)_i = \frac{e^{x_i}}{\sum_{j=1}^N e^{x_j}}$$

直接计算在 $x_i$ 较大时会上溢。数值稳定版本：

$$\text{softmax}(x)_i = \frac{e^{x_i - m}}{\sum_{j=1}^N e^{x_j - m}},\quad m = \max_j x_j$$

### 6.2 分块 Softmax 的正确性

设 $x = [x^{(1)}, x^{(2)}]$ 被分为两块，分别计算：

$$m^{(1)} = \max x^{(1)},\quad \ell^{(1)} = \sum e^{x^{(1)} - m^{(1)}}$$

$$m^{(2)} = \max x^{(2)},\quad \ell^{(2)} = \sum e^{x^{(2)} - m^{(2)}}$$

合并：
$$m = \max(m^{(1)}, m^{(2)})$$

$$\ell = e^{m^{(1)}-m} \ell^{(1)} + e^{m^{(2)}-m} \ell^{(2)}$$

输出向量：

$$\text{softmax}(x)^{(k)}_i = \frac{e^{x^{(k)}_i - m^{(k)}} \cdot e^{m^{(k)} - m}}{\ell}$$

这正是 FlashAttention 中分块 Softmax 合并时使用的公式，证明了分块计算的数学等价性。

### 6.3 分块 Attention 输出的合并

设注意力得分被分为两块 $S^{(1)} = QK_1^T/\sqrt{d}$ 和 $S^{(2)} = QK_2^T/\sqrt{d}$，对应值矩阵 $V_1, V_2$。

最终输出应为：

$$O = \text{softmax}([S^{(1)}, S^{(2)}]) \cdot \begin{bmatrix}V_1 \\ V_2\end{bmatrix}$$

分块计算的部分输出为：

$$\tilde{O}^{(1)} = \text{diag}(\ell^{(1)})^{-1} e^{S^{(1)} - m^{(1)}} V_1$$

$$\tilde{O}^{(2)} = \text{diag}(\ell^{(2)})^{-1} e^{S^{(2)} - m^{(2)}} V_2$$

合并（以 $m = \max(m^{(1)}, m^{(2)})$，$\ell = e^{m^{(1)}-m}\ell^{(1)} + e^{m^{(2)}-m}\ell^{(2)}$）：

$$O = \text{diag}(\ell)^{-1}\left(e^{m^{(1)}-m}\text{diag}(\ell^{(1)})\tilde{O}^{(1)} + e^{m^{(2)}-m}\text{diag}(\ell^{(2)})\tilde{O}^{(2)}\right)$$

化简即得：

$$O = \text{diag}(\ell)^{-1}\left(e^{S^{(1)}-m}V_1 + e^{S^{(2)}-m}V_2\right) = \text{softmax}([S^{(1)}, S^{(2)}])\begin{bmatrix}V_1\\V_2\end{bmatrix} \quad \checkmark$$

---

## 7. 工程实现要点 {#engineering}

### 7.1 块大小的选择

块大小 $B_r, B_c$ 需满足 SRAM 容量约束：

$$B_r \cdot d + B_c \cdot d + B_r \cdot B_c \leq M_{\text{SRAM}}$$

典型取值（d=64, A100 SRAM ~100KB per SM）：$B_r = B_c = 64$ 或 128。

### 7.2 因果掩码（Causal Masking）

对于 Decoder 的因果 Attention（只关注左侧 token），FA 只需对对角线附近的块应用掩码，其他块无需修改，减少约 **50% 计算量**（三角矩阵）。

### 7.3 多头注意力的并行化

不同的 head 和 batch 完全独立，可以并行运行在不同的 SM 上，充分利用 GPU 的大规模并行性。

### 7.4 反向传播的重计算策略

FlashAttention 的反向传播采用：

1. **前向 pass**：存储 $m, \ell$（各 $O(N)$），不存储 $P = \text{softmax}(QK^T/\sqrt{d})$
2. **反向 pass**：利用 $Q, K, V, O, m, \ell$ 重新计算 $P$，再计算梯度

额外计算 FLOPs 约为前向的 **1.1–1.2×**，但节省了大量 HBM I/O。

---

## 8. 应用场景与局限 {#applications}

### 8.1 主要应用

- **长上下文 LLM**：GPT-4、Llama 3、Gemma 等模型训练和推理
- **视觉 Transformer**：高分辨率图像处理（ViT、DiT 等）
- **多模态模型**：视频、音频的长序列处理
- **代码生成**：处理长文件上下文

### 8.2 被广泛采用

FlashAttention 已被集成到：

- PyTorch（`torch.nn.functional.scaled_dot_product_attention`）
- HuggingFace Transformers
- NVIDIA cuDNN
- xFormers, Megatron-LM, DeepSpeed

### 8.3 局限性

| 局限             | 说明                                        |
| ---------------- | ------------------------------------------- |
| 硬件绑定         | v3 仅支持 Hopper GPU（H100/H200）           |
| 自定义 Attention | 对复杂变体（如线性 Attention）需要重新实现  |
| 调试困难         | CUDA 内核难以调试，需要专业知识             |
| 滑动窗口等变体   | 需要额外适配（FlashAttention 已有对应扩展） |

---

## 参考文献

1. Dao et al. (2022). *FlashAttention: Fast and Memory-Efficient Exact Attention with IO-Awareness*. NeurIPS 2022. [arXiv:2205.14135](https://arxiv.org/abs/2205.14135)

2. Dao (2023). *FlashAttention-2: Faster Attention with Better Parallelism and Work Partitioning*. ICLR 2024. [arXiv:2307.08691](https://arxiv.org/abs/2307.08691)

3. Shah et al. (2024). *FlashAttention-3: Fast and Accurate Attention with Asynchrony and Low-precision*. [arXiv:2407.08608](https://arxiv.org/abs/2407.08608)

4. Vaswani et al. (2017). *Attention Is All You Need*. NeurIPS 2017.

5. NVIDIA (2023). *Hopper Architecture Whitepaper*.