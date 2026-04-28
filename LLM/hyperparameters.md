---
title: 大模型训练超参数详解：常用值、调参逻辑与优化方向
author: AI Training Guide
description: 系统梳理大语言模型训练过程中核心超参数的常用值范围、调优策略与工程实践，覆盖预训练、微调、RLHF等全阶段。
---

# 大模型训练超参数详解：常用值、调参逻辑与优化方向

大模型训练是一个多因素耦合的复杂工程，超参数的选择直接影响模型的收敛速度、最终性能与训练稳定性。本文系统梳理训练各阶段的核心超参数，提供实用的调参参考与优化思路。

---

## 一、学习率（Learning Rate）

### 常用值范围

| 训练阶段 | 典型范围 | 代表模型参考值 |
|--------|---------|------------|
| 预训练（from scratch） | `1e-4` ~ `3e-4` | LLaMA-3: `3e-4`，GPT-3: `6e-4` |
| 全量微调（Full Fine-tuning） | `1e-5` ~ `5e-5` | Alpaca: `2e-5` |
| LoRA / QLoRA | `1e-4` ~ `3e-4` | LoRA 论文: `2e-4` |
| RLHF PPO 阶段 | `1e-6` ~ `1e-5` | InstructGPT: `5e-7` |

### 调参逻辑

**学习率是最敏感的超参数之一**，过高导致 loss 震荡甚至 NaN，过低则收敛极慢甚至陷入局部最优。

核心调参原则如下：

**Scaling Law 视角**：大模型普遍遵循"模型越大，学习率越小"的规律。参数量从 1B 到 70B，学习率通常下调 2~5 倍。

**与 Batch Size 联动**：根据 Linear Scaling Rule，当 Batch Size 扩大 k 倍时，学习率应相应扩大 √k 或 k 倍。大模型实践中多用 √k 缩放以保持稳定性。

**预热策略（Warmup）**：
- 训练初期，参数尚未校准，应从极小值线性升至目标学习率
- 典型 Warmup 步数：`max_steps * 0.01` ~ `max_steps * 0.03`
- 大模型常见 Warmup Steps：500 ~ 2000 步

### 优化方向

- 优先使用 **Cosine Decay + Warmup** 的组合调度策略
- 若训练中途出现 loss spike，可临时降低学习率至 1/3，观察是否恢复
- 建议使用 **学习率查找（LR Finder）** 或 **网格搜索** 确定初始值
- 分层学习率（Layerwise LR Decay）：底层使用更小的 LR，逐层递增，对微调尤为有效

---

## 二、批量大小（Batch Size）

### 常用值范围

| 场景 | Global Batch Size | Micro Batch Size |
|-----|-----------------|-----------------|
| 预训练（A100 集群） | 1M ~ 4M tokens/step | 1 ~ 4 per GPU |
| 监督微调（SFT） | 128 ~ 512 samples | 1 ~ 8 per GPU |
| RLHF | 64 ~ 256 | 取决于显存 |

> [!NOTE]
> Global Batch Size = Micro Batch Size × 梯度累积步数 × GPU 数量

### 调参逻辑

大 Batch Size 的优势与代价需要权衡：

**优势**：
- 更稳定的梯度估计，训练曲线更平滑
- 更高的硬件利用率（GPU 吞吐提升）
- 允许使用更大的学习率

**代价**：
- 泛化能力下降（大 Batch 容易陷入 sharp minima）
- 显存消耗成倍增加

实践中，**通过梯度累积（Gradient Accumulation）** 在不增加显存的前提下模拟大 Batch 是主流方案。

### 优化方向

- 预训练阶段以 **token 数量** 为单位衡量 Batch Size（如 LLaMA 使用 4M tokens/step）
- 微调阶段更关注 **样本多样性**，Batch 不宜过大（512~1024 samples 通常够用）
- 使用 **动态批次（Dynamic Batching）** 按序列长度分桶，提升 GPU 效率

