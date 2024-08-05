## enum

#### 不限定作用域的枚举类型

```c
enum color { red, yellow, green };
enum { floatPrec = 6, doublePrec = 10 };
```

## enum class

`enum class` 是 C++11 引入的一种强类型枚举（scoped enumeration），与传统的 `enum` 相比，具有更严格的类型检查和更好的作用域控制。以下是 `enum class` 的一些关键特点和用法示例。

### 关键特点

1. **强类型**：`enum class` 枚举类型不会隐式转换为整数类型，这有助于防止意外的类型错误。
2. **作用域控制**：`enum class` 枚举成员在枚举类型的作用域内，而不是全局作用域。这避免了名字冲突的问题。

### 定义和使用示例

#### 定义 `enum class`
```cpp
#include <iostream>

// 定义一个 enum class
enum class Color {
    Red,
    Green,
    Blue
};

int main() {
    // 声明一个 Color 类型的变量
    Color color = Color::Red;

    // 使用 switch 语句处理不同枚举值
    switch (color) {
        case Color::Red:
            std::cout << "Color is Red" << std::endl;
            break;
        case Color::Green:
            std::cout << "Color is Green" << std::endl;
            break;
        case Color::Blue:
            std::cout << "Color is Blue" << std::endl;
            break;
    }

    return 0;
}
```

在这个示例中，`Color` 是一个 `enum class`，其成员 `Red`, `Green`, `Blue` 必须通过 `Color::` 来访问，从而避免了命名冲突。

#### 与传统 `enum` 的对比
```cpp
#include <iostream>

// 传统 enum
enum OldColor {
    Red,
    Green,
    Blue
};

// enum class
enum class NewColor {
    Red,
    Green,
    Blue
};

int main() {
    OldColor oldColor = Red; // 无需作用域限定符
    NewColor newColor = NewColor::Red; // 需要作用域限定符

    // 隐式转换为整数（传统 enum）
    int oldValue = oldColor; // OK

    // 隐式转换为整数（enum class）
    // int newValue = newColor; // 编译错误

    // 显式转换为整数（enum class）
    int newValue = static_cast<int>(newColor); // OK

    std::cout << "Old value: " << oldValue << std::endl;
    std::cout << "New value: " << newValue << std::endl;

    return 0;
}
```

在这个示例中，传统的 `enum` 可以隐式转换为整数，而 `enum class` 则需要显式转换。

#### 指定基础类型
可以为 `enum class` 指定基础类型（默认是 `int`）：
```cpp
#include <iostream>

enum class Fruit : char {
    Apple = 'A',
    Orange = 'O',
    Banana = 'B'
};

int main() {
    Fruit fruit = Fruit::Apple;

    char fruitChar = static_cast<char>(fruit);
    
    std::cout << "Fruit char: " << fruitChar << std::endl;

    return 0;
}
```

在这个示例中，`Fruit` 的基础类型被指定为 `char`，并且可以通过显式转换将枚举值转换为字符。

### 总结

- **强类型**：避免了意外的隐式转换。
- **作用域控制**：避免了名称冲突。
- **可指定基础类型**：灵活性更高。

总之，使用 `enum class` 可以提供更安全和更清晰的代码结构，是现代 C++ 编程中的推荐做法。