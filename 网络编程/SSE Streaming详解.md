# SSE Streaming 详解：原理、实现与主流项目实践

## 一、SSE 是什么

SSE（Server-Sent Events）是建立在 HTTP 之上的服务器推送机制。客户端发起一次 HTTP 请求，服务端返回响应头后保持连接打开，并持续写入事件数据；客户端收到一条完整事件后立即处理，而不必等待完整结果生成。

它的主要特征：

- 单向通信：服务器 → 客户端；
- 基于 HTTP；
- 文本协议；
- 浏览器原生支持；
- `EventSource` 默认支持自动重连；
- 适合日志、进度、通知和 AI 文本生成。

## 二、SSE 与普通 HTTP 响应的区别

普通 HTTP 是“一次请求、一次完整响应”；SSE 是“一次请求、持续产生多个响应片段”。

普通接口可能返回：

```json
{"answer":"这是完整答案"}
```

SSE 则可能连续发送：

```text
data: 这

data: 是

data: 完整答案

```

前端可以在生成过程中逐步展示内容。

## 三、SSE 协议格式

服务端响应类型必须是：

```http
Content-Type: text/event-stream
```

事件由若干行组成，事件之间用空行（通常是 `\n\n`）分隔：

```text
data: hello

```

### 3.1 常见字段

#### `data`

```text
data: hello

```

传 JSON 时：

```text
data: {"type":"message","content":"hello"}

```

#### `event`

```text
event: progress
data: {"current":60,"total":100}

```

浏览器端可按事件类型监听：

```javascript
const source = new EventSource("/events");

source.addEventListener("progress", (event) => {
  console.log(JSON.parse(event.data));
});
```

#### `id`

```text
id: 42
data: hello

```

浏览器重连时会发送 `Last-Event-ID: 42`，服务端可据此恢复推送位置。

#### `retry`

```text
retry: 5000
```

表示客户端断开后建议 5 秒后重连。

#### 注释行

```text
: heartbeat

```

注释不会触发普通消息处理，常用于心跳保活。

### 3.2 多行 `data`

下面是一条事件，而不是三条事件：

```text
data: line 1
data: line 2
data: line 3

```

客户端通常把它们拼接为 `line 1\nline 2\nline 3`。

## 四、一次 SSE 请求的生命周期

客户端请求：

```http
GET /events HTTP/1.1
Accept: text/event-stream
Cache-Control: no-cache
```

服务端返回：

```http
HTTP/1.1 200 OK
Content-Type: text/event-stream
Cache-Control: no-cache
Connection: keep-alive
Transfer-Encoding: chunked
```

随后持续写入：

```text
event: message
data: hello

event: message
data: world

```

SSE 规定的是事件格式，底层可以使用 HTTP/1.1，也可以使用 HTTP/2 或 HTTP/3。

## 五、服务端实现

### 5.1 Node.js 原生 HTTP

```javascript
import http from "node:http";

const server = http.createServer((req, res) => {
  if (req.url !== "/events") {
    res.writeHead(404);
    res.end();
    return;
  }

  res.writeHead(200, {
    "Content-Type": "text/event-stream",
    "Cache-Control": "no-cache",
    "Connection": "keep-alive",
    "X-Accel-Buffering": "no",
  });

  res.write(": connected\n\n");

  let count = 0;
  const timer = setInterval(() => {
    count += 1;
    res.write("event: progress\n");
    res.write(`id: ${count}\n`);
    res.write(`data: ${JSON.stringify({ count })}\n\n`);

    if (count >= 10) {
      clearInterval(timer);
      res.end();
    }
  }, 1000);

  req.on("close", () => clearInterval(timer));
});

server.listen(3000);
```

关键点是 `text/event-stream`、每条事件后的空行、持续 `write`，以及客户端断开后的资源清理。

### 5.2 FastAPI

