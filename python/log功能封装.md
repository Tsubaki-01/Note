支持**自动创建目录**、**日志分级存储**（例如 `info` 和 `error` 分开存）以及**异步写入**。

------

## 1. 封装代码 (`logger_manager.py`)

首先安装依赖：`pip install loguru`

Python

```
import os
import sys
from loguru import logger

class LogManager:
    def __init__(self, log_dir="logs"):
        self.log_dir = log_dir
        self._configure_logger()

    def _configure_logger(self):
        # 1. 清除默认配置
        logger.remove()

        # 2. 定义统一的日志格式
        # <green> 等标签是 loguru 特有的色彩控制
        log_format = (
            "<green>{time:YYYY-MM-DD HH:mm:ss.SSS}</green> | "
            "<level>{level: <8}</level> | "
            "<cyan>{name}</cyan>:<cyan>{function}</cyan>:<cyan>{line}</cyan> - "
            "<level>{message}</level>"
        )

        # 3. 添加控制台输出
        logger.add(sys.stdout, format=log_format, level="DEBUG", enumerate=True)

        # 4. 添加常规日志文件 (INFO及以上)
        logger.add(
            os.path.join(self.log_dir, "runtime.log"),
            format=log_format,
            level="INFO",
            rotation="10 MB",     # 满10MB自动切分
            retention="1 week",   # 保留最近一周
            compression="zip",    # 旧日志压缩保存
            enqueue=True,         # 异步写入，不阻塞主线程
            encoding="utf-8"
        )

        # 5. 添加错误日志文件 (仅 ERROR 及以上)
        logger.add(
            os.path.join(self.log_dir, "error.log"),
            format=log_format,
            level="ERROR",
            rotation="1 week",
            backtrace=True,       # 记录详细回溯
            diagnose=True         # 记录变量值
        )

    def get_logger(self):
        return logger

# 实例化
log = LogManager().get_logger()
```

------

## 2. 项目中的进阶用法

封装好后，你可以利用 `loguru` 的一些“黑科技”来简化代码：

### A. 自动捕捉异常 (装饰器)

不需要写大量的 `try...except`，直接在函数上加装饰器，报错时会自动记录到日志并包含所有局部变量的值。

Python

```
from logger_manager import log

@log.catch
def divide(a, b):
    return a / b

divide(10, 0) # 这里会直接触发漂亮的错误日志
```

### B. 普通记录

```Python
from logger_manager import log

log.debug("调试信息：变量 x = 5")
log.info("用户登录成功")
log.warning("磁盘空间不足 10%")
log.error("数据库连接超时")
```

------

## 3. 为什么这样封装？

- **异步写入 (`enqueue=True`)**：在高并发场景下（比如 Web 服务器），写日志不会拖慢你的业务逻辑。
- **诊断模式 (`diagnose=True`)**：这是 `loguru` 的杀手锏。当程序崩溃时，它不仅告诉你哪行代码错了，还会告诉你**那一刻所有相关变量的值**是多少。
- **日志切分**：通过 `rotation` 参数，你再也不用担心日志文件把服务器硬盘撑爆了。

## 多目录

为了实现“多目录且不互相干扰”，我们不能使用全局的 `logger.remove()`。相反，我们需要利用 loguru 的两个特性：

1. **Binding (绑定上下文)**：给每个 Logger 实例打上唯一的“标签”（例如 `channel_id`）。
2. **Filter (过滤器)**：在添加文件 Sink 时，指定该文件**只接收**带有特定标签的日志。

```python
import sys
import os
from loguru import logger

class ContextLogManager:
    """
    支持多实例、多目录隔离的日志管理器
    """
    # 类变量，确保控制台只初始化一次（避免重复打印）
    _console_initialized = False

    def __init__(self, log_dir: str, channel_name: str = "default") -> None:
        """
        初始化日志管理器
        :param log_dir: 日志存储目录
        :param channel_name: 唯一的通道名称（用于区分不同的logger实例）
        """
        self.log_dir = log_dir
        self.channel_name = channel_name
        
        # 确保目录存在
        os.makedirs(self.log_dir, exist_ok=True)

        # 1. 配置全局控制台输出 (只执行一次)
        self._ensure_global_console()

        # 2. 配置该实例独享的文件输出
        self._configure_file_sinks()

    def _ensure_global_console(self):
        """配置控制台输出，确保只添加一次，防止重复"""
        if not ContextLogManager._console_initialized:
            # 清除 loguru 默认的 handler
            logger.remove()
            
            # 定义控制台格式
            console_format = (
                "<green>{time:HH:mm:ss}</green> | "
                "<level>{level: <8}</level> | "
                "<cyan>{extra[channel]}</cyan> - "  # 显示通道名
                "<level>{message}</level>"
            )
            
            # 添加控制台 handler (通常控制台我们希望看到所有日志，所以不加 filter，或者只加 level 限制)
            logger.add(sys.stdout, format=console_format, level="DEBUG")
            
            ContextLogManager._console_initialized = True

    def _configure_file_sinks(self):
        """配置具体的文件输出，使用 filter 进行隔离"""
        
        # 定义文件日志格式
        file_format = (
            "{time:YYYY-MM-DD HH:mm:ss.SSS} | "
            "{level: <8} | "
            "{name}:{function}:{line} | "
            "{message}"
        )

        # 定义过滤器：只有当日志的 extra["channel"] 等于当前实例名称时，才写入
        # record 是 loguru 传递给 filter 的日志记录对象
        def specific_filter(record):
            return record["extra"].get("channel") == self.channel_name

        # --- INFO 日志 (包含 INFO, WARNING, ERROR...) ---
        logger.add(
            os.path.join(self.log_dir, "runtime.log"),
            format=file_format,
            level="INFO",
            filter=specific_filter,  # 关键：绑定过滤器
            rotation="10 MB",
            retention="1 week",
            compression="zip",
            enqueue=True,
            encoding="utf-8"
        )

        # --- ERROR 日志 (仅 ERROR, CRITICAL) ---
        logger.add(
            os.path.join(self.log_dir, "error.log"),
            format=file_format,
            level="ERROR",
            filter=specific_filter,  # 关键：绑定过滤器
            rotation="10 MB",
            retention="1 week",
            encoding="utf-8"
          	backtrace=True,  # 记录详细回溯
            diagnose=True,  # 记录变量值
        )

    def get_logger(self):
        """
        返回一个绑定了当前 channel 上下文的 logger
        """
        # 关键：使用 bind 方法，将 channel_name 注入到 extra 字典中
        return logger.bind(channel=self.channel_name)
      
if __name__ == "__main__":
    # --- 场景模拟 ---
    
    # 1. 创建 任务A 的日志管理器，存放在 logs/task_a
    manager_a = ContextLogManager(log_dir="logs/task_a", channel_name="TaskA")
    log_a = manager_a.get_logger()

    # 2. 创建 任务B 的日志管理器，存放在 logs/task_b
    manager_b = ContextLogManager(log_dir="logs/task_b", channel_name="TaskB")
    log_b = manager_b.get_logger()

    # --- 开始打印日志 ---
    
    # 这条只会出现在 logs/task_a/runtime.log
    log_a.info("我是任务 A 的普通日志") 
    
    # 这条只会出现在 logs/task_b/runtime.log
    log_b.info("我是任务 B 的普通日志")
    
    # 这条只会出现在 logs/task_a/error.log 和 runtime.log
    log_a.error("任务 A 出错了！")

    # 验证：你可以去文件夹里看，A 的文件里绝对没有 B 的日志
    print("日志写入完成，请查看 logs 文件夹。")
```

