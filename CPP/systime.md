```c++
time_t timer = time(nullptr);
struct tm t = *localtime(&timer);
```

`time(nullptr)` 返回当前时间的秒数（自1970年1月1日00:00:00 UTC起的秒数）。

`localtime(&timer)` 接受一个指向 `time_t` 类型变量的指针，并将该时间转换为本地时间。

该函数返回一个指向 `tm` 结构体的指针，该结构体包含了表示本地时间的各个字段（年、月、日、时、分、秒等）。

```c++
struct tm {
    int tm_sec;   // 秒，范围从 0 到 60
    int tm_min;   // 分，范围从 0 到 59
    int tm_hour;  // 小时，范围从 0 到 23
    int tm_mday;  // 一月中的第几天，范围从 1 到 31
    int tm_mon;   // 月份（从 0 开始），范围从 0 到 11
    int tm_year;  // 自 1900 年起的年数
    int tm_wday;  // 一周中的第几天，范围从 0 到 6
    int tm_yday;  // 一年中的第几天，范围从 0 到 365
    int tm_isdst; // 夏令时标识符
};
```

