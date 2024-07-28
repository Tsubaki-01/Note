在 C++ 中，`struct` 和 `class` 都用于定义数据结构，但它们之间有一些关键的区别。主要的区别在于默认的访问控制权限和一些语义上的细微差别。以下是详细解释：

### 1. 默认访问控制权限

- **`struct`**：默认情况下，成员（包括数据成员和成员函数）的访问控制权限是 `public`。
- **`class`**：默认情况下，成员（包括数据成员和成员函数）的访问控制权限是 `private`。

```cpp
struct MyStruct {
    int x; // 默认是 public
};

class MyClass {
    int x; // 默认是 private
};
```

在上述代码中：

- 在 `MyStruct` 中，`x` 是公有的，可以直接通过对象访问。
- 在 `MyClass` 中，`x` 是私有的，不能直接通过对象访问。

### 2. 继承时的默认访问控制权限

- **`struct`**：当使用 `struct` 继承时，默认的继承方式是 `public`。
- **`class`**：当使用 `class` 继承时，默认的继承方式是 `private`。

```cpp
struct Base1 {};
struct Derived1 : Base1 {}; // 默认 public 继承

class Base2 {};
class Derived2 : Base2 {}; // 默认 private 继承
```

在上述代码中：

- `Derived1` 从 `Base1` 公有继承。
- `Derived2` 从 `Base2` 私有继承。

### 3. 用法和语义上的差异

虽然在语法上可以互换使用，但在实际编程中，通常会遵循以下约定：

- **使用 `struct`**：通常用于定义简单的数据结构或 POD（Plain Old Data）类型，其中主要包含公共数据成员，没有复杂的行为或封装需求。

```cpp
struct Point {
    int x;
    int y;
};
```

- **使用 `class`**：通常用于定义具有封装、继承和多态特性的复杂类型，包括私有数据成员、公共接口和保护机制。

```cpp
class Rectangle {
private:
    int width;
    int height;

public:
    Rectangle(int w, int h) : width(w), height(h) {}

    int area() const {
        return width * height;
    }
};
```

### 总结

尽管在 C++ 中 `struct` 和 `class` 在很多方面是等价的，但它们之间的主要区别在于默认的访问控制权限和继承方式。此外，在编程实践中，它们通常用于不同类型的数据结构，以反映其设计意图：

- 使用 `struct`: 定义简单的数据结构，所有成员默认是公有。
- 使用 `class`: 定义复杂的数据类型，强调封装和信息隐藏，所有成员默认是私有。

这种区分不仅帮助保持代码清晰，还能更好地传达设计意图。