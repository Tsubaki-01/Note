# Model Context Protocol（MCP）2026-07-28 完整学习总结

> 这是一份面向学习、实现和排错的中文导读，依据官方 MCP 规范 2026-07-28 版本整理。它解释协议的心智模型、报文、生命周期、传输、服务端与客户端能力、授权、安全和版本迁移；字段的最终约束仍以同目录下的官方 Schema 与原文为准。
>
> 官方规范入口：<https://modelcontextprotocol.io/specification/2026-07-28>
> 官方完整索引：<https://modelcontextprotocol.io/llms.txt>
> 本地规范快照：[README.md](README.md)

## 0. 先建立正确的边界：协议、SDK、配置分别是什么

MCP 不是某个固定的配置文件格式，也不是某个特定厂商的插件系统。它是线上的通信契约：

1. 一端是 LLM 应用中的 Host，Host 内可以管理多个 Client。
2. 每个 Client 与一个 Server 建立一对一的 MCP 通信关系。
3. 双方使用 JSON-RPC 2.0 消息，通过 STDIO 或 Streamable HTTP 等传输方式交互。
4. Server 暴露工具、资源、提示词等能力；Client 代表 Host 提供用户输入、模型采样或工作区信息等能力。

配置文件属于 MCP 之上的启动和部署层。例如，配置可以描述：

- 运行哪个 Server 命令、传哪些参数和环境变量；
- 连接哪个 HTTP URL；
- 使用什么凭据、代理或本地权限；
- Host 如何把 Server 展示给用户。

配置决定“连接谁、怎么启动”，MCP 规范决定“连接后消息必须长什么样、语义是什么”。不同 Host 的配置文件可以完全不同，只要它们在线上遵守同一版本的 MCP 协议即可。

## 1. 一句话心智模型

**MCP 是一个以 JSON-RPC 2.0 为消息格式、以每请求元数据和能力声明为协商基础、以工具（Tools）、资源（Resources）和提示词（Prompts）为主要服务端原语的无状态上下文连接协议。**

一次典型调用可以抽象为：

~~~text
用户/模型意图
    ↓
Host 选择一个 Client
    ↓
Client 发出带协议版本和能力的 JSON-RPC 请求
    ↓
Transport（STDIO 或 Streamable HTTP）传送报文
    ↓
Server 校验、执行工具或读取资源
    ↓
Server 返回 complete / input_required / error
    ↓
Client 展示结果、继续多轮输入，或把结果交给模型
~~~

四个最重要的判断：

1. MCP 不等于“把函数注册给模型”：工具只是服务端原语之一，资源和提示词同样重要。
2. MCP 不等于“长连接状态协议”：2026-07-28 的核心设计是每个请求自描述，不能依赖之前的连接状态。
3. MCP 不等于“服务端可以随时调用客户端”：现代规范禁止服务端发起普通 JSON-RPC request，改用 Multi Round-Trip Requests（MRTR）。
4. MCP 不等于授权协议：OAuth 授权是 HTTP 场景下的可选但规范化的传输层安全机制，不能替代 MCP 报文协议。

## 2. 架构：Host、Client、Server 各自负责什么

| 角色 | 所在位置 | 主要职责 | 不应承担的职责 |
|---|---|---|---|
| Host | LLM 应用、IDE、聊天应用或 Agent 容器 | 管理多个 Client，聚合上下文，连接模型，执行用户同意与安全策略 | 不应把一个 Server 的内部上下文泄露给另一个 Server |
| Client | Host 中的连接器实例 | 与一个 Server 通信，路由请求、响应、通知，管理订阅和能力声明 | 不应把 Server 自报的名称当作安全身份 |
| Server | 本地进程或远程服务 | 提供 Tools、Resources、Prompts，按请求执行并返回结果 | 不应假设自己能看到完整对话、其他 Server 或 Host 内部状态 |

规范鼓励可组合 Server：一个 Server 可以只提供文件资源，另一个只提供 Git 工具，Host 再把它们聚合给模型。Server 之间不直接共享协议上下文。

### 2.1 控制权的三分法

| 原语 | 谁控制是否使用 | 典型用途 | 典型方法 |
|---|---|---|---|
| Prompts | 用户控制 | 斜杠命令、菜单、预设工作流 | prompts/list、prompts/get |
| Resources | 应用控制 | 把文件、文档、记录等上下文交给模型或用户 | resources/list、resources/read |
| Tools | 模型控制，但必须受用户授权 | 查询、写入、执行动作、调用外部 API | tools/list、tools/call |

“模型控制”不等于“无需确认”。Host 应允许用户查看工具及参数、拒绝调用，并对副作用操作设置人工确认。

## 3. 基础不变量：实现 MCP 时必须先记住这些规则

### 3.1 消息层

- 所有消息都是 UTF-8 编码的 JSON-RPC 2.0。
- JSON-RPC request 必须有 jsonrpc: 2.0、非空字符串或整数 id、method，可带 params。
- 同一端所有未完成请求的 ID 必须唯一；重试是新请求，必须使用新 ID。
- response 必须带对应 ID，并且二选一：result 或 error。
- notification 没有 ID，不期待 response。
- params 和 result 的具体形状由 MCP 方法 Schema 决定。

### 3.2 结果类型

现代 MCP 的每一个成功 result 都必须有 resultType：

- complete：本次请求完成，结果是最终结果；
- input_required：本次请求尚未完成，客户端需要收集额外输入，再重试原请求；
- 扩展可以注册其他值，但必须通过能力或扩展协商；客户端遇到不认识的值必须视为无效。

为兼容旧版本，客户端收到缺失 resultType 的旧服务器响应时，必须按 complete 处理。

### 3.3 服务端不是普通 JSON-RPC 请求发起者

现代核心模式是：

- Client → Server：request、notification；
- Server → Client：response、notification；
- Server 需要客户端提供额外信息时：返回 input_required，而不是发一个普通的服务端 request。

