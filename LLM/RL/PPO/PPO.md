# PPO（Proximal Policy Optimization）

> **文档结构**
> 1. 数学推导与目标函数
> 2. Actor-Critic 架构设计
> 3. 完整算法流程（含 GAE + 伪代码）
> 4. RLHF 中的 PPO（4 步拆解 + 速查表）
> 5. 优缺点对比

---

## 一、数学推导与目标函数

PPO 的目标函数推导是一个从"策略梯度"→"重要性采样"→"裁剪限制"的演进过程。

---

### 1. 起点：策略梯度（Policy Gradient, PG）

标准的策略梯度试图最大化期望累积奖励 $J(\theta)$，其梯度公式为：

$$\nabla_\theta J(\theta) = \mathbb{E}_{\tau \sim \pi_\theta} \left[ \nabla_\theta \log \pi_\theta(a_t|s_t) \hat{A}_t \right]$$

| 符号 | 含义 |
|------|------|
| $\pi_\theta(a_t\|s_t)$ | 在状态 $s_t$ 下采取动作 $a_t$ 的概率 |
| $\hat{A}_t$ | 优势函数（Advantage），衡量该动作比平均期望好多少 |

**PG 的致命缺点**：它是**同策略（On-policy）**的。一旦用这批数据更新了参数 $\theta$，策略就变了，这批数据就作废了。**样本利用率极低。**

---

### 2. 改进：重要性采样（Importance Sampling）

为了能**重复利用**收集到的数据进行多次更新，引入重要性采样，将同策略转化为异策略。

**定义概率比率（Probability Ratio）**：

$$r_t(\theta) = \frac{\pi_\theta(a_t|s_t)}{\pi_{\theta_{old}}(a_t|s_t)}$$

基于此得到**替代目标函数（Surrogate Objective）**：

$$L^{CPI}(\theta) = \mathbb{E}_t \left[ \frac{\pi_\theta(a_t|s_t)}{\pi_{\theta_{old}}(a_t|s_t)} \hat{A}_t \right] = \mathbb{E}_t \left[ r_t(\theta) \hat{A}_t \right]$$

*（注：CPI 指 Conservative Policy Iteration）*

**问题**：不对 $r_t(\theta)$ 加以限制时，最大化此目标函数会导致新策略偏离旧策略太远，从而**模型崩溃**。

---

### 3. 核心魔法：PPO-Clip 目标函数

PPO 放弃 TRPO 复杂的 KL 散度约束，直接用 `clip` 函数强行截断，得到 $L^{CLIP}$：

$$L^{CLIP}(\theta) = \mathbb{E}_t \left[ \min \left( r_t(\theta)\hat{A}_t,\ \text{clip}(r_t(\theta), 1-\epsilon, 1+\epsilon)\hat{A}_t \right) \right]$$

超参数 $\epsilon$ 通常取 **0.2**，即将 $r_t(\theta)$ 限制在 $[0.8,\ 1.2]$ 之间。外部的 `min` 起到**悲观下界（Pessimistic Bound）**的作用：

**情况 A：$\hat{A}_t > 0$（动作很好）**

- 模型希望 $r_t(\theta)$ 越大越好（增加概率）。
- 当 $r_t(\theta) \le 1+\epsilon$（如 1.1）时，`min` 取原值 $1.1\hat{A}_t$。
- 当 $r_t(\theta) > 1+\epsilon$（如 1.5）时，`min` 取截断值 $(1+\epsilon)\hat{A}_t$。**梯度被切断，防止更新过头。**

**情况 B：$\hat{A}_t < 0$（动作很差）**

- 模型希望 $r_t(\theta)$ 越小越好（减少概率）。
- 当 $r_t(\theta) \ge 1-\epsilon$（如 0.9）时，`min` 取原值 $0.9\hat{A}_t$。
- 当 $r_t(\theta) < 1-\epsilon$（如 0.5）时，由于 $\hat{A}_t$ 是负数，`min` 取截断值 $(1-\epsilon)\hat{A}_t$。**梯度被切断，防止策略骤变。**

---

### 4. 最终总目标函数（Total Objective）

PPO 实际训练时同时优化 Actor（策略）和 Critic（价值网络），并加入熵奖励鼓励探索。**最大化**的联合目标函数为：

$$L^{PPO}(\theta) = \mathbb{E}_t \left[ L^{CLIP}(\theta) - c_1 L^{VF}(\theta) + c_2 S[\pi_\theta](s_t) \right]$$

