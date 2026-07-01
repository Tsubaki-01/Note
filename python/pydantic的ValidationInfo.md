### ValidationInfo 概述
`ValidationInfo` 是 **Pydantic v2** 验证体系中的核心上下文对象，会作为参数自动传入字段验证器、模型验证器、自定义验证函数中，提供验证过程的元数据、已验证字段数据、全局自定义上下文等信息，替代了 Pydantic v1 中验证器的 `values` 字典。

导入方式：
```python
from pydantic import ValidationInfo
```

---

### 核心属性
`ValidationInfo` 为只读对象，包含以下常用属性：

| 属性 | 类型 | 说明 |
|------|------|------|
| `data` | `dict[str, Any]` | 已成功验证的字段数据，仅包含**当前字段之前**、按模型声明顺序验证完成的字段 |
| `field_name` | `str \| None` | 当前正在验证的字段名称；字段验证器中有效，模型级验证器中为 `None` |
| `mode` | `Literal['python', 'json', 'strings']` | 当前验证模式：<br>- `python`：对应 `model_validate()` 传入 Python 对象<br>- `json`：对应 `model_validate_json()` 传入 JSON 字符串<br>- `strings`：对应 `model_validate_strings()` 纯字符串场景 |
| `context` | `Any` | 用户自定义全局上下文，通过 `model_validate(..., context=xxx)` 传入，**==全验证流程共享==**，会**自动沿嵌套模型层级向下透传** |
| `config` | `ConfigDict` | 当前模型的配置字典，即 `model_config` 的完整内容 |

---

### 典型使用场景与示例

#### 1. 字段间依赖验证（最常用）
利用 `info.data` 获取已验证字段的值，实现跨字段校验。
> ⚠️ 注意：字段验证按模型声明顺序执行，只能获取**当前字段之前**的字段数据。

```python
from pydantic import BaseModel, field_validator, ValidationInfo

class RegisterForm(BaseModel):
    password: str
    confirm_password: str

    @field_validator("confirm_password")
    @classmethod
    def check_password_match(cls, value: str, info: ValidationInfo) -> str:
        # 从 info.data 中取前面已经验证过的 password 字段
        if "password" in info.data and value != info.data["password"]:
            raise ValueError("两次输入的密码不一致")
        return value

# 验证通过
RegisterForm(password="abc123", confirm_password="abc123")
# 抛出 ValidationError：两次输入的密码不一致
RegisterForm(password="abc123", confirm_password="abc456")
```

#### 2. 传递自定义全局上下文
通过 `context` 向验证器传入动态配置（如最小长度、租户ID、业务规则开关等），无需硬编码校验规则。

```python
from pydantic import BaseModel, field_validator, ValidationInfo

class User(BaseModel):
    username: str

    @field_validator("username")
    @classmethod
    def validate_username_length(cls, value: str, info: ValidationInfo) -> str:
        # 从上下文读取最小长度配置，无则默认 3
        min_len = info.context.get("min_username_length", 3) if info.context else 3
        if len(value) < min_len:
            raise ValueError(f"用户名长度不能少于 {min_len} 位")
        return value

# 传入自定义上下文，抛出错误：用户名长度不能少于 5 位
User.model_validate(
    {"username": "ab"},
    context={"min_username_length": 5}
)
```

#### 3. 编写通用可复用验证器
利用 `field_name` 实现一个验证器复用于多个字段，错误提示自动适配当前字段名。

```python
from pydantic import BaseModel, ValidationInfo
from typing import Annotated
from pydantic import WithValidator

# 定义可复用的字符串去空+非空验证
def non_empty_strip(value: str, info: ValidationInfo) -> str:
    if not isinstance(value, str):
        return value
    stripped = value.strip()
    if not stripped:
        raise ValueError(f"{info.field_name} 不能是空字符串")
    return stripped

# 封装为 Annotated 类型，任意模型可直接复用
NonEmptyStr = Annotated[str, WithValidator(non_empty_strip)]

class Product(BaseModel):
    title: NonEmptyStr
    description: NonEmptyStr
```

#### 4. 模型级验证中使用
`@model_validator` 同样支持 `ValidationInfo`，适合全字段校验 + 全局上下文结合的场景。

```python
from pydantic import BaseModel, model_validator, ValidationInfo

class Order(BaseModel):
    start_date: str
    end_date: str

    @model_validator(mode="after")
    def check_date_range(self, info: ValidationInfo) -> "Order":
        # after 模式下可直接访问 self 全部属性，同时通过 info 获取上下文
        if self.start_date > self.end_date:
            raise ValueError("结束日期不能早于开始日期")
        return self
```

---

### 重要注意事项
1. **字段验证顺序**：字段验证严格按照模型中字段的声明顺序执行，`info.data` 仅包含当前字段之前已验证通过的字段。若需要访问全部字段，请使用 `mode="after"` 的模型验证器。
2. **只读特性**：`ValidationInfo` 是只读对象，修改其 `data`、`context` 等属性不会影响实际验证流程。