例外能力（Roots、Sampling、Elicitation）都通过 MRTR 携带在 inputRequests 中。

### 3.4 每请求自描述与无状态

现代每个 Client request 都必须在 params._meta 中携带：

- io.modelcontextprotocol/protocolVersion：请求使用的协议版本；
- io.modelcontextprotocol/clientCapabilities：该请求允许 Server 使用的客户端能力。

io.modelcontextprotocol/clientInfo 可选，但客户端应尽量每次携带；它和服务端的 serverInfo 都是自报信息，不能用作认证或安全决策。

这意味着：

- Server 不能从上一个请求推断本请求的能力；
- 一条连接或一个 STDIO 进程不等于一个会话；
- 需要跨请求的状态必须显式放在业务参数、返回的 opaque handle，或受完整性保护的 requestState 中；
- 不能用“连接没有断”来代替权限、租约、身份或业务状态管理。

## 4. _meta：现代 MCP 的请求上下文

### 4.1 请求的公共形状

~~~json
{
  "jsonrpc": "2.0",
  "id": 42,
  "method": "tools/list",
  "params": {
    "_meta": {
      "io.modelcontextprotocol/protocolVersion": "2026-07-28",
      "io.modelcontextprotocol/clientCapabilities": {},
      "io.modelcontextprotocol/clientInfo": {
        "name": "ExampleClient",
        "version": "1.0.0"
      }
    }
  }
}
~~~

缺少协议版本或客户端能力时，Server 应返回 -32602 Invalid params；HTTP 同时返回 400 Bad Request。

### 4.2 常用保留字段

| _meta 键 | 作用 |
|---|---|
| io.modelcontextprotocol/protocolVersion | 本请求采用的 MCP 版本；HTTP 还必须与 MCP-Protocol-Version 一致 |
| io.modelcontextprotocol/clientCapabilities | 本请求可用的客户端能力 |
| io.modelcontextprotocol/clientInfo | 客户端名称和版本，供显示、日志、调试 |
| io.modelcontextprotocol/logLevel | 请求级日志选择；Logging 已弃用 |
| progressToken | 请求进度通知的关联令牌 |
| io.modelcontextprotocol/subscriptionId | 订阅流上通知与 subscriptions/listen 的关联 |
| traceparent、tracestate、baggage | OpenTelemetry/W3C trace 上下文 |

自定义 _meta 键应使用带前缀的命名空间。MCP 保留 io.modelcontextprotocol/ 和相应 MCP 前缀；第三方扩展应使用自己的反向 DNS 风格前缀。

### 4.3 响应元数据

Server 应在 result 的 _meta 中携带 io.modelcontextprotocol/serverInfo。订阅流上的每个通知必须携带 io.modelcontextprotocol/subscriptionId，否则客户端无法在共享通道上分流。

## 5. JSON-RPC 错误与工具执行错误

### 5.1 主要错误码

| 错误码 | 含义 | 典型原因 |
|---:|---|---|
| -32700 | Parse error | JSON 无法解析 |
| -32600 | Invalid Request | 不是合法 JSON-RPC 请求 |
| -32601 | Method not found | 方法或能力不存在 |
| -32602 | Invalid params | 参数、Schema、cursor、URI 等无效 |
| -32603 | Internal error | Server 内部异常 |
| -32020 | HeaderMismatch | HTTP 头与 JSON body 不一致 |
| -32021 | MissingRequiredClientCapability | 请求需要但客户端未声明的能力 |
| -32022 | UnsupportedProtocolVersion | Server 不支持请求版本 |

资源不存在在当前版本使用 -32602；客户端可以为旧实现兼容旧的 -32002。

### 5.2 协议错误 vs 工具业务错误

- **协议错误**：请求无法按 MCP 语义处理，例如未知方法、参数结构错误、版本不支持；使用 JSON-RPC error response。
- **工具执行错误**：工具已经被正确识别并尝试执行，但业务动作失败；使用正常 result，并设置 isError: true，在 content 或 structuredContent 中返回可理解的错误信息。

客户端应把工具执行错误传给模型或用户，让模型有机会修正参数；不要把所有业务失败都伪装成 -32603。

## 6. 版本、能力与发现

### 6.1 现代版本机制

2026-07-28 不再使用旧的 initialize/initialized 握手作为现代连接的基础。每个请求直接携带版本和客户端能力；Server 必须实现 server/discover。

server/discover 返回：

- supportedVersions：Server 支持的版本集合；
- capabilities：Server 能力；
- io.modelcontextprotocol/serverInfo；
- 可选 instructions；
- 必须有缓存提示 ttlMs、cacheScope。

客户端可以先调用 server/discover，也可以直接请求后根据 UnsupportedProtocolVersion 处理。Server 不能把 serverInfo 当作可信身份。

### 6.2 能力协商原则

能力是 map，而不是“默认都支持”。常见 Server 能力包括：

~~~json
{
  "capabilities": {
    "tools": { "listChanged": true },
    "resources": { "listChanged": true, "subscribe": true },
    "prompts": { "listChanged": true },
    "completions": {}
  }
}
~~~

常见 Client 能力包括 elicitation、sampling、roots 和协商成功的 extensions。Server 只有在当前请求的 clientCapabilities 中看到对应能力时，才可以把相应的 input request 放进 MRTR。

扩展必须显式协商；不支持扩展时应回退到核心协议，或明确拒绝，而不能静默假设扩展存在。

### 6.3 与旧版本兼容

- **现代版本**：2026-07-28 及以后，使用每请求 _meta 和 server/discover。
- **旧版本**：2025-11-25 及以前，使用 initialize 握手和旧传输行为。
- STDIO 可先探测 server/discover；识别出明确的现代版本错误后按现代方式重试，超时或普通旧错误可回退 initialize。
- HTTP 可先发送现代 POST；仅在收到可识别的现代 JSON-RPC 400 错误时按规则判断，不能把任意空响应都当作旧协议成功。