```python
import asyncio
import json

from fastapi import FastAPI, Request
from fastapi.responses import StreamingResponse

app = FastAPI()


@app.get("/events")
async def events(request: Request):
    async def generator():
        try:
            for i in range(10):
                if await request.is_disconnected():
                    break
                yield "event: progress\n"
                yield f"data: {json.dumps({'count': i})}\n\n"
                await asyncio.sleep(1)
        finally:
            print("cleanup")

    return StreamingResponse(
        generator(),
        media_type="text/event-stream",
        headers={
            "Cache-Control": "no-cache",
            "Connection": "keep-alive",
            "X-Accel-Buffering": "no",
        },
    )
```

### 5.3 Go

```go
func events(w http.ResponseWriter, r *http.Request) {
    w.Header().Set("Content-Type", "text/event-stream")
    w.Header().Set("Cache-Control", "no-cache")
    w.Header().Set("Connection", "keep-alive")

    flusher, ok := w.(http.Flusher)
    if !ok {
        http.Error(w, "streaming unsupported", http.StatusInternalServerError)
        return
    }

    for i := 0; i < 10; i++ {
        fmt.Fprintf(w, "event: progress\n")
        fmt.Fprintf(w, "data: {\"count\":%d}\n\n", i)
        flusher.Flush()

        select {
        case <-r.Context().Done():
            return
        case <-time.After(time.Second):
        }
    }
}
```

Go 中必须调用 `Flush()`，否则数据可能仍停留在缓冲区。

## 六、浏览器客户端

### 6.1 原生 `EventSource`

```javascript
const source = new EventSource("/events");

source.onopen = () => console.log("connected");
source.onmessage = (event) => console.log(event.data);
source.onerror = (error) => console.error(error);

source.close();
```

优点是 API 简单、浏览器原生支持并自动重连。限制是只能方便地发起 GET，不能直接设置自定义 `Authorization` 请求头或发送 POST body。

### 6.2 使用 `fetch` 读取流

AI 接口通常使用 `fetch`，因为它需要 POST body、请求头和更细粒度的控制：

```javascript
const response = await fetch("/chat", {
  method: "POST",
  headers: { "Content-Type": "application/json" },
  body: JSON.stringify({ message: "你好" }),
});

const reader = response.body
  .pipeThrough(new TextDecoderStream())
  .getReader();

let buffer = "";

while (true) {
  const { value, done } = await reader.read();
  if (done) break;

  buffer += value;
  const chunks = buffer.split("\n\n");
  buffer = chunks.pop() ?? "";

  for (const chunk of chunks) {
    const data = chunk
      .split("\n")
      .filter((line) => line.startsWith("data:"))
      .map((line) => line.slice(5).trimStart())
      .join("\n");

    console.log(data);
  }
}
```

生产环境还需要处理 `\r\n`、半截事件、多行 `data`、JSON 解析错误、AbortController、事件 ID 和业务错误。

## 七、AI 项目中的 SSE

### 7.1 OpenAI

传统 Chat Completions 流通常类似：

```text
data: {"choices":[{"delta":{"content":"你"}}]}

data: {"choices":[{"delta":{"content":"好"}}]}

data: [DONE]

```

客户端把每个 `delta.content` 拼接起来。较新的 Responses API 使用更丰富的事件类型，例如：

```text
response.created
response.output_text.delta
response.output_item.added
response.completed
response.failed
```

因此不能假设每一条事件都只是文本 token，还要处理工具调用、结构化输出、完成和错误。

### 7.2 Anthropic

Anthropic Messages Streaming 常见生命周期事件包括：

```text
message_start
content_block_start
content_block_delta
content_block_stop
message_delta
message_stop
```

文本增量一般位于 `content_block_delta` 的 `text_delta` 中。

### 7.3 LangChain

LangChain 常用：

- `stream()`：逐块输出最终结果；
- `astream()`：异步流式输出；
- `astream_events()`：更细粒度的生命周期事件；
- agent 的 token、工具调用和状态更新流。

LangChain 负责产生内部事件，HTTP 层仍要负责 SSE 编码、连接取消、错误传播、代理缓冲和前端协议设计。

