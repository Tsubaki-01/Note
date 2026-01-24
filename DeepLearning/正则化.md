你想了解 L1 正则化、L2 正则化以及 weight decay（权重衰减）这三种正则化方法的含义、区别和用法，我会从新手易懂的角度，由浅入深地为你讲解。

### 先理解核心：正则化是做什么的？
在机器学习/深度学习中，模型容易出现**过拟合**（在训练集上表现极好，在新数据上表现很差）。正则化的核心目的就是**给模型的复杂度“加限制”**，通过惩罚过大的权重参数，让模型变得更简单、更泛化，从而缓解过拟合。

可以把它类比成：考试时不让你背太多“偏难怪”的知识点（复杂权重），只让你掌握核心公式（简单权重），这样你面对新题目时反而能做对。

---

## 1. L1 正则化（L1 Regularization）
### 定义
L1 正则化是在损失函数中，加入**模型权重参数的绝对值之和**作为惩罚项。
数学公式（以线性回归为例）：
$$\text{Loss} = \text{原始损失} + \lambda \sum_{i=1}^n |w_i|$$
- $w_i$：模型的第 $i$ 个权重参数
- $\lambda$（lambda）：正则化系数，控制惩罚力度（$\lambda$ 越大，惩罚越重）

### 核心特点
- **产生稀疏权重**：L1 正则化会让模型中**不重要的特征对应的权重变为 0**，相当于自动做了“特征选择”。
  比如模型有 100 个特征，L1 可能会让其中 80 个特征的权重为 0，只保留 20 个关键特征。
- **几何解释**：L1 的惩罚项在二维空间中是“菱形”，与损失函数的等高线相交时，更容易落在坐标轴上（权重为 0）。

### 代码示例（PyTorch 实现 L1 正则化）
```python
import torch
import torch.nn as nn
import torch.optim as optim

# 定义一个简单的线性模型
class SimpleModel(nn.Module):
    def __init__(self):
        super().__init__()
        self.linear = nn.Linear(10, 1)  # 10个输入特征，1个输出

    def forward(self, x):
        return self.linear(x)

model = SimpleModel()
criterion = nn.MSELoss()  # 原始损失（均方误差）
optimizer = optim.SGD(model.parameters(), lr=0.01)

lambda_l1 = 0.01  # L1正则化系数

# 训练过程中加入L1正则化
for epoch in range(100):
    # 模拟输入和标签
    x = torch.randn(32, 10)
    y = torch.randn(32, 1)
    
    # 前向传播
    y_pred = model(x)
    loss = criterion(y_pred, y)
    
    # 计算L1惩罚项：所有权重的绝对值之和
    l1_penalty = torch.tensor(0., requires_grad=True)
    for param in model.parameters():
        l1_penalty = l1_penalty + torch.sum(torch.abs(param))
    
    # 总损失 = 原始损失 + λ*L1惩罚项
    total_loss = loss + lambda_l1 * l1_penalty
    
    # 反向传播
    optimizer.zero_grad()
    total_loss.backward()
    optimizer.step()
```

---

## 2. L2 正则化（L2 Regularization）
### 定义
L2 正则化是在损失函数中，加入**模型权重参数的平方和**作为惩罚项（也叫“岭回归”）。
数学公式（以线性回归为例）：
$$\text{Loss} = \text{原始损失} + \lambda \sum_{i=1}^n w_i^2$$

### 核心特点
- **权重衰减（核心）**：L2 不会让权重变为 0，而是让所有权重都尽可能小（趋近于 0），但不会等于 0。
  比如原本权重是 10，L2 会把它压到 1 左右，避免单个权重过大主导模型。
- **几何解释**：L2 的惩罚项在二维空间中是“圆形”，与损失函数等高线相交时，权重会被压缩，但不会落在坐标轴上。
- **更常用**：在深度学习中，L2 正则化比 L1 更常用，因为它的梯度更平滑，训练更稳定。

