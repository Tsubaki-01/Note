### `${CMAKE_CURRENT_SOURCE_DIR}`

目前的源文件文件夹

### `${CMAKE_CURRENT_BINARY_DIR}`

cmake之后的文件夹，一般是`build`文件夹

### `target_include_directories`

相当于是`-I`

### `target_link_libraries`

相当于是`-L`

### `generator expressions`

生成器表达式是CMake中的一种强大的语法功能,用于在构建过程中动态生成和评估表达式。它们可以用于控制目标属性、配置编译器选项等。

生成器表达式的基本语法如下:

复制

```
$<...>
```

其中`...`部分是表达式的内容。生成器表达式可以嵌套使用,形成更复杂的表达式。

下面是一些常见的生成器表达式示例:

1. `$<BOOL:...>`: 根据括号内的 `1` 或 `0` 返回后面的内容。
2. `$<AND:?[,?]...>`: 根据括号内的多个条件进行逻辑"与"操作。
3. `$<OR:?[,?]...>`: 根据括号内的多个条件进行逻辑"或"操作。
4. `$<STREQUAL:a,b>`: 如果字符串 `a` 等于字符串 `b`，则返回 `1`，否则返回 `0`。
5. `$<TARGET_PROPERTY:target,prop>`: 返回指定目标的属性值。
6. `$<COMPILE_LANGUAGE:lang>`: 如果当前编译语言与 `lang` 匹配，则返回 `1`。
7. `$<COMPILE_LANG_AND_ID:lang,id1[,id2...]>`: 如果当前编译语言为 `lang`，并且编译器ID匹配 `id1`、`id2` 等之一，则返回 `1`。

生成器表达式可以广泛应用于CMake项目的各个方面,如:

- 设置目标属性(如`PRIVATE`、`PUBLIC`、`INTERFACE`)
- 根据编译器类型设置编译器标志
- 条件编译特定的源文件
- 动态配置项目设置

### `EXPORT`

 `EXPORT` 参数在 `install()` 命令中的作用是生成一个目标导出文件(target export file)。

当你使用 `install(TARGETS ${installable_libs} EXPORT MathFunctionsTargets ...)` 时,CMake 会在安装过程中生成一个名为 `MathFunctionsTargets.cmake` 的文件,并将其安装到指定的目录(通常是 `lib/cmake/MathFunctions/`)。

这个导出文件包含了 `${installable_libs}` 中所有目标的信息,例如:

- 目标的名称
- 目标的链接依赖
- 目标的包含目录
- 目标的编译器选项

当其他项目需要使用这些库时,可以通过导入 `MathFunctionsTargets.cmake` 文件来获取这些信息,而无需再次构建这些库。这大大简化了库的使用和集成过程。

使用 `EXPORT` 参数的主要好处包括:

1. 提高项目的可重用性和可移植性。其他项目可以直接使用你的库,而无需重新构建。
2. 确保库的使用方式与构建方式一致。目标导出文件包含了构建时的所有必要信息。
3. 简化依赖管理。其他项目只需要导入目标导出文件,即可获取所有必要的链接信息。

总之,`EXPORT` 参数是 CMake 中一个非常强大的功能,它可以帮助你更好地管理和分发你的库文件,提高项目的整体质量。