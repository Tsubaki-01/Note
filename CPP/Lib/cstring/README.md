`<cstring>` 是 C++ 标准库中用于处理 C 风格字符串和内存操作的头文件，它是 C 标准库 `<string.h>` 的 C++ 版本。以下是一些常用的函数：

### 1. **字符串操作函数**

- **`strcpy`**：将一个字符串复制到另一个字符串。

  ```
  char *strcpy(char *dest, const char *src);
  ```

- **`strncpy`**：将指定长度的字符串复制到另一个字符串。

  ```
  char *strncpy(char *dest, const char *src, size_t n);
  ```

- **`strcat`**：将一个字符串追加到另一个字符串的末尾。

  ```
  char *strcat(char *dest, const char *src);
  ```

- **`strncat`**：将指定长度的字符串追加到另一个字符串的末尾。

  ```
  char *strncat(char *dest, const char *src, size_t n);
  ```

- **`strlen`**：返回字符串的长度，不包括终止字符 `\0`。

  ```
  size_t strlen(const char *str);
  ```

- **`strcmp`**：比较两个字符串，返回比较结果。

  ```
  int strcmp(const char *str1, const char *str2);
  ```

- **`strncmp`**：比较指定长度的两个字符串。

  ```
  int strncmp(const char *str1, const char *str2, size_t n);
  ```

- **`strchr`**：查找字符串中首次出现的字符。

  ```
  char *strchr(const char *str, int c);
  ```

- **`strrchr`**：查找字符串中最后一次出现的字符。

  ```
  char *strrchr(const char *str, int c);
  ```

- **`strstr`**：查找字符串中首次出现的子串。

  ```
  char *strstr(const char *haystack, const char *needle);
  ```

- **`strtok`**：分割字符串。

  ```
  char *strtok(char *str, const char *delim);
  ```

### 2. **内存操作函数**

- **`memset`**：将一块内存中的所有字节设置为指定值。

  ```
  void *memset(void *ptr, int value, size_t num);
  ```

- **`memcpy`**：将一块内存中的数据复制到另一块内存中。

  ```
  void *memcpy(void *dest, const void *src, size_t num);
  ```

- **`memmove`**：类似于 `memcpy`，但更安全，支持重叠内存区域的复制。

  ```
  void *memmove(void *dest, const void *src, size_t num);
  ```

- **`memcmp`**：比较两块内存区域的内容。

  ```
  int memcmp(const void *ptr1, const void *ptr2, size_t num);
  ```

- **`memchr`**：在内存块中查找指定值的第一个匹配字节。

  ```
  void *memchr(const void *ptr, int value, size_t num);
  ```

### 3. **其他辅助函数**

- **`strerror`**：返回描述错误码的字符串。

  ```
  char *strerror(int errnum);
  ```

这些函数都直接操作 C 风格的字符串或内存块，是处理低级字符串操作和内存管理的基础。