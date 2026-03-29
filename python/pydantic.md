### 一、什么是Pydantic？
Pydantic是Python生态中**最主流的数据验证与数据解析库**，核心基于Python「类型注解」实现：
- 自动校验输入数据的类型、格式、约束（比如数值范围、字符串长度）；
- 自动将不规范的数据转换为指定类型（如字符串"123"转整数123）；
- 提供清晰的错误提示，快速定位数据问题；
- 支持数据序列化/反序列化（比如字典↔模型、JSON↔模型）。

简单来说：**Pydantic让你用「类型注解」的方式，给数据定“规矩”，自动校验+格式化，告别手写大量if-else验证逻辑**。

Pydantic目前有两个主要版本：
- Pydantic v1：兼容Python 3.6+，生态更成熟；
- Pydantic v2：2023年发布，重构为Rust核心，速度提升10-50倍，推荐新项目使用（下文以v2为例）。

### 二、安装Pydantic
```bash
# 安装最新版（v2）
pip install pydantic>=2.0

# 如需JSON Schema等扩展功能，安装完整版
pip install "pydantic[full]"
```

### 三、核心概念与基础使用
#### 1. 核心类：`BaseModel`
Pydantic的所有验证规则都通过「继承`BaseModel`的自定义模型类」实现，字段通过**类型注解**定义，约束通过`Field`函数补充。

#### 2. 基础示例：验证用户数据
需求：验证一个用户数据，要求：
- 用户名：字符串，非空，长度3-20；
- 年龄：整数，≥18；
- 邮箱：符合邮箱格式；
- 注册时间：可选，默认当前时间；
- 爱好：列表，元素为字符串（可选）。

```python
from pydantic import BaseModel, Field, EmailStr
from datetime import datetime
from typing import List, Optional  # Python3.9+可直接用list，3.8及以下需用typing.List

# 定义数据模型（核心：用类型注解+Field约束）
class User(BaseModel):
    # 字段名: 类型 = Field(默认值/约束, 描述)
    username: str = Field(..., min_length=3, max_length=20, description="用户名，3-20个字符")
    age: int = Field(..., ge=18, description="年龄，至少18岁")  # ge=greater than or equal（≥）
    email: EmailStr = Field(..., description="邮箱，需符合格式")  # Pydantic内置邮箱校验类型
    register_time: datetime = Field(default_factory=datetime.now, description="注册时间，默认当前时间")
    hobbies: Optional[List[str]] = Field(None, description="爱好，可选的字符串列表")  # Optional表示可选（可传None）

# --------------------------
# 场景1：验证合法数据
# --------------------------
# 方式1：直接传参创建模型实例
valid_user = User(
    username="zhangsan",
    age=25,
    email="zhangsan@example.com",
    hobbies=["reading", "coding"]
)
print("✅ 合法数据验证通过：")
# 访问字段
print(f"用户名：{valid_user.username}")
print(f"注册时间：{valid_user.register_time}")

# 序列化：模型→字典/JSON
print("\n📝 模型转字典：")
print(valid_user.model_dump())  # Pydantic v2用model_dump()（v1是dict()）
print("\n📝 模型转JSON字符串：")
print(valid_user.model_dump_json(indent=2))  # indent=2美化格式

# --------------------------
# 场景2：验证非法数据（自动报错）
# --------------------------
try:
    # 非法数据：用户名太短、年龄小于18、邮箱格式错误
    invalid_user = User(
        username="zs",  # 长度2，违反min_length=3
        age=17,         # 17<18，违反ge=18
        email="zhangsan.example.com"  # 无@，邮箱格式错误
    )
except Exception as e:
    print("\n❌ 数据验证失败，错误详情：")
    print(e)
```

#### 输出结果说明
- 合法数据：成功创建模型实例，可正常访问字段、序列化；
- 非法数据：抛出`ValidationError`，包含**具体错误位置+原因**（比如「username字段长度必须≥3」「age必须≥18」），定位问题非常直观。

