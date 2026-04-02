# Pre-LN 与 Post-LN 本质差异解析

Layer Normalization（LN）在残差块中的位置，直接决定了模型的训练稳定性、梯度特性和可扩展性——Pre-LN（LN在子层前）与 Post-LN（LN在残差相加后）的核心差异，并非“是否对下一层输入标准化”，而是**梯度传递路径的本质不同**。

## 一、统一符号与结构定义

- \(x_l\)：第 \(l\) 个残差块的输入（即上一层残差块的输出）；
- \(x_{l+1}\)：第 \(l\) 个残差块的输出（即下一层残差块的输入）；
- \(F_l(\cdot)\)：第 \(l\) 层的子网络（通常为「Self-Attention + Feed-Forward Network」，简称 Sublayer）；
- \(\text{LN}(z)\)：Layer Normalization 操作，核心是对输入 \(z\) 做标准化（均值为0、方差为1），再通过可学习参数缩放和偏移；
- \(\mathcal{L}\)：模型最终的损失函数；
- \(\frac{\partial \mathcal{L}}{\partial x_l}\)：损失对第 \(l\) 层输入的梯度（即从顶层往底层传递的梯度，是模型参数更新的核心）；
- \(I\)：单位矩阵，用于描述残差连接的恒等映射特性。

残差连接的核心思想是：通过“恒等映射通道”让信息无损传递，同时让子网络 \(F_l(\cdot)\) 学习“残差信号”（即输入与输出的差异），降低深层网络的优化难度。而 LN 的位置，直接破坏或保留了这条恒等映射通道的梯度传递特性。

## 二、Post-LN 结构：梯度不稳定的本质（数学推导+问题剖析）

### 2.1 Post-LN 前向传播公式（原始 Transformer 结构）

Post-LN 的核心特征是：**先进行残差相加，再对相加结果做 LayerNorm**，具体公式如下：

1. 残差相加：\(s_l = x_l + F_l(x_l)\)（子网络 \(F_l(\cdot)\) 直接作用于未标准化的原始输入 \(x_l\)）；
2. LayerNorm 归一化：\(x_{l+1} = \text{LN}(s_l)\)（归一化后的结果作为下一层输入）。

从表面看，Post-LN 确实会对下一层输入 \(x_{l+1}\) 做标准化，但这种标准化仅作用于“前向信号”，无法影响“反向梯度”的传递——这是其所有问题的根源。

### 2.2 Post-LN 梯度推导（核心：梯度连乘灾难）

模型训练的关键是梯度从顶层（输出层）传递到底层（输入层），我们重点推导梯度递推式：\(\frac{\partial \mathcal{L}}{\partial x_l} = \frac{\partial \mathcal{L}}{\partial x_{l+1}} \cdot \frac{\partial x_{l+1}}{\partial x_l}\)，其中 \(\frac{\partial x_{l+1}}{\partial x_l}\) 是决定梯度稳定性的核心雅可比矩阵。

根据链式法则，拆解 \(\frac{\partial x_{l+1}}{\partial x_l}\)：

$$\frac{\partial x_{l+1}}{\partial x_l} = \frac{\partial \text{LN}(s_l)}{\partial s_l} \cdot \frac{\partial s_l}{\partial x_l}$$

第一步：计算 \(\frac{\partial s_l}{\partial x_l}\)（残差相加对输入的导数）：

$$\frac{\partial s_l}{\partial x_l} = \frac{\partial (x_l + F_l(x_l))}{\partial x_l} = I + \frac{\partial F_l(x_l)}{\partial x_l}$$

这里的 \(I\) 本应是残差连接的“恒等梯度通道”，但后续的 LN 导数会破坏这一通道。

第二步：代入完整梯度递推式：

$$\boxed{ \frac{\partial \mathcal{L}}{\partial x_l} = \underbrace{\frac{\partial \text{LN}}{\partial s_l}}_{\text{LN 梯度缩放因子}} \cdot \underbrace{\left(I + \frac{\partial F_l}{\partial x_l}\right)}_{\text{残差项}} \cdot \frac{\partial \mathcal{L}}{\partial x_{l+1}} }$$

从顶层 \(L\) 传递到底层 \(1\)，梯度需经过 \(L-1\) 层的连乘，最终形式为：

$$\frac{\partial \mathcal{L}}{\partial x_1} = \left( \prod_{l=1}^{L-1} \frac{\partial \text{LN}}{\partial s_l} \cdot \left(I + \frac{\partial F_l}{\partial x_l}\right) \right) \cdot \frac{\partial \mathcal{L}}{\partial x_L}$$

### 2.3 从梯度推导看 Post-LN 的核心问题（本质缺陷）

