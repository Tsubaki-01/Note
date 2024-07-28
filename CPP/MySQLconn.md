### #include <mysql/mysql.h>找不到头文件

找到路径C:\Program Files\MySQL\MySQL Server 8.0\include

可以发现mysql文件夹里面并没有包含mysql.h，反而是在include文件夹下直接包含，可能是mysql安装时的差异

因此将C:\Program Files\MySQL\MySQL Server 8.0\include添加到环境变量后有两种解决方案

1. 使用#include <mysql.h>
2. 将头文件copy一份到mysql文件夹中

实在不行就在编译器include下面把mysql文件夹复制进去吧……

事实证明vscode是真的难用C++连接数据库

还是linux或者vs吧