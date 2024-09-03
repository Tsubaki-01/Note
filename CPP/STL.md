容器的`[]`运算符不是`const` 的。`const`的变量、对象、函数不能使用`[]`运算符



### 获取一个字符串的首地址`char*`指针

- `.data()`
- `.c_str()`

### `std::chrono` 

`std::chrono` 是 C++11 标准库中的一个时间库，提供了用于处理时间点、时间段和时钟的类和函数。它在 `<chrono>` 头文件中定义。`std::chrono` 提供了强大且灵活的工具，用于时间测量和计算，常用于定时器、延迟执行和性能分析等场景。

以下是 `std::chrono` 的主要组成部分及其功能解释：

**时间点（Time Point）**

时间点表示某个具体的时刻，通常用 `std::chrono::time_point` 类表示。`time_point` 是基于某个时钟的时间点。

```C++
using Clock = std::chrono::high_resolution_clock;
std::chrono::time_point<Clock> now = Clock::now();
```

**时间段（Duration）**

时间段表示时间长度，通常用 `std::chrono::duration` 类表示。`duration` 是一个模板类，表示从某个时间点到另一个时间点的时间跨度。

```C++
std::chrono::seconds sec(1);          // 1秒
std::chrono::milliseconds ms(1000);   // 1000毫秒（1秒）
std::chrono::microseconds us(1000000); // 1000000微秒（1秒）
```

**时钟（Clock）**

时钟用于提供当前时间点，可以是系统时钟、稳定时钟或高分辨率时钟。常用的时钟类型有：

- `std::chrono::system_clock`：表示系统时钟，表示当前的时间和日期。
- `std::chrono::steady_clock`：表示单调时钟，不会被系统时间调整影响，适合用于测量时间间隔。
- `std::chrono::high_resolution_clock`：表示高分辨率时钟，提供尽可能精确的时间点。

```C++ 
auto now_system = std::chrono::system_clock::now(); // 获取系统时钟当前时间
auto now_steady = std::chrono::steady_clock::now(); // 获取单调时钟当前时间
auto now_high = std::chrono::high_resolution_clock::now(); // 获取高分辨率时钟当前时间
```



以下是一个使用 `std::chrono` 的示例，展示如何测量函数执行时间：

```c++
#include <iostream>
#include <chrono>
#include <thread>

// 模拟一个耗时操作
void some_work() {
    std::this_thread::sleep_for(std::chrono::seconds(1)); // 休眠1秒
}

int main() {
    using Clock = std::chrono::high_resolution_clock;
    
    auto start = Clock::now(); // 获取开始时间点
    
    some_work(); // 执行耗时操作
    
    auto end = Clock::now(); // 获取结束时间点
    
    std::chrono::duration<double> duration = end - start; // 计算时间差
    
    std::cout << "Time taken: " << duration.count() << " seconds" << std::endl;
    
    return 0;
}
```

### `vector.resize()` & `vector.reserve()`

`vector.reserve()`修改的只是`vector`的容量（即`capacity`）
`vector.resize()`修改的是`vector`的容量以及内容（即修改了`capacity`，`size`）
例如：
一个没有元素的`vector`调用`reserve`，里面还是没有元素，只是预留了空间
一个没有元素的`vector`调用`resize`，里面是有元素存在的（不指定就按默认构造）

### `push_back()` & `emplace_back()`

**`push_back`**

- **用法**：`push_back`用于将一个已存在的对象副本添加到容器的末尾。
- **调用方式**：`v.push_back(obj);`
- **机制**：`push_back`接受一个对象的引用或者右值引用，将其复制（或移动）到容器中。因此，对于非基础类型的对象，可能会涉及构造和析构的开销。

**`emplace_back`**

- **用法**：`emplace_back`用于在容器的末尾原位构造一个对象。它通过直接调用对象的构造函数，在容器分配的内存位置上构造对象，从而避免了不必要的拷贝或移动。
- **调用方式**：`v.emplace_back(args...);`，其中`args...`是对象构造函数所需的参数。
- **机制**：`emplace_back`接受一组参数，并直接在容器末尾的位置调用对象的构造函数，从而构造出新对象。

**区别总结**

1. **构造方式**：
   - `push_back`需要先构造对象，然后再将其拷贝或移动到容器中。
   - `emplace_back`直接在容器末尾原位构造对象。
2. **效率**：
   - `push_back`可能会涉及额外的拷贝或移动操作，因此效率可能稍低。
   - `emplace_back`通过避免不必要的拷贝或移动操作，通常效率更高，特别是在对象的构造和析构开销较大的情况下。
3. **使用场景**：
   - 当对象已经存在或者你只有一个现成的对象需要添加时，使用`push_back`。
   - 当你希望直接在容器中构造对象，以避免额外的拷贝或移动时，使用`emplace_back`。

**注：**

```c++
std::vector<std::vector<int>> res;
res.push_back({1, 2});      // 正确
res.emplace_back(1, 2);     // 正确
res.emplace_back({1, 2});   // 错误
```

`{1, 2}`是一个初始化列表，它可以隐式地转换为 `std::vector<int>`

在使用 `emplace_back` 时，应该传递构造函数的参数，而不是一个初始化列表

> !!!
>
> `res.emplace_back(1, 2)`
>
> 传入的构造参数是(1,2),扩展出来就是vector<int>(1,2),构建的是一个长度为1，元素值为2的数组。
>
> 而不是一个包含了1,2的数组！！！！

### 初始化列表

**初始化列表语法**

初始化列表使用花括号 `{}` 包围元素，元素之间用逗号分隔。例如：

```c++
std::vector<int> vec = {1, 2, 3, 4, 5};
```

**`std::initializer_list` 类模板**

C++11 引入了 `std::initializer_list` 类模板，它允许使用初始化列表来初始化对象。标准库容器通常都提供了接受 `std::initializer_list` 的构造函数，以便可以用初始化列表进行初始化。

类可以定义接受 `std::initializer_list` 的构造函数，从而允许对象使用初始化列表进行初始化。例如：

```c++
#include <initializer_list>
#include <vector>

class MyClass {
public:
    MyClass(std::initializer_list<int> initList) {
        for (auto elem : initList) {
            data.push_back(elem);
        }
    }

    void printData() {
        for (auto elem : data) {
            std::cout << elem << " ";
        }
        std::cout << std::endl;
    }

private:
    std::vector<int> data;
};

int main() {
    MyClass obj = {1, 2, 3, 4}; // 使用初始化列表构造对象
    obj.printData();
    return 0;
}

```

### `SET、MAP、VECTOR`的查询

![set](E:\C++\Note\img\set.png)

![map](E:\C++\Note\img\map.png)

注意查询效率！！！

`vector`容器使用`std::find`是线性查询，时间复杂度是`O(n)`