实现应按进程或 HTTP origin 缓存“该对端属于哪个时代”，但不能把一次请求的业务状态当成永久协议状态。

## 7. 生命周期：一条现代 MCP 交互怎么发生

### 7.1 客户端侧

1. 读取配置或 UI 输入，确定 STDIO 命令或 HTTP endpoint。
2. 建立传输；HTTP 场景先按需要完成 OAuth。
3. 发送带 protocolVersion 和 clientCapabilities 的请求。
4. 调用 server/discover，或直接调用已知方法。
5. 按能力决定是否读取 tools/prompts/resources 列表。
6. 对分页结果持续使用不透明 cursor；对缓存结果遵守 TTL 和 scope。
7. 对工具调用执行用户确认、超时、取消、审计和输出展示。
8. 断线后重新建立传输、重新发现能力、重新建立订阅；不要假设 Server 保留了订阅状态。

### 7.2 服务端侧

1. 解析传输帧，验证 JSON-RPC 和请求 _meta。
2. 校验请求版本、客户端能力、权限和方法参数。
3. 若需要额外客户端输入，对 prompts/get、resources/read 或 tools/call 返回 input_required。
4. 否则执行操作，发送 progress/log 通知（若已请求），最后发送 complete 或 JSON-RPC error。
5. 对 HTTP 请求进行 Origin、Authorization、头体一致性和输入校验。
6. 进程退出或连接中断时清理正在运行的操作和订阅。

## 8. Transport：相同语义，不同承载

Transport 只负责帧如何分隔、如何交付、如何取消和如何承载 HTTP 元数据；JSON-RPC 和 MCP 方法语义不应因 Transport 改变。

### 8.1 STDIO

- Client 启动子进程；Server 从 stdin 读、向 stdout 写。
- 每行一个 UTF-8 JSON-RPC 消息；消息不得跨行。
- stdout 只能有合法 MCP 消息；日志应写 stderr。
- Server 不得向 stdout 写普通文本，也不得发服务端 JSON-RPC request。
- 所有请求共享一条通道，客户端用 ID 匹配 response，并用 subscriptionId 区分订阅通知。
- 取消使用 notifications/cancelled；没有每请求 SSE 可关闭。
- Client 关闭 stdin 后等待 Server 退出，必要时强制结束；Server 应在 EOF 后及时退出。
- Server 意外退出时，in-flight 请求视为丢失；客户端应以新 ID 重试，并重新建立订阅。
- Windows 实现可使用 TerminateProcess 或 Job Object 防止子进程泄漏。

一个最小 STDIO 消息序列：

~~~text
Client -> stdin: {"jsonrpc":"2.0","id":1,"method":"server/discover","params":{...}}
Server -> stdout: {"jsonrpc":"2.0","id":1,"result":{"resultType":"complete",...}}
Client -> stdin: {"jsonrpc":"2.0","id":2,"method":"tools/call","params":{...}}
Server -> stdout: {"jsonrpc":"2.0","method":"notifications/progress","params":{...}}
Server -> stdout: {"jsonrpc":"2.0","id":2,"result":{"resultType":"complete",...}}
~~~

### 8.2 Streamable HTTP

现代 Streamable HTTP 的核心约束：

- 一个 MCP endpoint，现代请求使用 POST；GET/DELETE 对现代-only 实现应返回 405。
- 每个请求或 notification 都是独立 POST。
- 请求响应可以是一个 application/json，也可以是 request-scoped 的 text/event-stream；客户端必须接受两者。
- 客户端应在 POST 的 Accept 中同时声明 application/json, text/event-stream。
- notification 成功通常返回 202 且无 body；request 返回 JSON 或 SSE。
- 不再有 Mcp-Session-Id 协议会话，也不再有 Last-Event-ID 恢复语义；流丢失后，以新 ID 重发请求。
- 长期变更通知通过 subscriptions/listen 的 SSE 流，而不是旧的 GET stream。
- Server 必须校验 Origin；无效 Origin 返回 403；本机服务推荐只绑定 localhost。

现代 POST 的协议头：

| Header | 作用 |
|---|---|
| MCP-Protocol-Version | 必填；必须等于 body 的 protocolVersion |
| Mcp-Method | 必须等于 JSON-RPC method |
| Mcp-Name | 对 tools/call、resources/read、prompts/get 镜像其名称或 URI |
| Mcp-Param-{Name} | 可选；由 x-mcp-header Schema 注解声明可镜像的简单参数 |

Header 与 body 不一致必须返回 400 和 -32020 HeaderMismatch。包含不安全 ASCII、首尾空白或 sentinel 冲突的自定义值，使用规范要求的 =?base64?...?= 编码。Server 不得把敏感值标记为普通参数头。

x-mcp-header 只适合静态属性链上的 primitive string/integer/boolean；不支持数组和一般 number。客户端应验证 tools/list 中的注解，发现 malformed tool 时拒绝该工具，而不是猜测。

### 8.3 旧 HTTP+SSE

HTTP+SSE 是 2025-03-26 起被 Streamable HTTP 替代的旧传输。2026-07-28 仍为弃用兼容项，不应用于新实现。旧的 session、GET 事件流和 resumability 不能混入现代无状态实现。

## 9. 三种核心交互模式

### 9.1 普通 request/response

Client 发 request，Server 可发送与该请求关联的 progress 或 log notification，最后返回 result/error。

### 9.2 Multi Round-Trip Requests（MRTR）

MRTR 解决“Server 执行到一半需要用户、模型或工作区信息”的问题，同时保持跨请求无状态。

支持返回 input_required 的初始请求只有：

- prompts/get
- resources/read
- tools/call

Server 返回：