### 代码示例（PyTorch 实现 L2 正则化）
```python
import torch
import torch.nn as nn
import torch.optim as optim

model = SimpleModel()  # 复用上面定义的SimpleModel
criterion = nn.MSELoss()
lambda_l2 = 0.01  # L2正则化系数

# 训练过程中加入L2正则化
for epoch in range(100):
    x = torch.randn(32, 10)
    y = torch.randn(32, 1)
    
    y_pred = model(x)
    loss = criterion(y_pred, y)
    
    # 计算L2惩罚项：所有权重的平方和
    l2_penalty = torch.tensor(0., requires_grad=True)
    for param in model.parameters():
        l2_penalty = l2_penalty + torch.sum(torch.square(param))
    
    # 总损失 = 原始损失 + λ*L2惩罚项
    total_loss = loss + lambda_l2 * l2_penalty
    
    optimizer.zero_grad()
    total_loss.backward()
    optimizer.step()
```

---

## 3. Weight Decay（权重衰减）
### 定义
Weight decay 是**实现 L2 正则化的一种工程手段**，本质上和 L2 正则化是等价的（在大多数优化器下），只是实现方式不同：
- L2 正则化：修改**损失函数**，在原始损失上加惩罚项；
- Weight decay：修改**优化器**，在参数更新时直接让权重乘以一个小于 1 的系数（衰减）。

### 为什么等价？
以 SGD 优化器为例：
- 普通 SGD 更新公式：$w = w - \eta \cdot \nabla L$（$\eta$ 是学习率，$\nabla L$ 是损失对权重的梯度）；
- 带 weight decay 的 SGD 更新公式：$w = w(1 - \eta \cdot \alpha) - \eta \cdot \nabla L$（$\alpha$ 是 weight decay 系数）；
- 推导后会发现，这和在损失函数中加 L2 惩罚项的更新结果完全一致。

### 代码示例（PyTorch 用 weight decay 实现 L2 正则化）
这是深度学习中最常用的方式（比手动加 L2 惩罚项更简洁）：
```python
import torch
import torch.nn as nn
import torch.optim as optim

model = SimpleModel()
criterion = nn.MSELoss()

# 直接在优化器中设置weight_decay参数（等价于L2正则化）
optimizer = optim.SGD(model.parameters(), lr=0.01, weight_decay=0.01)

# 训练过程无需手动加惩罚项，优化器会自动处理
for epoch in range(100):
    x = torch.randn(32, 10)
    y = torch.randn(32, 1)
    
    y_pred = model(x)
    loss = criterion(y_pred, y)  # 只用原始损失即可
    
    optimizer.zero_grad()
    loss.backward()
    optimizer.step()  # 优化器会自动对权重做衰减
```

---

### 关键区别对比
| 特性                | L1 正则化                | L2 正则化（Weight Decay） |
|---------------------|--------------------------|---------------------------|
| 惩罚项形式          | 权重绝对值之和           | 权重平方和                |
| 权重结果            | 产生稀疏权重（部分为0）  | 权重都变小（趋近于0）     |
| 梯度特性            | 梯度不连续（可能震荡）   | 梯度平滑（训练稳定）      |
| 适用场景            | 特征选择、稀疏模型       | 通用正则化、深度学习主流  |
| 实现方式            | 手动修改损失函数         | 优化器中设置weight_decay  |

### 总结
1. **L1 正则化**：通过惩罚权重绝对值，让不重要的特征权重变为 0，适合需要特征选择的场景；
2. **L2 正则化**：通过惩罚权重平方，让所有权重变小，避免过拟合，训练更稳定；
3. **Weight decay**：是 L2 正则化的工程实现方式，在优化器中设置即可，是深度学习中最常用的正则化手段。

核心记住：L1 做“特征选择”，L2/weight decay 做“权重压缩”，两者都是为了缓解过拟合，其中 weight decay（L2）是实际项目中最常使用的。