Post-LN 的所有缺点，均源于上述梯度递推式的结构缺陷——恒等通道被 LN 缩放包裹，导致梯度连乘出现“指数级偏差”，具体剖析如下：

#### （1）梯度消失与梯度爆炸（核心问题）

梯度连乘的稳定性，取决于每一层 \(\frac{\partial \text{LN}}{\partial s_l} \cdot \left(I + \frac{\partial F_l}{\partial x_l}\right)\) 的范数大小：

- 若该乘积的范数 < 1：多层连乘后，梯度会**指数级衰减**，最终趋近于0（梯度消失），底层参数无法更新，模型无法学习到浅层特征；
- 若该乘积的范数 > 1：多层连乘后，梯度会**指数级放大**，最终数值溢出（梯度爆炸），导致权重损坏、训练崩溃（如 Loss 骤升、梯度 NaN）。

关键原因：子网络 \(F_l(\cdot)\) 的输入是未标准化的 \(x_l\)，其分布不稳定，导致 \(\frac{\partial F_l}{\partial x_l}\) 波动极大；而 LN 导数 \(\frac{\partial \text{LN}}{\partial s_l}\) 会进一步缩放梯度，加剧这种波动——LN 仅能修正前向信号的分布，无法修正反向梯度的波动。

#### （2）对超参数极其敏感（依赖学习率 Warm-up）

训练初期，权重随机初始化，未标准化的 \(x_l\) 分布差异极大，导致 \(\frac{\partial F_l}{\partial x_l}\) 数值剧烈波动，梯度极易爆炸。为避免模型“跑飞”，必须使用极小的学习率进行数千步的 Warm-up，逐步调整参数分布——一旦 Warm-up 设置不当（如步数不足、初始学习率过大），模型会立即崩溃。

对比 Pre-LN，Post-LN 对 Warm-up 的依赖并非“设计需求”，而是“梯度不稳定的被动妥协”。

#### （3）深层模型训练困难（可扩展性差）

随着层数 \(L\) 增加，梯度连乘的“指数级偏差”会不断累积，不稳定性呈指数级增长。在不使用 Fixup、ReZero 等特殊初始化技巧的情况下，纯 Post-LN Transformer 很难训练超过 20-30 层——这与大模型“深层堆叠”的需求完全矛盾，也是 Post-LN 被淘汰的核心原因。

#### （4）恒等映射路径被破坏

残差连接的核心是“恒等映射通道”，让信息无损传递。但 Post-LN 中，每一层的残差相加结果 \(s_l\) 必须经过 LN 重新缩放和偏移，才能作为下一层输入——这意味着原始输入信号在传递过程中被不断“扭曲”，恒等通道的梯度被 LN 导数包裹，无法直接传递，削弱了残差连接的核心作用，导致深层神经元难以微调浅层特征。

## 三、Pre-LN 结构：梯度稳定的本质（数学推导+优势剖析）

### 3.1 Pre-LN 前向传播公式（主流大模型结构）

Pre-LN 的核心特征是：**先对输入做 LayerNorm，再将标准化后的结果输入子网络，最后直接进行残差相加**，具体公式如下：

1. LayerNorm 归一化：\(z_l = \text{LN}(x_l)\)（先标准化输入，让子网络在稳定分布下工作）；
2. 残差相加：\(x_{l+1} = x_l + F_l(z_l)\)（子网络输出与原始输入直接相加，不经过 LN 干扰）。

Pre-LN 不仅对下一层输入 \(x_{l+1}\) 间接实现了分布稳定（子网络输出被残差稀释，整体分布更平缓），更关键的是，它保留了恒等映射的梯度通道，从根源上解决了梯度不稳定问题。

### 3.2 Pre-LN 梯度推导（核心：恒等通道保护）

同样推导梯度递推式 \(\frac{\partial \mathcal{L}}{\partial x_l} = \frac{\partial \mathcal{L}}{\partial x_{l+1}} \cdot \frac{\partial x_{l+1}}{\partial x_l}\)，重点分析 \(\frac{\partial x_{l+1}}{\partial x_l}\)：

$$\frac{\partial x_{l+1}}{\partial x_l} = \frac{\partial \left( x_l + F_l(z_l) \right)}{\partial x_l} = \frac{\partial x_l}{\partial x_l} + \frac{\partial F_l(\text{LN}(x_l))}{\partial x_l}$$

根据链式法则拆解第二项，最终得到：

$$\boxed{ \frac{\partial \mathcal{L}}{\partial x_l} = \underbrace{\left( I + \frac{\partial F_l}{\partial z_l} \cdot \frac{\partial \text{LN}}{\partial x_l} \right)}_{\text{恒等项始终存在}} \cdot \frac{\partial \mathcal{L}}{\partial x_{l+1}} }$$

