decltype 关键字用于检查实体的声明类型或表达式的类型及值分类。语法：

```c++
decltype ( expression )
```



https://blog.csdn.net/qq_15041569/article/details/110782202

https://www.zhihu.com/question/605296860?utm_id=0



decltype 使用

```c++
// 尾置返回允许我们在参数列表之后声明返回类型
template <typename It>
auto fcn(It beg, It end) -> decltype(*beg)
{
    // 处理序列
    return *beg;    // 返回序列中一个元素的引用
}
// 为了使用模板参数成员，必须用 typename
template <typename It>
auto fcn2(It beg, It end) -> typename remove_reference<decltype(*beg)>::type
{
    // 处理序列
    return *beg;    // 返回序列中一个元素的拷贝
}
```





decltype和priority_queue

```c++
#include <iostream>
#include <queue>
#include <vector>

int main() {
    auto cmp = [](int lhs, int rhs) {
        return lhs > rhs; // 定义从小到大的排序规则
    };
    
    // 使用 decltype 推导 cmp 的类型，并传给 priority_queue
    std::priority_queue<int, std::vector<int>, decltype(cmp)> pq(cmp);

    pq.push(3);
    pq.push(5);
    pq.push(1);
    pq.push(8);

    while (!pq.empty()) {
        std::cout << pq.top() << " ";  // 输出最小堆的元素
        pq.pop();
    }

    return 0;	
}
	
```

`std::priority_queue` 的第三个模板参数是比较函数的类型

这里，`decltype(cmp)` 获取了 `cmp` 的类型并传递给 `std::priority_queue`，这样编译器知道优先队列的比较规则是由这个 Lambda 表达式定义的。

你可能会想使用 `auto` 来自动推导类型，但 `auto` 只能用于变量声明，不能用于模板参数列表。因此，`decltype` 是解决这一问题的关键。