---

## 三、序列长度（Sequence Length / Context Length）

### 常用值范围

| 模型规模 | 预训练长度 | 微调/推理长度 |
|--------|---------|------------|
| 7B 级别 | 2K ~ 4K | 4K ~ 32K |
| 13B ~ 70B | 4K | 4K ~ 128K |
| 100B+ | 2K ~ 8K | 根据需求扩展 |

### 调参逻辑

序列长度直接影响显存消耗（Attention 的计算复杂度为 O(n²)）：

- **短序列训练** 成本低，适合快速验证
- **长序列扩展** 通常采用两阶段：先用短序列预训练，再用长序列继续训练（Long Context Fine-tuning）
- RoPE 位置编码支持通过 **YaRN / LongRoPE** 等方法低成本扩展上下文

### 优化方向

- 预训练时控制长序列比例（如 5%~10% 超长样本），避免 GPU 利用率下降
- 使用 **Flash Attention 2** 将显存复杂度降至 O(n)，突破长序列瓶颈
- 序列长度扩展时，学习率应相应降低（约原来的 1/3 ~ 1/5）

---

## 四、优化器参数（Optimizer Hyperparameters）

### AdamW 核心参数

大模型训练的标准优化器为 **AdamW（Adam + Weight Decay）**。

| 参数 | 常用值 | 说明 |
|-----|------|-----|
| `β₁`（一阶动量） | `0.9` | 控制梯度的指数移动平均，几乎不需调整 |
| `β₂`（二阶动量） | `0.95` ~ `0.999` | 控制梯度平方的平均，大模型推荐 `0.95` |
| `ε`（数值稳定性） | `1e-8` ~ `1e-5` | 防止除零，BF16 训练建议 `1e-6` ~ `1e-5` |
| `weight_decay` | `0.01` ~ `0.1` | L2 正则化强度，通常 `0.1` |

> [!WARNING]
> β₂ 过大（如 0.999）在训练初期会导致梯度估计滞后，对 loss spike 的响应变慢；推荐 `0.95` ~ `0.98` 作为大模型默认值。

### 调参逻辑

**Weight Decay** 对不同参数的影响差异显著：
- **Embedding 层**：通常不施加 Weight Decay（设为 0）
- **Bias 项**：同样建议不施加 Weight Decay
- **线性层权重**：标准施加，`0.01` ~ `0.1`

**新兴优化器选择**：

| 优化器 | 特点 | 适用场景 |
|------|-----|--------|
| AdamW | 稳定，成熟 | 通用默认选择 |
| Lion | 仅符号更新，显存更省 | 超大模型，资源受限 |
| Muon | 矩阵正交更新 | 实验性，部分场景更快收敛 |
| Sophia | 二阶优化近似 | 研究阶段 |

### 优化方向

- 大模型（>30B）建议尝试将 `β₂` 降至 `0.95`，提升训练稳定性
- 使用 **分层 Weight Decay**：越靠近输入层，Weight Decay 越小
- 8-bit Adam（bitsandbytes）可降低优化器状态显存至 1/4

---

## 五、梯度裁剪（Gradient Clipping）

### 常用值范围

| 参数 | 常用值 |
|-----|------|
| `max_grad_norm` | `0.5` ~ `1.0` |
| 大模型常用值 | `1.0` |
| 不稳定训练时 | `0.3` ~ `0.5` |

### 调参逻辑

梯度裁剪是**防止梯度爆炸的核心手段**，在大模型训练中几乎是标配：

- 当梯度全局 L2 范数超过阈值时，按比例缩放所有梯度
- `clip_norm = 1.0` 是绝大多数大模型的默认选择（LLaMA、GPT 系列均如此）
- 若训练中频繁出现 loss spike，可临时降低至 `0.5`

### 优化方向

- **监控梯度范数**（grad_norm）是必要的日志指标，突然升高是 loss spike 的预警信号
- 梯度裁剪值不宜过小，否则会人为压制有效更新，导致收敛变慢
- 混合精度训练（BF16/FP16）下，梯度裁剪前需确保数值缩放正确