从顶层 \(L\) 传递到底层 \(1\)，梯度连乘形式为：

$$\frac{\partial \mathcal{L}}{\partial x_1} = \left( \prod_{l=1}^{L-1} \left( I + \frac{\partial F_l}{\partial z_l} \cdot \frac{\partial \text{LN}}{\partial x_l} \right) \right) \cdot \frac{\partial \mathcal{L}}{\partial x_L}$$

### 3.3 从梯度推导看 Pre-LN 的核心优势（本质优势）

Pre-LN 的所有优势，均源于梯度递推式中“恒等矩阵 \(I\) 始终存在”——无论子网络和 LN 的导数如何波动，恒等通道都会“拉住”梯度，避免其出现指数级偏差，具体剖析如下：

#### （1）保护恒等映射路径（核心优势）

梯度递推式中，\(I\) 不依赖任何子网络或 LN 的导数，是一条“纯恒等梯度通道”——梯度可以不经过任何子网络、不被任何 LN 缩放，直接从顶层传递到底层。这种结构让模型初始化时，更接近“多个浅层网络的集成”，极大降低了优化难度。

#### （2）彻底解决梯度消失与爆炸

由于恒等矩阵 \(I\) 始终存在，梯度连乘具有“自我稳定”特性：

- 若子网络梯度偏小（\(\frac{\partial F_l}{\partial z_l} \cdot \frac{\partial \text{LN}}{\partial x_l} \approx 0\)），则 \(\frac{\partial x_{l+1}}{\partial x_l} \approx I\)，梯度几乎原样传递，不会消失；
- 若子网络梯度偏大，恒等项 \(I\) 会限制整体梯度的缩放幅度，避免梯度指数级放大（即爆炸）。

更关键的是，多层连乘时，由于每一项都是 \(I + \text{小量}\)（子网络梯度经过 LN 标准化后，波动被限制在小范围），根据近似公式：

$$\prod_{l=1}^{L-1} \Big(I + \text{小量}\Big) \approx I + \sum_{l=1}^{L-1} \text{小量} \approx I$$

梯度整体始终围绕 \(I\) 波动，不会出现指数级偏差，从根源上解决了两种梯度问题。

#### （3）支持深层网络架构（可扩展性极强）

梯度的稳定性的让 Pre-LN 可以轻松堆叠数百层甚至上千层——GPT-3（175B 参数，96 层）、LLaMA 系列等主流大模型，均采用 Pre-LN 结构。相比 Post-LN 20-30 层的层数限制，Pre-LN 完全适配大模型“深层堆叠、能力涌现”的需求。

#### （4）超参数鲁棒性强（无需或缩短 Warm-up）

Pre-LN 中，子网络 \(F_l(\cdot)\) 的输入是标准化后的 \(z_l\)（分布稳定），\(\frac{\partial F_l}{\partial z_l}\) 波动极小，初始化阶段梯度就很稳定。因此，Pre-LN 无需 Warm-up 或仅需极短步数的 Warm-up，就能平稳启动训练；同时，它对学习率的波动也不敏感，适合超大规模分布式训练（避免因超参数微小偏差导致训练崩溃）。

## 四、关键疑问解答：为什么 LN 必须在子层之前？

有疑问：为何不将 LN 放在子层之后、残差相加之前（即 \(x_{l+1} = x_l + \text{LN}(F_l(x_l))\)）？核心原因是「保护子网络输入分布，控制方差爆炸」，具体分析如下：

1. 子网络对输入分布极度敏感：Self-Attention 和 FFN 子网络的参数，需要在稳定的数值范围内更新才能收敛。Pre-LN 确保子网络的输入 \(z_l = \text{LN}(x_l)\) 始终是均值为0、方差为1的稳定分布，让参数从训练第一步就处于合理范围。
2. 控制子网络输出的方差爆炸：若不在子层前做 LN，子网络 \(F_l(x_l)\) 的输出方差会随着层数加深而迅速膨胀（因为 \(x_l\) 是残差累加的结果，方差不断累积）。在子层前做 LN，本质是限制了子网络对残差主干的“干扰强度”，避免方差爆炸破坏梯度传递。

简言之：LN 的核心作用是“稳定子网络的输入分布”，而不是“修正残差相加后的输出分布”——放在子层前，才能真正发挥 LN 的作用，同时保护恒等梯度通道。

## 五、Pre-LN 与 Post-LN 核心差异总结（表格对比）