| 项 | 含义 |
|----|------|
| $L^{CLIP}(\theta)$ | Actor 的策略损失（见上方推导） |
| $L^{VF}(\theta) = (V_\theta(s_t) - V_t^{target})^2$ | Critic 的价值损失（MSE），让预测值接近真实累积回报 |
| $S[\pi_\theta](s_t)$ | 策略的信息熵（Entropy Bonus）。熵越大，输出概率越均匀，鼓励**探索**，防止陷入局部最优 |
| $c_1,\ c_2$ | 平衡三项的权重系数 |

---

## 二、架构设计（Actor-Critic Architecture）

PPO 通常采用 Actor-Critic 架构，有两种常见实现方式：

### 1. 独立网络设计（Separate Networks）— LLM 中最常用

| 网络 | 输入 | 输出 |
|------|------|------|
| **Actor 网络** | 当前状态 $s_t$（LLM 中为 Prompt + 已生成 tokens） | 动作分布 $\pi(a_t\|s_t)$（LLM 中为 Softmax 词表概率） |
| **Critic 网络** | 当前状态 $s_t$ | 标量 $V(s_t)$，对当前状态预期总奖励的评估 |

> 在 ChatGPT（RLHF）中，Actor 和 Critic 通常是结构相同但参数不同的两个独立 LLM，Critic 的最后一层被替换为输出标量的线性层。

### 2. 参数共享设计（Shared Network）— 游戏 / RL 基准测试中常用

- **主干网络（Backbone）**：Actor 和 Critic 共享前面的特征提取层（如 CNN/Transformer）。
- **Actor 头（Actor Head）**：分支出全连接层，输出动作概率。
- **Critic 头（Critic Head）**：分支出全连接层，输出标量价值。

> **优点**：减少参数量，特征共享。  
> **缺点**：Actor 和 Critic 的优化方向可能冲突（Destructive Interference）。

---

## 三、完整算法流程（Algorithm Flow）

PPO 标配使用 **GAE（Generalized Advantage Estimation，广义优势估计）** 计算优势函数，在偏差（Bias）和方差（Variance）之间取得良好平衡。

### 1. GAE 计算公式

**Step 1** — 计算单步时序差分误差（TD Error）：

$$\delta_t = r_t + \gamma V(s_{t+1}) - V(s_t)$$

其中 $r_t$ 为真实奖励，$\gamma$ 为折扣因子。

**Step 2** — 将未来 TD Error 衰减累加，得到优势估计：

$$\hat{A}_t = \sum_{l=0}^\infty (\gamma \lambda)^l \delta_{t+l}$$

其中 $\lambda$ 为 GAE 平滑参数，控制对长远未来的看重程度。

---

### 2. PPO-Clip 完整伪代码

以下是大多数 RL 库（如 Stable-Baselines3, TRL）底层的核心逻辑：

#### 【初始化阶段】

1. 初始化策略网络（Actor）参数 $\theta$，价值网络（Critic）参数 $\phi$。

#### 【外层循环：迭代 N 次】

对于每一次迭代 $iteration = 1, 2, \dots, N$：

**步骤 1 — Rollout（收集数据）**

2. 让当前策略 $\pi_\theta$ 在环境中交互 $T$ 个时间步。
3. 记录所有状态、动作、奖励等：$D = \{s_t, a_t, r_t, s_{t+1}, \pi_{\theta_{old}}(a_t|s_t)\}_{t=1}^T$

**步骤 2 — Compute Advantage（计算优势）**

4. 使用当前 Critic 网络评估每个状态的价值 $V_\phi(s_t)$，RM算真实奖励 $r_t$ 。
5. 通过 **GAE 公式**，利用真实奖励 $r_t$ 和预测值 $V_\phi(s_t)$，计算每步的优势 $\hat{A}_t$ 及目标回报 $V_t^{target}$。
6. **（关键）** 对 $\hat{A}_1 \dots \hat{A}_T$ 进行**标准化（Normalization）**处理（减均值、除标准差），这将极大提升训练稳定性。

**步骤 3 — Mini-batch PPO Update（多轮小批量更新）**

