## C语言的输入输出

`#include <stdio.h>`

### `printf`

格式控制字符串有两种：格式字符串、非格式字符串，非格式字符串在输出的时候原样打印；格式字符串是以 `%` 开头的字符串，后面跟不同格式字符，用来说明输出数据的类型、形式、长度、小数位数等，就像一个模具，用来控制希望得到的物体的形态

| 类型      | 格式字符串 |
| --------- | ---------- |
| int       | %d         |
| float     | %f         |
| double    | %lf        |
| char      | %c         |
| char[]    | %s         |
| long long | %lld       |

格式字符串的形式为：`% [对齐方式][输出最小宽度] [.精度] 类型`

1. 对齐方式：`-` 表示左对齐，不填表示右对齐（没有`% +`的写法），默认为右对齐，如 `% -d`，表示左对齐
2. 最小宽度 N ：当实际宽度小于 N 时，用指定的字符填充剩余部分（默认以空格填充），使其长度为 N 并输出；当实际宽度大于 N 时，按实际位数输出，如 `% 010d`，表示最小宽度为 10，不足部分用 0 填充
3. 精度控制：这里精度表示保留小数点后几位，如 `%.2f`，表示保留小数点后两位

### `scanf`

格式控制字符串的作用与`printf`函数相同，地址表项中的地址给出各变量的地址，地址是由地址运算符 `&` 后跟变量名组成的

**返回值**

`scanf()` 函数返回成功读入的项目的个数，如果它没有读取任何项目（比如它期望接收一个数字而实际却输入的一个非数字字符时就会发生这种情况），`scanf()` 则会返回 0

当它检测到文件末尾 `(end of file)` 时，它返回EOF， EOF 是文件 `stdio.h` 中的定义好的一个特殊值，通常 `#define` 指令会将EOF 的值定义为− 1，则可以结合 while循环实现多行输入，通常有两种 c 的写法：

1.`while(scanf("%d",&a) != EOF)`

2.`while(~scanf("%d",&a))`

**输入字符串**

1. 利用 `%s` 输入字符串

🔺注意：%s 输入遇到空格或者回车就会停下（截断）

2. 利用正则表达式输入字符串

`[]` 是正则表达式，表示只要不是回车就读入字符串（空格不会截断）

```C
#include<stdio.h>
int main(){
    char s0[15];
    scanf("%[^\n]", s0);
    printf("%s", s0);
    return 0;
}
```

### * 的应用

① * 在`printf`中的应用

控制宽度和精度:

假如在输出时，不想事先指定字段宽度/精度，而是希望由程序来制定该值则可以在字段宽度/精度部分使用代替数字来达到目的，同时需要在后面多增加一个参数来告诉函数宽度/精度的值是多少，具体的说，如果转换说明符为%d，那么参数列表中应该包括一个*的值和一个d的值，来控制宽度/精度和变量的值

```C
#include<stdio.h>
int main(void)
{
    printf("%0*d\n", 5, 10);       //00010
    printf("%.*f\n", 2, 10.12345); //10.12
    printf("%*.*f\n", 5, 1, 10.0); // 10.0
    return 0;
}
```

② * 在`scanf`中的应用

忽略赋值:

 `*` 在`scanf` 函数中通常用来指定忽略某个输入值的赋值，这在处理输入时非常有用，因为有时候我们可能只想跳过一些输入，而不需要将其赋值给特定的变量
 `*` 例如:有以下的输入数据“10 20"，但是我们只想读取第二个整数而不关心第一个整数，则可以使用  * :

```C
#include <stdio.h>
int main(){
    int num;
    scanf("%*d %d", &num);  //10 20
    printf("%d", num);      //20
    return 0;
}
```

### %n 的应用

① `%n` 在 `printf`中的应用

统计字符数: printf()中， %n是一个特殊的格式说明符，它不打印某些内容，而是会将目前为止打印输出字符的数量存储到参数列表中指向的变量中

```C
#include<stdio.h>
int main()
{
    int k=0;
    printf("hello %nworld", &k);      //输出 hello world
    printf("%d",k);                   //6
    return 0;
}

```

② `%n` 在 `scanf`中的应用

