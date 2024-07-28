## Socket

### Socket的基本概念

套接字可以被看作是通信的端点，每个套接字都唯一标识一个网络连接。套接字通常用在客户端和服务器之间的通信，它支持基于流的（TCP）和数据报的（UDP）传输。

### 创建Socket的步骤

在类 Unix 系统（如 Linux）中，使用套接字进行网络编程的一般步骤如下：

1. **创建套接字**：使用 `socket` 函数创建一个套接字。
2. **绑定套接字**（服务器端）：使用 `bind` 函数将套接字绑定到一个特定的IP地址和端口号。
3. **监听连接**（服务器端）：使用 `listen` 函数使套接字进入监听状态，等待客户端连接。
4. **接受连接**（服务器端）：使用 `accept` 函数接受客户端的连接请求，创建一个新的套接字用于通信。
5. **连接到服务器**（客户端）：使用 `connect` 函数连接到服务器的套接字。
6. **发送和接收数据**：使用 `send` 和 `recv` 函数（或者 `read` 和 `write` 函数）进行数据传输。
7. **关闭套接字**：使用 `close` 函数关闭套接字。

### 示例代码

#### TCP 服务器

以下是一个简单的 TCP 服务器示例代码：

```c++
#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    char buffer[1024];

    // 创建套接字
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        return 1;
    }

    // 绑定套接字到端口
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8080);
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        close(server_fd);
        return 1;
    }

    // 监听连接请求
    if (listen(server_fd, 5) == -1) {
        perror("listen");
        close(server_fd);
        return 1;
    }

    std::cout << "Server is listening on port 8080..." << std::endl;

    // 接受客户端连接
    client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
    if (client_fd == -1) {
        perror("accept");
        close(server_fd);
        return 1;
    }

    // 接收并回显消息
    ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received == -1) {
        perror("recv");
        close(client_fd);
        close(server_fd);
        return 1;
    }

    buffer[bytes_received] = '\0';
    std::cout << "Received message: " << buffer << std::endl;

    // 发送回显消息
    if (send(client_fd, buffer, bytes_received, 0) == -1) {
        perror("send");
        close(client_fd);
        close(server_fd);
        return 1;
    }

    // 关闭连接
    close(client_fd);
    close(server_fd);

    return 0;
}
```

#### TCP 客户端

以下是一个简单的 TCP 客户端示例代码：

```c++
#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

int main() {
    int sock_fd;
    struct sockaddr_in server_addr;
    char buffer[1024] = "Hello, Server!";

    // 创建套接字
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        perror("socket");
        return 1;
    }

    // 设置服务器地址
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // 连接到服务器
    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        close(sock_fd);
        return 1;
    }

    // 发送消息
    if (send(sock_fd, buffer, strlen(buffer), 0) == -1) {
        perror("send");
        close(sock_fd);
        return 1;
    }

    // 接收回显消息
    ssize_t bytes_received = recv(sock_fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received == -1) {
        perror("recv");
        close(sock_fd);
        return 1;
    }

    buffer[bytes_received] = '\0';
    std::cout << "Received message: " << buffer << std::endl;

    // 关闭连接
    close(sock_fd);

    return 0;
}
```



## Epoll

`epoll` 是 Linux 内核提供的一个高效的 I/O 事件通知机制，主要用于解决大量并发连接时 I/O 多路复用的性能问题。`epoll` 相对于传统的 `select` 和 `poll`，在处理大量文件描述符时具有更高的效率和更好的性能，因此被广泛应用于高并发服务器编程中。

### epoll 的基本概念

`epoll` 提供了一组系统调用，用于管理和监控多个文件描述符上的事件。它的工作方式是通过一个内核事件表（epoll 实例）来跟踪文件描述符上的事件，并在事件发生时通知应用程序。

### epoll 的系统调用

`epoll` 主要有三个系统调用：

