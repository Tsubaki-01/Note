# 投机解码：核心原理、KV Cache 与工程要点

## 1. 为什么需要投机解码

自回归解码每生成一个 Token，都要执行一次大模型前向传播。单 Token、低批量推理通常不是算力不够，而是反复从显存读取模型权重，属于典型的 **Memory-Bound** 场景：一次昂贵的权重搬运只换来一个 Token 的概率分布。

投机解码（Speculative Decoding）的想法是：

1. 用一个便宜的草稿模型（Drafter）先生成连续的 $\gamma$ 个候选 Token；
2. 让目标模型（Target）一次前向并行计算这 $\gamma$ 个位置；
3. 逐个检查候选是否符合目标模型分布，接受正确前缀，拒绝错误部分。

目标模型一次验证的权重读取成本，通常接近一次普通解码步，但可能确认多个 Token，因此可以降低每个输出 Token 的平均延迟。

## 2. 经典投机采样算法

设草稿模型分布为 $p$，目标模型分布为 $q$。草稿模型依次采样：

$$x_i \sim p(\cdot \mid y_{1:L},x_{<i}), \quad i=1,\ldots,\gamma$$

目标模型得到对应分布后，对每个候选 $x_i$ 使用拒绝采样：

$$u \sim \mathcal U(0,1),\qquad
\text{accept}(x_i)\iff u < \min\left(1,\frac{q_i(x_i)}{p_i(x_i)}\right)$$

如果第 $i$ 个 Token 被拒绝：

- 保留前面的 $x_1,\ldots,x_{i-1}$；
- 从残差分布中采样一个修正 Token：

$$r_i(v)=\frac{\max(0,q_i(v)-p_i(v))}
{\sum_z\max(0,q_i(z)-p_i(z))}$$

- 丢弃 $x_i$ 及其后的草稿 Token，开始下一轮。

如果 $\gamma$ 个候选全部接受，则从目标模型在下一个位置的分布中采样一个 **bonus token**。

该拒绝采样构造保证最终样本仍服从目标模型分布；因此，在采样设置一致、概率计算准确的前提下，投机解码是 **无损加速**，不是用质量换速度。

## 3. 带 KV Cache 的投机解码

### 3.1 普通解码中的 KV Cache

设：

- $B$：Batch Size；
- $L$：已经生成的上下文长度；
- $N_l$：Transformer 层数；
- $H_q$：Query 头数；
- $H_{kv}$：Key/Value 头数；MHA 中 $H_{kv}=H_q$，GQA/MQA 中 $H_{kv}\le H_q$；
- $d_h$：每个注意力头的维度。

对某一层，KV Cache 通常记为：

$$K,V\in\mathbb R^{B\times H_{kv}\times L\times d_h}$$

生成下一个 Token 时，只需计算新 Token 的 Query、Key、Value：

$$Q_{new}\in\mathbb R^{B\times H_q\times 1\times d_h}$$
$$K_{new},V_{new}\in\mathbb R^{B\times H_{kv}\times 1\times d_h}$$

将 $K_{new},V_{new}$ 追加到 Cache 后，注意力分数的概念形状为：

$$Q_{new}K_{all}^{T}
\;:\; (B,H_q,1,d_h)\times(B,H_q,d_h,L+1)
\to(B,H_q,1,L+1)$$

然后用这个权重读取 $V_{all}$，得到新位置的隐藏状态。KV Cache 的关键作用是避免每一步重复计算前面 $L$ 个 Token 的 K/V。

没有 Cache 时，第 $t$ 步需要重新计算长度为 $t$ 的 $Q,K,V$，注意力矩阵约为 $[B,H_q,t,t]$；有 Cache 时，新增计算的 Query 长度只有 1，注意力矩阵约为 $[B,H_q,1,t]$。

### 3.2 投机验证时的矩阵变化

假设已有长度为 $L$ 的前缀，草稿模型提出 $\gamma$ 个候选 $x_1,\ldots,x_\gamma$。

#### 草稿阶段

草稿模型仍然是自回归的，但它较小，所以可以较快生成候选。草稿模型每生成一个 Token，就把它的 K/V 追加到自己的 Cache：

$$K_d,V_d:
[B,H^d_{kv},L,d^d_h]
\to[B,H^d_{kv},L+\gamma,d^d_h]$$

目标模型和草稿模型参数通常不同，因此两者的 KV Cache **不能直接共用**；它们各自维护一套 Cache。

#### 目标模型验证阶段

目标模型保留前缀 Cache，并把 $\gamma$ 个候选作为一个块输入。对这一层，新增部分的形状为：

$$Q_{new}\in\mathbb R^{B\times H_q\times\gamma\times d_h}$$
$$K_{new},V_{new}\in\mathbb R^{B\times H_{kv}\times\gamma\times d_h}$$

拼接前缀后：

$$K_{all},V_{all}
\in\mathbb R^{B\times H_{kv}\times(L+\gamma)\times d_h}$$

概念上的注意力分数矩阵变为：

$$Q_{new}K_{all}^{T}
\to\mathbb R^{B\times H_q\times\gamma\times(L+\gamma)}$$

其中每个候选位置只能看见前缀和它之前的候选位置，不能看见未来候选。工程实现通常使用因果 Mask 和高效 Attention Kernel；并不一定真的物化这个完整四维矩阵。

