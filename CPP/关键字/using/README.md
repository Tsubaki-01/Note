# using那些事
## 关于作者：

个人公众号：

![](../img/wechat.jpg)

## 基本使用

局部与全局using，具体操作与使用见下面案例：

```c++
#include <iostream>
#define isNs1 1
//#define isGlobal 2
using namespace std;
void func() 
{
    cout<<"::func"<<endl;
}

namespace ns1 {
    void func()
    {
        cout<<"ns1::func"<<endl; 
    }
}

namespace ns2 {
#ifdef isNs1 
    using ns1::func;    /// ns1中的函数
#elif isGlobal
    using ::func; /// 全局中的函数
#else
    void func() 
    {
        cout<<"other::func"<<endl; 
    }
#endif
}

int main() 
{
    /**
     * 这就是为什么在c++中使用了cmath而不是math.h头文件
     */
    ns2::func(); // 会根据当前环境定义宏的不同来调用不同命名空间下的func()函数
    return 0;
}
```
完整代码见：[using_global.cpp](using_global.cpp)
## 改变访问性

```c++
class Base{
public:
 std::size_t size() const { return n;  }
protected:
 std::size_t n;
};
class Derived : private Base {
public:
 using Base::size;
protected:
 using Base::n;
};
```

类Derived私有继承了Base，对于它来说成员变量n和成员函数size都是私有的，如果使用了using语句，可以改变他们的可访问性，如上述例子中，size可以按public的权限访问，n可以按protected的权限访问。
完整代码见：[derived_base.cpp](derived_base.cpp)

### 构造函数中的using

在 C++11 中，派生类能够重用其直接基类定义的构造函数。

```c++
class Derived : Base {
public:
    using Base::Base;
    /* ... */
};
```



如上 using 声明，对于基类的每个构造函数，编译器都生成一个与之对应（形参列表完全相同）的派生类构造函数。生成如下类型构造函数：

```c++
Derived(parms) : Base(args) { }
```

## 函数重载

在继承过程中，派生类可以覆盖重载函数的0个或多个实例，一旦定义了一个重载版本，那么其他的重载版本都会变为不可见。

如果对于基类的重载函数，我们需要在派生类中修改一个，又要让其他的保持可见，必须要重载所有版本，这样十分的繁琐。

```c++
#include <iostream>
using namespace std;

class Base{
    public:
        void f(){ cout<<"f()"<<endl;
        }
        void f(int n){
            cout<<"Base::f(int)"<<endl;
        }
};

class Derived : private Base {
    public:
        using Base::f;
        void f(int n){
            cout<<"Derived::f(int)"<<endl;
        }
};

int main()
{
    Base b;
    Derived d;
    d.f();
    d.f(1);
    return 0;
}
```
如上代码中，在派生类中使用using声明语句指定一个名字而不指定形参列表，所以一条基类成员函数的using声明语句就可以把该函数的所有重载实例添加到派生类的作用域中。此时，派生类只需要定义其特有的函数就行了，而无需为继承而来的其他函数重新定义。

完整代码见：[using_derived.cpp](using_derived.cpp)
## 取代typedef

C中常用typedef A B这样的语法，将B定义为A类型，也就是给A类型一个别名B

对应typedef A B，使用using B=A可以进行同样的操作。

```c++
typedef vector<int> V1; 
using V2 = vector<int>;
```
完整代码见：[using_typedef.cpp](using_typedef.cpp)

### 类中的using定义别名

```C++
#include <iostream>
#include <memory>

class PlayerBase {
public:
    // 定义类型别名
    using Ptr = std::shared_ptr<PlayerBase>;

    void play() {
        std::cout << "Playing..." << std::endl;
    }
};

int main() {
    // 使用类型别名创建 PlayerBase 对象的共享指针
    PlayerBase::Ptr player = std::make_shared<PlayerBase>();

    // 调用成员函数
    player->play();

    return 0;
}
```

这个声明定义了一个类型别名 `Ptr`，它代表 `std::shared_ptr<PlayerBase>`。这意味着 `Ptr` 是一个指向 `PlayerBase` 对象的共享指针。

它们只是编译时的替换，类似于C中的宏替换。类型别名的定义在编译时被替换成实际的类型，之后编译器再处理这些实际的类型。因此，它们的作用范围（scope）仅限于定义它们的作用域。通常，类型别名会被定义在类内部或命名空间内部，使得它们在该作用域内有效。

在类的作用域内使用 `PlayerBase::Ptr` 来表示 `std::shared_ptr<PlayerBase>`

通过使用类型别名，可以使代码更简洁，特别是在处理复杂类型时。将类型别名放在类的内部，可以提高代码的可读性和可维护性，使其更符合面向对象编程的原则。