1. **`epoll_create1`**: 创建一个 epoll 实例。

   ```c++
   #include <sys/epoll.h>
    
   int epoll_create(int size);
    
   参数：
   size:目前内核还没有实际使用，只要大于0就行
    
   返回值：
   返回epoll文件描述符
   ```

   

2. **`epoll_ctl`**: 控制 epoll 实例，添加、修改或删除监控的文件描述符。

   ```c++
   #include <sys/epoll.h>
    
   int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
    
   参数：
   epfd：epoll文件描述符
   op：操作码
   EPOLL_CTL_ADD:插入事件
   EPOLL_CTL_DEL:删除事件
   EPOLL_CTL_MOD:修改事件
   fd：事件绑定的套接字文件描述符
   events：事件结构体
    
   返回值：
   成功：返回0
   失败：返回-1
   ```

   #### struct epoll_event结构体

   epoll_event事件结构体

   ```C++
   #include <sys/epoll.h>
    
   struct epoll_event{
     uint32_t events; //epoll事件，参考事件列表 
     epoll_data_t data;
   } ;
   typedef union epoll_data {  
       void *ptr;  
       int fd;  //套接字文件描述符
       uint32_t u32;  
       uint64_t u64;
   } epoll_data_t;
   ```

   epoll事件列表：

   ```c++
   头文件：<sys/epoll.h>
    
   enum EPOLL_EVENTS
   {
       EPOLLIN = 0x001, //读事件
       EPOLLPRI = 0x002,
       EPOLLOUT = 0x004, //写事件
       EPOLLRDNORM = 0x040,
       EPOLLRDBAND = 0x080,
       EPOLLWRNORM = 0x100,
       EPOLLWRBAND = 0x200,
       EPOLLMSG = 0x400,
       EPOLLERR = 0x008, //出错事件
       EPOLLHUP = 0x010, //出错事件
       EPOLLRDHUP = 0x2000,
       EPOLLEXCLUSIVE = 1u << 28,
       EPOLLWAKEUP = 1u << 29,
       EPOLLONESHOT = 1u << 30,
       EPOLLET = 1u << 31 //边缘触发
     };
   ```

   

3. **`epoll_wait`**: 等待事件发生，并返回就绪的文件描述符列表。

   ```c++
   #include <sys/epoll.h>
    
   int epoll_wait(int epfd, struct epoll_event *events,              
   int maxevents, int timeout);
    
   参数：
   epfd：epoll文件描述符
   events：epoll事件数组
   maxevents：epoll事件数组长度
   timeout：超时时间
   小于0：一直等待
   等于0：立即返回
   大于0：等待超时时间返回，单位毫秒
    
   返回值：
   小于0：出错
   等于0：超时
   大于0：返回就绪事件个数
   ```

   

### 使用 epoll 的步骤

下面是使用 `epoll` 的典型步骤：

1. **创建 epoll 实例**：

```c++
int epoll_fd = epoll_create1(0);
if (epoll_fd == -1) {
    perror("epoll_create1");
    exit(EXIT_FAILURE);
}
```

2. **添加文件描述符到 epoll 实例**：

```c++
struct epoll_event event;
event.events = EPOLLIN; // 监控可读事件
event.data.fd = socket_fd; // 要监控的文件描述符

if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &event) == -1) {
    perror("epoll_ctl: EPOLL_CTL_ADD");
    close(epoll_fd);
    exit(EXIT_FAILURE);
}
```

3. **等待事件发生**：

```c++
struct epoll_event events[MAX_EVENTS]; // MAX_EVENTS 是一个常量，表示最大事件数
int num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1); // -1 表示无限等待

if (num_events == -1) {
    perror("epoll_wait");
    close(epoll_fd);
    exit(EXIT_FAILURE);
}

for (int i = 0; i < num_events; i++) {
    if (events[i].events & EPOLLIN) {
        // 处理可读事件
        int fd = events[i].data.fd;
        // 读取数据或进行其他处理
    }
}
```

### 示例代码

以下是一个使用 `epoll` 实现简单服务器的示例代码：

