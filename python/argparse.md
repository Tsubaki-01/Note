`argparse` 是 Python 标准库中用于编写 **命令行接口（CLI）** 的模块。简单来说，它能让你的 Python 脚本像专业的命令行工具（如 `git` 或 `npm`）一样，接收用户输入的参数、选项和开关，并自动生成帮助文档。

### 核心流程：使用 argparse 的四个步骤

你可以把 `argparse` 的工作流程想象成去餐厅点餐：

1. **进店 (Import):** 导入模块。
2. **找服务员 (Create Parser):** 创建一个解析器对象。
3. **看菜单点菜 (Add Arguments):** 告诉解析器你需要什么参数（是必选的主食，还是可选的甜点）。
4. **下单制作 (Parse Args):** 解析用户输入的命令，把结果端上来（存入变量）。

------

### 1. 快速上手示例

假设我们有一个脚本 `demo.py`，我们想通过命令行传入一个名字，让它打印问候语。

Python

```
import argparse

# 1. 创建解析器对象
parser = argparse.ArgumentParser(description="这是一个打招呼的脚本")

# 2. 添加参数
# --name 是参数名，help 是帮助信息
parser.add_argument("--name", help="请输入你的名字")

# 3. 解析参数
args = parser.parse_args()

# 4. 使用参数
if args.name:
    print(f"你好, {args.name}!")
else:
    print("你好, 陌生人!")
```

如何运行：

在终端（Terminal/CMD）中输入：

Bash

```
# 查看自动生成的帮助
python demo.py -h 

# 传入参数
python demo.py --name Gemini
# 输出: 你好, Gemini!
```

------

### 2. 深入理解：参数的两种类型

在 `add_argument` 中，最重要的概念是 **位置参数** 和 **可选参数**。

#### A. 位置参数 (Positional Arguments)

- **特点：** 必须填，不需要写参数名（如 `--xxx`），完全按照顺序解析。
- **场景：** 文件路径、核心输入数据。

Python

```
parser.add_argument("filename", help="要处理的文件名")
# 运行: python script.py data.txt
```

#### B. 可选参数 (Optional Arguments)

- **特点：** 非必填，通常以 `-` (简写) 或 `--` (全称) 开头。
- **场景：** 配置选项、开关、模式选择。

Python

```
parser.add_argument("-v", "--verbose", help="显示详细信息")
# 运行: python script.py --verbose 或 python script.py -v
```

------

### 3. 常用配置功能 (Toolbox)

`add_argument` 方法非常强大，以下是最常用的几个参数配置：

| **参数**     | **说明**                    | **示例代码**                                               |
| ------------ | --------------------------- | ---------------------------------------------------------- |
| **type**     | 指定输入类型 (默认是字符串) | `type=int` (自动将输入转为整数)                            |
| **default**  | 如果用户没填，使用的默认值  | `default=10`                                               |
| **required** | 强制可选参数变为必填        | `required=True`                                            |
| **choices**  | 限制输入范围                | `choices=['tcp', 'udp']`                                   |
| **action**   | 特殊动作 (常用作布尔开关)   | `action='store_true'` (出现该flag即为True)                 |
| **nargs**    | 指定参数接受的值数量        | `'+'`（1 个或多个）、`'*'`（0 个或多个）、`2`（固定 2 个） |



------

### 4. 综合实战案例

让我们写一个 **计算矩形面积** 的工具 `calc.py`。

- 它需要两个数字（长、宽）。
- 宽是可选的，如果不填，默认等于长（正方形）。
- 有一个 `--quiet` 开关，开启后只输出结果数字，不输出文字描述。

Python

```
import argparse

def main():
    parser = argparse.ArgumentParser(description="计算矩形面积的命令行工具")

    # 1. 位置参数：长度 (必须是整数)
    parser.add_argument("length", type=int, help="矩形的长度")

    # 2. 可选参数：宽度 (整数，默认值设为None，以便我们在逻辑中判断)
    parser.add_argument("-w", "--width", type=int, default=None, help="矩形的宽度 (默认为长度)")

    # 3. 开关参数：安静模式 (一旦输入 --quiet，变量就变为 True)
    parser.add_argument("-q", "--quiet", action="store_true", help="只输出结果数字")

    args = parser.parse_args()

    # 逻辑处理
    length = args.length
    # 如果没提供 width，则默认为正方形
    width = args.width if args.width else length
    
    area = length * width

    # 输出处理
    if args.quiet:
        print(area)
    else:
        print(f"长: {length}, 宽: {width}, 面积为: {area}")

if __name__ == "__main__":
    main()
```

#### 运行测试：

1. **查看帮助：**

   Bash

   ```
   python calc.py -h
   ```

   *系统会自动生成漂亮的帮助文档，列出所有参数说明。*

2. **计算正方形 (只输长度)：**

   Bash

   ```
   python calc.py 5
   # 输出: 长: 5, 宽: 5, 面积为: 25
   ```

3. **计算矩形 (指定宽度)：**

   Bash

   ```
   python calc.py 5 --width 3
   # 输出: 长: 5, 宽: 3, 面积为: 15
   ```

4. **安静模式 (只看结果)：**

   Bash

   ```
   python calc.py 10 -w 2 -q
   # 输出: 20
   ```

5. **错误测试 (输入非数字)：**

   Bash

   ```
   python calc.py five
   # 输出: error: argument length: invalid int value: 'five'
   # (argparse 帮你自动处理了类型检查报错)
   ```

------

### 5.进阶技巧：子命令（如 `git add`/`git commit`）

如果你的脚本有多个功能（如训练、测试、预测），可以用**子命令**让结构更清晰：

```c++
import argparse

# 1. 创建主解析器
parser = argparse.ArgumentParser(description="支持子命令的示例脚本")
subparsers = parser.add_subparsers(dest="command", help="子命令：train/test/predict")

# 2. 创建训练子命令
train_parser = subparsers.add_parser("train", help="训练模型")
train_parser.add_argument("--lr", type=float, default=0.01, help="学习率")
train_parser.add_argument("--epochs", type=int, default=10, help="训练轮数")

# 3. 创建测试子命令
test_parser = subparsers.add_parser("test", help="测试模型")
test_parser.add_argument("--model_path", required=True, help="模型文件路径")

# 4. 解析参数
args = parser.parse_args()

# 5. 根据子命令执行不同逻辑
if args.command == "train":
    print(f"开始训练：学习率={args.lr}，轮数={args.epochs}")
elif args.command == "test":
    print(f"开始测试：模型路径={args.model_path}")
else:
    parser.print_help()  # 未指定子命令时显示帮助
```

**运行测试**：
$$

$$

```c++
# 训练子命令
python demo.py train --lr 0.001 --epochs 50
# 输出：开始训练：学习率=0.001，轮数=50

# 测试子命令
python demo.py test --model_path model.pth
# 输出：开始测试：模型路径=model.pth

# 查看子命令帮助
python demo.py -h
# 输出包含：子命令：train/test/predict
python demo.py train -h
# 输出训练子命令的专属帮助
```

