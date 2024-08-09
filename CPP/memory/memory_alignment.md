### #pragma pack(n)



设定结构体、联合以及类成员变量以 n 字节方式对齐

\#pragma pack(n) 使用

```c++
#pragma pack(push)  // 保存对齐状态
#pragma pack(4)     // 设定为 4 字节对齐

struct test
{
    char m1;
    double m4;
    int m3;
};

#pragma pack(pop)   // 恢复对齐状态
```



[博客](https://blog.csdn.net/hello_dear_you/article/details/123739899)



### 实现内存对齐的小trick

```c++
size_t aligned_size = (raw_size + align - 1) & ~(align - 1);  -->向上对齐
    
size_t aligned_size = (raw_size) & ~(align - 1);              -->向下对齐
```

- 以64位系统中的四字节（32位）对齐为例

  `unsigned int (align - 1) = 31 = （0B）0001 1111`

  则`~(align - 1) = （0B）1110 0000`（**更高位都是1**）

  则之后进行与（`&`）运算，结果只保留了32的倍数

- 同时，内存对齐一般需要向上或向下对齐，向上对齐时需要加上`(align - 1)`，类似于四舍五入