统计字符数: scanf()中，n会将目前为止读入字符的数量存储到参数列表中指向的变量中

```c
#include <stdio.h>
int main() {
    char str[100];
    int count;
    scanf("%s%n", str, &count); //abcd
    printf("%d\n", count);      //4
    return 0;
}
```

### %[] 的应用

常见用法：

`%[0-9]`： 表示只读入’0’到’9’之间的字符
`%[a-zA-Z]`： 表示只读入字母
`%[^\n]`： 就表示读入除换行符之外的字符，^ 表示除…之外
`%k[^=]`： 读入"="号前的至多 k 个字符
当读取到范围之外的字符时，就会做截取，也就是之后的数据将不会放入字符串中

```C
//1
#include <stdio.h>
int main() {
    char number[10];
    scanf("%[0-9]", number);    //123abc
    printf("%s\n", number);     //123

    return 0;
}

//2
#include <stdio.h>
int main() {
    char letters[20];
    scanf("%[a-zA-Z]", letters);    //abc121efg
    printf("%s\n", letters);        //abc

    return 0;
}

//3
#include<stdio.h>
int main(){
    char s0[15];
    scanf("%[^\n]", s0);    //123abc abc
    printf("%s", s0);       //123abc abc

    return 0;
}

//4
#include <stdio.h>

int main() {
    char str[20];
    scanf("%10[^=]", str);  //123=abc
    printf("%s\n", str);    //123

    return 0;
}

```

## C++中的输入输出

`#include <iostream>`

### `cout`

`cout` 是 C++ 标准库中的标准输出流对象

```C
cout << "hello world" << endl;
```

`<<` 是输出运算符，左侧必须是 `ostream` 对象，右侧是要打印的值：此运算符将给定的值写到给定的 `ostream` 对象中，计算结果就是我们写入给定值的那个 `ostream`对象

`cout` 是一个输出对象，输出语句本质上就是不断创造  `ostream` 对象的过程，第一个计算的结果是第二个的输入，从而形成链条，最终将所有信息输出到屏幕上，例如：

```C
cout << "hello" << endl;
等价于
1.cout << "hello";
2.cout << endl;
```

#### 缓冲区（buffer）

当程序向输出设备（比如屏幕、打印机）输出数据时，数据通常不是立即传递到设备上，而是先存储到缓冲区中，这样做的好处是可以减少向设备频繁传输数据的开销，而是利用缓冲区将数据一次性传输，从而提高效率，类似地，当数据从输入设备（比如键盘、网络）输入进来时，也会先存储到输入缓冲区中，程序可以逐步读取这些数据。

在 C++ 中，对于标准输出流 `cout`，也存在一个输出缓冲区，当程序使用`cout` 进行输出时，数据首先被存储在输出缓冲区中，直到缓冲区满了或者遇到显式的刷新操作时，数据才会被真正输出到屏幕上：

- 使用 `std::flush` 操纵符：手动刷新输出缓冲区
- 输出末尾添加 `std::endl` ：自动刷新缓冲区并插入换行符

#### `cout`之格式化输出

通过 `#include <iomanip>` 头文件来实现格式化的输出：

1. `std::setw(int n)`： 设置域宽为 n，默认为右对齐
2. `std::setprecision(int n)`： 设置浮点数的精度为 n 位小数
3. `std::setfill(char c)`： 设置填充字符为 c
4. `std::left`： 设置输出在设定的宽度内左对齐
5. `std::right`： 设置输出在设定的宽度内右对齐
6. `std::boolalpha`： 将布尔类型的输出从01 改为 true false
7. `std::hex`： 以十六进制形式输出值
8. `std::oct`： 以八进制形式输出值
9. `std::dec`： 以十进制形式输出值

```C 
#include <iostream>
#include <iomanip>

int main() {
    int n = 255;
    double pi = 3.1415926;

    std::cout << "Default: " << n << " " << pi << std::endl;
    std::cout << "setw: " << std::setw(10) << n << " " << std::setw(10) << pi << std::endl;
    std::cout << "setprecision: " << std::setprecision(4) << n << " " << std::setprecision(3) << pi << std::endl;
    std::cout << "setfill: " << std::setfill('*') << std::setw(10) << n << " " << std::setfill('#') << std::setw(10) << pi << std::endl;
    std::cout << "left/right: " << std::left << std::setw(10) << n << std::right << std::setw(10) << pi << std::endl;
    std::cout << "boolalpha: " << std::boolalpha << true << " " << false << std::endl;
    std::cout << "Hex/Oct/Dec: " << std::hex << n << " " << std::oct << n << " " << std::dec << n << std::endl;

    return 0;
}
```