### 四、进阶用法
#### 1. 自定义验证器（实现复杂业务规则）
如果内置约束（如`ge`、`min_length`）不够，可用`field_validator`（字段级）或`model_validator`（模型级）自定义验证逻辑。

```python
from pydantic import BaseModel, Field, field_validator

class User(BaseModel):
    username: str = Field(..., min_length=3)
    password: str = Field(..., min_length=8)

    # 字段级验证器：密码必须包含数字和字母
    @field_validator("password")
    def password_must_have_num_and_letter(cls, value):
        has_num = any(char.isdigit() for char in value)
        has_letter = any(char.isalpha() for char in value)
        if not (has_num and has_letter):
            raise ValueError("密码必须同时包含数字和字母")
        return value

# 测试非法密码
try:
    User(username="lisi", password="12345678")  # 只有数字，无字母
except Exception as e:
    print("\n❌ 自定义验证失败：")
    print(e)
```

#### 2. 从字典/JSON加载数据（实战高频）
实际开发中，数据常以字典/JSON形式传入（比如API请求体、配置文件），Pydantic可直接解析并验证：

```python
# 模拟API请求的JSON数据（字典形式）
user_data = {
    "username": "wangwu",
    "age": 30,
    "email": "wangwu@example.com",
    "hobbies": ["swimming", "running"]
}

# 从字典加载并验证
user = User(**user_data)  # 方式1：解包字典
# 或 user = User.model_validate(user_data)  # 方式2：显式调用validate

# 从JSON字符串加载
user_json = '{"username":"zhaoliu","age":28,"email":"zhaoliu@example.com"}'
user_from_json = User.model_validate_json(user_json)
```

#### 3. 类型自动转换（Pydantic的核心优势）
Pydantic会自动将「兼容类型」转为目标类型，无需手动转换：

```python
class Demo(BaseModel):
    num: int
    dt: datetime

# 字符串"123"自动转int，字符串"2024-01-01"自动转datetime
demo = Demo(num="123", dt="2024-01-01")
print(demo.num, type(demo.num))  # 123 <class 'int'>
print(demo.dt, type(demo.dt))    # 2024-01-01 00:00:00 <class 'datetime.datetime'>
```

#### 4. 嵌套模型（处理复杂数据结构）
如果数据包含嵌套结构（比如用户包含地址），可定义嵌套模型：

```python
class Address(BaseModel):
    province: str
    city: str
    detail: Optional[str] = None

class UserWithAddress(BaseModel):
    username: str
    age: int
    address: Address  # 嵌套Address模型

# 验证嵌套数据
user_data = {
    "username": "qianqi",
    "age": 22,
    "address": {
        "province": "广东省",
        "city": "深圳市"
    }
}
user = UserWithAddress(**user_data)
print(user.address.city)  # 深圳市
```

### 五、Pydantic的核心适用场景
1. **API接口数据校验**：FastAPI框架原生集成Pydantic，自动校验请求体/参数，生成接口文档；
2. **配置管理**：替代`configparser`/`yaml`手动解析，用模型校验配置文件的合法性；
3. **数据解析与清洗**：处理外部数据（如CSV/Excel/数据库返回的字典），自动校验+类型转换；
4. **团队协作规范**：通过模型定义统一数据格式，避免因数据类型不一致导致的bug；
5. **ORM模型校验**：结合SQLAlchemy等ORM，校验入库数据的合法性。

### 总结
1. **核心价值**：Pydantic通过「类型注解+BaseModel」实现自动化数据验证、类型转换，替代手写大量验证逻辑，提升代码健壮性；
2. **核心流程**：定义继承`BaseModel`的模型（指定字段类型+约束）→ 用`model_validate()`/直接实例化验证数据 → 处理验证结果（正常使用/捕获错误）；
3. **关键技巧**：用`Field`设置基础约束、用`field_validator`实现自定义规则、支持字典/JSON直接加载、支持嵌套模型处理复杂数据。