目标模型这一轮同时得到多个位置的 logits：已有的前缀输出提供第一个候选的分布，其余 $\gamma$ 个位置的块前向提供后续分布，合起来即可检查 $x_1,\ldots,x_\gamma$，并得到 bonus token 的分布。

#### 接受与回滚

如果前 $r$ 个候选被接受，则目标模型最终保留：

$$K_t,V_t
[B,H_{kv},L+\gamma,d_h]
\to[B,H_{kv},L+r,d_h]$$

被拒绝候选之后的临时 KV 必须回滚或标记为无效。若残差采样产生了修正 Token，它已经被输出，但它的目标模型 K/V 通常要在下一轮前向时补入；具体做法取决于推理框架的 Cache 管理策略。

### 3.3 一个具体的尺寸例子

取 $B=1$、$H_q=32$、$H_{kv}=8$（GQA）、$d_h=128$、前缀长度 $L=2048$、草稿长度 $\gamma=4$：

| 阶段 | 单层目标模型 K/V 形状 | 说明 |
|---|---|---|
| 验证前 | $[1,8,2048,128]$ | 只包含已确认前缀 |
| 验证中 | $[1,8,2052,128]$ | 临时追加 4 个候选 |
| 接受 3 个后 | $[1,8,2051,128]$ | 回滚 1 个被拒候选 |

每层新增的 K/V 元素数为：

$$2\times B\times H_{kv}\times\gamma\times d_h
=2\times1\times8\times4\times128=8192$$

若使用 BF16/FP16（2 bytes），每层约增加 $16$ KiB 的临时 K/V；总量还要乘以 Transformer 层数。总体 KV Cache 显存近似为：

$$M_{KV}\approx2\times B\times N_l\times L\times H_{kv}\times d_h
\times\text{bytes per element}$$

因此，投机解码额外消耗的显存主要来自：目标模型的临时候选 KV、草稿模型的独立 KV，以及树状投机时更多候选节点的 KV。

### 3.4 树状投机时的变化

线性投机只有一条长度为 $\gamma$ 的候选链；树状投机可能有 $S$ 个候选节点。此时目标模型临时 Cache 的长度按节点总数增长：

$$K,V\in\mathbb R^{B\times H_{kv}\times(L+S)\times d_h}$$

树 Attention Mask 规定每个节点只能访问自己的祖先路径。逻辑上仍是一次并行验证，但 Cache 管理从“按长度回滚”变成“按分支保留/释放”。树越宽，接受机会越多，同时显存和 Mask/索引管理成本也越高。

## 4. 草稿器与验证方式的常见变体

不需要记住所有论文，按草稿器来源可分为三类：

1. **独立小模型**：实现简单，目标模型与草稿模型可分别部署；缺点是额外显存和两套 KV Cache。
2. **目标模型内部草稿器**：例如跳过部分层的 Self-Speculation，减少额外模型，但需要特殊推理支持。
3. **辅助预测头/特征预测器**：例如 Medusa、EAGLE，在目标模型上增加预测头或轻量草稿模块，通常能减少独立草稿模型的开销，但需要训练或适配。

验证方式也有两种主线：

- **线性验证**：一次失败后丢弃后续 Token，逻辑简单、Cache 回滚容易；
- **树状验证**：同时验证多条候选路径，可能提高接受长度，但需要树 Attention、分支索引和更复杂的 Cache 管理。

## 5. 什么时候有效，什么时候可能变慢

投机解码是否加速，取决于以下因素：

- 草稿模型生成 $\gamma$ 个 Token 的成本；
- 目标模型块验证的成本；
- 草稿 Token 被目标模型接受的比例；
- Batch Size、上下文长度、GPU 利用率和 KV Cache 显存压力。

低并发、目标模型访存受限、草稿模型足够快且接受率较高时，收益通常最明显。并发升高后，目标模型可能已经接近算力饱和，此时额外的草稿计算和验证计算反而会带来开销。生产系统通常需要按 Batch Size 或实时吞吐动态关闭投机解码，并限制草稿长度、树宽和最大并发。

## 6. 外部资料

- [Fast Inference from Transformers via Speculative Decoding](https://arxiv.org/abs/2211.17192)：Leviathan 等，投机解码的经典工作。
- [Accelerating Large Language Model Decoding with Speculative Sampling](https://arxiv.org/abs/2302.01318)：Chen 等，拒绝采样与无损性说明。
- [Medusa: Simple LLM Inference Acceleration Framework with Multiple Decoding Heads](https://arxiv.org/abs/2401.10774)：多预测头路线。
- [EAGLE: Speculative Sampling Requires Rethinking Feature Uncertainty](https://arxiv.org/abs/2401.15077)：特征级草稿路线。
- [EAGLE-2: Faster Inference of Language Models with Dynamic Draft Trees](https://arxiv.org/abs/2406.16858)：动态草稿树。
- [Hugging Face Transformers：KV Cache](https://huggingface.co/docs/transformers/main/en/kv_cache)：KV Cache 的实现与内存管理说明。
- [vLLM：Speculative Decoding](https://docs.vllm.ai/en/latest/features/spec_decode.html)：推理框架中的投机解码配置与限制。

