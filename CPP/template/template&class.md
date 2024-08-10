### 1. 类模板中的虚函数

**类模板**是一个泛型类，它的成员函数可以是虚函数。这意味着你可以在类模板中定义虚函数，并在派生类中重写这些虚函数。

**示例：**

```c++
cpp复制代码#include <iostream>

template <typename T>
class Base {
public:
    virtual void func() {
        std::cout << "Base class function" << std::endl;
    }
};

class Derived : public Base<int> {
public:
    void func() override {
        std::cout << "Derived class function" << std::endl;
    }
};

int main() {
    Base<int>* base = new Derived();
    base->func();  // 输出 "Derived class function"
    delete base;
    return 0;
}
```

在这个例子中，`Base`是一个类模板，`func`是一个虚函数，派生类`Derived`对其进行了重写。

### 2. 成员模板不能是虚函数

**成员模板**是类中的成员函数，并且是模板函数。它本身也是一个模板，但与类模板不同，它不能是虚函数。这是因为C++的虚函数机制依赖于固定的函数签名和虚函数表（vtable），而成员模板的类型是在实例化时才确定的，因此无法在编译时生成对应的虚函数表。

**示例：**

```c++
cpp复制代码template <typename T>
class MyClass {
public:
    template <typename U>
    void myFunc(U u) {
        // do something
    }

    // 错误的代码：成员模板不能是虚函数
    // virtual template <typename U>
    // void myVirtualFunc(U u);
};
```

上述注释掉的代码是不合法的，编译器会报错，因为C++不允许成员模板为虚函数。

### 总结：

- **类模板**中的成员函数可以是虚函数。
- **成员模板**（即模板成员函数）不能是虚函数。