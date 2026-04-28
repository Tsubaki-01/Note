# Python 常用数据结构完全指南

> 本指南涵盖 Python 中最常用的数据结构，包括内置类型和标准库扩展类型，帮助你在不同场景下选择最合适的工具。

---

## 目录

- [list — 列表](#list--列表)
- [tuple — 元组](#tuple--元组)
- [dict — 字典](#dict--字典)
- [set — 集合](#set--集合)
- [deque — 双端队列](#deque--双端队列)
- [defaultdict — 带默认值的字典](#defaultdict--带默认值的字典)
- [OrderedDict — 有序字典](#ordereddict--有序字典)
- [Counter — 计数器](#counter--计数器)
- [heapq — 堆（优先队列）](#heapq--堆优先队列)
- [选型速查表](#选型速查表)

---

## `list` — 列表

### 是什么

`list` 是 Python 中最通用的序列类型，**有序、可变、允许重复元素**，支持任意类型混合存储。

### 何时使用

- 需要保持元素顺序
- 需要频繁增删改元素
- 需要通过索引随机访问元素

### 基本用法

```python
# 创建列表
fruits = ["apple", "banana", "cherry"]
mixed = [1, "hello", 3.14, True]
empty = []

# 访问元素（支持负索引）
print(fruits[0])   # apple
print(fruits[-1])  # cherry

# 切片
print(fruits[1:3])  # ['banana', 'cherry']
print(fruits[::-1]) # ['cherry', 'banana', 'apple'] 倒序
```

### 常用操作

```python
nums = [3, 1, 4, 1, 5, 9]

# 增
nums.append(2)        # 末尾追加：[3, 1, 4, 1, 5, 9, 2]
nums.insert(0, 0)     # 指定位置插入：[0, 3, 1, 4, 1, 5, 9, 2]
nums.extend([6, 5])   # 批量追加

# 删
nums.remove(1)        # 删除第一个值为 1 的元素
popped = nums.pop()   # 弹出并返回最后一个元素
del nums[0]           # 按索引删除

# 查
print(1 in nums)      # True
print(nums.index(5))  # 返回第一个 5 的索引
print(nums.count(1))  # 统计 1 出现的次数

# 排序
nums.sort()                        # 原地升序排序
nums.sort(reverse=True)            # 原地降序排序
sorted_nums = sorted(nums)         # 返回新列表，不修改原列表

# 列表推导式（List Comprehension）
squares = [x**2 for x in range(10)]
evens = [x for x in range(20) if x % 2 == 0]
# Output: [0, 2, 4, 6, 8, 10, 12, 14, 16, 18]
```

### 性能特性

| 操作 | 时间复杂度 |
|------|-----------|
| 末尾追加 `append` | O(1) 均摊 |
| 随机访问 `list[i]` | O(1) |
| 头部插入 `insert(0, x)` | O(n) |
| 搜索 `in` | O(n) |
| 排序 `sort` | O(n log n) |

> ⚠️ **注意**：频繁在头部插入/删除时，请改用 `deque`。

---

## `tuple` — 元组

### 是什么

`tuple` 是**有序、不可变**的序列。一旦创建，元素不能增删改。

### 何时使用

- 数据不应被修改（配置项、坐标、RGB 值等）
- 作为字典的 key（list 不可哈希，tuple 可以）
- 函数返回多个值
- 比 list 占用更少内存

### 基本用法

```python
# 创建元组
point = (3, 5)
rgb = (255, 128, 0)
single = (42,)       # 单元素元组必须加逗号
empty = ()

# 解包（Unpacking）
x, y = point
print(x, y)  # 3 5

# 函数返回多值
def get_dimensions():
    return 1920, 1080  # 实际上返回的是 tuple

width, height = get_dimensions()

# 用作字典键
locations = {
    (40.7, -74.0): "New York",
    (51.5, -0.1):  "London"
}

# 命名元组（更具可读性）
from collections import namedtuple
Point = namedtuple("Point", ["x", "y"])
p = Point(3, 5)
print(p.x, p.y)  # 3 5
```

### list vs tuple 对比

| 特性 | list | tuple |
|------|------|-------|
| 可变性 | ✅ 可变 | ❌ 不可变 |
| 可哈希 | ❌ | ✅ |
| 内存占用 | 较多 | 较少 |
| 迭代速度 | 略慢 | 略快 |
| 适用场景 | 动态数据集合 | 固定数据记录 |

---

## `dict` — 字典

### 是什么

`dict` 是**键值对（key-value）**映射类型，**无序（Python 3.7+ 保持插入顺序）、可变、键唯一**。

### 何时使用

- 需要通过唯一键快速查找值
- 存储结构化数据（类似 JSON）
- 统计、分组、缓存场景

### 基本用法

```python
# 创建字典
person = {"name": "Alice", "age": 30, "city": "Singapore"}
empty = {}
from_keys = dict.fromkeys(["a", "b", "c"], 0)  # {'a': 0, 'b': 0, 'c': 0}

# 访问
print(person["name"])               # Alice（键不存在时抛出 KeyError）
print(person.get("email", "N/A"))   # N/A（键不存在时返回默认值，推荐使用）

# 增/改
person["email"] = "alice@example.com"
person.update({"age": 31, "phone": "1234"})

# 删
del person["phone"]
age = person.pop("age", None)  # 删除并返回值，键不存在时返回 None

# 遍历
for key in person:
    print(key)

for key, value in person.items():
    print(f"{key}: {value}")

for value in person.values():
    print(value)
```

### 字典推导式

```python
# 平方映射
squares = {x: x**2 for x in range(6)}
# {0: 0, 1: 1, 2: 4, 3: 9, 4: 16, 5: 25}

# 过滤
adults = {name: age for name, age in {"Alice": 30, "Bob": 15}.items() if age >= 18}
# {'Alice': 30}

# 键值反转
original = {"a": 1, "b": 2}
reversed_dict = {v: k for k, v in original.items()}
# {1: 'a', 2: 'b'}
```

### 合并字典（Python 3.9+）

```python
d1 = {"a": 1, "b": 2}
d2 = {"b": 3, "c": 4}

merged = d1 | d2          # {'a': 1, 'b': 3, 'c': 4}（d2 覆盖 d1）
d1 |= d2                  # 原地合并
```

### 性能特性

| 操作 | 时间复杂度 |
|------|-----------|
| 查找 `dict[key]` | O(1) 均摊 |
| 插入 | O(1) 均摊 |
| 删除 | O(1) 均摊 |
| 遍历 | O(n) |

---

## `set` — 集合

### 是什么

`set` 是**无序、不重复**的元素集合，支持高效的集合运算。

### 何时使用

- 去重
- 快速判断成员资格
- 集合运算（交集、并集、差集）

### 基本用法

```python
# 创建集合
fruits = {"apple", "banana", "cherry", "apple"}  # 自动去重
print(fruits)  # {'apple', 'banana', 'cherry'}

# 注意：空集合必须用 set()，不能用 {}（那是空 dict）
empty = set()

# 增删
fruits.add("mango")
fruits.discard("banana")  # 不存在时不报错（推荐）
fruits.remove("cherry")   # 不存在时抛出 KeyError

# 成员检查（O(1)，比 list 快得多）
print("apple" in fruits)  # True
```

### 集合运算

```python
a = {1, 2, 3, 4}
b = {3, 4, 5, 6}

print(a | b)   # 并集：{1, 2, 3, 4, 5, 6}
print(a & b)   # 交集：{3, 4}
print(a - b)   # 差集：{1, 2}（在 a 但不在 b）
print(a ^ b)   # 对称差集：{1, 2, 5, 6}（仅属于其中一个）

# 子集和超集
print({1, 2} <= a)  # True（是 a 的子集）
print(a >= {1, 2})  # True（a 是其超集）

# 去重示例
names = ["Alice", "Bob", "Alice", "Charlie", "Bob"]
unique = list(set(names))
print(unique)  # ['Alice', 'Bob', 'Charlie']（顺序不保证）
```

---

## `deque` — 双端队列

### 是什么

`deque`（double-ended queue）来自 `collections` 模块，支持在**两端以 O(1) 时间复杂度**进行插入和删除。

### 何时使用

- 实现队列（FIFO）或栈（LIFO）
- 需要频繁在头部插入/删除（list 此操作为 O(n)）
- 滑动窗口算法
- 保留最近 N 条记录（设置 `maxlen`）

### 基本用法

```python
from collections import deque

# 创建
dq = deque([1, 2, 3])
dq_limited = deque(maxlen=3)  # 限制最大长度，超出自动丢弃旧元素

# 两端操作
dq.append(4)        # 右端追加：deque([1, 2, 3, 4])
dq.appendleft(0)    # 左端追加：deque([0, 1, 2, 3, 4])
dq.pop()            # 右端弹出：返回 4
dq.popleft()        # 左端弹出：返回 0

# 旋转
dq = deque([1, 2, 3, 4, 5])
dq.rotate(2)   # 右旋 2 步：deque([4, 5, 1, 2, 3])
dq.rotate(-1)  # 左旋 1 步：deque([5, 1, 2, 3, 4])
```

### 实战：用 maxlen 保留最近记录

```python
from collections import deque

# 浏览历史（只保留最近 5 条）
history = deque(maxlen=5)
for page in ["home", "about", "blog", "contact", "projects", "home"]:
    history.append(page)

print(history)
# deque(['about', 'blog', 'contact', 'projects', 'home'], maxlen=5)
```

### 实战：BFS（广度优先搜索）

```python
from collections import deque

def bfs(graph, start):
    visited = set()
    queue = deque([start])
    order = []

    while queue:
        node = queue.popleft()  # O(1)，比 list.pop(0) 快
        if node not in visited:
            visited.add(node)
            order.append(node)
            queue.extend(graph[node])

    return order
```

### deque vs list 性能对比

| 操作 | list | deque |
|------|------|-------|
| 末尾追加 | O(1) | O(1) |
| 末尾弹出 | O(1) | O(1) |
| **头部插入** | **O(n)** | **O(1)** |
| **头部弹出** | **O(n)** | **O(1)** |
| 随机访问 | O(1) | O(n) |

---

## `defaultdict` — 带默认值的字典

### 是什么

`defaultdict` 来自 `collections` 模块，是 `dict` 的子类。访问**不存在的键**时，会自动用指定工厂函数创建默认值，避免 `KeyError`。

### 何时使用

- 分组/归类数据
- 统计频次（配合 `int`）
- 构建多值映射（配合 `list`）
- 嵌套数据结构

### 基本用法

```python
from collections import defaultdict

# int 作为工厂：默认值为 0，适合计数
word_count = defaultdict(int)
for word in ["apple", "banana", "apple", "cherry", "banana", "apple"]:
    word_count[word] += 1

print(dict(word_count))
# {'apple': 3, 'banana': 2, 'cherry': 1}

# list 作为工厂：默认值为 []，适合分组
from_city = defaultdict(list)
trips = [("Alice", "Singapore"), ("Bob", "Tokyo"), ("Alice", "London")]
for person, city in trips:
    from_city[person].append(city)

print(dict(from_city))
# {'Alice': ['Singapore', 'London'], 'Bob': ['Tokyo']}

# set 作为工厂：默认值为 set()，去重分组
tags = defaultdict(set)
data = [("Python", "OOP"), ("Python", "FP"), ("Go", "concurrent")]
for lang, feature in data:
    tags[lang].add(feature)

print(dict(tags))
# {'Python': {'OOP', 'FP'}, 'Go': {'concurrent'}}
```

### 嵌套 defaultdict

```python
from collections import defaultdict

# 构建嵌套结构（如邻接表）
graph = defaultdict(lambda: defaultdict(int))
graph["A"]["B"] += 1
graph["A"]["C"] += 1
graph["B"]["C"] += 2

print(graph["A"])  # defaultdict(<class 'int'>, {'B': 1, 'C': 1})
```

### defaultdict vs 普通 dict

```python
# 普通 dict 需要额外判断
counts = {}
for word in words:
    if word not in counts:       # 需要检查键是否存在
        counts[word] = 0
    counts[word] += 1

# defaultdict 更简洁
counts = defaultdict(int)
for word in words:
    counts[word] += 1            # 直接操作，无需判断
```

---

## `OrderedDict` — 有序字典

### 是什么

`OrderedDict` 来自 `collections` 模块，明确**保证键的插入顺序**，并提供针对顺序的额外方法。

> **注意**：Python 3.7+ 起，普通 `dict` 也保持插入顺序，但 `OrderedDict` 仍有其特殊用途。

### 何时使用

- 需要 `move_to_end` 等顺序操作
- 实现 LRU 缓存
- 需要顺序敏感的相等性比较

### 基本用法

```python
from collections import OrderedDict

od = OrderedDict()
od["first"] = 1
od["second"] = 2
od["third"] = 3

# 移动元素到末尾/开头
od.move_to_end("first")          # 移到末尾
od.move_to_end("third", last=False)  # 移到开头

print(list(od.keys()))  # ['third', 'second', 'first']

# 顺序敏感的比较
od1 = OrderedDict([("a", 1), ("b", 2)])
od2 = OrderedDict([("b", 2), ("a", 1)])
print(od1 == od2)  # False（顺序不同）

d1 = {"a": 1, "b": 2}
d2 = {"b": 2, "a": 1}
print(d1 == d2)    # True（普通 dict 不关心顺序）
```

### 实战：LRU 缓存

```python
from collections import OrderedDict

class LRUCache:
    def __init__(self, capacity: int):
        self.cache = OrderedDict()
        self.capacity = capacity

    def get(self, key):
        if key not in self.cache:
            return -1
        self.cache.move_to_end(key)  # 标记为最近使用
        return self.cache[key]

    def put(self, key, value):
        if key in self.cache:
            self.cache.move_to_end(key)
        self.cache[key] = value
        if len(self.cache) > self.capacity:
            self.cache.popitem(last=False)  # 移除最久未使用的

cache = LRUCache(2)
cache.put(1, "one")
cache.put(2, "two")
print(cache.get(1))   # "one"，并标记 1 为最近使用
cache.put(3, "three") # 容量满，淘汰键 2
print(cache.get(2))   # -1（已被淘汰）
```

---

## `Counter` — 计数器

### 是什么

`Counter` 来自 `collections` 模块，专门用于**统计元素出现频次**，是 `dict` 的子类。

### 何时使用

- 统计词频、字符频率
- 找出最常见元素
- 两个序列的频率对比

### 基本用法

```python
from collections import Counter

# 创建
c = Counter("mississippi")
print(c)  # Counter({'i': 4, 's': 4, 'p': 2, 'm': 1})

words = ["apple", "banana", "apple", "cherry", "banana", "apple"]
wc = Counter(words)
print(wc)  # Counter({'apple': 3, 'banana': 2, 'cherry': 1})

# 最常见的 N 个
print(wc.most_common(2))  # [('apple', 3), ('banana', 2)]

# 更新计数
wc.update(["apple", "mango"])
print(wc["apple"])  # 4

# 算术运算
c1 = Counter({"a": 3, "b": 2})
c2 = Counter({"a": 1, "b": 4, "c": 1})

print(c1 + c2)  # Counter({'b': 6, 'a': 4, 'c': 1})
print(c1 - c2)  # Counter({'a': 2})（只保留正值）
print(c1 & c2)  # Counter({'a': 1, 'b': 2})（取最小值）
print(c1 | c2)  # Counter({'b': 4, 'a': 3, 'c': 1})（取最大值）
```

### 实战：变位词检测

```python
from collections import Counter

def is_anagram(s1: str, s2: str) -> bool:
    return Counter(s1) == Counter(s2)

print(is_anagram("listen", "silent"))  # True
print(is_anagram("hello", "world"))    # False
```

---

## `heapq` — 堆（优先队列）

### 是什么

`heapq` 是 Python 的**最小堆**实现（min-heap），通过普通 list 维护堆结构。最小元素始终在 `heap[0]`。

### 何时使用

- 优先队列（任务调度）
- 找出最小/最大的 N 个元素
- Dijkstra 最短路径算法
- 合并多个有序序列

### 基本用法

```python
import heapq

# 堆化（将 list 转为堆）
nums = [3, 1, 4, 1, 5, 9, 2, 6]
heapq.heapify(nums)
print(nums[0])  # 1（最小元素）

# 推入元素
heapq.heappush(nums, 0)
print(nums[0])  # 0

# 弹出最小元素
smallest = heapq.heappop(nums)
print(smallest)  # 0

# 获取最小/最大的 N 个元素
data = [3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5]
print(heapq.nsmallest(3, data))  # [1, 1, 2]
print(heapq.nlargest(3, data))   # [9, 6, 5]
```

### 最大堆（取负值模拟）

```python
import heapq

# Python 只有最小堆，用负值模拟最大堆
max_heap = []
for num in [3, 1, 4, 1, 5, 9]:
    heapq.heappush(max_heap, -num)

largest = -heapq.heappop(max_heap)
print(largest)  # 9
```

### 实战：优先任务队列

```python
import heapq

class PriorityQueue:
    def __init__(self):
        self._queue = []
        self._index = 0

    def push(self, priority, task):
        # 优先级越小越先执行；_index 用于相同优先级时保持 FIFO
        heapq.heappush(self._queue, (priority, self._index, task))
        self._index += 1

    def pop(self):
        priority, _, task = heapq.heappop(self._queue)
        return task

pq = PriorityQueue()
pq.push(3, "低优先级任务")
pq.push(1, "高优先级任务")
pq.push(2, "中优先级任务")

print(pq.pop())  # 高优先级任务
print(pq.pop())  # 中优先级任务
```

---

## 选型速查表

| 需求场景 | 推荐数据结构 | 原因 |
|---------|-------------|------|
| 有序、可变的元素集合 | `list` | 通用序列，功能完备 |
| 固定不变的数据记录 | `tuple` | 不可变、可哈希、省内存 |
| 键值查找 | `dict` | O(1) 查找 |
| 键不存在时自动初始化 | `defaultdict` | 省去键存在性检查 |
| 元素统计/计频 | `Counter` | 专为计数设计，API 丰富 |
| 保证键顺序并需顺序操作 | `OrderedDict` | `move_to_end` 等方法 |
| 头尾频繁插入/删除 | `deque` | 两端操作均为 O(1) |
| 固定容量的滑动窗口 | `deque(maxlen=N)` | 自动丢弃旧元素 |
| 去重/快速成员检查 | `set` | O(1) 查找，自动去重 |
| 最小/最大元素高效访问 | `heapq` | O(log n) 插入/弹出 |
| Top-K 问题 | `heapq.nsmallest/nlargest` | 比全排序更高效 |
| LRU 缓存 | `OrderedDict` 或 `functools.lru_cache` | 顺序感知 |

---

## 快速导入参考

```python
# 内置（无需导入）
list, tuple, dict, set

# collections 模块
from collections import deque, defaultdict, OrderedDict, Counter, namedtuple

# heapq 模块
import heapq
```

---

*文档版本：Python 3.10+*  
*参考：[Python 官方文档 - collections](https://docs.python.org/3/library/collections.html) | [heapq](https://docs.python.org/3/library/heapq.html)*
