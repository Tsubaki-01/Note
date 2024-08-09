## extern “C”

编译时候要注意库的内容

如果是C的库，要加extern “C”

从编译器的详细输出来看，链接器仍然无法找到FFmpeg库中的函数。以下是一些进一步的诊断步骤和解决方案：

### 1. 确认库路径和符号
首先，确认库文件确实存在并且包含所需的符号。运行以下命令来检查库文件的位置和符号：

```sh
find /usr/lib /usr/local/lib -name "libavformat*"
find /usr/lib /usr/local/lib -name "libavcodec*"
find /usr/lib /usr/local/lib -name "libavutil*"
find /usr/lib /usr/local/lib -name "libswscale*"

nm -D /usr/lib/x86_64-linux-gnu/libavformat.so | grep avformat_open_input
nm -D /usr/lib/x86_64-linux-gnu/libavcodec.so | grep avcodec_alloc_context3
```

### 2. 使用`pkg-config`验证标志
确保`pkg-config`返回了正确的标志。运行以下命令单独检查每个库的编译和链接标志：

```sh
pkg-config --cflags --libs libavformat
pkg-config --cflags --libs libavcodec
pkg-config --cflags --libs libavutil
pkg-config --cflags --libs libswscale
```

### 3. 检查FFmpeg库版本
确认您使用的是兼容的FFmpeg库版本。不同版本的FFmpeg可能会有API变化，导致链接失败。

### 4. 手动指定库路径
如果知道库文件的位置，可以手动指定库路径。例如，如果库文件在`/usr/local/lib`中，可以使用以下编译命令：

```sh
g++ -std=c++14 -o decode_video test.cpp -L/usr/local/lib -lavformat -lavcodec -lswscale -lavutil
```

### 5. 环境变量设置
确保`LD_LIBRARY_PATH`包含FFmpeg库的路径。例如，如果库文件在`/usr/local/lib`中，可以运行：

```sh
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
```

然后再尝试编译。

### 6. 使用绝对路径链接
使用绝对路径来链接，例如：

```sh
g++ -std=c++14 -o decode_video test.cpp /usr/local/lib/libavformat.so /usr/local/lib/libavcodec.so /usr/local/lib/libswscale.so /usr/local/lib/libavutil.so
```

### 7. 检查系统版本和FFmpeg版本兼容性
确保您的操作系统和FFmpeg版本兼容。如果您从源码编译FFmpeg，请确保所有依赖项都正确配置。

### 示例代码及编译命令

假设您的代码文件名为 `test.cpp`，并且包含如下内容：
```cpp
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

int main() {
    AVFormatContext *fmt_ctx = nullptr;
    if (avformat_open_input(&fmt_ctx, "input.mp4", nullptr, nullptr) != 0) {
        return -1; // Failed to open input file.
    }
    avformat_close_input(&fmt_ctx);
    return 0;
}
```

使用以下命令进行编译：
```sh
g++ -std=c++14 -o decode_video test.cpp $(pkg-config --cflags --libs libavformat libavcodec libavutil libswscale)
```

如果以上步骤仍然无法解决问题，请提供更多详细信息，例如您的操作系统版本、FFmpeg库的安装方式（通过包管理器安装还是从源码编译），以及完整的错误输出。这将有助于进一步诊断问题。