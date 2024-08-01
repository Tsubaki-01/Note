在面向对象编程中，如果父类有多个重载函数，而子类只需要重写其中的一个，可以通过在子类中定义具有相同签名的函数并使用 `override` 关键字来实现。其余未重写的重载函数将会继承自父类。

以下是一个示例，展示了如何在子类中只重写父类的某个重载函数：

### 示例代码

```cpp
#include <iostream>

class Base {
public:
    // 多个重载函数
    virtual void display(int i) const {
        std::cout << "Base display(int): " << i << std::endl;
    }

    virtual void display(double d) const {
        std::cout << "Base display(double): " << d << std::endl;
    }

    virtual void display(const std::string& s) const {
        std::cout << "Base display(string): " << s << std::endl;
    }
};

class Derived : public Base {
public:
    // 只重写其中一个重载函数
    void display(int i) const override {
        std::cout << "Derived display(int): " << i << std::endl;
    }
};

int main() {
    Derived d;
    Base* b = &d;

    // 调用不同的重载函数
    b->display(42);          // 调用 Derived::display(int)
    b->display(3.14);        // 调用 Base::display(double)
    b->display("Hello");     // 调用 Base::display(string)

    return 0;
}
```

### 解释

1. **父类 `Base`**：
   - 定义了三个重载的 `display` 函数，分别接受 `int`、`double` 和 `std::string` 类型的参数。
   - 这些函数被定义为虚函数（`virtual`），以便可以在子类中进行重写。

2. **子类 `Derived`**：
   - 只重写了接受 `int` 参数的 `display` 函数，并使用 `override` 关键字明确表示这是对基类虚函数的重写。

3. **主程序 (`main`)**：
   - 创建一个 `Derived` 类的实例 `d`，并通过基类指针 `b` 指向该实例。
   - 当调用 `b->display(42)` 时，实际调用的是 `Derived::display(int)`，因为这个版本的函数在子类中被重写了。
   - 当调用 `b->display(3.14)` 和 `b->display("Hello")` 时，调用的是基类中的对应版本的函数，因为这些版本没有在子类中被重写。

通过这种方式，你可以在子类中选择性地重写父类中的某些重载函数，而其余未重写的函数将会自动继承自父类。