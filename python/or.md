你这个问题问到了关键！`or` 确实是「或」的意思，但在 Python 里，它的行为和我们日常理解的「逻辑或」有一点区别——**它不只是返回 True/False，还会返回具体的取值**，这也是新手最容易困惑的点。

### 先澄清：`or` 的两层含义
`or` 本质还是「或」，但 Python 对它的实现是「短路求值 + 返回实际值」，而非单纯返回布尔值。我们分两步讲清楚：

#### 1. 先理解「逻辑或」的本质
日常逻辑中，`A or B` 的规则是：
- 只要 A 为真，结果就为真；
- 只有 A 为假时，才看 B。

Python 完全遵循这个逻辑，但额外做了一件事：**不返回 True/False，而是返回「导致结果为真的那个值」**。

#### 2. 拆解你的代码
```python
self.token = token or config.MINERU_TOKEN
```
这里的 `or` 依然是「或」的逻辑，只是结果不是布尔值，而是具体的变量值：
- 逻辑判断：先看 `token` 是否为真（非空、非 0、非 False 等）；
- 取值返回：
  - 如果 `token` 为真 → 「或逻辑」的结果就是 `token`（因为只要第一个为真，结果就为真），所以把 `token` 赋值给 `self.token`；
  - 如果 `token` 为假 → 继续看第二个值 `config.MINERU_TOKEN`，把它赋值给 `self.token`（不管它是不是真，都会返回它）。

### 举个具体例子（更易理解）
假设：
```python
# 场景1：传入了有效的 token
token = "abc123"
config.MINERU_TOKEN = "default_token"
self.token = token or config.MINERU_TOKEN
# 结果：self.token = "abc123"（因为 token 为真，或逻辑直接返回它）

# 场景2：传入的 token 是空值
token = None
config.MINERU_TOKEN = "default_token"
self.token = token or config.MINERU_TOKEN
# 结果：self.token = "default_token"（token 为假，或逻辑返回第二个值）
```

### 对比纯布尔型「或」（帮你区分）
如果是单纯判断真假，`or` 才返回布尔值：
```python
# 纯逻辑判断：返回 True/False
print(bool("abc123" or "default"))  # True
print(bool(None or "default"))      # True
print(bool(None or ""))             # False
```

### 总结
1. `or` 始终是「或」的意思，核心逻辑没变：只要有一个值为真，结果就为真；
2. Python 中 `or` 的特殊点：返回「第一个为真的具体值」，而非布尔值，这也是它能用来「设置默认值」的原因；
3. 你的代码本质：用「或逻辑」实现「优先用传入的 token，无有效值则用默认值」的赋值逻辑。