### 7.4 AI 流式接口不只有“输出 token”

一个完整的 AI 请求通常经历多个阶段：排队、模型开始、文本增量、工具调用、工具结果、引用来源、用量统计和最终完成。因此推荐把模型 SDK 的事件先转换为业务事件，再暴露给前端，而不是把第三方 SDK 的原始 JSON 直接透传。

例如可以统一为：

```text
event: request.started
data: {"request_id":"r1","model":"gpt-5"}

event: output.text.delta
data: {"text":"答案"}

event: tool.call.delta
data: {"call_id":"c1","name":"search","arguments":"{\"q\":"}

event: tool.call.completed
data: {"call_id":"c1","arguments":{"q":"SSE"}}

event: tool.result
data: {"call_id":"c1","items":[...]}

event: usage
data: {"input_tokens":100,"output_tokens":20}

event: request.completed
data: {"finish_reason":"stop"}
```

这样做的好处是前端不依赖某个模型厂商的字段命名，未来切换模型、增加本地模型或接入网关时，客户端协议可以保持不变。

### 7.5 Agent 与工具调用的流式过程

Agent 的一次执行不是简单的“模型输入 → 文本输出”，而是一个循环：

```text
用户输入
   ↓
模型决定是否调用工具
   ↓
流式输出工具名称和参数
   ↓
服务端执行工具
   ↓
推送工具结果或工具进度
   ↓
再次调用模型
   ↓
流式输出最终答案
```

工具参数可能跨多个事件逐步生成，不能在收到第一段参数时立即执行。服务端通常要按 `call_id` 缓冲参数，收到 `tool.call.completed` 后再解析 JSON 和执行工具。

如果工具执行时间较长，可以继续推送：

```text
event: tool.progress
data: {"call_id":"c1","message":"正在查询数据库"}

event: tool.progress
data: {"call_id":"c1","message":"已读取 1000 条记录"}
```

工具结果应当经过权限校验和大小限制，不能因为 SSE 是流式的就绕过原有的鉴权、审计和数据脱敏逻辑。

### 7.6 RAG/搜索场景的事件设计

RAG 应用通常需要同时展示回答和来源。可以把检索阶段与生成阶段分开：

```text
event: retrieval.started
data: {"query":"SSE 原理"}

event: retrieval.result
data: {"document_id":"doc-1","title":"网络编程笔记","score":0.92}

event: context.ready
data: {"count":5}

event: output.text.delta
data: {"text":"SSE 是一种服务器推送机制"}

event: citation
data: {"document_id":"doc-1","start":0,"end":12}
```

前端可以在答案生成前展示“正在检索”，在生成过程中建立文本片段与文档来源的关联。引用不要只在最终事件中返回，否则用户无法在流式输出时知道答案依据。

### 7.7 多轮对话和状态事件

对于 Agent、工作流和长任务，建议把“文本输出”和“状态更新”分开：

```text
event: state
data: {"phase":"planning","step":1}

event: output.text.delta
data: {"text":"我先分析问题。"}

event: state
data: {"phase":"executing_tool","tool":"search"}

event: state
data: {"phase":"synthesizing"}
```

这样前端可以显示阶段、进度条和取消按钮，而不需要从自然语言中猜测 Agent 当前状态。

### 7.8 常见 AI SDK 和前端框架的差异

AI 项目中的“流式输出”不一定都严格使用浏览器原生 SSE：

| 项目/方式 | 常见传输形态 | 主要特点 |
|---|---|---|
| OpenAI/Anthropic 服务端 API | SSE | 事件类型和 JSON 字段由厂商定义 |
| LangChain/LangGraph 服务端 | SSE、WebSocket 或自定义事件流 | 重点是 token、节点、工具和状态事件 |
| Vercel AI SDK | `fetch` 文本流和 AI Data Stream 协议 | 前端封装较完整，适合 React/Next.js |
| 自建 FastAPI/Node 网关 | SSE 或 NDJSON | 可统一多个模型供应商的协议 |
| 本地推理服务 | SSE、HTTP chunked 或 NDJSON | 具体格式取决于 vLLM、Ollama 等服务实现 |