```c++
#include <sys/epoll.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <iostream>

#define MAX_EVENTS 10
#define PORT 8080

int main() {
    int server_fd, new_socket, epoll_fd;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // 创建服务器套接字
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // 设置服务器地址可重用
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // 绑定套接字到端口
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 监听连接
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 创建 epoll 实例
    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("epoll_create1");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 将服务器套接字添加到 epoll 实例
    struct epoll_event event;
    event.events = EPOLLIN; // 可读事件
    event.data.fd = server_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event) == -1) {
        perror("epoll_ctl: server_fd");
        close(server_fd);
        close(epoll_fd);
        exit(EXIT_FAILURE);
    }

    // 事件循环
    while (true) {
        struct epoll_event events[MAX_EVENTS];
        int num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (num_events == -1) {
            perror("epoll_wait");
            close(server_fd);
            close(epoll_fd);
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < num_events; i++) {
            if (events[i].data.fd == server_fd) {
                // 处理新的连接
                new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
                if (new_socket == -1) {
                    perror("accept");
                    continue;
                }

                // 将新连接的套接字添加到 epoll 实例
                event.events = EPOLLIN;
                event.data.fd = new_socket;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_socket, &event) == -1) {
                    perror("epoll_ctl: new_socket");
                    close(new_socket);
                    continue;
                }
            } else {
                // 处理已连接的客户端数据
                int client_fd = events[i].data.fd;
                char buffer[1024] = {0};
                ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer));
                if (bytes_read <= 0) {
                    // 客户端断开连接或发生错误
                    close(client_fd);
                } else {
                    // 回显收到的数据
                    send(client_fd, buffer, bytes_read, 0);
                }
            }
        }
    }

    close(server_fd);
    close(epoll_fd);
    return 0;
}
```

### epoll的优势

- **高效性**：`epoll` 在处理大量文件描述符时，比 `select` 和 `poll` 更高效，因为它避免了每次调用时线性扫描文件描述符列表。
- **水平触发和边缘触发**：`epoll` 支持两种触发模式，水平触发（LT）和边缘触发（ET）。边缘触发在高性能服务器中非常有用，因为它减少了事件通知的次数。
- **内核空间与用户空间通信效率高**：`epoll` 通过一个事件列表与内核通信，避免了重复的数据拷贝。

### epoll为什么用红黑树而不是哈希表或者B+树

**红黑树的优点**

1. **自平衡二叉搜索树**： 红黑树是一种自平衡二叉搜索树，能够确保插入、删除和查找操作的时间复杂度为 𝑂(log⁡𝑁)*O*(log*N*)。这种自平衡特性使得红黑树能够高效地管理动态变化的文件描述符集合，特别是在有大量插入和删除操作的情况下。
2. **有序性**： 红黑树中的节点是有序的，能够方便地进行范围查询或排序操作。尽管 `epoll` 本身不需要有序性，但这种特性使得红黑树在处理需要排序或范围查询的任务时非常高效。

**为什么不选择哈希表**

1. **不好缩容**：红黑树缩容简单，哈希表扩容缩容困难。

2. **性能一致性**： 哈希表的平均时间复杂度为 𝑂(1)*O*(1)，但在最坏情况下可能退化为 𝑂(𝑁)*O*(*N*)，因为哈希冲突可能导致所有元素落入同一个桶，从而退化成链表。尽管通过设计合理的哈希函数可以降低这种风险，但哈希表在最坏情况下的性能不可预测性使得其在某些实时或高要求的应用场景中不如红黑树可靠。

3. **有序性**： 哈希表不保证元素的有序性。如果应用需要对文件描述符进行有序管理，哈希表将不适用。

4. **内存使用**： 哈希表需要处理哈希冲突，通常会有额外的内存开销（如链表或其他冲突解决方法），这在内核的实现中需要权衡内存使用效率。

**为什么不选择B+树**

