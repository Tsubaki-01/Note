# PyTorch 常用内容完全参考手册

> **PyTorch** 是 Facebook 开源的深度学习框架，以动态计算图、Pythonic 风格和强大的 GPU 加速能力著称。本文档系统整理了 `torch` 库在日常开发中最常用的模块、函数和设计模式，适合作为速查手册或学习指南。

---

## 目录

1. [安装与环境配置](#1-安装与环境配置)
2. [张量 (Tensor) 基础](#2-张量-tensor-基础)
3. [张量操作与运算](#3-张量操作与运算)
4. [索引、切片与变形](#4-索引切片与变形)
5. [自动求导 (Autograd)](#5-自动求导-autograd)
6. [神经网络模块 (torch.nn)](#6-神经网络模块-torchnn)
7. [常用层详解](#7-常用层详解)
8. [损失函数](#8-损失函数)
9. [优化器 (torch.optim)](#9-优化器-torchoptim)
10. [学习率调度器](#10-学习率调度器)
11. [数据加载与处理 (torch.utils.data)](#11-数据加载与处理-torchutilsdata)
12. [模型的保存与加载](#12-模型的保存与加载)
13. [GPU 加速与设备管理](#13-gpu-加速与设备管理)
14. [模型训练完整流程](#14-模型训练完整流程)
15. [模型评估与推理](#15-模型评估与推理)
16. [正则化与防止过拟合](#16-正则化与防止过拟合)
17. [权重初始化](#17-权重初始化)
18. [分布式训练 (torch.distributed)](#18-分布式训练-torchdistributed)
19. [混合精度训练 (AMP)](#19-混合精度训练-amp)
20. [TorchScript 与模型导出](#20-torchscript-与模型导出)
21. [常用工具函数](#21-常用工具函数)
22. [torchvision 常用功能](#22-torchvision-常用功能)
23. [调试与性能优化技巧](#23-调试与性能优化技巧)
24. [常见错误与排查](#24-常见错误与排查)

---

## 1. 安装与环境配置

### 使用 pip 安装

```bash
# CPU 版本
pip install torch torchvision torchaudio

# CUDA 11.8 版本（根据实际 CUDA 版本选择）
pip install torch torchvision torchaudio --index-url https://download.pytorch.org/whl/cu118

# CUDA 12.1 版本
pip install torch torchvision torchaudio --index-url https://download.pytorch.org/whl/cu121
```

### 使用 conda 安装

```bash
# CPU 版本
conda install pytorch torchvision torchaudio cpuonly -c pytorch

# CUDA 版本
conda install pytorch torchvision torchaudio pytorch-cuda=12.1 -c pytorch -c nvidia
```

### 验证安装

```python
import torch

print(torch.__version__)        # 查看版本号
print(torch.cuda.is_available())  # 是否支持 CUDA
print(torch.cuda.device_count())  # GPU 数量
print(torch.cuda.get_device_name(0))  # GPU 名称
print(torch.backends.cudnn.version())  # cuDNN 版本
```

---

## 2. 张量 (Tensor) 基础

张量是 PyTorch 中最核心的数据结构，可以理解为多维数组，类似于 NumPy 的 `ndarray`，但支持 GPU 加速和自动求导。

### 2.1 创建张量

```python
import torch

# ── 从 Python 数据创建 ──
a = torch.tensor([1, 2, 3])                  # 从列表创建
b = torch.tensor([[1, 2], [3, 4]])            # 二维张量
c = torch.tensor([1.0, 2.0], dtype=torch.float32)  # 指定数据类型

# ── 特殊张量 ──
z = torch.zeros(3, 4)         # 全零张量, shape = (3, 4)
o = torch.ones(2, 3)          # 全一张量, shape = (2, 3)
e = torch.eye(3)              # 单位矩阵, shape = (3, 3)
f = torch.full((2, 3), 7)     # 填充指定值, 全部为 7
emp = torch.empty(2, 3)       # 未初始化张量（值随机）

# ── 序列张量 ──
r = torch.arange(0, 10, 2)    # [0, 2, 4, 6, 8]
l = torch.linspace(0, 1, 5)   # [0.00, 0.25, 0.50, 0.75, 1.00]
lg = torch.logspace(0, 2, 3)  # [1, 10, 100] — 对数等间距

# ── 随机张量 ──
rn = torch.rand(3, 4)         # 均匀分布 [0, 1)
rn2 = torch.randn(3, 4)       # 标准正态分布
ri = torch.randint(0, 10, (3, 4))  # 随机整数 [0, 10)
rp = torch.randperm(10)       # 随机排列 [0, 10)

# ── 类似已有张量创建 ──
x = torch.randn(3, 4)
x_like = torch.zeros_like(x)     # 与 x 同 shape、dtype 的全零张量
x_like2 = torch.ones_like(x)     # 与 x 同 shape、dtype 的全一张量
x_like3 = torch.randn_like(x)    # 与 x 同 shape、dtype 的随机张量

# ── 从 NumPy 创建 ──
import numpy as np
arr = np.array([1, 2, 3])
t = torch.from_numpy(arr)        # 共享内存！修改一个会影响另一个
t2 = torch.tensor(arr)           # 拷贝数据，不共享内存
```

### 2.2 数据类型 (dtype)

| dtype | 说明 | 别名 |
|-------|------|------|
| `torch.float32` | 32 位浮点（**默认**） | `torch.float` |
| `torch.float64` | 64 位浮点 | `torch.double` |
| `torch.float16` | 16 位浮点 | `torch.half` |
| `torch.bfloat16` | Brain 浮点 16 | — |
| `torch.int32` | 32 位整数 | `torch.int` |
| `torch.int64` | 64 位整数 | `torch.long` |
| `torch.int16` | 16 位整数 | `torch.short` |
| `torch.int8` | 8 位整数 | — |
| `torch.uint8` | 无符号 8 位整数 | — |
| `torch.bool` | 布尔 | — |
| `torch.complex64` | 复数（float32 实/虚） | — |

```python
# 类型转换
x = torch.tensor([1, 2, 3])
x_float = x.float()       # 转为 float32
x_double = x.double()     # 转为 float64
x_long = x.long()         # 转为 int64
x_half = x.half()         # 转为 float16
x_bool = x.bool()         # 转为 bool
x_to = x.to(torch.float16)  # 通用转换方法
```

### 2.3 张量属性

```python
x = torch.randn(2, 3, 4)

x.shape       # torch.Size([2, 3, 4]) — 形状
x.size()      # torch.Size([2, 3, 4]) — 同 shape
x.ndim        # 3 — 维度数
x.numel()     # 24 — 元素总数
x.dtype       # torch.float32 — 数据类型
x.device      # device(type='cpu') — 所在设备
x.requires_grad  # False — 是否需要梯度
x.is_cuda     # False — 是否在 GPU 上
x.is_contiguous()  # True — 内存是否连续
```

---

## 3. 张量操作与运算

### 3.1 基本数学运算

```python
a = torch.tensor([1.0, 2.0, 3.0])
b = torch.tensor([4.0, 5.0, 6.0])

# ── 逐元素运算 ──
a + b          # tensor([5., 7., 9.])
torch.add(a, b)  # 等价写法
a - b          # tensor([-3., -3., -3.])
a * b          # tensor([4., 10., 18.]) — 逐元素乘（Hadamard 积）
a / b          # tensor([0.25, 0.40, 0.50])
a ** 2         # tensor([1., 4., 9.]) — 幂运算
a % 2          # 取模
a // 2         # 整除

# ── 原地操作（以 _ 结尾，不创建新张量） ──
a.add_(b)      # a = a + b，直接修改 a
a.mul_(2)      # a = a * 2
a.zero_()      # 全部置零
a.fill_(5)     # 全部填充 5

# ── 常用数学函数 ──
torch.abs(a)        # 绝对值
torch.sqrt(a)       # 平方根
torch.exp(a)        # e 的幂
torch.log(a)        # 自然对数
torch.log2(a)       # 以 2 为底的对数
torch.log10(a)      # 以 10 为底的对数
torch.ceil(a)       # 向上取整
torch.floor(a)      # 向下取整
torch.round(a)      # 四舍五入
torch.clamp(a, min=0, max=5)  # 裁剪到 [0, 5]
torch.sign(a)       # 符号函数 (-1, 0, 1)
torch.reciprocal(a) # 倒数 1/a

# ── 三角函数 ──
torch.sin(a)
torch.cos(a)
torch.tan(a)
torch.asin(a)
torch.acos(a)
torch.atan(a)
torch.atan2(a, b)   # 二元反正切
```

### 3.2 统计运算

```python
x = torch.randn(3, 4)

# ── 全局统计 ──
x.sum()          # 所有元素之和
x.mean()         # 均值
x.std()          # 标准差
x.var()          # 方差
x.median()       # 中位数
x.min()          # 最小值
x.max()          # 最大值
x.prod()         # 所有元素之积
x.norm()         # L2 范数

# ── 按维度统计 ──
x.sum(dim=0)         # 按列求和, shape = (4,)
x.sum(dim=1)         # 按行求和, shape = (3,)
x.mean(dim=0)        # 按列均值
x.max(dim=1)         # 返回 (values, indices)
x.min(dim=0)         # 返回 (values, indices)
x.cumsum(dim=0)      # 按列累加和
x.cumprod(dim=1)     # 按行累乘

# ── 保持维度 ──
x.sum(dim=1, keepdim=True)   # shape = (3, 1) 保持维度

# ── 排序与取值 ──
x.argmax()           # 最大值的全局索引
x.argmax(dim=1)      # 每行最大值的列索引
x.argmin(dim=0)      # 每列最小值的行索引
x.argsort(dim=1)     # 按行排序的索引
torch.topk(x, k=2, dim=1)  # 每行前 2 大的 (values, indices)
torch.kthvalue(x, k=2, dim=1)  # 每行第 2 小的值

# ── 唯一值 ──
torch.unique(x)      # 去重
```

### 3.3 矩阵与线性代数

```python
A = torch.randn(3, 4)
B = torch.randn(4, 5)

# ── 矩阵乘法 ──
C = A @ B              # 矩阵乘法, shape = (3, 5)
C = torch.mm(A, B)     # 等价写法（仅 2D）
C = torch.matmul(A, B) # 通用矩阵乘法（支持广播和高维）
C = A.mm(B)            # 方法形式

# ── 批量矩阵乘法 ──
batch_A = torch.randn(10, 3, 4)
batch_B = torch.randn(10, 4, 5)
batch_C = torch.bmm(batch_A, batch_B)  # shape = (10, 3, 5)

# ── 向量运算 ──
v1 = torch.tensor([1.0, 2.0, 3.0])
v2 = torch.tensor([4.0, 5.0, 6.0])
torch.dot(v1, v2)         # 点积 = 32
torch.cross(v1, v2)       # 叉积（仅 3D 向量）
torch.outer(v1, v2)       # 外积, shape = (3, 3)

# ── 转置 ──
A.T                  # 转置（2D）
A.t()                # 等价写法（仅 2D）
A.mT                 # 最后两维转置（支持批量）
A.transpose(0, 1)    # 交换指定维度

# ── 线性代数（torch.linalg） ──
S = torch.randn(3, 3)
S = S @ S.T  # 构造对称正定矩阵

torch.linalg.det(S)          # 行列式
torch.linalg.inv(S)          # 逆矩阵
torch.linalg.eigh(S)         # 特征分解（对称矩阵）
torch.linalg.svd(A)          # 奇异值分解
torch.linalg.norm(A)         # 矩阵范数
torch.linalg.matrix_rank(A)  # 矩阵秩
torch.linalg.solve(S, torch.randn(3, 1))  # 解线性方程组
torch.linalg.cholesky(S)     # Cholesky 分解
torch.linalg.qr(A)           # QR 分解
torch.trace(S)               # 迹（对角线元素之和）
torch.diag(S)                # 提取对角线 / 构造对角矩阵
```

### 3.4 比较运算

```python
a = torch.tensor([1, 2, 3, 4, 5])
b = torch.tensor([3, 3, 3, 3, 3])

a > b        # tensor([False, False, False,  True,  True])
a >= b       # tensor([False, False,  True,  True,  True])
a < b        # tensor([ True,  True, False, False, False])
a == b       # tensor([False, False,  True, False, False])
a != b       # tensor([ True,  True, False,  True,  True])

torch.eq(a, b)       # 逐元素相等
torch.gt(a, b)       # 逐元素大于
torch.lt(a, b)       # 逐元素小于
torch.ge(a, b)       # 大于等于
torch.le(a, b)       # 小于等于

torch.equal(a, a)    # 两个张量是否完全相等（返回 bool）
torch.allclose(a.float(), a.float(), atol=1e-8)  # 近似相等

torch.where(a > 3, a, b)  # 条件选择: 满足条件取 a，否则取 b
torch.isnan(a)       # 检测 NaN
torch.isinf(a)       # 检测 Inf
torch.isfinite(a)    # 检测有限数
```

### 3.5 逻辑运算

```python
x = torch.tensor([True, False, True])
y = torch.tensor([True, True, False])

x & y           # 逻辑与
x | y           # 逻辑或
~x              # 逻辑非
x ^ y           # 逻辑异或

torch.all(x)    # 是否全为 True
torch.any(x)    # 是否存在 True

# 对整数张量的位运算
a = torch.tensor([5, 3])   # 二进制: 101, 011
a & 3           # 按位与
a | 3           # 按位或
a << 1          # 左移
a >> 1          # 右移
```

---

## 4. 索引、切片与变形

### 4.1 索引与切片

```python
x = torch.arange(20).reshape(4, 5)
# tensor([[ 0,  1,  2,  3,  4],
#          [ 5,  6,  7,  8,  9],
#          [10, 11, 12, 13, 14],
#          [15, 16, 17, 18, 19]])

# ── 基本索引 ──
x[0]           # 第 0 行: tensor([0, 1, 2, 3, 4])
x[0, 2]        # 第 0 行第 2 列: tensor(2)
x[-1]          # 最后一行
x[:, 0]        # 第 0 列
x[1:3]         # 第 1、2 行
x[1:3, 2:4]    # 子矩阵
x[::2]         # 每隔一行
x[:, ::2]      # 每隔一列

# ── 高级索引 ──
idx = torch.tensor([0, 2, 3])
x[idx]              # 取第 0、2、3 行
x[:, idx]           # 取第 0、2、3 列
x[torch.tensor([0, 1]), torch.tensor([2, 3])]  # 取 (0,2) 和 (1,3)

# ── 布尔索引 ──
mask = x > 10
x[mask]              # 取所有大于 10 的元素，返回一维张量
x[x % 2 == 0]        # 取所有偶数

# ── 花式赋值 ──
x[0, 0] = 100        # 修改单个元素
x[mask] = 0           # 满足条件的元素置零
x[:, -1] = 99         # 最后一列全部赋值 99
```

### 4.2 形状变换

```python
x = torch.arange(12)

# ── reshape / view ──
x.reshape(3, 4)          # 改变形状为 (3, 4)
x.view(3, 4)             # 同上，要求内存连续
x.reshape(3, -1)         # -1 自动推导: (3, 4)
x.reshape(-1, 2, 3)      # (2, 2, 3)

# ── 展平 ──
y = torch.randn(2, 3, 4)
y.flatten()              # 展平为一维, shape = (24,)
y.flatten(1)             # 从 dim=1 开始展平, shape = (2, 12)
y.flatten(1, 2)          # 展平 dim 1-2, shape = (2, 12)
torch.ravel(y)           # 等价于 flatten()

# ── 增加/移除维度 ──
x = torch.randn(3, 4)
x.unsqueeze(0)           # shape: (3,4) → (1,3,4) 在 dim=0 增加维度
x.unsqueeze(1)           # shape: (3,4) → (3,1,4)
x.unsqueeze(-1)          # shape: (3,4) → (3,4,1)

y = torch.randn(1, 3, 1, 4)
y.squeeze()              # 移除所有大小为 1 的维度: (3, 4)
y.squeeze(0)             # 只移除 dim=0: (3, 1, 4)

# ── 维度交换 ──
x = torch.randn(2, 3, 4)
x.permute(2, 0, 1)       # shape: (4, 2, 3) — 任意维度重排
x.transpose(0, 2)        # shape: (4, 3, 2) — 交换两个维度
x.movedim(0, -1)         # 将 dim=0 移到最后

# ── 连续化 ──
x = torch.randn(3, 4).T   # 转置后内存不连续
x.contiguous()            # 强制连续（view 前可能需要）
```

### 4.3 拼接与拆分

```python
a = torch.randn(2, 3)
b = torch.randn(2, 3)

# ── 拼接 ──
torch.cat([a, b], dim=0)    # shape: (4, 3) — 沿 dim=0 拼接
torch.cat([a, b], dim=1)    # shape: (2, 6) — 沿 dim=1 拼接

torch.stack([a, b], dim=0)  # shape: (2, 2, 3) — 沿新维度堆叠
torch.stack([a, b], dim=1)  # shape: (2, 2, 3)

torch.vstack([a, b])        # 垂直堆叠 = cat(dim=0)
torch.hstack([a, b])        # 水平堆叠 = cat(dim=1)

# ── 拆分 ──
x = torch.randn(10, 4)
torch.chunk(x, 3, dim=0)       # 尽量均分为 3 份
torch.split(x, 3, dim=0)       # 每份 3 行 (3,3,3,1)
torch.split(x, [2, 3, 5], dim=0)  # 按指定大小拆分

# ── 重复 ──
x = torch.tensor([1, 2, 3])
x.repeat(3)             # tensor([1, 2, 3, 1, 2, 3, 1, 2, 3])
x.repeat(2, 3)          # shape: (2, 9)
x.unsqueeze(0).expand(4, -1)  # 不分配新内存的广播扩展: (4, 3)
torch.tile(x, (2, 3))   # 类似 NumPy 的 tile
```

### 4.4 广播机制 (Broadcasting)

```python
# 广播规则：从后向前对齐维度
# 1. 维度大小相同 → 正常运算
# 2. 某维大小为 1 → 沿该维扩展
# 3. 维度数不同 → 在前面补 1

a = torch.randn(3, 4)    # (3, 4)
b = torch.randn(4)       # (4,) → 广播为 (3, 4)
c = a + b                # 合法: (3, 4) + (1, 4) → (3, 4)

a = torch.randn(3, 1)    # (3, 1)
b = torch.randn(1, 4)    # (1, 4)
c = a + b                # 合法: (3, 4)

# 不合法示例:
# a = torch.randn(3, 4)
# b = torch.randn(3)
# a + b → 报错！(3,4) 与 (3,) 最后一维不匹配
```

---

## 5. 自动求导 (Autograd)

PyTorch 的自动求导引擎可以自动计算任意计算图的梯度，是训练神经网络的基础。

### 5.1 基本用法

```python
# ── 创建需要梯度的张量 ──
x = torch.tensor(3.0, requires_grad=True)
y = x ** 2 + 2 * x + 1   # y = x² + 2x + 1

# ── 反向传播 ──
y.backward()    # 计算 dy/dx
print(x.grad)   # tensor(8.) — dy/dx = 2x + 2 = 8

# ── 多变量 ──
a = torch.tensor(2.0, requires_grad=True)
b = torch.tensor(3.0, requires_grad=True)
z = a ** 2 * b + b ** 3
z.backward()
print(a.grad)  # dz/da = 2ab = 12
print(b.grad)  # dz/db = a² + 3b² = 31
```

### 5.2 梯度管理

```python
x = torch.randn(3, requires_grad=True)
y = (x * 2).sum()

# ── 计算梯度 ──
y.backward()
print(x.grad)  # 梯度值

# ── 梯度累积（默认累加，需手动清零） ──
x.grad.zero_()   # 清零梯度

# ── 禁止梯度追踪 ──
with torch.no_grad():
    # 这个块内的运算不会被追踪
    z = x * 2  # z.requires_grad = False

# ── 分离张量 ──
z = x.detach()       # 返回不追踪梯度的新张量（共享数据）
z = x.detach().clone()  # 完全独立的副本

# ── 修改 requires_grad ──
x.requires_grad_(False)   # 原地修改
x.requires_grad_(True)

# ── 梯度裁剪（训练时防止梯度爆炸） ──
torch.nn.utils.clip_grad_norm_(model.parameters(), max_norm=1.0)   # 按范数裁剪
torch.nn.utils.clip_grad_value_(model.parameters(), clip_value=0.5)  # 按值裁剪

# ── 保留计算图 ──
y.backward(retain_graph=True)  # 可以多次 backward

# ── 检查梯度（调试用） ──
torch.autograd.gradcheck(func, inputs)    # 数值梯度检查
torch.autograd.gradgradcheck(func, inputs)  # 二阶梯度检查
```

### 5.3 计算图与高阶梯度

```python
x = torch.tensor(2.0, requires_grad=True)
y = x ** 3  # y = x³

# 一阶导数
dy_dx = torch.autograd.grad(y, x, create_graph=True)[0]
print(dy_dx)  # 12.0 (3x² = 12)

# 二阶导数
d2y_dx2 = torch.autograd.grad(dy_dx, x, create_graph=True)[0]
print(d2y_dx2)  # 12.0 (6x = 12)

# 三阶导数
d3y_dx3 = torch.autograd.grad(d2y_dx2, x)[0]
print(d3y_dx3)  # 6.0
```

---

## 6. 神经网络模块 (torch.nn)

`torch.nn` 提供了构建神经网络所需的所有组件。

### 6.1 定义模型

```python
import torch.nn as nn

# ── 方式一：继承 nn.Module（最灵活） ──
class MyModel(nn.Module):
    def __init__(self, input_dim, hidden_dim, output_dim):
        super().__init__()
        self.fc1 = nn.Linear(input_dim, hidden_dim)
        self.relu = nn.ReLU()
        self.dropout = nn.Dropout(0.5)
        self.fc2 = nn.Linear(hidden_dim, output_dim)

    def forward(self, x):
        x = self.fc1(x)
        x = self.relu(x)
        x = self.dropout(x)
        x = self.fc2(x)
        return x

model = MyModel(784, 256, 10)

# ── 方式二：nn.Sequential（简单堆叠） ──
model = nn.Sequential(
    nn.Linear(784, 256),
    nn.ReLU(),
    nn.Dropout(0.5),
    nn.Linear(256, 128),
    nn.ReLU(),
    nn.Linear(128, 10),
)

# ── 方式三：nn.Sequential + OrderedDict（带命名） ──
from collections import OrderedDict

model = nn.Sequential(OrderedDict([
    ('fc1', nn.Linear(784, 256)),
    ('relu1', nn.ReLU()),
    ('fc2', nn.Linear(256, 10)),
]))

# ── 方式四：nn.ModuleList（动态层） ──
class DynamicModel(nn.Module):
    def __init__(self, layer_sizes):
        super().__init__()
        self.layers = nn.ModuleList([
            nn.Linear(layer_sizes[i], layer_sizes[i+1])
            for i in range(len(layer_sizes) - 1)
        ])

    def forward(self, x):
        for layer in self.layers:
            x = torch.relu(layer(x))
        return x

# ── 方式五：nn.ModuleDict（按名称选层） ──
class MultiHeadModel(nn.Module):
    def __init__(self):
        super().__init__()
        self.backbone = nn.Linear(784, 256)
        self.heads = nn.ModuleDict({
            'classifier': nn.Linear(256, 10),
            'regressor': nn.Linear(256, 1),
        })

    def forward(self, x, task='classifier'):
        x = torch.relu(self.backbone(x))
        return self.heads[task](x)
```

### 6.2 模型信息

```python
model = MyModel(784, 256, 10)

# ── 查看结构 ──
print(model)

# ── 参数统计 ──
total = sum(p.numel() for p in model.parameters())
trainable = sum(p.numel() for p in model.parameters() if p.requires_grad)
print(f"总参数: {total:,}  可训练: {trainable:,}")

# ── 遍历参数 ──
for name, param in model.named_parameters():
    print(f"{name}: shape={param.shape}, requires_grad={param.requires_grad}")

# ── 遍历子模块 ──
for name, module in model.named_modules():
    print(name, type(module))

# ── 冻结参数 ──
for param in model.fc1.parameters():
    param.requires_grad = False

# ── 解冻参数 ──
for param in model.fc1.parameters():
    param.requires_grad = True
```

---

## 7. 常用层详解

### 7.1 线性层

```python
# nn.Linear(in_features, out_features, bias=True)
# 执行 y = xW^T + b
fc = nn.Linear(128, 64)           # 输入 128 维，输出 64 维
fc_no_bias = nn.Linear(128, 64, bias=False)

x = torch.randn(32, 128)          # batch=32
output = fc(x)                    # shape: (32, 64)
```

### 7.2 卷积层

```python
# ── 一维卷积（时序/信号） ──
# nn.Conv1d(in_channels, out_channels, kernel_size, stride=1, padding=0)
conv1d = nn.Conv1d(3, 16, kernel_size=3, stride=1, padding=1)
# 输入: (batch, channels, length) → 输出: (batch, 16, length)

# ── 二维卷积（图像） ──
# nn.Conv2d(in_channels, out_channels, kernel_size, stride=1, padding=0)
conv2d = nn.Conv2d(3, 64, kernel_size=3, stride=1, padding=1)
# 输入: (batch, 3, H, W) → 输出: (batch, 64, H, W)

conv2d_k5 = nn.Conv2d(64, 128, kernel_size=5, stride=2, padding=2)
# 输出尺寸: floor((H + 2*padding - kernel_size) / stride + 1)

# ── 分组卷积 / 深度可分离卷积 ──
dw_conv = nn.Conv2d(64, 64, kernel_size=3, padding=1, groups=64)  # Depthwise
pw_conv = nn.Conv2d(64, 128, kernel_size=1)                        # Pointwise

# ── 空洞卷积 ──
dilated = nn.Conv2d(64, 64, kernel_size=3, padding=2, dilation=2)

# ── 转置卷积（上采样） ──
deconv = nn.ConvTranspose2d(64, 32, kernel_size=4, stride=2, padding=1)
# 输入 (batch, 64, H, W) → 输出 (batch, 32, 2H, 2W)

# ── 三维卷积（视频/体积数据） ──
conv3d = nn.Conv3d(3, 64, kernel_size=3, stride=1, padding=1)
```

### 7.3 池化层

```python
# ── 最大池化 ──
maxpool = nn.MaxPool2d(kernel_size=2, stride=2)
# (batch, C, H, W) → (batch, C, H/2, W/2)

maxpool_ret = nn.MaxPool2d(2, return_indices=True)  # 同时返回索引（用于反池化）

# ── 平均池化 ──
avgpool = nn.AvgPool2d(kernel_size=2, stride=2)

# ── 全局平均池化（常用于分类网络末端） ──
gap = nn.AdaptiveAvgPool2d(1)  # 输出固定为 (batch, C, 1, 1)
gap = nn.AdaptiveAvgPool2d((7, 7))  # 输出固定为 (batch, C, 7, 7)

# ── 全局最大池化 ──
gmp = nn.AdaptiveMaxPool2d(1)

# ── 1D 池化 ──
pool1d = nn.MaxPool1d(kernel_size=2)
pool1d_avg = nn.AvgPool1d(kernel_size=2)
pool1d_adaptive = nn.AdaptiveAvgPool1d(1)
```

### 7.4 归一化层

```python
# ── Batch Normalization ──
# 训练时用 batch 统计量，推理时用滑动平均
bn1d = nn.BatchNorm1d(256)         # 用于全连接层后, 输入 (batch, 256)
bn2d = nn.BatchNorm2d(64)          # 用于卷积层后, 输入 (batch, 64, H, W)

# ── Layer Normalization ──
# 对最后几个维度做归一化，不依赖 batch 大小，常用于 Transformer
ln = nn.LayerNorm(256)             # 输入 (..., 256)
ln2d = nn.LayerNorm([64, 32, 32])  # 输入 (batch, 64, 32, 32)

# ── Group Normalization ──
# 对 channels 分组归一化, 不依赖 batch
gn = nn.GroupNorm(num_groups=8, num_channels=64)

# ── Instance Normalization ──
# 对每个样本的每个通道独立归一化（常用于风格迁移）
inst_norm = nn.InstanceNorm2d(64)

# ── RMS Normalization（需 PyTorch ≥ 2.4） ──
# rmsnorm = nn.RMSNorm(256)
```

### 7.5 激活函数

```python
# ── 常用激活函数 ──
relu = nn.ReLU(inplace=True)       # max(0, x) — 最常用
leaky_relu = nn.LeakyReLU(0.01)    # x if x > 0 else 0.01x
prelu = nn.PReLU()                 # 可学习的斜率
elu = nn.ELU(alpha=1.0)            # x if x > 0 else α(eˣ - 1)
selu = nn.SELU()                   # 自归一化 ELU
gelu = nn.GELU()                   # Transformer 中常用
silu = nn.SiLU()                   # x * σ(x), 也叫 Swish
mish = nn.Mish()                   # x * tanh(softplus(x))

sigmoid = nn.Sigmoid()             # 1 / (1 + e⁻ˣ) — 二分类输出
tanh = nn.Tanh()                   # (eˣ - e⁻ˣ) / (eˣ + e⁻ˣ)
softmax = nn.Softmax(dim=-1)       # 多分类输出
log_softmax = nn.LogSoftmax(dim=-1)
softplus = nn.Softplus()           # log(1 + eˣ)
hardswish = nn.Hardswish()         # 高效近似 Swish
hardsigmoid = nn.Hardsigmoid()     # 高效近似 Sigmoid

# ── 函数式调用 ──
import torch.nn.functional as F
output = F.relu(x)
output = F.gelu(x)
output = F.softmax(x, dim=-1)
output = F.sigmoid(x)
```

### 7.6 Dropout

```python
dropout = nn.Dropout(p=0.5)           # 随机丢弃 50% 的神经元
dropout2d = nn.Dropout2d(p=0.25)      # 随机丢弃整个通道（卷积用）
dropout3d = nn.Dropout3d(p=0.25)      # 3D 版本
alpha_dropout = nn.AlphaDropout(p=0.5)  # 用于 SELU 的 dropout

# 注意: Dropout 在 model.eval() 时自动关闭
```

### 7.7 嵌入层

```python
# nn.Embedding(num_embeddings, embedding_dim)
# 将整数索引映射为稠密向量
embedding = nn.Embedding(num_embeddings=10000, embedding_dim=256)
# 输入: (batch, seq_len) 的 LongTensor, 值在 [0, 9999]
# 输出: (batch, seq_len, 256)

input_ids = torch.tensor([[1, 2, 4, 5], [4, 3, 2, 9]])  # shape: (2, 4)
output = embedding(input_ids)  # shape: (2, 4, 256)

# ── 预训练嵌入 ──
pretrained_weights = torch.randn(10000, 256)
embedding = nn.Embedding.from_pretrained(pretrained_weights, freeze=False)

# ── 填充索引 ──
embedding = nn.Embedding(10000, 256, padding_idx=0)  # 索引 0 的向量全为零
```

### 7.8 循环神经网络 (RNN / LSTM / GRU)

```python
# ── RNN ──
rnn = nn.RNN(input_size=128, hidden_size=256, num_layers=2,
             batch_first=True, dropout=0.5, bidirectional=True)
# 输入: (batch, seq_len, 128)
# 输出: output (batch, seq_len, 512), h_n (4, batch, 256)

# ── LSTM ──
lstm = nn.LSTM(input_size=128, hidden_size=256, num_layers=2,
               batch_first=True, dropout=0.5, bidirectional=True)
# 输出: output (batch, seq_len, 512), (h_n, c_n) 各 (4, batch, 256)

x = torch.randn(32, 50, 128)       # batch=32, seq=50, features=128
output, (h_n, c_n) = lstm(x)

# 取最后时刻的隐状态
last_hidden = output[:, -1, :]      # shape: (32, 512)

# ── GRU ──
gru = nn.GRU(input_size=128, hidden_size=256, num_layers=2,
             batch_first=True, dropout=0.5)
output, h_n = gru(x)

# ── LSTM Cell（手动控制时间步） ──
lstm_cell = nn.LSTMCell(input_size=128, hidden_size=256)
h, c = torch.zeros(32, 256), torch.zeros(32, 256)
for t in range(50):
    h, c = lstm_cell(x[:, t, :], (h, c))
```

### 7.9 Transformer

```python
# ── 完整 Transformer ──
transformer = nn.Transformer(
    d_model=512, nhead=8,
    num_encoder_layers=6, num_decoder_layers=6,
    dim_feedforward=2048, dropout=0.1,
    batch_first=True
)
src = torch.randn(32, 10, 512)   # (batch, src_len, d_model)
tgt = torch.randn(32, 20, 512)   # (batch, tgt_len, d_model)
output = transformer(src, tgt)    # (32, 20, 512)

# ── 仅 Encoder ──
encoder_layer = nn.TransformerEncoderLayer(
    d_model=512, nhead=8, dim_feedforward=2048,
    dropout=0.1, batch_first=True, activation='gelu'
)
encoder = nn.TransformerEncoder(encoder_layer, num_layers=6)
output = encoder(src)  # (32, 10, 512)

# ── 仅 Decoder ──
decoder_layer = nn.TransformerDecoderLayer(
    d_model=512, nhead=8, dim_feedforward=2048,
    dropout=0.1, batch_first=True
)
decoder = nn.TransformerDecoder(decoder_layer, num_layers=6)
output = decoder(tgt, memory=encoder(src))

# ── 多头注意力 ──
mha = nn.MultiheadAttention(embed_dim=512, num_heads=8, batch_first=True)
attn_output, attn_weights = mha(query, key, value)

# ── 生成 Mask ──
# 因果 Mask (Causal Mask, 用于 Decoder)
seq_len = 20
causal_mask = nn.Transformer.generate_square_subsequent_mask(seq_len)
# 上三角为 -inf 的方阵

# Padding Mask
# True 表示对应位置会被忽略
key_padding_mask = torch.tensor([
    [False, False, False, True, True],  # 最后两个 token 是 padding
    [False, False, False, False, True],
])

# ── 位置编码（标准实现） ──
class PositionalEncoding(nn.Module):
    def __init__(self, d_model, max_len=5000, dropout=0.1):
        super().__init__()
        self.dropout = nn.Dropout(p=dropout)
        pe = torch.zeros(max_len, d_model)
        position = torch.arange(0, max_len).unsqueeze(1).float()
        div_term = torch.exp(
            torch.arange(0, d_model, 2).float() * (-torch.log(torch.tensor(10000.0)) / d_model)
        )
        pe[:, 0::2] = torch.sin(position * div_term)
        pe[:, 1::2] = torch.cos(position * div_term)
        pe = pe.unsqueeze(0)  # (1, max_len, d_model)
        self.register_buffer('pe', pe)

    def forward(self, x):
        x = x + self.pe[:, :x.size(1)]
        return self.dropout(x)
```

---

## 8. 损失函数

### 8.1 分类损失

```python
# ── 交叉熵损失（多分类，最常用） ──
# 输入: logits (batch, num_classes), 目标: (batch,) 整数标签
criterion = nn.CrossEntropyLoss()
logits = torch.randn(32, 10)          # 未经 softmax
target = torch.randint(0, 10, (32,))  # 类别标签
loss = criterion(logits, target)

# 带权重的交叉熵（处理类别不平衡）
weights = torch.tensor([1.0, 2.0, 1.5, 1.0, 3.0])
criterion = nn.CrossEntropyLoss(weight=weights)

# 忽略某些标签（如 padding）
criterion = nn.CrossEntropyLoss(ignore_index=-100)

# Label Smoothing
criterion = nn.CrossEntropyLoss(label_smoothing=0.1)

# ── 负对数似然损失 ──
# 输入需要先经过 log_softmax
criterion = nn.NLLLoss()
log_probs = F.log_softmax(logits, dim=-1)
loss = criterion(log_probs, target)

# ── 二元交叉熵 ──
# 输入: 概率值 (经过 sigmoid)
criterion = nn.BCELoss()
probs = torch.sigmoid(torch.randn(32, 1))
target = torch.randint(0, 2, (32, 1)).float()
loss = criterion(probs, target)

# 带 logits 的 BCE（数值更稳定，推荐）
criterion = nn.BCEWithLogitsLoss()
logits = torch.randn(32, 1)
loss = criterion(logits, target)

# 多标签分类
criterion = nn.BCEWithLogitsLoss()
logits = torch.randn(32, 5)             # 5 个标签
target = torch.randint(0, 2, (32, 5)).float()
loss = criterion(logits, target)

# ── Focal Loss（自定义，处理极端类别不平衡） ──
class FocalLoss(nn.Module):
    def __init__(self, alpha=1.0, gamma=2.0):
        super().__init__()
        self.alpha = alpha
        self.gamma = gamma

    def forward(self, logits, target):
        ce_loss = F.cross_entropy(logits, target, reduction='none')
        pt = torch.exp(-ce_loss)
        focal_loss = self.alpha * (1 - pt) ** self.gamma * ce_loss
        return focal_loss.mean()
```

### 8.2 回归损失

```python
pred = torch.randn(32, 1)
target = torch.randn(32, 1)

# ── 均方误差 MSE ──
criterion = nn.MSELoss()
loss = criterion(pred, target)

# ── 平均绝对误差 MAE / L1 ──
criterion = nn.L1Loss()
loss = criterion(pred, target)

# ── Smooth L1（Huber 损失） ──
# 小误差用 L2，大误差用 L1，更鲁棒
criterion = nn.SmoothL1Loss(beta=1.0)
loss = criterion(pred, target)

# ── Huber 损失 ──
criterion = nn.HuberLoss(delta=1.0)
loss = criterion(pred, target)
```

### 8.3 其他常用损失

```python
# ── KL 散度 ──
criterion = nn.KLDivLoss(reduction='batchmean')
log_pred = F.log_softmax(torch.randn(32, 10), dim=-1)
target_dist = F.softmax(torch.randn(32, 10), dim=-1)
loss = criterion(log_pred, target_dist)

# ── Cosine Embedding Loss（相似度学习） ──
criterion = nn.CosineEmbeddingLoss()
x1 = torch.randn(32, 128)
x2 = torch.randn(32, 128)
label = torch.ones(32)  # 1 表示相似, -1 表示不相似
loss = criterion(x1, x2, label)

# ── Triplet Margin Loss（度量学习） ──
criterion = nn.TripletMarginLoss(margin=1.0)
anchor = torch.randn(32, 128)
positive = torch.randn(32, 128)
negative = torch.randn(32, 128)
loss = criterion(anchor, positive, negative)

# ── CTC Loss（序列到序列，如语音识别） ──
criterion = nn.CTCLoss()

# ── Margin Ranking Loss ──
criterion = nn.MarginRankingLoss(margin=0.5)
```

---

## 9. 优化器 (torch.optim)

### 9.1 常用优化器

```python
import torch.optim as optim

model = MyModel(784, 256, 10)

# ── SGD ──
optimizer = optim.SGD(model.parameters(), lr=0.01, momentum=0.9, weight_decay=1e-4)

# ── SGD + Nesterov 动量 ──
optimizer = optim.SGD(model.parameters(), lr=0.01, momentum=0.9, nesterov=True)

# ── Adam（最常用） ──
optimizer = optim.Adam(model.parameters(), lr=1e-3, betas=(0.9, 0.999),
                       eps=1e-8, weight_decay=1e-4)

# ── AdamW（解耦权重衰减，推荐） ──
optimizer = optim.AdamW(model.parameters(), lr=1e-3, weight_decay=0.01)

# ── RMSprop ──
optimizer = optim.RMSprop(model.parameters(), lr=0.01, alpha=0.99)

# ── Adagrad ──
optimizer = optim.Adagrad(model.parameters(), lr=0.01)

# ── Adadelta ──
optimizer = optim.Adadelta(model.parameters(), lr=1.0)

# ── RAdam（带热身的 Adam） ──
optimizer = optim.RAdam(model.parameters(), lr=1e-3)

# ── LBFGS（二阶优化器，适合小模型） ──
optimizer = optim.LBFGS(model.parameters(), lr=1)
```

### 9.2 参数组（不同层使用不同学习率）

```python
optimizer = optim.Adam([
    {'params': model.backbone.parameters(), 'lr': 1e-5},   # 预训练层低学习率
    {'params': model.classifier.parameters(), 'lr': 1e-3}, # 新层高学习率
], weight_decay=1e-4)
```

### 9.3 优化器的基本操作

```python
# ── 标准训练步骤 ──
optimizer.zero_grad()       # 1. 清零梯度
loss = criterion(output, target)
loss.backward()             # 2. 反向传播
optimizer.step()            # 3. 更新参数

# ── 更高效的清零方式（PyTorch ≥ 1.7） ──
optimizer.zero_grad(set_to_none=True)

# ── 获取/设置学习率 ──
for pg in optimizer.param_groups:
    print(pg['lr'])            # 查看学习率
    pg['lr'] = 1e-4            # 手动修改

# ── 保存/加载优化器状态 ──
torch.save(optimizer.state_dict(), 'optimizer.pt')
optimizer.load_state_dict(torch.load('optimizer.pt'))
```

---

## 10. 学习率调度器

```python
from torch.optim.lr_scheduler import (
    StepLR, MultiStepLR, ExponentialLR, CosineAnnealingLR,
    ReduceLROnPlateau, OneCycleLR, CosineAnnealingWarmRestarts,
    LinearLR, SequentialLR, LambdaLR
)

optimizer = optim.Adam(model.parameters(), lr=0.001)

# ── StepLR: 每 step_size 个 epoch 衰减 ──
scheduler = StepLR(optimizer, step_size=30, gamma=0.1)
# epoch 0-29: lr=0.001, epoch 30-59: lr=0.0001, ...

# ── MultiStepLR: 在指定 epoch 衰减 ──
scheduler = MultiStepLR(optimizer, milestones=[30, 80, 120], gamma=0.1)

# ── ExponentialLR: 指数衰减 ──
scheduler = ExponentialLR(optimizer, gamma=0.95)  # 每 epoch × 0.95

# ── CosineAnnealingLR: 余弦退火 ──
scheduler = CosineAnnealingLR(optimizer, T_max=100, eta_min=1e-6)

# ── CosineAnnealingWarmRestarts: 带重启的余弦退火 ──
scheduler = CosineAnnealingWarmRestarts(optimizer, T_0=10, T_mult=2)

# ── ReduceLROnPlateau: 监控指标停止下降时衰减 ──
scheduler = ReduceLROnPlateau(optimizer, mode='min', factor=0.1,
                               patience=10, verbose=True)

# ── OneCycleLR: 一个完整的学习率周期（超级收敛） ──
scheduler = OneCycleLR(optimizer, max_lr=0.01,
                       steps_per_epoch=len(train_loader),
                       epochs=num_epochs)

# ── 线性热身 + 余弦衰减 ──
warmup = LinearLR(optimizer, start_factor=0.01, total_iters=5)
cosine = CosineAnnealingLR(optimizer, T_max=95)
scheduler = SequentialLR(optimizer, schedulers=[warmup, cosine], milestones=[5])

# ── 自定义调度（Lambda） ──
scheduler = LambdaLR(optimizer, lr_lambda=lambda epoch: 0.95 ** epoch)

# ── 使用方式 ──
for epoch in range(num_epochs):
    train(...)
    val_loss = validate(...)

    # 大多数 scheduler 在 epoch 结束后调用
    scheduler.step()

    # ReduceLROnPlateau 需要传入监控指标
    # scheduler.step(val_loss)

    # OneCycleLR 在每个 batch 后调用
    # scheduler.step()  # 放在训练循环内部
```

---

## 11. 数据加载与处理 (torch.utils.data)

### 11.1 Dataset

```python
from torch.utils.data import Dataset, DataLoader

# ── 自定义 Dataset ──
class MyDataset(Dataset):
    def __init__(self, data, labels, transform=None):
        self.data = data
        self.labels = labels
        self.transform = transform

    def __len__(self):
        return len(self.data)

    def __getitem__(self, idx):
        x = self.data[idx]
        y = self.labels[idx]
        if self.transform:
            x = self.transform(x)
        return x, y

# ── 使用示例 ──
data = torch.randn(1000, 784)
labels = torch.randint(0, 10, (1000,))
dataset = MyDataset(data, labels)
print(len(dataset))      # 1000
x, y = dataset[0]        # 获取第 0 个样本

# ── TensorDataset（最简单的方式） ──
from torch.utils.data import TensorDataset
dataset = TensorDataset(data, labels)

# ── 数据集拆分 ──
from torch.utils.data import random_split
train_set, val_set = random_split(dataset, [800, 200])

# 指定比例拆分
generator = torch.Generator().manual_seed(42)
train_set, val_set, test_set = random_split(
    dataset, [0.7, 0.15, 0.15], generator=generator
)

# ── 合并多个 Dataset ──
from torch.utils.data import ConcatDataset
combined = ConcatDataset([dataset1, dataset2])

# ── 子集 ──
from torch.utils.data import Subset
subset = Subset(dataset, indices=[0, 5, 10, 15])
```

### 11.2 DataLoader

```python
train_loader = DataLoader(
    dataset,
    batch_size=64,           # 每批样本数
    shuffle=True,            # 训练集打乱
    num_workers=4,           # 多进程加载
    pin_memory=True,         # 锁页内存（GPU 训练时开启加速）
    drop_last=True,          # 丢弃不完整的最后一批
    prefetch_factor=2,       # 预加载因子
    persistent_workers=True, # 保持 worker 进程
)

val_loader = DataLoader(
    val_set,
    batch_size=128,          # 验证时可以用更大 batch
    shuffle=False,           # 验证集不打乱
    num_workers=4,
    pin_memory=True,
)

# ── 遍历 DataLoader ──
for batch_idx, (inputs, targets) in enumerate(train_loader):
    inputs = inputs.to(device)
    targets = targets.to(device)
    # 训练步骤...

# ── 自定义采样器 ──
from torch.utils.data import WeightedRandomSampler

# 处理类别不平衡
class_counts = [800, 150, 50]
weights = 1.0 / torch.tensor(class_counts, dtype=torch.float)
sample_weights = weights[labels]
sampler = WeightedRandomSampler(sample_weights, num_samples=len(sample_weights))
loader = DataLoader(dataset, batch_size=64, sampler=sampler)

# ── 自定义 collate 函数（处理变长序列等） ──
def custom_collate(batch):
    data = [item[0] for item in batch]
    labels = [item[1] for item in batch]
    data = nn.utils.rnn.pad_sequence(data, batch_first=True)
    labels = torch.stack(labels)
    return data, labels

loader = DataLoader(dataset, batch_size=32, collate_fn=custom_collate)
```

---

## 12. 模型的保存与加载

```python
model = MyModel(784, 256, 10)
optimizer = optim.Adam(model.parameters())

# ── 方式一：仅保存参数（推荐） ──
torch.save(model.state_dict(), 'model_weights.pt')

# 加载
model = MyModel(784, 256, 10)
model.load_state_dict(torch.load('model_weights.pt'))
model.eval()

# ── 方式二：保存整个模型 ──
torch.save(model, 'model_full.pt')
model = torch.load('model_full.pt')

# ── 方式三：保存训练检查点（断点续训） ──
checkpoint = {
    'epoch': epoch,
    'model_state_dict': model.state_dict(),
    'optimizer_state_dict': optimizer.state_dict(),
    'scheduler_state_dict': scheduler.state_dict(),
    'loss': loss,
    'best_val_acc': best_val_acc,
}
torch.save(checkpoint, f'checkpoint_epoch{epoch}.pt')

# 恢复训练
checkpoint = torch.load('checkpoint_epoch10.pt')
model.load_state_dict(checkpoint['model_state_dict'])
optimizer.load_state_dict(checkpoint['optimizer_state_dict'])
scheduler.load_state_dict(checkpoint['scheduler_state_dict'])
start_epoch = checkpoint['epoch'] + 1
best_val_acc = checkpoint['best_val_acc']

# ── 部分加载（迁移学习、模型结构改变） ──
pretrained = torch.load('pretrained.pt')
model_dict = model.state_dict()
# 过滤掉不匹配的键
pretrained = {k: v for k, v in pretrained.items()
              if k in model_dict and v.shape == model_dict[k].shape}
model_dict.update(pretrained)
model.load_state_dict(model_dict)

# ── 跨设备加载 ──
model.load_state_dict(torch.load('model.pt', map_location='cpu'))
model.load_state_dict(torch.load('model.pt', map_location='cuda:0'))

# ── 安全加载（PyTorch ≥ 2.0） ──
model.load_state_dict(torch.load('model.pt', weights_only=True))
```

---

## 13. GPU 加速与设备管理

```python
# ── 设备选择 ──
device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
device = torch.device('cuda:0')     # 指定 GPU 0
device = torch.device('mps')        # Apple Silicon

# ── 移动数据到设备 ──
model = model.to(device)
x = x.to(device)
x = x.cuda()      # 移到默认 GPU
x = x.cpu()        # 移回 CPU

# ── GPU 信息 ──
torch.cuda.is_available()           # CUDA 是否可用
torch.cuda.device_count()           # GPU 数量
torch.cuda.current_device()         # 当前 GPU 索引
torch.cuda.get_device_name(0)       # GPU 名称
torch.cuda.memory_allocated()       # 已分配显存 (bytes)
torch.cuda.memory_reserved()        # 已预留显存 (bytes)
torch.cuda.max_memory_allocated()   # 峰值显存

# ── 显存管理 ──
torch.cuda.empty_cache()            # 释放未使用的缓存显存
torch.cuda.reset_peak_memory_stats()  # 重置峰值统计
torch.cuda.memory_summary()         # 显存详细报告

# ── 多 GPU 数据并行 ──
if torch.cuda.device_count() > 1:
    model = nn.DataParallel(model)  # 自动分发到多个 GPU
    model = model.to(device)

# 访问原始模型
original_model = model.module if isinstance(model, nn.DataParallel) else model

# ── 指定 GPU ──
import os
os.environ['CUDA_VISIBLE_DEVICES'] = '0,1'  # 只使用 GPU 0 和 1

# ── GPU 同步 ──
torch.cuda.synchronize()  # 等待所有 GPU 操作完成（计时时需要）

# ── 随机种子（可复现性） ──
def set_seed(seed=42):
    torch.manual_seed(seed)
    torch.cuda.manual_seed(seed)
    torch.cuda.manual_seed_all(seed)     # 多 GPU
    np.random.seed(seed)
    import random
    random.seed(seed)
    torch.backends.cudnn.deterministic = True
    torch.backends.cudnn.benchmark = False
```

---

## 14. 模型训练完整流程

```python
import torch
import torch.nn as nn
import torch.optim as optim
from torch.utils.data import DataLoader

# ── 1. 准备 ──
device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
model = MyModel(784, 256, 10).to(device)
criterion = nn.CrossEntropyLoss()
optimizer = optim.AdamW(model.parameters(), lr=1e-3, weight_decay=0.01)
scheduler = optim.lr_scheduler.CosineAnnealingLR(optimizer, T_max=100)

# ── 2. 训练循环 ──
num_epochs = 100
best_val_acc = 0.0

for epoch in range(num_epochs):
    # ---- 训练阶段 ----
    model.train()
    train_loss = 0.0
    correct = 0
    total = 0

    for inputs, targets in train_loader:
        inputs, targets = inputs.to(device), targets.to(device)

        optimizer.zero_grad(set_to_none=True)
        outputs = model(inputs)
        loss = criterion(outputs, targets)
        loss.backward()

        # 可选：梯度裁剪
        nn.utils.clip_grad_norm_(model.parameters(), max_norm=1.0)

        optimizer.step()

        train_loss += loss.item() * inputs.size(0)
        _, predicted = outputs.max(1)
        total += targets.size(0)
        correct += predicted.eq(targets).sum().item()

    train_loss /= total
    train_acc = correct / total

    # ---- 验证阶段 ----
    model.eval()
    val_loss = 0.0
    correct = 0
    total = 0

    with torch.no_grad():
        for inputs, targets in val_loader:
            inputs, targets = inputs.to(device), targets.to(device)
            outputs = model(inputs)
            loss = criterion(outputs, targets)

            val_loss += loss.item() * inputs.size(0)
            _, predicted = outputs.max(1)
            total += targets.size(0)
            correct += predicted.eq(targets).sum().item()

    val_loss /= total
    val_acc = correct / total

    scheduler.step()
    current_lr = optimizer.param_groups[0]['lr']

    print(f"Epoch {epoch+1}/{num_epochs} | "
          f"Train Loss: {train_loss:.4f} Acc: {train_acc:.4f} | "
          f"Val Loss: {val_loss:.4f} Acc: {val_acc:.4f} | "
          f"LR: {current_lr:.6f}")

    # ---- 保存最佳模型 ----
    if val_acc > best_val_acc:
        best_val_acc = val_acc
        torch.save(model.state_dict(), 'best_model.pt')
        print(f"  → Saved best model (val_acc={val_acc:.4f})")

print(f"\nBest Validation Accuracy: {best_val_acc:.4f}")
```

---

## 15. 模型评估与推理

```python
# ── 评估模式 ──
model.eval()  # 关闭 Dropout、使用 BN 的滑动均值

# ── 推理 ──
with torch.no_grad():  # 禁用梯度计算，节省内存和加速
    outputs = model(inputs)
    probabilities = F.softmax(outputs, dim=-1)
    predictions = outputs.argmax(dim=-1)
    confidence, predicted_class = probabilities.max(dim=-1)

# ── 批量推理 ──
all_preds = []
all_labels = []

model.eval()
with torch.no_grad():
    for inputs, targets in test_loader:
        inputs = inputs.to(device)
        outputs = model(inputs)
        preds = outputs.argmax(dim=-1)
        all_preds.append(preds.cpu())
        all_labels.append(targets)

all_preds = torch.cat(all_preds)
all_labels = torch.cat(all_labels)

# 准确率
accuracy = (all_preds == all_labels).float().mean().item()

# ── 推理优化 ──
# 使用 torch.inference_mode()（比 no_grad 更快）
with torch.inference_mode():
    outputs = model(inputs)

# ── 使用 torch.compile 加速（PyTorch ≥ 2.0） ──
compiled_model = torch.compile(model)
with torch.inference_mode():
    outputs = compiled_model(inputs)
```

---

## 16. 正则化与防止过拟合

### 16.1 Weight Decay（L2 正则化）

```python
# 通过优化器的 weight_decay 参数实现
optimizer = optim.AdamW(model.parameters(), lr=1e-3, weight_decay=0.01)
```

### 16.2 Dropout

```python
class RegularizedModel(nn.Module):
    def __init__(self):
        super().__init__()
        self.fc1 = nn.Linear(784, 512)
        self.dropout1 = nn.Dropout(0.5)
        self.fc2 = nn.Linear(512, 256)
        self.dropout2 = nn.Dropout(0.3)
        self.fc3 = nn.Linear(256, 10)

    def forward(self, x):
        x = F.relu(self.fc1(x))
        x = self.dropout1(x)      # 训练时随机丢弃
        x = F.relu(self.fc2(x))
        x = self.dropout2(x)
        return self.fc3(x)
```

### 16.3 早停 (Early Stopping)

```python
class EarlyStopping:
    def __init__(self, patience=10, min_delta=0.0):
        self.patience = patience
        self.min_delta = min_delta
        self.counter = 0
        self.best_loss = None
        self.early_stop = False

    def __call__(self, val_loss):
        if self.best_loss is None:
            self.best_loss = val_loss
        elif val_loss > self.best_loss - self.min_delta:
            self.counter += 1
            if self.counter >= self.patience:
                self.early_stop = True
        else:
            self.best_loss = val_loss
            self.counter = 0

# 使用
early_stopping = EarlyStopping(patience=10)
for epoch in range(num_epochs):
    val_loss = validate(...)
    early_stopping(val_loss)
    if early_stopping.early_stop:
        print("Early stopping triggered")
        break
```

### 16.4 数据增强

```python
from torchvision import transforms

train_transform = transforms.Compose([
    transforms.RandomResizedCrop(224),
    transforms.RandomHorizontalFlip(),
    transforms.RandomRotation(15),
    transforms.ColorJitter(brightness=0.2, contrast=0.2, saturation=0.2, hue=0.1),
    transforms.RandomErasing(p=0.5),
    transforms.ToTensor(),
    transforms.Normalize(mean=[0.485, 0.456, 0.406],
                         std=[0.229, 0.224, 0.225]),
])
```

### 16.5 Label Smoothing

```python
criterion = nn.CrossEntropyLoss(label_smoothing=0.1)
```

---

## 17. 权重初始化

```python
def init_weights(module):
    """按层类型自动初始化权重"""
    if isinstance(module, nn.Linear):
        nn.init.kaiming_normal_(module.weight, mode='fan_out', nonlinearity='relu')
        if module.bias is not None:
            nn.init.zeros_(module.bias)
    elif isinstance(module, nn.Conv2d):
        nn.init.kaiming_normal_(module.weight, mode='fan_out', nonlinearity='relu')
        if module.bias is not None:
            nn.init.zeros_(module.bias)
    elif isinstance(module, (nn.BatchNorm2d, nn.LayerNorm)):
        nn.init.ones_(module.weight)
        nn.init.zeros_(module.bias)
    elif isinstance(module, nn.Embedding):
        nn.init.normal_(module.weight, mean=0, std=0.02)

# 应用
model.apply(init_weights)

# ── 常用初始化方法 ──
nn.init.zeros_(tensor)                    # 全零
nn.init.ones_(tensor)                     # 全一
nn.init.constant_(tensor, val)            # 常数
nn.init.uniform_(tensor, a=0, b=1)       # 均匀分布
nn.init.normal_(tensor, mean=0, std=1)   # 正态分布

nn.init.xavier_uniform_(tensor)          # Xavier 均匀 (Glorot)
nn.init.xavier_normal_(tensor)           # Xavier 正态
nn.init.kaiming_uniform_(tensor)         # Kaiming/He 均匀 (适合 ReLU)
nn.init.kaiming_normal_(tensor)          # Kaiming/He 正态
nn.init.orthogonal_(tensor)              # 正交初始化 (适合 RNN)
nn.init.sparse_(tensor, sparsity=0.1)    # 稀疏初始化
nn.init.trunc_normal_(tensor, mean=0, std=0.02)  # 截断正态 (Vision Transformer)
```

---

## 18. 分布式训练 (torch.distributed)

### 18.1 DistributedDataParallel (DDP)

```python
import torch.distributed as dist
from torch.nn.parallel import DistributedDataParallel as DDP
from torch.utils.data.distributed import DistributedSampler

def setup(rank, world_size):
    dist.init_process_group("nccl", rank=rank, world_size=world_size)
    torch.cuda.set_device(rank)

def cleanup():
    dist.destroy_process_group()

def train_ddp(rank, world_size):
    setup(rank, world_size)

    model = MyModel().to(rank)
    model = DDP(model, device_ids=[rank])

    sampler = DistributedSampler(dataset, num_replicas=world_size, rank=rank)
    loader = DataLoader(dataset, batch_size=64, sampler=sampler)

    for epoch in range(num_epochs):
        sampler.set_epoch(epoch)   # 每个 epoch 打乱顺序
        for inputs, targets in loader:
            inputs, targets = inputs.to(rank), targets.to(rank)
            # 正常训练步骤...

    cleanup()

# 启动
import torch.multiprocessing as mp
world_size = torch.cuda.device_count()
mp.spawn(train_ddp, args=(world_size,), nprocs=world_size)
```

### 18.2 使用 torchrun 启动

```bash
# 单机多卡
torchrun --nproc_per_node=4 train.py

# 多机多卡
torchrun --nnodes=2 --nproc_per_node=4 \
         --rdzv_backend=c10d --rdzv_endpoint=HOST:PORT train.py
```

```python
# train.py 中
import os

def main():
    dist.init_process_group("nccl")
    rank = int(os.environ["LOCAL_RANK"])
    torch.cuda.set_device(rank)
    # ...

if __name__ == "__main__":
    main()
```

---

## 19. 混合精度训练 (AMP)

使用 FP16/BF16 加速训练并减少显存使用。

```python
from torch.amp import autocast, GradScaler

scaler = GradScaler('cuda')

for inputs, targets in train_loader:
    inputs, targets = inputs.to(device), targets.to(device)

    optimizer.zero_grad()

    # 前向传播使用混合精度
    with autocast('cuda'):
        outputs = model(inputs)
        loss = criterion(outputs, targets)

    # 反向传播使用 scaler
    scaler.scale(loss).backward()
    scaler.unscale_(optimizer)   # 如需梯度裁剪，先 unscale
    nn.utils.clip_grad_norm_(model.parameters(), max_norm=1.0)
    scaler.step(optimizer)
    scaler.update()

# ── 使用 BFloat16（Ampere 及以上 GPU） ──
with autocast('cuda', dtype=torch.bfloat16):
    outputs = model(inputs)
    loss = criterion(outputs, targets)
# BF16 不需要 GradScaler
```

---

## 20. TorchScript 与模型导出

### 20.1 TorchScript

```python
# ── Tracing（适合没有控制流的模型） ──
model.eval()
example_input = torch.randn(1, 784)
traced_model = torch.jit.trace(model, example_input)
traced_model.save('model_traced.pt')

# ── Scripting（支持控制流） ──
scripted_model = torch.jit.script(model)
scripted_model.save('model_scripted.pt')

# ── 加载 ──
loaded = torch.jit.load('model_traced.pt')
output = loaded(input_tensor)
```

### 20.2 导出 ONNX

```python
model.eval()
dummy_input = torch.randn(1, 3, 224, 224).to(device)

torch.onnx.export(
    model,
    dummy_input,
    'model.onnx',
    export_params=True,
    opset_version=17,
    input_names=['input'],
    output_names=['output'],
    dynamic_axes={
        'input': {0: 'batch_size'},
        'output': {0: 'batch_size'},
    }
)
```

### 20.3 torch.compile (PyTorch 2.0+)

```python
# 一行加速（推荐方式）
compiled_model = torch.compile(model)

# 指定后端和模式
compiled_model = torch.compile(model, backend='inductor', mode='reduce-overhead')

# 模式说明:
# 'default'         — 平衡编译时间和性能
# 'reduce-overhead' — 减少框架开销（适合小模型/小 batch）
# 'max-autotune'    — 最大性能（编译时间较长）
```

---

## 21. 常用工具函数

### 21.1 torch.nn.functional (F)

```python
import torch.nn.functional as F

# ── 激活函数 ──
F.relu(x)
F.gelu(x)
F.sigmoid(x)
F.softmax(x, dim=-1)
F.log_softmax(x, dim=-1)
F.tanh(x)
F.leaky_relu(x, negative_slope=0.01)
F.silu(x)

# ── 损失函数 ──
F.cross_entropy(logits, targets)
F.mse_loss(pred, target)
F.l1_loss(pred, target)
F.binary_cross_entropy_with_logits(logits, targets)
F.nll_loss(log_probs, targets)
F.smooth_l1_loss(pred, target)

# ── 卷积 ──
F.conv2d(input, weight, bias, stride, padding)
F.conv_transpose2d(input, weight)

# ── 池化 ──
F.max_pool2d(x, kernel_size=2)
F.avg_pool2d(x, kernel_size=2)
F.adaptive_avg_pool2d(x, output_size=1)

# ── 归一化 ──
F.batch_norm(x, running_mean, running_var)
F.layer_norm(x, normalized_shape)
F.normalize(x, p=2, dim=-1)  # L2 归一化

# ── Dropout ──
F.dropout(x, p=0.5, training=self.training)

# ── 其他 ──
F.one_hot(labels, num_classes=10)    # 独热编码
F.interpolate(x, scale_factor=2, mode='bilinear')  # 插值上采样
F.pad(x, (1, 1, 1, 1), mode='reflect')  # 填充
F.cosine_similarity(x1, x2, dim=-1)     # 余弦相似度
F.pairwise_distance(x1, x2)             # 欧式距离
```

### 21.2 序列处理工具

```python
from torch.nn.utils.rnn import (
    pad_sequence, pack_padded_sequence, pad_packed_sequence
)

# ── 变长序列填充 ──
seqs = [torch.tensor([1, 2, 3]), torch.tensor([4, 5]), torch.tensor([6])]
padded = pad_sequence(seqs, batch_first=True, padding_value=0)
# tensor([[1, 2, 3],
#          [4, 5, 0],
#          [6, 0, 0]])

# ── Pack（高效 RNN 输入） ──
lengths = torch.tensor([3, 2, 1])
packed = pack_padded_sequence(padded, lengths, batch_first=True, enforce_sorted=True)
output, (h_n, c_n) = lstm(packed)

# ── Unpack ──
output_padded, output_lengths = pad_packed_sequence(output, batch_first=True)
```

### 21.3 随机种子与可复现性

```python
import torch
import numpy as np
import random

def seed_everything(seed=42):
    random.seed(seed)
    np.random.seed(seed)
    torch.manual_seed(seed)
    torch.cuda.manual_seed(seed)
    torch.cuda.manual_seed_all(seed)
    torch.backends.cudnn.deterministic = True
    torch.backends.cudnn.benchmark = False
    # 可选：完全确定性模式（可能降低性能）
    torch.use_deterministic_algorithms(True)

seed_everything(42)
```

---

## 22. torchvision 常用功能

### 22.1 数据变换 (transforms)

```python
from torchvision import transforms

# ── 训练时变换 ──
train_transform = transforms.Compose([
    transforms.Resize(256),
    transforms.RandomResizedCrop(224, scale=(0.8, 1.0)),
    transforms.RandomHorizontalFlip(p=0.5),
    transforms.RandomVerticalFlip(p=0.1),
    transforms.RandomRotation(degrees=15),
    transforms.ColorJitter(brightness=0.2, contrast=0.2, saturation=0.2, hue=0.1),
    transforms.RandomAffine(degrees=0, translate=(0.1, 0.1)),
    transforms.RandomPerspective(distortion_scale=0.2, p=0.5),
    transforms.GaussianBlur(kernel_size=3),
    transforms.RandomGrayscale(p=0.1),
    transforms.RandomErasing(p=0.5, scale=(0.02, 0.33)),
    transforms.ToTensor(),
    transforms.Normalize(mean=[0.485, 0.456, 0.406],
                         std=[0.229, 0.224, 0.225]),
])

# ── 验证/测试时变换 ──
val_transform = transforms.Compose([
    transforms.Resize(256),
    transforms.CenterCrop(224),
    transforms.ToTensor(),
    transforms.Normalize(mean=[0.485, 0.456, 0.406],
                         std=[0.229, 0.224, 0.225]),
])

# ── V2 变换（推荐，PyTorch ≥ 2.0） ──
from torchvision.transforms import v2
train_transform_v2 = v2.Compose([
    v2.RandomResizedCrop(224),
    v2.RandomHorizontalFlip(),
    v2.ToDtype(torch.float32, scale=True),
    v2.Normalize(mean=[0.485, 0.456, 0.406], std=[0.229, 0.224, 0.225]),
])
```

### 22.2 预训练模型

```python
from torchvision import models

# ── 新 API（weights 参数，推荐） ──
resnet50 = models.resnet50(weights=models.ResNet50_Weights.IMAGENET1K_V2)
vgg16 = models.vgg16(weights=models.VGG16_Weights.DEFAULT)
efficientnet = models.efficientnet_b0(weights=models.EfficientNet_B0_Weights.DEFAULT)
vit = models.vit_b_16(weights=models.ViT_B_16_Weights.DEFAULT)
swin = models.swin_t(weights=models.Swin_T_Weights.DEFAULT)

# ── 修改分类头（迁移学习） ──
resnet50.fc = nn.Linear(resnet50.fc.in_features, num_classes)

# VGG
vgg16.classifier[-1] = nn.Linear(4096, num_classes)

# EfficientNet
efficientnet.classifier[-1] = nn.Linear(
    efficientnet.classifier[-1].in_features, num_classes
)

# ViT
vit.heads.head = nn.Linear(vit.heads.head.in_features, num_classes)

# ── 冻结特征提取层 ──
for param in resnet50.parameters():
    param.requires_grad = False
# 解冻分类头
for param in resnet50.fc.parameters():
    param.requires_grad = True

# ── 渐进解冻 ──
# 先冻结全部 → 训练分类头 → 逐层解冻
```

### 22.3 内置数据集

```python
from torchvision import datasets

# ── 常用数据集 ──
mnist = datasets.MNIST(root='./data', train=True, transform=transform, download=True)
cifar10 = datasets.CIFAR10(root='./data', train=True, transform=transform, download=True)
cifar100 = datasets.CIFAR100(root='./data', train=True, transform=transform, download=True)
imagenet = datasets.ImageNet(root='./data', split='train', transform=transform)
fashionmnist = datasets.FashionMNIST(root='./data', train=True, transform=transform, download=True)

# ── 从文件夹加载（按子目录分类） ──
# data/
#   train/
#     cat/
#     dog/
#   val/
#     cat/
#     dog/
train_dataset = datasets.ImageFolder(root='data/train', transform=train_transform)
val_dataset = datasets.ImageFolder(root='data/val', transform=val_transform)
print(train_dataset.classes)      # ['cat', 'dog']
print(train_dataset.class_to_idx) # {'cat': 0, 'dog': 1}
```

---

## 23. 调试与性能优化技巧

### 23.1 常用调试手段

```python
# ── 检查张量信息 ──
def debug_tensor(name, t):
    print(f"{name}: shape={t.shape}, dtype={t.dtype}, device={t.device}, "
          f"min={t.min():.4f}, max={t.max():.4f}, mean={t.mean():.4f}, "
          f"has_nan={t.isnan().any()}, has_inf={t.isinf().any()}")

# ── 检测异常值 ──
torch.autograd.set_detect_anomaly(True)  # 检测 NaN/Inf 产生位置（调试时开启）

# ── 使用 hooks 查看中间输出 ──
activations = {}
def get_activation(name):
    def hook(module, input, output):
        activations[name] = output.detach()
    return hook

model.fc1.register_forward_hook(get_activation('fc1'))
output = model(x)
print(activations['fc1'].shape)

# ── 梯度 hook ──
def grad_hook(name):
    def hook(grad):
        print(f"{name} grad: mean={grad.mean():.6f}, std={grad.std():.6f}")
    return hook

for name, param in model.named_parameters():
    param.register_hook(grad_hook(name))
```

### 23.2 性能优化

```python
# ── 1. torch.compile（PyTorch 2.0+, 最简单的加速方式） ──
model = torch.compile(model)

# ── 2. cuDNN benchmark（卷积网络加速） ──
torch.backends.cudnn.benchmark = True  # 自动选择最快的卷积算法

# ── 3. 减少 CPU-GPU 同步 ──
# 避免在训练循环中调用 .item()、print 张量值、或频繁 .cpu()
# 推荐每 N 步打印一次

# ── 4. 非阻塞传输 ──
inputs = inputs.to(device, non_blocking=True)
targets = targets.to(device, non_blocking=True)

# ── 5. 梯度累积（模拟大 batch） ──
accumulation_steps = 4
for i, (inputs, targets) in enumerate(train_loader):
    outputs = model(inputs.to(device))
    loss = criterion(outputs, targets.to(device)) / accumulation_steps
    loss.backward()

    if (i + 1) % accumulation_steps == 0:
        optimizer.step()
        optimizer.zero_grad()

# ── 6. 使用 Profiler 分析性能 ──
with torch.profiler.profile(
    activities=[
        torch.profiler.ProfilerActivity.CPU,
        torch.profiler.ProfilerActivity.CUDA,
    ],
    schedule=torch.profiler.schedule(wait=1, warmup=1, active=3),
    on_trace_ready=torch.profiler.tensorboard_trace_handler('./log'),
    record_shapes=True,
    profile_memory=True,
    with_stack=True,
) as prof:
    for step, (inputs, targets) in enumerate(train_loader):
        outputs = model(inputs.to(device))
        loss = criterion(outputs, targets.to(device))
        loss.backward()
        optimizer.step()
        optimizer.zero_grad()
        prof.step()
        if step >= 5:
            break

# ── 7. 使用 channels_last 内存格式（Conv2d 加速） ──
model = model.to(memory_format=torch.channels_last)
inputs = inputs.to(memory_format=torch.channels_last)

# ── 8. 使用 pin_memory + non_blocking ──
loader = DataLoader(dataset, batch_size=64, pin_memory=True)
inputs = inputs.to(device, non_blocking=True)
```

---

## 24. 常见错误与排查

### RuntimeError: CUDA out of memory

```python
# 减小 batch_size
# 使用混合精度训练 (AMP)
# 使用梯度累积
# 释放缓存
torch.cuda.empty_cache()
# 使用梯度检查点减少显存
from torch.utils.checkpoint import checkpoint
output = checkpoint(model.layer, input, use_reentrant=False)
```

### RuntimeError: Expected all tensors on the same device

```python
# 确保模型和数据在同一设备
model = model.to(device)
inputs = inputs.to(device)
targets = targets.to(device)
```

### RuntimeError: mat1 and mat2 shapes cannot be multiplied

```python
# 检查维度匹配
print(f"Input shape: {x.shape}")
print(f"Layer weight shape: {model.fc1.weight.shape}")
# 使用 flatten 或 reshape 调整
x = x.view(x.size(0), -1)  # 展平为 (batch, features)
```

### 梯度为 None

```python
# 检查 requires_grad
for name, p in model.named_parameters():
    print(name, p.requires_grad)
# 确保没有在 no_grad 上下文中做前向传播
# 确保 loss.backward() 被调用
```

### DataLoader worker 错误

```python
# Windows 上设 num_workers=0
# Linux 上如遇共享内存不足:
# docker run --shm-size=8g ...
# 或者设置 multiprocessing 启动方式
import torch.multiprocessing as mp
mp.set_start_method('spawn')
```

### 训练 Loss 不下降

```python
# 检查学习率是否合适（先尝试 1e-3）
# 检查数据标签是否正确
# 检查是否忘记 model.train()
# 检查是否忘记 optimizer.zero_grad()
# 检查损失函数与输出是否匹配（如 CrossEntropyLoss 不需要 softmax）
# 先在小数据上过拟合验证模型能力
```

---

> **提示**：本文档涵盖了 PyTorch 日常开发中最常用的内容。更多细节请参阅 [PyTorch 官方文档](https://pytorch.org/docs/stable/)。