因此接入前要确认三件事：响应的 `Content-Type`、事件边界格式、结束标记。不能因为接口名字叫 `stream` 就假设它一定可以用 `EventSource` 读取。

例如有些框架返回的是 `application/x-ndjson`：

```text
{"type":"token","text":"你"}\n
{"type":"token","text":"好"}\n
```

这仍然是 HTTP 流，但不是 SSE；客户端应按换行解析，而不是寻找 `data:` 和空行。

### 7.9 AI 流的取消、超时和背压

用户点击“停止生成”时，前端应主动取消 HTTP 请求：

```javascript
const controller = new AbortController();

const response = await fetch("/api/chat", {
  method: "POST",
  signal: controller.signal,
  body: JSON.stringify({ message: "你好" }),
});

// 用户点击停止按钮
controller.abort();
```

服务端收到断开后，需要把取消信号继续传递给模型 SDK、工具任务和数据库查询。仅关闭浏览器连接而不取消下游模型，仍会产生 token 费用。

另外，模型产生事件的速度可能高于浏览器渲染速度。服务端应避免无限堆积内存，可以：

- 合并很小的 token 片段后再发送；
- 限制单事件和单请求的最大缓存；
- 对日志、检索结果和工具输出做大小上限；
- 使用异步队列或框架提供的背压机制。

### 7.10 AI 流式接口的可观测性

不要只记录最终 HTTP 状态码。建议为每个请求记录：

- `request_id`、`conversation_id`、`user_id`；
- 首 token 时间（TTFT）；
- 总耗时和最后一个 token 时间；
- 输入、输出 token 数；
- 事件数量和发送字节数；
- 工具调用次数及耗时；
- 客户端主动取消、代理断开和模型错误；
- 最终完成原因：正常结束、长度限制、内容过滤或异常。

常用指标可以区分：

```text
请求耗时 = 建立连接到完成事件
TTFT = 建立连接到第一个 output.text.delta
生成耗时 = 第一个 token 到完成事件
```

这比只看平均响应时间更能反映 AI 流式体验。

### 7.11 内容安全与增量输出

流式输出会在完整答案生成前就展示给用户，因此内容审核策略需要提前设计：

- 对模型输入先做安全检查；
- 对增量输出做轻量规则或分类器检查；
- 对高风险任务先缓冲一段或等最终审核后再展示；
- 发现违规时发送 `error` 或 `content_filtered` 事件并停止后续输出；
- 不要把内部思考过程、系统提示词和敏感工具参数直接推送到浏览器。

流式并不意味着所有中间状态都应该对用户可见，事件协议必须明确“用户可见内容”和“仅用于内部观测的内容”。

## 八、主流框架用法

### Spring WebFlux

```java
@GetMapping(value = "/events",
            produces = MediaType.TEXT_EVENT_STREAM_VALUE)
public Flux<ServerSentEvent<String>> events() {
    return Flux.interval(Duration.ofSeconds(1))
        .map(i -> ServerSentEvent.builder("count: " + i)
            .event("progress")
            .id(String.valueOf(i))
            .build());
}
```

Spring MVC 也可以使用 `SseEmitter`，但大量长连接通常更适合 WebFlux 或其他异步模型。

### NestJS

```typescript
@Sse("events")
events() {
  return interval(1000).pipe(
    map((value) => ({
      data: { value },
      type: "progress",
      id: String(value),
    })),
  );
}
```

### Flask / Django

Flask 可通过生成器返回 `text/event-stream`，Django 可使用 `StreamingHttpResponse`。同步 worker 下每个连接可能长期占用一个 worker，高并发时应选择合适的异步部署方式或 ASGI 架构。

## 九、与 WebSocket 等技术的比较

