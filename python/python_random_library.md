# Python `random` 库完全指南

> Python 标准库中的 `random` 模块提供了生成伪随机数的各种工具，适用于模拟、游戏、测试等场景。

---

## 目录

- [快速开始](#快速开始)
- [随机整数](#随机整数)
- [随机浮点数](#随机浮点数)
- [序列操作](#序列操作)
- [随机分布](#随机分布)
- [随机种子](#随机种子)
- [实用示例](#实用示例)
- [注意事项](#注意事项)

---

## 快速开始

```python
import random

# 生成 0.0 ~ 1.0 的随机浮点数
print(random.random())        # 例如：0.7342

# 从列表中随机选一个
print(random.choice([1, 2, 3, 4, 5]))   # 例如：3

# 生成 1 ~ 10 的随机整数
print(random.randint(1, 10))  # 例如：7
```

---

## 随机整数

| 函数 | 说明 |
|------|------|
| `randint(a, b)` | 返回 `[a, b]` 范围内的随机整数（**包含两端**） |
| `randrange(stop)` | 返回 `[0, stop)` 范围内的随机整数 |
| `randrange(start, stop[, step])` | 返回指定范围和步长内的随机整数 |

```python
import random

# randint：包含 1 和 10
n = random.randint(1, 10)
print(n)   # 例如：5

# randrange：不包含右端
n = random.randrange(10)       # 0 ~ 9
n = random.randrange(1, 10)    # 1 ~ 9
n = random.randrange(0, 100, 5)  # 0, 5, 10, ..., 95 中随机一个
print(n)   # 例如：45
```

---

## 随机浮点数

| 函数 | 说明 |
|------|------|
| `random()` | 返回 `[0.0, 1.0)` 的随机浮点数 |
| `uniform(a, b)` | 返回 `[a, b]` 之间的随机浮点数 |

```python
import random

# 基础随机浮点
x = random.random()
print(x)             # 例如：0.4231

# 指定范围
x = random.uniform(1.5, 3.5)
print(x)             # 例如：2.718

# 常用技巧：映射到百分比
percent = random.random() * 100
print(f"{percent:.2f}%")  # 例如：73.42%
```

---

## 序列操作

### `choice` — 随机选取一个元素

```python
import random

fruits = ["苹果", "香蕉", "橙子", "葡萄"]
pick = random.choice(fruits)
print(pick)   # 例如：香蕉
```

> ⚠️ 序列不能为空，否则抛出 `IndexError`。

---

### `choices` — 有放回地随机选取多个（可设权重）

```python
import random

# 基础用法：选 3 个（可重复）
result = random.choices(["A", "B", "C"], k=3)
print(result)   # 例如：['A', 'A', 'C']

# 带权重：B 被选中的概率是 A 的两倍
result = random.choices(["A", "B", "C"], weights=[1, 2, 1], k=5)
print(result)   # 例如：['B', 'A', 'B', 'B', 'C']
```

| 参数 | 说明 |
|------|------|
| `population` | 待选序列 |
| `weights` | 相对权重列表（与 `cum_weights` 二选一） |
| `cum_weights` | 累积权重列表 |
| `k` | 选取数量，默认为 1 |

---

### `sample` — 无放回地随机选取多个

```python
import random

nums = list(range(1, 50))
lottery = random.sample(nums, k=6)
print(sorted(lottery))   # 例如：[3, 15, 22, 31, 40, 47]
```

> ✅ 结果中**不会有重复元素**，常用于抽奖、抽样等场景。

---

### `shuffle` — 原地打乱序列

```python
import random

deck = list(range(1, 14))   # 模拟一副牌
random.shuffle(deck)
print(deck)   # 例如：[9, 3, 11, 1, 7, 2, 13, 5, 4, 12, 6, 10, 8]
```

> ⚠️ `shuffle` **直接修改原列表**，无返回值。若需保留原顺序，先复制：
> ```python
> shuffled = deck[:]
> random.shuffle(shuffled)
> ```

---

## 随机分布

适用于科学计算、模拟等需要特定概率分布的场景。

| 函数 | 分布类型 | 常用参数 |
|------|----------|----------|
| `gauss(mu, sigma)` | 高斯（正态）分布 | 均值 `mu`，标准差 `sigma` |
| `normalvariate(mu, sigma)` | 正态分布（线程安全） | 同上 |
| `expovariate(lambd)` | 指数分布 | 速率参数 `lambd` |
| `betavariate(alpha, beta)` | Beta 分布 | `alpha > 0`, `beta > 0` |
| `triangular(low, high, mode)` | 三角分布 | 下界、上界、众数 |

```python
import random

# 模拟身高（均值 170cm，标准差 10cm）
height = random.gauss(170, 10)
print(f"{height:.1f} cm")   # 例如：163.5 cm

# 模拟等待时间（平均 5 分钟）
wait = random.expovariate(1/5)
print(f"{wait:.2f} 分钟")   # 例如：3.27 分钟
```

---

## 随机种子

设置种子可以使随机结果**可复现**，便于调试和测试。

```python
import random

# 设置种子
random.seed(42)
print(random.random())    # 0.6394267984578837
print(random.randint(1, 100))  # 2

# 再次设置相同种子，结果完全相同
random.seed(42)
print(random.random())    # 0.6394267984578837（与上面一致）
print(random.randint(1, 100))  # 2
```

> 💡 **生产环境**不要设置固定种子，否则结果可预测，存在安全风险。  
> 需要**密码学安全**的随机数，请使用 `secrets` 模块。

---

## 实用示例

### 示例 1：模拟掷骰子

```python
import random

def roll_dice(sides=6, times=1):
    """模拟掷骰子"""
    return [random.randint(1, sides) for _ in range(times)]

# 掷两个六面骰子
result = roll_dice(sides=6, times=2)
print(f"掷出：{result}，合计：{sum(result)}")
# 输出：掷出：[4, 6]，合计：10
```

---

### 示例 2：随机密码生成

```python
import random
import string

def generate_password(length=12):
    """生成随机密码"""
    chars = string.ascii_letters + string.digits + "!@#$%"
    return "".join(random.choices(chars, k=length))

print(generate_password())   # 例如：aB3!xQz9#mKp
```

> ⚠️ 生产环境的密码生成请使用 `secrets.choice()`，安全性更高。

---

### 示例 3：随机打乱并分组

```python
import random

students = ["Alice", "Bob", "Charlie", "Diana", "Eve", "Frank"]
random.shuffle(students)

# 分成两组
mid = len(students) // 2
group_a = students[:mid]
group_b = students[mid:]

print("A 组：", group_a)
print("B 组：", group_b)
# 输出：
# A 组：['Charlie', 'Frank', 'Alice']
# B 组：['Eve', 'Bob', 'Diana']
```

---

### 示例 4：蒙特卡罗估算 π

```python
import random

def estimate_pi(n=1_000_000):
    """用蒙特卡罗方法估算 π"""
    inside = 0
    for _ in range(n):
        x, y = random.random(), random.random()
        if x**2 + y**2 <= 1:
            inside += 1
    return 4 * inside / n

print(f"π ≈ {estimate_pi():.5f}")   # 例如：π ≈ 3.14159
```

---

## 注意事项

| 场景 | 推荐做法 |
|------|----------|
| 普通随机数、模拟、游戏 | ✅ 使用 `random` 模块 |
| 密码、Token、安全随机数 | ✅ 使用 `secrets` 模块 |
| 数值计算、机器学习 | ✅ 使用 `numpy.random` |
| 需要可复现结果 | ✅ 设置 `random.seed()` |
| 多线程环境 | ✅ 使用 `normalvariate` 而非 `gauss` |

> **`random` 模块生成的是伪随机数**，基于 Mersenne Twister 算法，周期长达 2¹⁹⁹³⁷−1，统计性质优秀，但**不适合密码学用途**。

---

## 常用函数速查

```
random.random()              → [0.0, 1.0) 浮点
random.uniform(a, b)         → [a, b] 浮点
random.randint(a, b)         → [a, b] 整数（含两端）
random.randrange(start, stop, step) → 指定范围整数
random.choice(seq)           → 随机选一个元素
random.choices(seq, k=n)     → 有放回选 n 个（可加权）
random.sample(seq, k=n)      → 无放回选 n 个
random.shuffle(seq)          → 原地打乱
random.gauss(mu, sigma)      → 正态分布浮点
random.seed(n)               → 设置随机种子
```

---

*参考文档：[Python 官方文档 — random](https://docs.python.org/3/library/random.html)*