---

## 六、学习率调度策略（LR Scheduler）

### 主流策略对比

| 策略 | 适用场景 | 特点 |
|-----|--------|-----|
| **Cosine Decay** | 预训练、大模型通用 | 平滑下降，尾段学习率趋近 0 |
| **Linear Decay** | 微调任务 | 简单稳定，可预测 |
| **Cosine with Restarts** | 持续训练/多任务 | 周期性重新激活 |
| **Constant + Warmup** | 快速实验 | 无衰减，适合短期验证 |
| **WSD（Warmup-Stable-Decay）** | 最新大模型趋势 | MiniCPM、Falcon 等采用 |

### WSD 调度详解

WSD（Warmup-Stable-Decay）是近期大模型训练的主流选择：

```
[Warmup] → [Stable LR 长期训练] → [快速 Decay 冲刺]
```

优势在于：
- **Stable 阶段** 可以随时保存 checkpoint 并延长训练
- **Decay 阶段** 快速收敛，收益显著
- 灵活应对数据增加或训练中断后的续训场景

### 优化方向

- 预训练首选 **Cosine Decay + Warmup**，效果稳定
- 持续训练（Continual Pre-training）推荐 **WSD** 策略
- Warmup 比例：`1% ~ 3%` 的总训练步数是安全范围

---

## 七、Dropout 与正则化参数

### 常用值范围

| 参数 | 预训练 | 微调 |
|-----|------|-----|
| `dropout` | `0.0` ~ `0.1` | `0.05` ~ `0.1` |
| `attention_dropout` | `0.0` | `0.0` ~ `0.05` |
| `residual_dropout` | `0.0` ~ `0.1` | `0.05` |

### 调参逻辑

大模型预训练阶段通常**不使用或使用极小的 Dropout**，原因如下：

- 大规模数据本身提供了充足的正则化效果
- Dropout 会降低训练效率（激活被随机丢弃）
- 超大模型（>30B）几乎不使用 Dropout

微调阶段适度引入 Dropout 可防止小数据集上的过拟合。

### 优化方向

- 数据量充足时：`dropout = 0.0`
- 数据量较少的微调：`dropout = 0.05` ~ `0.1`
- **Label Smoothing**（`0.0` ~ `0.1`）是比 Dropout 更稳定的正则化替代方案

---

## 八、LoRA 专属超参数

低秩适配（LoRA）是当前微调大模型的主流方法，有其独特的超参数体系。

### 核心参数

| 参数 | 常用值 | 说明 |
|-----|------|-----|
| `lora_r`（秩） | `8` ~ `64` | 越大表达能力越强，显存消耗越高 |
| `lora_alpha` | `16` ~ `128` | 缩放因子，通常设为 `2 × lora_r` |
| `lora_dropout` | `0.0` ~ `0.1` | LoRA 层的 Dropout |
| 应用位置 | Q, V（最小） → Q,K,V,O,FFN（最大） | 应用范围影响参数量 |

### alpha/r 比值的意义

```
effective_lr = lr × (lora_alpha / lora_r)
```

- `alpha/r = 1`：LoRA 贡献与原权重等量级
- `alpha/r = 2`：放大 LoRA 贡献（最常见设置）
- 固定 `alpha = 2r` 是通用的稳健策略

### 调参逻辑

- **任务复杂度低**（格式遵循、简单问答）：`r = 8 ~ 16` 足够
- **任务复杂度高**（代码生成、逻辑推理）：`r = 32 ~ 64`
- **全量微调预算受限**：`r = 64 ~ 128` + 应用于所有线性层，接近全量效果
- QLoRA（4-bit 量化）：`r = 16 ~ 64`，`alpha = 2r`

### 优化方向