7. **内层循环**：将 $T$ 条数据打乱，分成多个 Mini-batch，循环训练 $K$ 个 Epoch：

   对于每个 Epoch（$1$ 到 $K$），对于每个 Mini-batch：

   - a. 计算新策略动作概率 $\pi_\theta(a_t|s_t)$。
   - b. 计算概率比率：$r_t(\theta) = \dfrac{\pi_\theta(a_t|s_t)}{\pi_{\theta_{old}}(a_t|s_t)}$
   - c. 计算截断损失 $L^{CLIP}$。
   - d. 计算价值损失 $L^{VF}$（Critic 预测值与 $V_t^{target}$ 的 MSE）。
   - e. 计算熵奖励 $S$。
   - f. 整合为总损失：$Loss = -[L^{CLIP} - c_1 L^{VF} + c_2 S]$
     *（注：PyTorch 默认最小化损失，因此目标函数前加负号）*
   - g. **反向传播**：对 Actor 和 Critic 进行梯度下降，更新参数 $\theta$ 和 $\phi$。

**步骤 4 — 参数同步**

8. $K$ 个 Epoch 训练结束后，清空数据 $D$。
9. 更新旧策略参数（代码实现中，由于每轮收集数据时使用最新参数，此步隐式完成）。继续下一轮大迭代。

---

## 四、RLHF 中的 PPO

将整个 RLHF（基于 PPO）的单次迭代拆解为 **4 个标准步骤**。

每轮循环中，**4 个模型**轮番上阵，**2 套公式**（业务奖励公式 + PPO 底层更新公式）完美交接。

---

### 第一步：生成回答（Rollout 阶段）

**目的**：让当前大模型根据提示词（Prompt）生成完整文本。

- **出场模型**：**Actor 模型**（策略模型 $\pi_\theta$）——【需更新参数的主角】
- **操作**：接收环境给出的 Prompt $x$，逐词生成回复 $y$，记录每个词生成的概率。
- **涉及公式**：无特别目标函数，纯正向推理（Forward Pass）。

---

### 第二步：计算环境奖励（Reward Evaluation）

**目的**：给 Actor 的回答"打分"，既看人类偏好，也看是否胡言乱语。

**出场模型（均冻结，不更新）：**

- **Reward 模型**（$r_\phi$）：给完整句子打出人类偏好分。
- **Reference 模型**（$\pi_{ref}$）：和 Actor 一起计算每个词的概率差距（KL 散度）。

> **RM 的训练**基于 **Bradley-Terry 模型**：
>
> $$L_{RM}(\phi) = -\mathbb{E}_{(x, y_w, y_l)} \left[ \log \sigma(r_\phi(x, y_w) - r_\phi(x, y_l)) \right]$$
>
> 较好回答胜出的概率为 $\sigma(\text{Score}(w) - \text{Score}(l))$，RM 学习为偏好回复 $y_w$ 打更高分。

**采用公式（InstructGPT 业务目标函数）：**

$$Reward_t = \underbrace{r_\phi(x, y)}_{\text{仅在句末给分}} - \beta \cdot \underbrace{\log \frac{\pi_\theta(y_t|x, y_{<t})}{\pi_{ref}(y_t|x, y_{<t})}}_{\text{每生成一个词都给一次惩罚}}$$​

> 最后算的是E(Reward)，展开后后面一项就变成了KL散度，KL散度恒大于0，求最大值时候就是等于1的时候

将 RM 整句得分与每步 KL 惩罚融合，得到生成每个 Token 时的**真实环境奖励 $R_t$**。

> **为什么 KL 散度约束至关重要？**
>
> 没有它，策略模型会产生以下问题：
> - **奖励黑客（Reward Hacking）**：模型找到 RM 漏洞，生成得分高但质量差的回复（冗长无信息量、自信但错误）。
> - **模式坍塌（Mode Collapse）**：策略坍塌为少数高分但单调的输出。
> - **能力退化**：模型遗忘预训练和 SFT 阶段积累的语言能力。
>
> Per-token KL 惩罚实现：$r_t = -\beta \cdot \log(\pi_\theta(a_t|s_t) / \pi_{ref}(a_t|s_t))$，最后一个 token 处加上 RM 奖励。
>
> **$\beta$ 的自适应调节策略**：观测 KL > 1.5 × 目标值时加倍 $\beta$；观测 KL < 目标值 / 1.5 时减半 $\beta$。典型目标 KL 约 6 nats。

---

### 第三步：价值评估与优势计算（Advantage Computation）

**目的**：评估"当前这步的表现比平均预期好多少"，以便更稳定地指导 Actor 更新。

- **出场模型**：**Critic 模型**（价值模型 $V_\omega$）——【需更新参数的配角】
- **操作**：Critic 对每步状态（已生成的文本）给出"预期总得分"的预测值 $V_\omega(s_t)$。