1. **内存复杂度**： B+树是多叉树，通常用于数据库和文件系统中，以便在磁盘I/O时减少树的高度，从而减少磁盘访问次数。B+树的节点包含多个元素和指针，内存管理相对复杂，在内存中操作时没有明显优势。
2. **适用场景**： B+树特别适用于磁盘或其他块设备上的大数据量存储场景，因为它能够有效减少磁盘I/O次数。然而，在内存中的文件描述符管理中，红黑树已经足够高效且实现简单。

**epoll 的实现考量**

`epoll` 的设计目标是提供一种高效、可扩展的 I/O 多路复用机制。选择红黑树是基于其在平衡插入、删除和查找操作时提供一致性能的特性。红黑树能够在保证操作时间复杂度的同时，保持较低的实现复杂度和内存开销，这使其成为管理大量动态文件描述符集合的理想选择。

## **ET、LT、EPOLLONESHOT**

- LT水平触发模式

- - epoll_wait检测到文件描述符有事件发生，则将其通知给应用程序，应用程序可以不立即处理该事件。
  - 当下一次调用epoll_wait时，epoll_wait还会再次向应用程序报告此事件，直至被处理

- ET边缘触发模式

- - epoll_wait检测到文件描述符有事件发生，则将其通知给应用程序，应用程序必须立即处理该事件
  - 必须要一次性将数据读取完，使用非阻塞I/O，读取到出现eagain

- EPOLLONESHOT

- - 一个线程读取某个socket上的数据后开始处理数据，在处理过程中该socket上又有新数据可读，此时另一个线程被唤醒读取，此时出现两个线程处理同一个socket
  - 我们期望的是一个socket连接在任一时刻都只被一个线程处理，通过epoll_ctl对该文件描述符注册epolloneshot事件，一个线程处理socket时，其他线程将无法处理，**当该线程处理完后，需要通过epoll_ctl重置epolloneshot事件**

## **select/poll/epoll**

- 调用函数

- - select和poll都是一个函数，epoll是一组函数

- 文件描述符数量

- - select通过线性表描述文件描述符集合，文件描述符有上限，一般是1024，但可以修改源码，重新编译内核，不推荐
  - poll是链表描述，突破了文件描述符上限，最大可以打开文件的数目
  - epoll通过红黑树描述，最大可以打开文件的数目，可以通过命令ulimit -n number修改，仅对当前终端有效

- 将文件描述符从用户传给内核

- - select和poll通过将所有文件描述符拷贝到内核态，每次调用都需要拷贝
  - epoll通过epoll_create建立一棵红黑树，通过epoll_ctl将要监听的文件描述符注册到红黑树上

- 内核判断就绪的文件描述符

- - select和poll通过遍历文件描述符集合，判断哪个文件描述符上有事件发生
  - epoll_create时，内核除了帮我们在epoll文件系统里建了个红黑树用于存储以后epoll_ctl传来的fd外，还会再建立一个list链表，用于存储准备就绪的事件，当epoll_wait调用时，仅仅观察这个list链表里有没有数据即可。
  - epoll是根据每个fd上面的回调函数(中断函数)判断，只有发生了事件的socket才会主动的去调用 callback函数，其他空闲状态socket则不会，若是就绪事件，插入list

- 应用程序索引就绪文件描述符

- - select/poll只返回发生了事件的文件描述符的个数，若知道是哪个发生了事件，同样需要遍历
  - epoll返回的发生了事件的个数和结构体数组，结构体包含socket的信息，因此直接处理返回的数组即可

- 工作模式

- - select和poll都只能工作在相对低效的LT模式下
  - epoll则可以工作在ET高效模式，并且epoll还支持EPOLLONESHOT事件，该事件能进一步减少可读、可写和异常事件被触发的次数。 

- 应用场景

- - 当所有的fd都是活跃连接，使用epoll，需要建立文件系统，红黑书和链表对于此来说，效率反而不高，不如selece和poll
  - 当监测的fd数目较小，且各个fd都比较活跃，建议使用select或者poll
  - 当监测的fd数目非常大，成千上万，且单位时间只有其中的一部分fd处于就绪状态，这个时候使用epoll能够明显提升性能

