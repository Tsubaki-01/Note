`不加&`

获取的是副本

`加&`

获取的是引用



所以，需要修改值的时候，需要加&

同时，只是遍历内容的时候，加&也可以加快访问速度





# range-based for 的一个缺陷

https://zhuanlan.zhihu.com/p/701995823

临时对象的生命周期值得重点关注，咱们先从 C++ for 循环的一个缺陷谈起，争取能用这一篇文章将临时对象的生命周期讲明白。

先看一段代码：

```cpp
struct Rank {
  vector<string> top3_;
  auto& top3() { return top3_; }
};
auto getRank() {
  return Rank{{"a", "b", "c"}};
}
```

这段代码模拟了一个排行榜类 Rank，函数 getRank 根据实时数据创建并返回排行榜对象。

写到 for 循环里，对排行榜前三名进行遍历：

```cpp
for (auto& str: getRank().top3()) {
  cout << str << '\n';
}
```

这段代码简练流畅，看起来没什么问题，然而恰恰埋藏着一只大 bug，幸运的是产生了崩溃，让我直接定位到了这里。

根据语言标准，这段 for 循环等价于：

```cpp
auto&& r = getRank().top3();
for (auto i = r.begin(), e = r.end(); i != e; ++i) 
{
  auto& str = *i;
  cout << str << '\n';
}
```

注意，getRank() 返回的是临时对象，getRank().top3() 返回了临时对象成员的引用，而 auto&& r = getRank().top3(); 执行完毕后，临时对象的生命周期结束，与之相关的引用也随之失效，之后对迭代器的操作就全是错误的了。

如果要遍历临时对象的话，需要遍历的**临时对象**必须是**右值表达式**，而且也要注意表达式中间产生的其他临时对象是在循环开始前就会被销毁的，只有表达式返回的最后的临时对象才会被“存”起来。

这种问题是怪程序员不够小心，还是语言本身就存在缺陷呢？笔者认为不应一概而论，但现有的基于范围的 for 语句对临时对象不够友好是肯定的，C++23 标准已提出应延长相关临时对象的生命周期至 for 循环结束。

在 C++23 之前可以这么改：

```cpp
auto&& r = getRank();
for (auto& str: r.top3()) {
  cout << str << '\n';
}
```

或者将 top3 函数的返回类型改为按值返回，便延长了临时对象的生命周期。

从这个问题可以看出，**临时对象的生命周期什么时候可以延长，什么时候不能延长是需要特别注意的**，现总结如下：

如果“直接”引用临时对象，可以延长其生命周期：

```cpp
struct T {
  string member;
  string val() { return member; }
  string& ref() { return member; }
};
auto&& a = T();         // T() 延长
auto&& b = T().member;  // T() 延长
auto&& c = T().val();   // val() 返回值延长，T() 析构
auto&& d = T().ref();   // 引用无效
```

例中 a 和 b 都延长了临时对象的生命周期，c 延长了 val() 返回值的生命周期，即**将纯右值与引用绑定时可以使其生命周期与引用的生命周期保持一致**。ref() 返回的不是纯右值，T() 在语句执行完毕后析构，d 成了无效引用。

但在 C++17 之前，构造函数[初始化列表](https://zhida.zhihu.com/search?q=初始化列表)中的绑定不会延长临时对象的生命周期：

```cpp
struct T {
  const string& s;
  T(const string& s): s(s) {}
};
T obj("abc"); // 引用无效
```

例中 "abc" 会被转换成 string 型临时对象， 但临时对象的生命周期在构造函数返回后结束，成员引用 s 成了无效引用。

如果不通过构造函数，而是通过 [Aggregate initialization](https://link.zhihu.com/?target=https%3A//en.cppreference.com/w/cpp/language/aggregate_initialization) 初始化，如：

```cpp
struct T {
  const string& s;
};
T a{"a"};
T* b = new T{"b"};
const T& c = T{"c"};
T&& d = T{"d"};
```

理论上也可以将临时对象直接与引用绑定，但目前来看各编译器实现不一致，相关引用可能是有效的，也可能是无效的，所以还是应该**避免将临时对象与成员引用绑定**。

临时对象作为函数的参数、调用[成员函数](https://zhida.zhihu.com/search?q=成员函数)或重载的运算符时，生命周期无法延长，这正是本文开头时的情况，这里再举个栗子：

```cpp
auto&& r = vector<int>{1, 2, 3}[0];   // 引用无效
```

最后一种情况是函数返回临时对象的引用，如：

```cpp
const string& fun() {
  return string("abc");
}
int main() {
  auto& r = fun();  // 引用无效
  cout << r << '\n';
}
```

临时对象在函数返回后立即失效，相关引用都是无效的，这种情况各大编程规范已经强调多次，但值得再次强调。

