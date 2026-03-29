你想了解 Python 中 `collections` 库提供的常用数据结构，这个库确实是 Python 内置的宝藏工具，它扩展了基础数据结构（列表、字典、元组等）的功能，能解决很多实际开发中的痛点问题。

下面我会由浅入深地介绍 `collections` 中最常用的几个数据结构，每个结构都会包含**核心作用**、**代码示例**和**适用场景**，让你能快速理解和使用。

### 1. Counter（计数器）
#### 核心作用
专门用于**可哈希对象的计数**，本质是字典的子类，键是元素，值是该元素出现的次数，比手动用字典统计更简洁高效。

#### 代码示例
```python
from collections import Counter

# 1. 统计可迭代对象（字符串、列表等）的元素频次
text = "hello python hello world"
count = Counter(text.split())  # 按空格分割字符串统计单词
print(count)  # 输出: Counter({'hello': 2, 'python': 1, 'world': 1})

# 2. 常用方法
print(count.most_common(2))  # 获取出现次数最多的2个元素：[('hello', 2), ('python', 1)]
count.update(["python", "python"])  # 更新计数（新增/累加）
print(count["python"])  # 输出: 3

# 3. 转为普通字典
print(dict(count))  # 输出: {'hello': 2, 'python': 3, 'world': 1}
```

#### 适用场景
- 统计文本中单词/字符出现次数
- 统计列表中元素的重复次数
- 数据清洗中的频次分析

### 2. defaultdict（默认字典）
#### 核心作用
解决普通字典访问**不存在的键时抛出 KeyError** 的问题，它会为不存在的键自动创建一个默认值（由你指定的类型/函数决定），同样是字典的子类。

#### 代码示例
```python
from collections import defaultdict

# 1. 指定默认值类型（list、int、set等）
# 场景：将列表元素按首字母分组
words = ["apple", "banana", "cat", "ant", "book"]
grouped = defaultdict(list)  # 不存在的键默认值是空列表
for word in words:
    key = word[0]
    grouped[key].append(word)  # 无需判断key是否存在，直接追加

print(dict(grouped))
# 输出: {'a': ['apple', 'ant'], 'b': ['banana', 'book'], 'c': ['cat']}

# 2. 指定int类型（默认值0），替代Counter做简单计数
num_count = defaultdict(int)
nums = [1, 2, 1, 3, 2, 1]
for num in nums:
    num_count[num] += 1
print(num_count[4])  # 不存在的键返回0，不会报错：输出 0
```

#### 适用场景
- 分组统计（如按类别聚合数据）
- 避免手动写 `if key not in dict: dict[key] = []` 这类冗余代码
- 计数场景（默认值为0）

### 3. deque（双端队列）
#### 核心作用
双端队列，支持**从两端快速添加/删除元素**（时间复杂度 O(1)），而普通列表的 `pop(0)`/`insert(0, x)` 是 O(n)，效率远低于 deque。

#### 代码示例
```python
from collections import deque

# 1. 创建deque
dq = deque([1, 2, 3])

# 2. 两端操作（核心优势）
dq.append(4)  # 右端添加：deque([1,2,3,4])
dq.appendleft(0)  # 左端添加：deque([0,1,2,3,4])
dq.pop()  # 右端删除：返回4，deque([0,1,2,3])
dq.popleft()  # 左端删除：返回0，deque([1,2,3])

# 3. 其他实用方法
dq.rotate(1)  # 向右旋转1位：deque([3,1,2])
dq.rotate(-1)  # 向左旋转1位：恢复 deque([1,2,3])
dq.clear()  # 清空
dq.extend([4,5])  # 右端批量添加：deque([4,5])

# 4. 限制长度（超出自动丢弃另一端元素）
limited_dq = deque(maxlen=3)
limited_dq.extend([1,2,3])
limited_dq.append(4)  # 超出maxlen，左端丢弃1：deque([2,3,4])
```

#### 适用场景
- 队列/栈实现（如广度优先搜索 BFS）
- 滑动窗口（如最近N条日志/数据的缓存）
- 需要频繁在列表头部操作的场景（替代普通列表提升效率）

### 4. namedtuple（命名元组）
#### 核心作用
创建**有名字的元组**，既保留元组不可变的特性，又能通过属性名访问元素（而非下标），比普通元组更易读，比类更轻量（无需定义 __init__）。

#### 代码示例
```python
from collections import namedtuple

# 1. 定义命名元组（格式：名称，字段名（字符串/列表））
Point = namedtuple("Point", ["x", "y"])  # 或 "x y"
Person = namedtuple("Person", "name age gender")

# 2. 创建实例
p1 = Point(10, 20)
p2 = Person(name="Alice", age=25, gender="female")

# 3. 访问元素（两种方式：属性名/下标）
print(p1.x)  # 输出 10（推荐，易读）
print(p1[1])  # 输出 20（兼容元组方式）
print(p2.name)  # 输出 Alice

# 4. 实用方法
print(p1._asdict())  # 转为有序字典：{'x':10, 'y':20}
p3 = p2._replace(age=26)  # 生成新实例（元组不可变，replace返回新对象）
print(p3.age)  # 输出 26
```

#### 适用场景
- 替代普通元组（提升代码可读性，如坐标、点、记录类数据）
- 轻量级数据容器（无需定义完整的类）
- 数据库查询结果映射（字段名对应属性名）

### 5. OrderedDict（有序字典，Python 3.7+ 已弱化）
#### 核心作用
在 Python 3.7 之前，普通字典不保证插入顺序，OrderedDict 可以**保留键的插入顺序**；Python 3.7+ 中普通字典已默认有序，OrderedDict 仅保留一些特殊方法（如 `move_to_end`）。

#### 代码示例
```python
from collections import OrderedDict

od = OrderedDict()
od["a"] = 1
od["b"] = 2
od["c"] = 3

# 特殊方法：将指定键移到末尾
od.move_to_end("a")
print(list(od.keys()))  # 输出: ['b', 'c', 'a']

# 移到开头
od.move_to_end("a", last=False)
print(list(od.keys()))  # 输出: ['a', 'b', 'c']
```

#### 适用场景
- Python 3.6 及以下版本需要有序字典的场景
- 需要手动调整键顺序（如 `move_to_end`）的场景

---

### 总结
1. **Counter**：专注元素计数，简化频次统计，是字典的子类；
2. **defaultdict**：解决字典键不存在报错问题，自动生成默认值，适合分组/计数；
3. **deque**：双端队列，两端操作效率极高，适合队列/栈/滑动窗口场景；
4. **namedtuple**：带属性名的元组，不可变且易读，替代普通元组提升代码可读性；
5. **OrderedDict**：Python 3.7+ 仅在需要调整键顺序时使用，日常用普通字典即可。

这些数据结构都是对 Python 基础数据类型的补充，核心优势是**简化代码、提升效率、增强可读性**，在实际开发中能大幅减少冗余代码，建议根据场景灵活选用。