**采用公式（GAE 广义优势估计）：**

结合第二步的真实奖励 $R_t$ 与 Critic 预测值 $V_\omega$，计算：

$$\text{① 单步误差 (TD Error):}\quad \delta_t = R_t + \gamma V_\omega(s_{t+1}) - V_\omega(s_t)$$

$$\text{② 优势函数 (Advantage):}\quad \hat{A}_t = \sum_{l=0}^\infty (\gamma \lambda)^l \delta_{t+l}$$

$$\text{③ 目标总回报 (Target Value):}\quad V_t^{target} = \hat{A}_t + V_\omega(s_t)$$

---

### 第四步：模型参数更新（PPO Update 阶段）

**目的**：用优势 $\hat{A}_t$ 和目标回报 $V_t^{target}$ 修改神经网络权重，让 Actor 变更好、Critic 估分更准。

- **出场模型**：**Actor 模型**（$\pi_\theta$）和 **Critic 模型**（$V_\omega$）——均被更新。

**采用公式（PPO 底层联合 Loss）：**

$$Loss_{Total} = Loss_{Actor} + c_1 \cdot Loss_{Critic}$$

**Actor 的更新公式（PPO-Clip Loss）：**

$$Loss_{Actor} = - \mathbb{E}_t \left[ \min \left( \frac{\pi_\theta}{\pi_{\theta_{old}}} \hat{A}_t,\ \text{clip}\left(\frac{\pi_\theta}{\pi_{\theta_{old}}}, 1-\epsilon, 1+\epsilon\right) \hat{A}_t \right) \right]$$

*用 $\hat{A}_t$ 决定更新方向，用 `clip` 限制单次更新幅度不超过 $\epsilon$（如 20%）。*

**Critic 的更新公式（Value Loss）：**

$$Loss_{Critic} = \mathbb{E}_t \left[ (V_\omega(s_t) - V_t^{target})^2 \right]$$

*均方误差（MSE），让 Critic 预测值越来越接近真实局势走向。*

> 在此阶段，同一批数据会被打乱，用总 Loss 循环更新多个 Epoch，这正是 **PPO 高效利用数据**的体现。

---

### 💡 核心总结速查表

| 步骤 | 动作环节 | 参与模型 | 状态 | 核心产出 / 使用的公式 |
|:----:|:---------|:---------|:----:|:----------------------|
| **1** | **生成（Rollout）** | Actor（$\pi_\theta$） | 🟢 训练中 | 产出：文本序列 $y$ 和动作概率 |
| **2** | **评分（Scoring）** | Reward（$r_\phi$）<br>Reference（$\pi_{ref}$） | 🔴 冻结<br>🔴 冻结 | 产出：**奖励 $R_t$**<br>*（InstructGPT 业务公式：RM 得分 - KL 惩罚）* |
| **3** | **算优势（GAE）** | Critic（$V_\omega$） | 🟢 训练中 | 产出：**优势 $\hat{A}_t$** 和 **目标回报 $V_t^{target}$**<br>*（GAE 时序差分公式）* |
| **4** | **更新（Update）** | Actor（$\pi_\theta$）<br>Critic（$V_\omega$） | 🟢 被更新<br>🟢 被更新 | 执行梯度下降：**最小化联合 Loss**<br>*（PPO $L^{CLIP}$ 公式 + 均方误差 $L^{VF}$）* |

---

## 五、PPO 优缺点

| 维度 | 优点 ✅ | 缺点 ❌ |
|------|--------|--------|
| **验证程度** | 经过大规模验证：ChatGPT、InstructGPT 背后的核心算法 | — |
| **奖励设计** | 奖励信号灵活，可组合安全性、有用性等多维度奖励 | 受限于 RM 质量：奖励模型上限直接决定策略上限 |
| **学习方式** | 在线学习：允许模型探索新输出并获得即时反馈 | 训练不稳定：对初始化、学习率、批大小等超参数敏感 |
| **生态成熟度** | 开源实现完善（TRL、NeMo、veRL） | 实现细节繁多：奖励归一化、值裁剪、全局梯度裁剪等均影响效果 |
| **计算成本** | — | 计算成本极高：70B 模型需维护约 280B 参数量的模型集群 |
| **调参难度** | — | 调参困难：超参数众多（$\varepsilon$、$\beta$、$\lambda$、$\gamma$、$c_1$、$c_2$ 等） |