- 使用 **DoRA**（Weight-Decomposed LoRA）可进一步提升表达能力
- **RsLoRA**：自动将 alpha 缩放为 `alpha / √r`，更稳定
- 对长上下文任务，扩大 LoRA 应用范围至 FFN 层效果更佳

---

## 九、数据相关超参数

### 核心参数

| 参数 | 常用值 | 说明 |
|-----|------|-----|
| `num_epochs` | 预训练 `1~3`，微调 `1~5` | 大数据集单轮即可，小数据集可多轮 |
| `max_steps` | 根据数据量计算 | 优先使用 max_steps 而非 epochs |
| `data_mix_ratio` | 任务数据: 通用数据 = 1~9 : 1 | 防止灾难性遗忘 |
| `packing` | 启用（提升吞吐） | 将多个短样本打包至单一序列 |

### 调参逻辑

**Epochs vs Max Steps**：大模型训练中，以 token 数量（Max Steps × Global Batch Tokens）衡量训练量更准确。Chinchilla 法则建议：**参数量 × 20 = 最优训练 token 数**。

**数据混合策略**：
- 微调时加入少量通用数据（如 5%~10% 预训练数据），显著降低灾难性遗忘风险
- 数学、代码等专业领域数据混合比例通常为 20%~40%

### 优化方向

- 使用 **Data Curriculum**（先易后难的数据课程）可提升最终模型质量
- 预训练数据去重（MinHash LSH 等）比增加训练轮次更有效
- 监控训练集 perplexity，若过早收敛说明数据多样性不足

---

## 十、综合调参策略与最佳实践

### 调参优先级排序

```
学习率 > Batch Size > 序列长度 > 优化器参数 > Dropout > LoRA 参数
```

### 快速基线配置参考

**预训练基线（7B 规模）**：

```yaml
learning_rate: 3e-4
lr_scheduler: cosine
warmup_steps: 2000
batch_size_tokens: 4M
max_seq_len: 4096
optimizer: adamw
beta1: 0.9
beta2: 0.95
weight_decay: 0.1
grad_clip: 1.0
```

**SFT 微调基线（7B，LoRA）**：

```yaml
learning_rate: 2e-4
lr_scheduler: cosine
warmup_ratio: 0.03
num_epochs: 3
lora_r: 16
lora_alpha: 32
lora_dropout: 0.05
target_modules: [q_proj, v_proj, k_proj, o_proj]
```

### 常见问题诊断

| 现象 | 可能原因 | 解决方案 |
|-----|--------|--------|
| Loss NaN / 爆炸 | 学习率过高 / 梯度爆炸 | 降低 LR，增大 grad_clip |
| Loss 不下降 | 学习率过低 / 数据质量差 | 升高 LR，检查数据 |
| Loss 下降后反弹 | 学习率未衰减 / 数据噪声 | 启用 LR Decay，清洗数据 |
| 过拟合（train loss << val loss） | 数据量不足 / 轮数过多 | 增大正则化，减少 epochs |
| GPU 利用率低 | Batch 过小 / 序列不均匀 | 增大 Batch，启用 packing |

### 工程经验总结

- **先跑小规模验证**：用 1B 模型验证超参配置，再扩展到 7B/13B
- **监控关键指标**：loss、grad_norm、lr、tokens/sec、GPU 利用率
- **保存足够的 checkpoint**：至少保存最近 3 个，防止 loss spike 后无法回退
- **使用确定性种子**：`seed = 42` 保证可复现性，对比实验必须固定
- **渐进式扩展**：批量大小、序列长度不要一步到位，逐步扩大

---

## 结语

超参数调优没有万能公式，但遵循 **"先确定大方向，再精细调整"** 的原则，结合 Scaling Law 的指导，可以大幅缩短探索周期。随着 AutoML 和 Bayesian Optimization 工具的成熟，自动化超参搜索也逐渐成为大型团队的标配。

理解每个超参数背后的数学直觉，比死记忆推荐值更重要——参数是工具，理解才是核心竞争力。