| 技术 | 方向 | 基于 | 浏览器原生支持 | 适合场景 |
|---|---|---|---|---|
| SSE | 服务端 → 客户端 | HTTP | 是 | 通知、日志、AI 输出 |
| WebSocket | 双向 | 独立协议 | 是 | 聊天、协同编辑、游戏 |
| HTTP Streaming | 通常服务端 → 客户端 | HTTP | 需手动读取 | AI、文件、日志 |
| Long Polling | 模拟推送 | HTTP | 是 | 老系统、兼容性场景 |
| WebTransport | 双向、多流 | HTTP/3 | 较新 | 低延迟、高级实时应用 |

如果主要是“服务器不断告诉客户端发生了什么”，优先考虑 SSE；如果双方都需要持续发消息，考虑 WebSocket。

## 十、生产环境关键问题

### 10.1 代理缓冲

Nginx 常见配置：

```nginx
location /events {
    proxy_pass http://backend;
    proxy_buffering off;
    proxy_cache off;
    proxy_read_timeout 3600s;
}
```

也可以发送：

```http
X-Accel-Buffering: no
```

### 10.2 心跳和空闲超时

建议每 15～30 秒发送：

```text
: ping

```

避免负载均衡器、CDN 或防火墙关闭空闲连接。

### 10.3 客户端断开后的清理

需要取消模型生成、数据库订阅、消息队列消费者和定时器，否则会造成资源泄漏。

### 10.4 认证与 CORS

原生 `EventSource` 不方便设置自定义请求头。常见方案是 Cookie、短期签名 URL、支持 headers 的 polyfill，或改用 `fetch` 流式读取。使用 Cookie 时要同时正确配置 CORS 和凭证。

### 10.5 流内错误

响应开始后无法再修改 HTTP 状态码，因此业务错误要通过事件表达：

```text
event: error
data: {"code":"MODEL_TIMEOUT","message":"模型超时"}

```

### 10.6 多实例部署

SSE 只负责最后一跳。多实例环境通常需要 Redis Pub/Sub、Kafka、NATS 或 RabbitMQ 在任务执行器和 SSE 节点之间分发事件。

## 十一、断线重连与可靠性

浏览器会自动重连，并在请求头中携带 `Last-Event-ID`：

```text
id: 100
data: item-100

```

服务端可以从 101 继续发送。但自动重连不等于可靠消息投递。要实现可靠恢复，还需要事件持久化、可回放日志、重放窗口、去重和幂等处理。

## 十二、推荐的 AI 事件协议

不要只发送裸文本，建议定义稳定事件类型：

```text
event: message_start
data: {"message_id":"m1"}

event: token
data: {"text":"你"}

event: tool_call
data: {"name":"search","arguments":{"q":"SSE"}}

event: usage
data: {"input_tokens":100,"output_tokens":20}

event: done
data: {"reason":"stop"}
```

至少应考虑 `start`、`delta/token`、`metadata`、`error`、`done`，并根据业务增加 `tool_call`、`tool_result`、`citation`、`usage` 和 `heartbeat`。

## 十三、排查“不实时”的顺序

```bash
curl -N http://localhost:3000/events
```

按以下顺序检查：

1. `Content-Type` 是否为 `text/event-stream`；
2. 每条事件后是否有 `\n\n`；
3. 服务端是否真正 flush；
4. Nginx、CDN 或网关是否缓冲；
5. 是否启用了 gzip；
6. 客户端是否一次性读取完整响应；
7. 是否存在代理空闲超时；
8. 前端是否正确处理半截事件；
9. 服务端是否感知客户端断开；
10. 是否需要心跳保活。

## 十四、总结

SSE 的本质是：

```text
HTTP 长连接
+ text/event-stream
+ 事件边界 \n\n
+ 服务端持续写入
+ 客户端逐事件解析
+ 可选的 id / retry / Last-Event-ID
```

对于服务器推送和 AI 文本流，SSE 通常比 WebSocket 更简单；对于真正双向、低延迟、复杂实时交互，WebSocket 或 WebTransport 更合适。
