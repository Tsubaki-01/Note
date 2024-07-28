### C 语言中的 struct 和 typedef struct

在 C 语言中，结构体和类型定义有一些特殊的规则：

1. **使用 `struct` 定义结构体：**

```c
struct Student {
    int age;
};
```

这段代码定义了一个名为 `Student` 的结构体类型。要声明这种类型的变量，你需要使用 `struct Student`，例如：

```c
struct Student me;
```

1. **使用 `typedef` 简化类型名：**

```c
typedef struct Student {
    int age;
} S;
```

这段代码做了两件事：

- 定义了一个名为 `Student` 的结构体类型。
- 使用 `typedef` 为该结构体类型创建了一个别名 `S`。

现在，你可以使用 `S` 来声明变量，而不必每次都写 `struct Student`：

```c
S me;
```

1. **分开定义 struct 和 typedef：**

```c
struct Student {
    int age;
};

typedef struct Student S;
```

这段代码与前面的作用相同，只是将 `struct` 的定义和 `typedef` 分开写了。

1. **命名空间：**

C 语言中，结构体标签（如 `Student`）和普通标识符（如函数或变量名）位于不同的命名空间。因此，你可以有一个与结构体同名的函数：

```c
void Student() {
    // 函数实现
}
```

但你不能将两个普通标识符命名为同样的名字。例如，以下代码会报错：

```c
typedef struct Student {
    int age;
} S;

void S() {} // 错误，因为 "S" 已经是 "struct Student" 的别名
```

### C++ 语言中的 struct 和 typedef struct

在 C++ 中，编译器的符号解析规则不同于 C 语言。C++ 引入了更强大的命名空间和类机制，使得一些用法变得更加简洁。

1. **简化使用：**

在 C++ 中，当你定义一个结构体时，不需要像在 C 中那样总是使用 `struct` 关键字：

```cpp
struct Student {
    int age;
};

void f(Student me); // 正确，"struct" 关键字可省略
```

1. **名称冲突处理：**

如果存在与结构体同名的函数，则该名字只代表函数，而不再代表结构体。例如：

```cpp
typedef struct Student {
    int age;
} S;

void Student() {} // 正确，定义后 "Student" 只代表此函数

int main() {
    Student(); // 调用函数 "Student"
    struct Student me; // 或者 "S me";
    return 0;
}
```

在这个例子中，由于存在一个同名函数 `Student()`，所以在调用时，如果没有使用 `struct` 前缀，则编译器会认为你是在调用这个函数。

### 总结

- 在 C 语言中，通过 `typedef` 可以为结构体创建别名，并且结构体标签和普通标识符处于不同的命名空间。
- 在 C++ 中，编译器允许省略 `struct` 关键字来引用已定义的结构体，但如果存在同名的函数或其他标识符，则需要明确区分。