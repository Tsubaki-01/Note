二维（多维）的vector中，每行的vector在地址中可能并不是连续的

对于一个二维的 `vector`（即 `std::vector<std::vector<int>>`），情况有所不同。二维 `vector` 的每一行实际上是一个独立的一维 `vector`，而这些一维 `vector` 的内存位置是独立分配的。因此，虽然每个一维 `vector` 内部的元素是连续的，但在二维 `vector` 中，不同的行在内存中的地址不一定是连续的。

具体来说：

- 在 `std::vector<std::vector<int>>` 中，外层 `vector` 存储的是指向内层 `vector` 的指针（或者说引用）。
- 每个内层 `vector`（即每一行）在内存中都是独立分配的，因此不同行之间的地址不一定是连续的。

```c++
#include <iostream>
#include <vector>

int main() {
    std::vector<std::vector<int>> vec = {
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9}
    };

    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << "Address of row " << i << ": " << &vec[i] << std::endl;
    }

    return 0;
}

```

```
Address of row 0: 0x27a49f2a800
Address of row 1: 0x27a49f2a818
Address of row 2: 0x27a49f2a830
```