~~~json
{
  "jsonrpc": "2.0",
  "id": 1,
  "result": {
    "resultType": "input_required",
    "inputRequests": {
      "approval": {
        "method": "elicitation/create",
        "params": {
          "mode": "form",
          "message": "是否允许继续？",
          "requestedSchema": {
            "type": "object",
            "properties": {
              "approved": { "type": "boolean" }
            },
            "required": ["approved"],
            "additionalProperties": false
          }
        }
      }
    },
    "requestState": "opaque-server-state"
  }
}
~~~

客户端收集所有 inputRequests 的结果后，重试原 method，追加 inputResponses 和原样回显的 requestState：

~~~json
{
  "jsonrpc": "2.0",
  "id": 2,
  "method": "tools/call",
  "params": {
    "_meta": {
      "io.modelcontextprotocol/protocolVersion": "2026-07-28",
      "io.modelcontextprotocol/clientCapabilities": {
        "elicitation": { "form": {} }
      }
    },
    "name": "dangerous_operation",
    "arguments": {},
    "inputResponses": {
      "approval": {
        "action": "accept",
        "content": { "approved": true }
      }
    },
    "requestState": "opaque-server-state"
  }
}
~~~

MRTR 的关键安全要求：

- inputRequests 的键只在本次请求范围内唯一；
- Server 每次至少返回 inputRequests 或 requestState 之一；
- Server 不能要求客户端支持未声明的能力；
- 客户端必须原样回显 requestState，不得解析、修改或跨并行请求复用；
- requestState 应由 Server 完整性保护，通常绑定用户主体、短 TTL、原方法和关键参数摘要；
- 必须防止跨用户、跨请求重放；若业务要求一次性消费，还要在服务端记录已消费状态；
- 初始请求和重试的 JSON-RPC ID 必须不同；
- Server 不能假设客户端一定会完成输入或重试。

### 9.3 Subscribe/notify

客户端发送 subscriptions/listen，参数是通知过滤器：

~~~json
{
  "jsonrpc": "2.0",
  "id": 10,
  "method": "subscriptions/listen",
  "params": {
    "_meta": {
      "io.modelcontextprotocol/protocolVersion": "2026-07-28",
      "io.modelcontextprotocol/clientCapabilities": {}
    },
    "notifications": {
      "toolsListChanged": true,
      "promptsListChanged": true,
      "resourcesListChanged": true,
      "resourceSubscriptions": ["file:///project/config.json"]
    }
  }
}
~~~

Server 必须先发送 notifications/subscriptions/acknowledged，之后才可发送该订阅请求的通知。每条通知都在 _meta.io.modelcontextprotocol/subscriptionId 中携带原 subscriptions/listen 的 request ID。

支持的过滤项：

- toolsListChanged
- promptsListChanged
- resourcesListChanged
- resourceSubscriptions（指定资源 URI）

HTTP 通过关闭 SSE 流取消订阅；STDIO 发送引用订阅 request ID 的 notifications/cancelled。Server 主动优雅关闭时，应先返回 complete response，再关闭流。重连后客户端必须重新 listen。

## 10. 取消、超时与进度

### 10.1 取消

客户端应为所有请求设置可配置的超时；超时后停止等待并发出取消信号：

- Streamable HTTP：关闭该请求的 SSE response stream，断开即表示取消；
- STDIO：发送 notifications/cancelled，引用原 request ID。

Server 收到取消后应尽快停止处理、释放资源，并且不再发送被取消请求的 response。由于竞态，Server 可能已经完成并发出 response；双方必须能忽略这种迟到消息。

Server 发送 notifications/cancelled 只用于终止 subscriptions/listen，不能用来取消其他服务端动作。

### 10.2 进度

客户端在请求 _meta 中设置唯一 progressToken：

~~~json
{
  "_meta": {
    "io.modelcontextprotocol/protocolVersion": "2026-07-28",
    "io.modelcontextprotocol/clientCapabilities": {},
    "progressToken": "job-abc"
  }
}
~~~

Server 可以发送：

~~~json
{
  "jsonrpc": "2.0",
  "method": "notifications/progress",
  "params": {
    "progressToken": "job-abc",
    "progress": 50,
    "total": 100,
    "message": "正在处理文件"
  }
}
~~~

progress 必须逐次增加；total 可省略；通知应在人类可读且不过于频繁。进度在最终 response 后必须停止。超时可以因持续进度而延长，但必须有最大上限。

## 11. Server 原语一：Tools

Tools 是**模型可调用的动作接口**。Server 声明 tools 能力后提供 tools/list 和 tools/call。

### 11.1 Tool 定义

一个工具通常包含：

- name：Server 内唯一、区分大小写；建议 1–128 个字符，使用字母、数字、下划线、连字符、点；
- title：面向用户的显示名；
- description：用途与副作用说明；
- icons：展示图标；
- inputSchema：有效的 JSON Schema object，默认方言 2020-12；
- outputSchema：可选的结构化结果 Schema；
- annotations：行为提示，除非来自可信 Server，否则应视为不可信。

无参数工具推荐：

~~~json
{
  "type": "object",
  "properties": {},
  "additionalProperties": false
}
~~~

tools/list 支持分页、缓存和变更通知。工具顺序应稳定，方便模型和 UI 比较差异。多个 Server 聚合时，客户端应通过前缀或内部命名空间消除名称冲突。

### 11.2 调用与返回

~~~json
{
  "jsonrpc": "2.0",
  "id": 20,
  "method": "tools/call",
  "params": {
    "_meta": {
      "io.modelcontextprotocol/protocolVersion": "2026-07-28",
      "io.modelcontextprotocol/clientCapabilities": {}
    },
    "name": "get_weather",
    "arguments": {
      "city": "Nanjing"
    }
  }
}
~~~

Tool result 可包含：

- content：文本、图片、音频、resource link 或 embedded resource；
- structuredContent：任意 JSON 结构；
- isError：工具执行失败时为 true。

