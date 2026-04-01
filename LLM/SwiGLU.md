# SwiGLU 笔记

## 基本定义

SwiGLU 是结合 **Swish/SiLU 激活** 与 **GLU（门控线性单元）** 的激活结构，被广泛用于 LLaMA、PaLM 等大模型的 FFN 层，替代传统 ReLU、GeLU 等单激活函数。

## 核心公式

### 1. SiLU / Swish

大模型中默认 Swish 即 SiLU：

$$\text{SiLU}(x) = x \cdot \sigma(x)$$

其中 $$\sigma(x)$$ 为 Sigmoid 函数。

### 2. 标准 SwiGLU

$$\text{SwiGLU}(x) = \text{SiLU}(x W_1) \odot (x W_2)$$

- $$\odot$$：哈达玛积，逐元素相乘
- 两路并行计算后相乘，实现门控信息流

## 参数含义

- $$x$$：网络输入特征
- $$W_1, W_2$$：**两组独立可学习线性权重矩阵**
  - 分别对输入做线性投影
  - 一路经 SiLU 提取特征，一路作为门控系数

## 大模型 FFN 中实际结构

$$\text{FFN}(x) = \left( \text{SiLU}(x W_{\text{gate}}) \odot x W_{\text{up}} \right) W_{\text{down}}$$

1. $$W_{\text{gate}}, W_{\text{up}}$$ 升维投影到隐层维度
2. 门控相乘增强特征表达
3. $$W_{\text{down}}$$ 降维投影回模型原始维度

## 核心优势

1. 门控机制自适应控制信息流通，表达能力更强
2. 参数利用效率更高，效果优于传统单激活 FFN
3. 平滑激活函数，梯度更稳定，利于大模型训练