| 对比维度     | Post-LN                                                      | Pre-LN                                                       |
| :----------- | :----------------------------------------------------------- | :----------------------------------------------------------- |
| 前向公式     | $$x_{l+1} = \text{LN}(x_l + F_l(x_l))$$                      | $$x_{l+1} = x_l + F_l(\text{LN}(x_l))$$                      |
| 梯度核心项   | $$\frac{\partial \text{LN}}{\partial s_l} \cdot (I + \frac{\partial F_l}{\partial x_l})$$（恒等项被包裹） | $$I + \frac{\partial F_l}{\partial z_l} \cdot \frac{\partial \text{LN}}{\partial x_l}$$（恒等项始终存在） |
| 梯度稳定性   | 差，易出现梯度消失/爆炸（连乘灾难）                          | 好，梯度围绕 \(I\) 波动，无指数偏差                          |
| 超参数敏感性 | 极高，依赖复杂 Warm-up，易崩溃                               | 低，无需/短 Warm-up，鲁棒性强                                |
| 最大层数限制 | 约 20-30 层（无特殊技巧）                                    | 数百层甚至上千层（适配大模型）                               |
| 恒等映射路径 | 被 LN 破坏，信息传递有扭曲                                   | 被完整保留，信息无损传递                                     |
| 适用场景     | 浅层模型、简单任务                                           | 深层模型、大参数模型、复杂任务                               |

## 六、终极结论

Pre-LN 与 Post-LN 的本质差异，不在于“是否对下一层输入标准化”，而在于「梯度传递路径中是否保留了未被干扰的恒等通道」：

- Post-LN：先残差相加、后 LN，恒等梯度通道被 LN 导数包裹，梯度连乘出现指数级偏差，导致训练不稳定、深层难训练；
- Pre-LN：先 LN、后子网络、再残差相加，恒等梯度通道被完整保留，梯度连乘被 \(I\) 稳定在合理范围，从根源上解决了梯度问题，成为大模型的主流选择。

简言之：Pre-LN 保护的是「梯度传递」，Post-LN 仅保护了「前向输入」——梯度的稳定性，才是深层模型训练的核心关键。

# RMSNorm

标准 LayerNorm:



$$\bar{x}_i = \frac{x_i - \mu}{\sqrt{\frac{1}{n}\sum_{i=1}^n (x_i - \mu)^2 + \epsilon}} \cdot \gamma_i + \beta_i$$

RMSNorm:



$$\bar{x}_i = \frac{x_i}{\sqrt{\frac{1}{n}\sum_{i=1}^n x_i^2 + \epsilon}} \cdot \gamma_i$$

其中 $\text{RMS}(x) = \sqrt{\frac{1}{n}\sum_{i=1}^n x_i^2 + \epsilon}$。可以看到，它不再需要计算均值 $\mu$，也不再有偏置项 $\beta$。

#### A. 更高的计算效率

这是 RMSNorm 最直观的优点。

- **计算开销小**：省去了均值的计算以及后续的所有减法操作。在 GPU 上，这种简化能显著减少内存读写和计算延迟（大约能提升 10%~40% 的归一化层计算速度）。
- **参数量更少**：去掉了 $\beta$（偏置项）参数，虽然在超大模型中这部分节省的参数量微乎其微，但模型结构变得更加简洁。

#### B. 训练稳定性与收敛速度

- **保持向量方向**：RMSNorm 实际上是对神经元激活值的向量做了“投影”到固定长度球面的操作。它只关注激活值的**模长（Magnitude）**，而不改变向量的**方向**。
- **数值稳定性**：在混合精度训练（Mixed Precision Training）中，减少减法操作有助于降低由于浮点数精度限制带来的数值不稳定性（如 Round-off errors）。
- 无均值计算带来的精度误差，分母 RMS (x) 恒正，避免方差趋近于零导致的除零风险与梯度震荡。
- LayerNorm 的 “减均值” 是基于**当前层所有 token 的均值**做的，一旦均值偏移，相当于给每个 token 都减去了一个 “不稳定的数”，直接导致：
  - 归一化后的数值分布跟着均值一起漂移（比如均值从 0.1 变 0.5，所有 token 的归一化值都整体下降 0.4）；
  - 这种漂移会沿着网络层传递，越到深层越明显，最终引发严重的内部协变量偏移 —— 模型训练时要不断适配这种漂移，表现为 loss 震荡、收敛慢，甚至在万级 token 训练时直接不收敛。

#### C. 实证性能几乎等同于 LayerNorm

多项实验（包括 Llama 论文）证明，在各种下游任务上，RMSNorm 的表现与标准 LayerNorm 基本持平。既然性能一样，但速度更快、逻辑更简单，开发者自然倾向于选择 RMSNorm。