若存在 outputSchema，Server 必须生成符合 Schema 的 structuredContent；客户端应校验。structuredContent 是 MCP 结果结构，不等于模型 API 的 structured output；为兼容旧客户端，建议同时放一个可读的 JSON 文本 content。

### 11.3 工具安全

Server 必须验证输入、权限、租户边界、速率和资源限制，并清理输出中的凭据、内部路径和提示注入。Client/Host 应：

- 展示工具名称、说明、参数和副作用；
- 对写入、删除、发送、执行命令等动作请求用户确认；
- 设置超时和取消；
- 记录审计事件；
- 不把工具描述、annotations 或返回内容自动视为可信指令。

有状态工具应在普通参数和结果中使用不透明 handle；每次调用都重新检查 handle 与用户授权，设置 TTL 和明确的 expired 错误，不能借助 MCP session 隐式保存状态。

## 12. Server 原语二：Resources

Resources 是**由应用控制的上下文数据**，使用 URI 标识。Server 声明 resources 后提供：

- resources/list：分页列出具体资源；
- resources/read：读取 URI，返回 text 或 base64 blob；
- resources/templates/list：列出 RFC 6570 URI 模板；
- subscriptions/listen：接收资源列表和资源内容更新通知；
- completion/complete：为模板参数提供建议。

资源定义常见字段：

- uri、name；
- 可选 title、description、icons、mimeType、size；
- annotations.audience：user 或 assistant；
- annotations.priority：0–1；
- annotations.lastModified：ISO 8601 时间。

支持 https、file、git 和自定义 RFC 3986 URI。file 是类似文件系统的 URI 语义，不保证一定对应本地真实文件。

读取结果不能用空 contents 表示“不存在”；当前版本资源未找到使用 -32602。Server 应校验 URI、阻止路径穿越、执行授权检查，并正确区分文本和 base64 二进制。

资源列表可因授权上下文不同而不同，但不应因“连接已经建立多久”而偷偷改变。更新通知只说明“需要重新读取”，不直接携带完整资源内容。

## 13. Server 原语三：Prompts

Prompts 是**由用户选择的模板或工作流**。Server 声明 prompts 后提供：

- prompts/list：分页列出 prompt；
- prompts/get：按名称和参数生成消息；
- subscriptions/listen：接收 prompt 列表变更；
- completion/complete：为参数提供补全。

Prompt 定义可包含 name、title、description、icons 和参数声明。prompts/get 返回一组 role 为 user 或 assistant 的消息；每条消息内容可为文本、图片、音频、resource link 或 embedded resource。

Prompt 的参数、模板内容和引用资源都要做输入校验和输出安全检查。Prompt 是用户入口，不代表用户已经授权 Server 执行任何工具；模型收到 prompt 后仍须按照 Host 的工具确认和数据策略行动。

## 14. Client 能力：Elicitation、Sampling、Roots

### 14.1 Elicitation：让 Server 请求用户输入

Elicitation 是当前保留且推荐理解的 Client 能力。Server 通过 MRTR 携带 elicitation/create，客户端负责 UI 和用户同意。

两种模式：

1. **form**：受限的扁平 object Schema，只适合 string、number、integer、boolean、enum、多选和少量格式（email、uri、date、date-time）。
2. **url**：让用户在隔离浏览器中完成第三方认证或敏感操作；URL 不应包含秘密，客户端不得预取或自动打开，必须显示完整 URL 并取得明确同意。

响应 action：

- accept：用户提供内容；URL 模式通常不带 form content；
- decline：用户拒绝；
- cancel：用户取消。

accept 不代表外部 OAuth 流程已经完成；Server 可能要求客户端重试原请求。密码、令牌、私钥等秘密不得放进 form，使用 URL 模式并让第三方直接处理。

### 14.2 Sampling：让 Server 请求模型生成

Sampling 允许 Server 借用 Client/Host 的模型，而不持有 Host 的 API key。它支持消息、modelPreferences、systemPrompt、maxTokens、temperature、stop、metadata 和可选的工具使用。

要点：

- modelPreferences 是 advisory；客户端决定实际模型；
- maxTokens 必须尊重；
- tool use 必须让每个 tool_use 与 tool_result 配对；
- toolChoice 可为 auto、required、none；
- 建议保留人工确认和迭代上限。

Sampling 在 2026-07-28 中已标记弃用，至少保留十二个月；新系统应把它当作兼容能力，不要围绕它设计不可迁移的核心架构。includeContext 的 thisServer、allServers 也已弃用，默认应是 none。

### 14.3 Roots：向 Server 提供工作区根

Roots 是信息提示，不是访问控制。当前根 URI 必须是 file://；Server 仍须在每次访问时执行权限、路径和租户检查。

Roots 在 2026-07-28 中也已弃用；新的实现应优先：

- 通过工具参数传目录或文件；
- 通过 Resource URI 提供上下文；
- 在 Server 配置中明确允许的路径。

## 15. Utilities：分页、缓存、补全、日志、图标

### 15.1 分页

分页使用不透明字符串 cursor，而不是页码。支持分页的操作：

- tools/list
- prompts/list
- resources/list
- resources/templates/list

客户端只能判断 cursor 是否存在，不能解析、排序或修改它；空字符串也是有效 cursor。没有 nextCursor 才表示结束。cursor 无效通常返回 -32602。

### 15.2 缓存

以下 complete result 必须带 ttlMs 和 cacheScope：

- server/discover
- tools/list
- prompts/list
- resources/list
- resources/templates/list
- resources/read

缓存键至少由 method 和影响结果的请求参数组成；带 inputResponses 或 requestState 的 MRTR 重试不可缓存。

| 字段 | 语义 |
|---|---|
| ttlMs | 新鲜度提示，0 表示立即过期；负数按 0 处理；不是数据不变的保证 |
| cacheScope: public | 可跨用户共享；只适用于确实不含用户数据的结果 |
| cacheScope: private | 只能在同一授权上下文复用；不同 token 不得共享 |

