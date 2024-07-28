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