### `cin`

`cin` 是 C++ 标准库中的标准输入流对象，在查看输入流的时候，`cin` 会自动跳过空白（空格，换行符，制表符）直到遇到非空白字符

```C
int a, b;
cin >> a >> b;
```

`>>` 是输入运算符，其左侧为一个`istream`运算对象，再接受一个变量作为右侧运算对象，可以连续读取多个数据，并将它们存储到不同的变量中

🔺*注意*：`cin >>` 接受一个字符串时，遇到 ‘空格’、‘tab’、'回车’就会结束，当需要输入包含空格的整行文本时，可以使用 `std::getline` 函数来读取输入流，而不是直接使用 `>>` 运算符，这样可以确保整行文本被完整地存储到字符串中

#### `getline(cin, string)`：

- 头文件：`#include <string>`
- 第一个参数是输入流（比如 `std::cin` 或文件流），第二个参数是用来存储读取的文本的字符串

#### `cin.get()`

① `cin.get(字符变量名)`：

- 每次接受单个字符，逐个字符读取
- 当读取的输入流有空格或回车时，会读取空格或回车，并将其视为普通字符，而不会将其作为流结束的标志来处理：ASCII 中，空格-32，回车 -10

② `cin.get(字符数组名,接收字符数目,结束符)`：

- 接收一定长度的字符串
- 接受字符数目：如 `cin.get(s, 5)` 会读取最多5个字符到数组 s 中，并且在必要时加上空字符`'\0'` 以表示字符串的结束，也就是说，实际读取的长度会减 1 (4个)，因为结尾的 `'\0'` 占了一位
- 结束符：默认为回车 `'\n'`，也就是说，可以接受空格，不接受回车，但不读取不代表丢弃，**回车仍然在输入缓冲区内**

```c
#include <iostream>
using namespace std;
int main (){
    char ch1,ch2[10];
    cin.get(ch2,5);       //在不遇到结束符的情况下，最多可接收5-1=4个字符到ch2中，注意结束符为默认Enter
    cin.get(ch1);         //读取单个字符
    cout<<"ch2="<<ch2<<endl;
    cout<<ch1<<"="<<(int)ch1<<endl;
    return 0;
}

```



```c
示例1：输入 a[回车]
a
ch2=a

=10
由于第二个就读取到了换行符，因此直接结束，之后换行符被第二个 cin.get() 读取

示例2：输入长度为 7 的字符串 abcdefg
abcdefg
ch2=abcd
e=101

```

③ `cin.get()`：

- 无参数，可用于舍弃输入流中的不需要的字符，或者舍弃回车 `'\n'`，弥补 `cin.get(字符数组名,字符数目,结束符)` 的不足

#### `cin.getline()`

`cin.getline(字符数组名，接收长度，结束符)` 与 `cin.get(...)`的用法极为类似，但存在几个🔺注意点：

- `cin.get()` 当输入的字符串超长时，不会引起 `cin` 函数的错误，后面若有 `cin` 操作，会继续执行，只是直接从缓冲区中取数据，但是 `cin.getline()` 当输入超长时，会引起 `cin` 函数的错误，后面的`cin` 操作将不再执行

```c++
#include <iostream>
using namespace std;
int main()
{
    char ch1, ch2[10];
    cin.getline(ch2, 5);
    cin>>ch1;
    cout<<ch2<<endl;
    cout<<ch1<<"\n"<<(int)ch1<<endl;
    return 0;
}

输入 12345
输出 1234
    
    0
    
```

可以看到，此时出现输入超长，`ch2=1234`，此时缓冲区剩余：`5[回车]`，但是结果得到了 0(g++环境下)，说明 `cin` 已失效！

`cin.get()` 每次读取一整行并把由 `Enter`键 生成的换行符 `'\n'` 留在输入队列中，然而 `cin.getline()`每次读取一整行后会将换行符`\n`丢弃 !