通知可以使仍在 TTL 内的缓存立即失效。列表分页的每一页独立缓存，不能保证跨页一致快照；cursor 失效时应丢弃所有缓存页，从头读取。

### 15.3 Completion

Server 声明 completions 后实现 completion/complete，为 prompt 或 resource template 参数提供建议。

请求引用：

- ref/prompt：prompt 名称；
- ref/resource：资源 URI 或 URI 模板。

返回 values、可选 total 和 hasMore；每次最多 100 项。客户端应 debounce，Server 应限流、按相关性排序并避免泄露敏感候选。

### 15.4 Logging

Logging 已弃用。新实现不要依赖 logging/setLevel 或旧的持久日志通道：

- STDIO 优先写 stderr；
- 结构化观测优先使用 OpenTelemetry；
- 兼容实现可在单个请求 _meta.io.modelcontextprotocol/logLevel 中 opt in；
- 未设置 logLevel 时，Server 不得发送该请求的 notifications/message。

### 15.5 Icons 与内容块

图标是 UI 元数据，不是安全凭据。来源只应使用 HTTPS 或 data URI；PNG/JPEG 必须支持，SVG/WebP 可选但要当作不可信字节处理。不要带凭据抓取图标。

统一的 content block 可以是：

- text
- image（base64 + MIME）
- audio（base64 + MIME）
- resource_link
- embedded resource

资源链接只是链接，不保证一定出现在 resources/list 中。

## 16. HTTP 授权：OAuth 不是 MCP 方法，而是保护 Transport

授权规范只针对 HTTP；STDIO 通常从环境变量或启动配置取得凭据，不应套用浏览器 OAuth discovery。

### 16.1 角色和发现

- MCP Server 是 OAuth Resource Server；
- MCP Client 是 OAuth Client；
- Authorization Server 负责用户登录、同意和发 token。

受保护的 MCP Server 必须提供 OAuth 2.0 Protected Resource Metadata（RFC 9728），声明 authorization_servers。客户端依据 WWW-Authenticate 的 resource_metadata 找到它，并支持：

- OAuth Authorization Server Metadata（RFC 8414）；
- OpenID Connect Discovery 1.0。

### 16.2 注册优先级

客户端取得 client ID 的优先级：

1. 已预注册的凭据；
2. Client ID Metadata Document（推荐的新机制）；
3. Dynamic Client Registration（DCR，已弃用，兼容旧 AS）；
4. 让用户手动提供注册信息。

凭据必须绑定产生它的 AS issuer；AS 变化时不能复用另一 AS 的预注册或 DCR 凭据。CIMD 使用 HTTPS URL 作为 client_id，AS 必须校验文档中的 client_id 与 URL 完全一致，并防范 SSRF。

### 16.3 标准流程

1. Client 向 MCP endpoint 请求，未带 token。
2. Server 返回 401 和 WWW-Authenticate，可附 resource_metadata、所需 scope。
3. Client 获取 Protected Resource Metadata 和 AS metadata。
4. Client 注册或选择 client ID。
5. 生成 PKCE；现代实现必须支持并优先使用 S256。
6. 授权请求和 token 请求都必须携带 RFC 8707 resource，其值是目标 MCP Server 的规范 URI。
7. 浏览器完成用户授权，回调中检查 state 和 iss。
8. Client 用 authorization code + code_verifier 换 access token。
9. 每个 HTTP MCP 请求都发送 Authorization: Bearer <token>。
10. Server 校验 token 的签发者、有效期、scope 和 audience；无效 token 返回 401。

### 16.4 不可违反的授权安全边界

- token 不能放在 URI query；
- Server 只能接受为自己签发的 token；
- MCP Client 的 token 不能被 Server 原样转发给上游 API；上游必须使用另一套、面向上游 audience 的 token；
- 所有 AS endpoint 使用 HTTPS；redirect URI 必须是 localhost 或 HTTPS；
- 精确校验 redirect URI、PKCE、state 和授权响应 issuer；
- refresh token 必须保密，公开客户端应轮换；
- scope 遵循最小权限，WWW-Authenticate 中当前挑战的 scope 对当前操作具有权威性；
- 代理 Server 必须防范 confused deputy，不能仅凭静态 client ID 替用户越权授权。

## 17. 安全模型：把 MCP 当作高权限边界

MCP 能触发代码执行、文件访问、外部 API 和模型嵌套调用，因此安全责任主要落在 Host、Client、Server 和部署者，而不是 JSON-RPC 本身。

### 17.1 Host/Client 清单

- 明确展示 Server 身份、工具描述、参数和副作用；
- 工具调用前提供可拒绝的用户确认；
- 只向 Server 暴露用户同意的数据；
- 对资源、图标、工具描述、prompt 和返回内容做不可信数据处理；
- 对每个请求设置超时、取消和最大响应大小；
- 隔离不同 Server 的上下文、凭据和缓存；
- 记录调用、授权、输入、输出和取消审计；
- 不把 clientInfo、serverInfo 或 tool annotations 当作可信身份。

### 17.2 Server 清单

- 每次调用重新验证认证主体、scope、工具参数和资源 URI；
- 防止路径穿越、命令注入、模板注入、SSRF、任意 URL fetch 和提示注入；
- 对二进制内容、base64、Schema 深度、组合关键字和响应大小设上限；
- 业务错误用 isError，协议错误用 JSON-RPC error；
- requestState 使用 HMAC/AEAD 等完整性保护，绑定主体、请求和 TTL；
- 处理速率限制、并发限制、取消和资源释放；
- 缓存 scope 必须真实反映数据可见性；
- 不把 MCP Client token 传给第三方上游；
- 对动态工具列表保持稳定顺序，列表变化发送对应订阅通知。

### 17.3 Schema 安全

