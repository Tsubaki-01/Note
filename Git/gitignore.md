### 1. 基本语法规则

- **空行**：空行不做任何处理，可以用于分隔和美化。

- **注释行**：以 `#` 开头的行表示注释，不会被解析，通常用于说明。

  ```
  gitignore
  复制代码
  # 这是一个注释
  ```

- **文件或目录名**：直接指定文件名或目录名。

  ```
  gitignore复制代码example.txt     # 忽略名为 example.txt 的文件
  logs/           # 忽略 logs 目录及其下所有内容
  ```

### 2. 通配符

- **星号 `\*`**：匹配零个或多个字符。

  ```
  gitignore复制代码*.log           # 忽略所有 .log 文件
  temp*           # 忽略所有以 temp 开头的文件
  ```

- **问号 `?`**：匹配任意一个字符。

  ```
  gitignore
  复制代码
  file?.txt       # 忽略 file1.txt, file2.txt 等文件
  ```

- **方括号 `[abc]`**：匹配方括号内的任意一个字符。

  ```
  gitignore
  复制代码
  file[123].txt   # 忽略 file1.txt, file2.txt, file3.txt
  ```

- **范围 `[a-z]`**：匹配指定范围内的任意一个字符。

  ```
  gitignore
  复制代码
  file[a-c].txt   # 忽略 filea.txt, fileb.txt, filec.txt
  ```

### 3. 目录和路径

- **斜杠 `/`**：用于区分目录和文件。

  ```
  gitignore复制代码/foo            # 忽略根目录下的 foo 文件或目录
  foo/            # 忽略所有名为 foo 的目录
  /foo/bar.txt    # 仅忽略根目录下 foo 目录中的 bar.txt 文件
  ```

### 4. 取反

- **感叹号 `!`**：用于取反，表示不要忽略匹配到的文件或目录。

  ```
  gitignore复制代码*.log           # 忽略所有 .log 文件
  !important.log  # 但不忽略 important.log 文件
  ```

### 5. 特殊用法

- **双星号 `\**`**：匹配任意中间路径。

  ```
  gitignore复制代码**/logs         # 忽略任意目录下的 logs 目录
  logs/**/debug.log  # 忽略 logs 目录下任意子目录中的 debug.log 文件
  ```

- **结尾斜杠 `/`**：明确指定目录。

  ```
  gitignore
  复制代码
  temp/           # 忽略 temp 目录及其下所有内容
  ```

### 6. 示例

假设有以下目录结构：

```
plaintext复制代码project/
├── build/
├── src/
│   ├── main/
│   │   ├── app.js
│   │   ├── config/
│   │   └── temp/
│   ├── test/
│   │   └── test.js
├── .gitignore
└── README.md
```

一个典型的 .gitignore 文件可能如下：

```
gitignore复制代码# 忽略 build 目录及其所有内容
build/

# 忽略所有 .log 文件
*.log

# 忽略 src/main/config 目录中的所有内容，但不忽略 src/main/config/settings.json 文件
src/main/config/
!src/main/config/settings.json

# 忽略 src/main/temp 目录及其所有内容
src/main/temp/

# 忽略所有以 temp 开头的文件
temp*
```

### 7. 处理顺序

Git 会按照从上到下的顺序处理 .gitignore 文件中的规则。更具体的规则应该放在更前面，以防被后面的规则覆盖。例如：

```
gitignore复制代码*.log
!important.log
temp/
temp/backup/
!temp/backup/not_ignored.txt
```

在这个例子中，尽管 `*.log` 会忽略所有 `.log` 文件，但 `!important.log` 使得 `important.log` 文件不会被忽略。同样的，尽管 `temp/` 会忽略整个 `temp` 目录，但 `!temp/backup/not_ignored.txt` 会确保 `temp/backup/not_ignored.txt` 文件不被忽略。

### 8. 全局 .gitignore

有时需要为整个系统配置一个全局的 .gitignore 文件，这样可以在所有 Git 仓库中使用相同的忽略规则。可以通过以下命令设置全局 .gitignore 文件：

```
sh
复制代码
git config --global core.excludesfile ~/.gitignore_global
```

然后在 `~/.gitignore_global` 文件中添加全局忽略规则。