```c++
#include <iostream>
using namespace std;

int main() {
    cout << "Enter your name:";
    char name[15];
    cin.getline(name, 15);  //输入xjc(enter)
    cout << "name:" << name << endl;
    char ch;
    cin.get(ch);    // 输入123(enter) 注：因为cin.getline把最后一个换行符丢弃了，所以此处ch读取字符'1'
    cout << ch << "=" << (int)ch << endl;  //输出49  '1'的ASCII码值
    return 0;
}

```

### 流状态

流状态是顾名思义就是输入或输出流的状态，`cin` 和 `cout`对象都包含了一个描述流状态的变量（`iostate`类型），它由三个元素组成：`eofbit`, `failbit`, `badbit`，其中每一个元素都由一位来表示（1代表是，0代表否）

|          成员          |                             描述                             |
| :--------------------: | :----------------------------------------------------------: |
|         eofbit         |                   到达文件尾则此位设置为1                    |
|         badbit         |                  如果流被破坏则此位设置为1                   |
|        failbit         | 如果输入未能读取预期的字符或输出操作没有写入预期字符，则此位设置为1 |
|        goodbit         |            流状态正常，也代表所有位为 0 即表示 0             |
|         good()         |                    如果流正常则返回 true                     |
|         eof()          |                  如果 eofbit 为 1 返回 true                  |
|         bad()          |                  如果 badbit 为 1 返回 true                  |
|         fail()         |            如果 badbit 或 failbit 为 1 返回 true             |
|       rdstate()        |                          返回流状态                          |
|      exceptions()      |          返回一个位掩码，指出哪些标记将导致异常发生          |
| exceptions(iostate ex) |             设置哪些状态将导致 clear() 引发异常              |
|    clear(iostate s)    |            将流状态设置为 s，s 默认为 0 即goodbit            |
|  setstate(iostate s)   | 设置与 s 中对应位的流状态，其他位保持不变，相当于用 s 与 当前流状态做 “或运算” |

来看一个缓冲区未读取完，导致 failbit = 1 的例子：

```c++
#include <iostream>
using namespace std;

int main()
{
char ch, str[20];
    cin.getline(str, 5);
    cout<<"flag:"<<cin.good()<<endl;    // 查看goodbit状态，即是否有异常
    cin.clear();                        // 清除错误标志
    cout<<"flag:"<<cin.good()<<endl;    // 清除标志后再查看异常状态
    cin>>ch;
    cout<<"str:"<<str<<endl;
    cout<<"ch :"<<ch<<endl;

    system("pause");
    return 0;
}
```

![在这里插入图片描述](https://img-blog.csdnimg.cn/direct/b5364baec1954e8690b561ba387f62ed.png)

如果没有 `cin.clear()`，`cin>>ch` 将读取失败，ch 为空

![在这里插入图片描述](https://img-blog.csdnimg.cn/direct/f4be3dba9f3049eb9969601a6cd929ab.png)

## 取消同步流

在算法题中，涉及到大量数据读入的时候，通常避免使用`cin`读入数据而改用`scanf`，原因是`scanf`相对速度更快

🚀*解决方法:*

1. `cin` 效率低的原因一是在于默认 C++ 中的 `cin` (输入流对象) 与 C语言中的 `stdin` (输入流指针) 总是保持同步，`cin`会把要输出的东西先存入缓冲区，进而消耗时间，通过关闭同步，可以有效提高`cin`效率，可以用 `sync_with_stdio(0)` 实现异步
2. 默认情况下`cin` 绑定的是 `cout`，每次执行 `<<` 的时候都要调用 `flush`，当 `cout` 的缓冲区刷新的时候，`cin` 的缓冲区由于绑定的存在也同时进行了刷新，进而增加I/O负担，因此可以通过 `tie(0)` 解绑

关闭同步后，`cin` 的速度将与 `scanf` 相差无几：

```c++
int main(){
    //取消同步流
    ios::sync_with_stdio(0), cin.tie(0), cout.tie(0);

    //其余操作不变
    int x;
    cin >> x;
    cout << x;

    system("pause");
    return 0;
}

```