默认 JSON Schema 方言是 2020-12。实现必须验证 Schema，不得自动从任意网络 URL 解析 $ref。如果提供可选远程 $ref，必须默认关闭，使用主机白名单、超时、大小限制，并拒绝回环、链路本地和私网地址。对 anyOf、oneOf、allOf、if/then/else 和 $defs 设置深度、子 Schema 数量或验证时间上限，避免 Schema DoS。

## 18. 方法与通知速查表

### 18.1 Client → Server 请求

| 方法 | 作用 | 是否分页/流 |
|---|---|---|
| server/discover | 发现版本、能力和服务端信息 | 可缓存 |
| tools/list | 列工具 | 分页、可缓存 |
| tools/call | 调用工具 | 可 MRTR |
| resources/list | 列资源 | 分页、可缓存 |
| resources/read | 读资源 | 可 MRTR、可缓存 |
| resources/templates/list | 列 URI 模板 | 分页、可缓存 |
| prompts/list | 列提示词 | 分页、可缓存 |
| prompts/get | 获取渲染后的提示词 | 可 MRTR |
| completion/complete | 获取参数补全 | 普通请求 |
| subscriptions/listen | 打开长期通知流 | 长流 |

### 18.2 MRTR 中可能出现的 Client 能力请求对象

这些不是现代 Server 独立发出的 JSON-RPC request，而是 inputRequests 中的对象：

- roots/list
- sampling/createMessage
- elicitation/create

### 18.3 通知

| 通知 | 发送方 → 接收方 | 作用 |
|---|---|---|
| notifications/cancelled | Client → Server；Server → Client 仅用于结束订阅 | 取消请求或订阅 |
| notifications/progress | Server → Client | 长任务进度 |
| notifications/subscriptions/acknowledged | Server → Client | 订阅建立确认 |
| notifications/tools/list_changed | Server → Client（订阅流） | 工具列表失效 |
| notifications/prompts/list_changed | Server → Client（订阅流） | Prompt 列表失效 |
| notifications/resources/list_changed | Server → Client（订阅流） | 资源列表失效 |
| notifications/resources/updated | Server → Client（订阅流） | 指定资源失效 |
| notifications/message | Server → Client；兼容旧日志 | 已弃用的请求级日志 |

旧的 notifications/roots/list_changed、ping、logging/setLevel、HTTP GET stream 等不应加入新的现代核心实现。

## 19. 最小实现蓝图

### 19.1 Server 端

建议按下面的流水线组织，而不是把所有逻辑塞进一个“处理消息”函数：

~~~text
Transport reader
  → JSON parse
  → JSON-RPC envelope validation
  → request _meta/version/capability validation
  → auth and tenant context
  → method dispatch
  → method-specific Schema validation
  → operation / MRTR decision
  → result or JSON-RPC error
  → transport writer
~~~

最低可用 Server 可以先实现：

1. server/discover
2. tools/list
3. 一个无副作用 tools/call
4. 正确的 resultType、错误码、STDIO framing

之后再增加 resources、prompts、分页、缓存、进度、取消、订阅、Elicitation 和 OAuth。

### 19.2 Client 端

Client 至少需要：

1. 一个能严格解析 JSON-RPC、按 ID 关联 response 的 reader/writer；
2. 每个 request 自动注入版本和 clientCapabilities；
3. 方法能力检查和未知 resultType 处理；
4. 完整的 timeout/cancel/retry 语义；
5. 分页和缓存；
6. 工具确认、资源权限和审计；
7. HTTP 的 Origin、OAuth、Header 校验；
8. 订阅通知的 subscriptionId 分流。

### 19.3 第一条端到端测试

建议用确定性工具做最小测试：

1. Client 发现 Server；
2. 列出一个工具；
3. 调用工具并得到 complete；
4. 工具返回业务失败并确认 isError: true；
5. 缺参数得到 -32602；
6. Client 未声明能力时，Server 得到 -32021；
7. 长任务能收到递增 progress；
8. 取消后不再收到正常 response；
9. 断开 STDIO 后能重新启动并重建订阅；
10. HTTP 头体不一致得到 -32020。

## 20. 兼容性与排错顺序

遇到“某个客户端能连但不能调用”时，按这个顺序排查：

1. **Transport**：STDIO stdout 是否混入日志？HTTP Content-Type、Accept、Origin 是否正确？
2. **JSON-RPC**：是否有合法 jsonrpc、唯一 ID、method、params？
3. **现代元数据**：是否每个 request 都有版本和 clientCapabilities？
4. **版本**：Server 是否支持该版本？是否错误地把现代请求当旧 initialize？
5. **能力**：Server 是否宣告目标能力？Client 是否宣告 Elicitation/Sampling/Roots？
6. **Schema**：tool 参数、resource URI、prompt 参数是否通过 2020-12 校验？
7. **结果**：是否有 resultType？工具业务失败是否正确使用 isError？
8. **流**：订阅是否先 ack？是否每条消息都有 subscriptionId？SSE 是否被代理缓冲？
9. **缓存**：cursor 是否被误解析？private 数据是否被 public cache 复用？
10. **授权**：resource、issuer、audience、scope、PKCE、redirect URI 是否一致？

不要先把问题归因于“SDK 不兼容”；先抓原始 JSON-RPC、HTTP 状态、响应头和 stderr，再按上述层次定位。

## 21. 2026-07-28 相比旧版的重要变化

这一版最值得记住的不是新增了多少字段，而是通信模型发生了变化：

- 去掉现代实现的 initialize/notifications/initialized 流程，改为每请求元数据；
- 去掉 HTTP session、Mcp-Session-Id 和 SSE resumability；
- 增加 server/discover；
- Server → Client 的请求改为 MRTR；
- subscriptions/listen 取代资源订阅和 HTTP GET stream；
- result 强制 resultType；
- 资源未找到改用 -32602；
- 增加扩展协商、OpenTelemetry trace、标准 HTTP 头、x-mcp-header；
- 列表和读取结果增加 ttlMs、cacheScope；
- 工具输出支持 outputSchema 和 structuredContent；
- Dynamic Client Registration、Roots、Sampling、Logging、HTTP+SSE 进入弃用或兼容阶段；
- tasks 不再是核心页面中的隐式能力，而是官方扩展 io.modelcontextprotocol/tasks。

### 21.1 已弃用特性的处理原则

弃用不等于“当前立刻删除”。规范的生命周期至少保留十二个月；但新实现不应把这些能力作为核心依赖：

- Roots：迁移到显式目录/文件参数、Resource URI 或 Server 配置；
- Sampling：只在需要跨模型嵌套生成且能接受未来迁移成本时启用；
- Logging：STDIO 用 stderr，结构化场景用 OpenTelemetry；
- DCR：优先 Client ID Metadata Documents；
- HTTP+SSE：迁移到 Streamable HTTP。

## 22. 推荐学习路线

### 第 1 阶段：读懂线上的一来一回

掌握 JSON-RPC request/result/error/notification、ID、resultType、_meta 和 server/discover。先用 STDIO，手工抓一条完整消息。

### 第 2 阶段：做一个只读 Server

实现 resources/list、resources/read、分页、缓存和 URI 校验。不要一开始加入模型调用、数据库和复杂权限。

### 第 3 阶段：加入 Tool

实现严格 inputSchema、tools/list、tools/call、业务错误、超时、取消和用户确认。测试危险参数、重复调用和工具输出注入。

### 第 4 阶段：理解 MRTR 和订阅

分别实现 Elicitation、input_required 重试、requestState 完整性保护，以及 subscriptions/listen 的 ack、过滤和重连。

### 第 5 阶段：再接 HTTP 和授权

先做无认证 Streamable HTTP，再加入 Origin、标准头、SSE request response、OAuth metadata、PKCE、resource audience 和 token validation。

### 第 6 阶段：扩展与生产化

再考虑 Prompts、Completion、OpenTelemetry、Tasks 等扩展；为每个 Server 建立权限矩阵、审计、速率限制、缓存策略和兼容性测试矩阵。

## 23. 术语表

| 术语 | 含义 |
|---|---|
| Host | 承载 LLM 和多个 MCP Client 的应用 |
| Client | Host 中连接一个 Server 的连接器 |
| Server | 提供 Tools、Resources、Prompts 的服务 |
| Primitive | MCP 的服务端原语，主要指 Tool、Resource、Prompt |
| Transport | 消息承载方式，如 STDIO、Streamable HTTP |
| Capability | 一端声明的可用能力 map |
| MRTR | Multi Round-Trip Requests，Server 需要额外输入时的重试模式 |
| ResultType | result 的解析分支，核心值为 complete/input_required |
| RequestState | Server 生成、客户端原样回显的 opaque 状态 |
| Cursor | 分页位置的不透明 token |
| SubscriptionId | 订阅流中用于分流通知的原始 request ID |
| Resource | 用 URI 标识的上下文数据 |
| Tool | 可执行的模型动作接口 |
| Prompt | 用户选择的消息模板或工作流 |
| Protected Resource | OAuth 中需要 access token 的 MCP Server |
| CIMD | Client ID Metadata Document，推荐的新客户端注册方式 |

## 24. 官方页面与本地快照索引

本目录已经保存官方 2026-07-28 规范的 31 个 Markdown 页面，建议按以下顺序阅读：

1. [规范总览](specification/2026-07-28/index.md)
2. [架构](specification/2026-07-28/architecture/index.md)
3. [基础协议](specification/2026-07-28/basic/index.md)
4. [版本与兼容性](specification/2026-07-28/basic/versioning.md)
5. [Transport 总览](specification/2026-07-28/basic/transports/index.md)
6. [STDIO](specification/2026-07-28/basic/transports/stdio.md)
7. [Streamable HTTP](specification/2026-07-28/basic/transports/streamable-http.md)
8. [Patterns 总览](specification/2026-07-28/basic/patterns/index.md)
9. [MRTR](specification/2026-07-28/basic/patterns/mrtr.md)
10. [Subscriptions](specification/2026-07-28/basic/patterns/subscriptions.md)
11. [Cancellation](specification/2026-07-28/basic/patterns/cancellation.md)
12. [Progress](specification/2026-07-28/basic/patterns/progress.md)
13. [Server 总览](specification/2026-07-28/server/index.md)
14. [Server Discovery](specification/2026-07-28/server/discover.md)
15. [Tools](specification/2026-07-28/server/tools.md)
16. [Resources](specification/2026-07-28/server/resources.md)
17. [Prompts](specification/2026-07-28/server/prompts.md)
18. [Client Elicitation](specification/2026-07-28/client/elicitation.md)
19. [Client Sampling](specification/2026-07-28/client/sampling.md)
20. [Client Roots](specification/2026-07-28/client/roots.md)
21. [分页](specification/2026-07-28/server/utilities/pagination.md)
22. [缓存](specification/2026-07-28/server/utilities/caching.md)
23. [补全](specification/2026-07-28/server/utilities/completion.md)
24. [日志](specification/2026-07-28/server/utilities/logging.md)
25. [HTTP 授权总览](specification/2026-07-28/basic/authorization/index.md)
26. [授权服务器发现](specification/2026-07-28/basic/authorization/authorization-server-discovery.md)
27. [客户端注册](specification/2026-07-28/basic/authorization/client-registration.md)
28. [授权安全](specification/2026-07-28/basic/authorization/security-considerations.md)
29. [Schema Reference](specification/2026-07-28/schema.md)
30. [变更记录](specification/2026-07-28/changelog.md)
31. [已弃用特性](specification/2026-07-28/deprecated.md)

如果这份总结与具体实现冲突，应优先看 Schema 和相应专题页面中的 MUST/MUST NOT；本总结用于建立整体模型和实现顺序，不替代规范原文。
