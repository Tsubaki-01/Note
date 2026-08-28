# OpenClaw Agent 系统调研报告

## 摘要

本报告聚焦 OpenClaw agent 系统本体，即官方仓库 `openclaw/openclaw` 与官方文档 `docs.openclaw.ai` 所描述的 OpenClaw。调研范围包括工具调用、记忆与状态持久化、上下文管理、会话策略、任务分解和 sub-agent 机制。

结论上，OpenClaw 不是单一 LLM 或普通聊天机器人，而是一个本地优先的 Gateway + embedded agent runtime 组合。Gateway 负责渠道接入、WebSocket 控制面、会话、配置、cron/webhook、Canvas host 等；agent runtime 负责模型调用、工具执行、上下文组装、记忆检索和持久化。它的核心工程特点是：强执行能力、跨渠道常驻、工具权限可控、记忆可审计、上下文成本可观测。

## 项目定位

### 调研对象边界

本报告只研究 OpenClaw 官方 agent 系统，不研究 Claude Code、Hermes 或其他 agent 系统。检索中出现过多个同名或相关项目，本报告采用以下边界：

- `openclaw/openclaw`：官方 GitHub 仓库，README 与官方文档互链，仓库结构包含 `apps`、`docs`、`extensions`、`packages`、`skills`、`src` 等。
- `docs.openclaw.ai`：官方文档站，覆盖 Gateway architecture、Agent runtime、Agent loop、Tools、Memory、Context、Sessions、Sub-agents、Sandboxing 等主题。
- `OpenClaw-rocks/openclaw-operator`：Kubernetes operator，定位是部署和管理 OpenClaw 实例，不是 agent 系统本体。
- 若干教程站、SEO 站点、镜像站或二手介绍页面：未作为实现细节依据。

因此，本文中的 OpenClaw 指 `github.com/openclaw/openclaw` 与 `docs.openclaw.ai` 对应的 agent 系统本体。

### 系统定位

OpenClaw 官方 README 将其定位为运行在用户自有设备上的个人 AI assistant，可通过 WhatsApp、Telegram、Slack、Discord、Google Chat、Signal、iMessage、Teams、Matrix、WebChat 等渠道接入。官方文档进一步描述它是连接聊天渠道与 AI coding agents，例如 Pi agent runtime 的 self-hosted gateway/agent platform。

从公开资料看，OpenClaw 的核心不是模型，而是本地优先的 agent 运行时与消息网关组合：

- Gateway 长驻进程负责渠道连接、WebSocket 控制面、会话、配置、cron/webhook、Canvas host 等。
- 内嵌 agent runtime 基于 Pi agent core，OpenClaw 在其上负责会话管理、工具接线、技能发现和渠道投递。
- agent 的行动能力由 typed tools、skills、plugins、sandbox、session tools、sub-agents 等机制共同提供。

## 核心结论

1. OpenClaw 的工程重点是“本地常驻 + 强工具执行 + 多渠道消息路由”，而不是单纯包装模型 API。
2. 它将能力拆成 tools、skills、plugins 三层：工具是可执行 API，skill 是 Markdown 操作规程，plugin 是能力注册与分发边界。
3. 工具安全依赖配置和运行时边界，而不是只靠提示词，包括 allow/deny、tool profiles、provider-specific restrictions、sandbox backend、exec approval、plugin hooks 等。
4. 记忆系统以普通 Markdown 文件作为事实源，SQLite/FTS/vector/hybrid search 是可重建的索引和检索层。
5. Active Memory 和 Dreaming 分别解决在线召回与离线巩固：前者阻塞主回复链路，后者后台整理并在满足阈值后提升到长期记忆。
6. 上下文管理把 system prompt、bootstrap files、skills list、tool schema、history、attachments、compaction summaries、pruning artifacts 都视为 token 成本的一部分，并提供观测命令。
7. 会话并发通过 per-session lane、global lane 和 write lock 控制，同一 session 串行，跨 session 可并行。
8. sub-agent 运行在独立 session 中，默认工具面和递归能力受限，用来降低主上下文污染和递归失控风险。

## 工具调用

### Tools、Skills、Plugins 三层模型

OpenClaw 将 agent 能力拆为三层：

- **Tools**：agent 实际调用的 typed functions，例如 `exec`、`browser`、`web_search`、`read`、`write`、`apply_patch`、`message`、`cron`、`sessions_*`、`memory_search` 等。工具以结构化 function definitions 的形式提供给模型。
- **Skills**：Markdown `SKILL.md`，用于告诉 agent 何时、如何使用工具。完整 skill 指令默认不全部注入上下文，system prompt 中只放紧凑列表，模型需要时再读取对应 `SKILL.md`。
- **Plugins**：注册 channels、model providers、tools、skills、speech、media、web fetch/search、context engine、memory/wiki 等能力，是能力打包和扩展边界。

这个分层的取舍很清晰：Tools 负责可执行 API，Skills 负责操作规程，Plugins 负责分发与注册。它避免把大量技能说明常驻塞入上下文，同时保留扩展工具面的能力。

### 内置工具面

官方 Tools 文档列出的内置工具覆盖以下方向：

- 运行时：`exec` / `process`、`code_execution`
- 文件系统：`read` / `write` / `edit`、`apply_patch`
- UI/Web：`browser`、`web_search`、`x_search`、`web_fetch`、`canvas`
- 消息与设备：`message`、`nodes`
- 自动化与网关：`cron`、`gateway`
- 媒体：`image`、`image_generate`、`music_generate`、`video_generate`、`tts`
- 会话与编排：`sessions_list`、`sessions_history`、`sessions_send`、`sessions_spawn`、`sessions_yield`、`subagents`、`session_status`
- 记忆：`memory_search`、`memory_get`，由 memory plugin 提供

工具调用会在 agent loop 中产生 tool start/update/end 事件，并通过 OpenClaw 的 agent event stream 桥接到 Gateway。工具结果在记录或发出前会进行大小控制和图像 payload 处理。`sessions_history` 等跨会话读取工具不是原始 transcript dump，而是有边界的历史读取，会清理 thinking tags、工具调用 XML、控制 token 和疑似凭据，并做截断或脱敏。

### 权限与安全边界

OpenClaw 的工具授权是配置驱动的：

- `tools.allow` / `tools.deny` 控制可调用工具，deny 优先。
- `tools.profile` 提供 `full`、`coding`、`messaging`、`minimal` 等预设。
- `group:*` 提供工具组简写，例如 `group:fs`、`group:runtime`、`group:web`、`group:sessions`、`group:memory`、`group:ui`、`group:automation`。
- `tools.byProvider` 可按模型或 provider 限制工具面。
- sandboxed sessions 默认会被限制到更窄的工具可见范围。README 示例中，非 `main` sandbox 常见允许文件、进程、会话等基础工具，拒绝 browser、canvas、nodes、cron、discord、gateway 等更危险工具。

Sandboxing 是另一个关键边界。Gateway 本身仍运行在 host 上，但工具执行可以放入 Docker、SSH 或 OpenShell sandbox。官方文档明确说明这不是完美安全边界，但可以显著降低模型误操作对文件系统和进程的影响。sandbox 可配置：

- mode：`off`、`non-main`、`all`
- scope：`agent`、`session`、`shared`
- backend：`docker`、`ssh`、`openshell`
- workspace access、bind mounts、sandbox browser、CDP allowlist/noVNC token 等

工程上，OpenClaw 没有把工具安全只交给提示词，而是组合使用工具 allow/deny、profile、per-agent override、sandbox backend、exec approval/elevated mode、DM pairing/allowlist 等多层防护。

### Agent Loop 中的执行链路

OpenClaw 的 agent loop 可概括为：

`intake -> context assembly -> model inference -> tool execution -> streaming replies -> persistence`

公开文档确认了以下流程：

- `agent` RPC 接收请求后解析 session，持久化 session metadata，并立即返回 `runId`。
- `agentCommand` 解析模型、thinking、verbose、trace，加载 skills snapshot，然后调用 `runEmbeddedPiAgent`。
- `runEmbeddedPiAgent` 通过 per-session + global queues 串行化运行，解析 model/auth profile，构建 Pi session，订阅 Pi events，桥接 assistant/tool/lifecycle stream，并用 timeout abort。
- `subscribeEmbeddedPiSession` 将 Pi tool events 转为 OpenClaw `stream: "tool"`，将 assistant deltas 转为 `stream: "assistant"`。
- plugin hooks 可在 `before_tool_call` / `after_tool_call` 拦截工具参数和结果。
- `tool_result_persist` 可在工具结果写入 transcript 前同步转换。

未从公开资料确认：模型侧具体 function-calling schema 的 TypeScript 类型定义、每个工具的完整 JSON Schema、Pi agent core 内部如何选择工具。这些内容可能可在源码中进一步查证，但本次调研未下载或执行仓库源码。

## 记忆与状态持久化

### Markdown 是记忆事实源

OpenClaw Memory 文档强调：系统通过在 agent workspace 写入普通 Markdown 文件来“记住”内容；模型只记得被保存到磁盘的内容，没有隐藏状态。默认记忆文件包括：

- `MEMORY.md`：长期记忆，保存持久事实、偏好和决策；每个 DM session 开始时加载。
- `memory/YYYY-MM-DD.md`：每日笔记，保存运行上下文和观察；今天和昨天的笔记会自动加载。
- `DREAMS.md`：可选文件，用于 Dreaming 汇总和人工审阅。

这个设计的优点是可审计、可备份、可人工编辑，不把长期记忆锁在不可见数据库中。代价是写入质量依赖 agent 是否正确落盘，因此 OpenClaw 又补充了 memory search、active memory、compaction 前 memory flush、dreaming 等机制。

### Memory Tools 与检索机制

OpenClaw 默认 memory plugin 是 `memory-core`，提供：

- `memory_search`：使用语义搜索查找相关笔记。
- `memory_get`：读取具体 memory 文件或指定行范围。

内置 memory engine 使用 per-agent SQLite 索引，支持 FTS5/BM25 keyword search、embedding vector search、hybrid search、CJK trigram tokenization 和可选 sqlite-vec。

索引范围包括 `MEMORY.md` 和 `memory/*.md`。官方文档给出的默认切块约为 400 tokens，80-token overlap，索引位置为 `~/.openclaw/memory/<agentId>.sqlite`。memory 文件变化会触发 debounce reindex；embedding provider、model 或 chunking config 变化时会自动重建索引。

Memory Search 文档说明检索会并行运行两条路径：

- vector search：寻找语义相似内容。
- BM25 keyword search：寻找 ID、错误字符串、配置 key 等精确匹配。

如果没有 embedding 或没有 FTS，单一路径仍可运行；没有 embedding 时会使用改进的 lexical ranking，而不是简单 exact-match。

### Active Memory：在线召回

Active Memory 是可选插件能力。它会在主回复生成前，对符合条件的交互式持久聊天 session 启动一个 blocking memory sub-agent，为系统提供一次有边界的主动记忆检索机会。公开资料确认：

- 默认建议只对 `main` agent 和 direct chat 开启。
- Active Memory sub-agent 只能使用 `memory_search` 和 `memory_get`。
- query mode 可选 `message`、`recent`、`full`，分别发送最新消息、近期对话尾部或完整对话给记忆子 agent。
- prompt style 可选 `balanced`、`strict`、`contextual`、`recall-heavy`、`precision-heavy`、`preference-only`。
- 默认临时写入 blocking sub-agent transcript，运行结束后删除；如需调试可开启 `persistTranscripts`，单独存到 agent sessions folder 下的相对目录。

工程取舍是：Active Memory 把“是否应该回忆”从主 agent 中剥离出来，能提升连续性和个性化，但会阻塞回复链路并增加延迟。因此官方建议限制在 direct chat 中使用，控制 timeout，并默认关闭 thinking。

### Dreaming：离线巩固

Dreaming 是 `memory-core` 中的可选后台记忆巩固系统，默认关闭。它将短期信号提升到长期 `MEMORY.md`，同时保留可审阅记录。公开资料确认：

- 机器状态写在 `memory/.dreams/`。
- 人类可读输出写在 `DREAMS.md` 或 `memory/dreaming/<phase>/YYYY-MM-DD.md`。
- 长期 promotion 只写 `MEMORY.md`。
- 三阶段协作：light、REM、deep。
- light 阶段整理近期每日记忆、recall traces、redacted session transcripts，去重并记录强化信号，不写长期记忆。
- REM 阶段抽取主题和反思信号，不写长期记忆。
- deep 阶段基于加权分数和阈值决定是否长期提升，要求 `minScore`、`minRecallCount`、`minUniqueQueries` 等通过后才写入 `MEMORY.md`。
- deep ranking 信号包括 frequency、relevance、query diversity、recency、consolidation、conceptual richness，Light/REM 命中也会产生衰减 boost。

### Session 与持久化状态

会话状态由 Gateway 统一拥有：

- Session store：`~/.openclaw/agents/<agentId>/sessions/sessions.json`
- Transcript：`~/.openclaw/agents/<agentId>/sessions/<sessionId>.jsonl`

会话按来源路由：

- DM 默认共享 session，用于单用户连续性。
- group/room/channel 按群或房间隔离。
- cron 每次 fresh session。
- webhook 按 hook 隔离。

对多用户 DM，官方文档明确建议启用 `session.dmScope: "per-channel-peer"`，否则不同用户可能共享同一个 DM 上下文。

## 上下文管理

### Context 的组成

OpenClaw 将 context 定义为每次 run 发送给模型的全部内容，受模型 token window 限制。官方文档确认包括：

- OpenClaw 构建的 system prompt：规则、工具、skills list、时间/运行时元数据、workspace files。
- 当前 session 的 conversation history。
- 工具调用和结果。
- 附件、图片、音频、文件等。
- compaction summaries、pruning artifacts。
- provider wrappers 或隐藏 header。这些内容也可能消耗 token，但不一定可见。

OpenClaw 明确区分 context 与 memory：memory 可以存盘并在后续重载；context 是当前模型窗口里的内容。

### System Prompt 与 Bootstrap Files

每次 run 都会重建 system prompt。OpenClaw 会注入 workspace 中一组用户可编辑文件：

- `AGENTS.md`
- `SOUL.md`
- `TOOLS.md`
- `IDENTITY.md`
- `USER.md`
- `HEARTBEAT.md`
- `BOOTSTRAP.md`，仅 first-run

Agent runtime 文档还列出 `BOOTSTRAP.md`、`IDENTITY.md`、`USER.md` 等初始化文件。空文件会跳过，大文件会被修剪和截断，并保留 marker。Context 文档确认默认 per-file cap `agents.defaults.bootstrapMaxChars` 为 12000 chars，total cap `agents.defaults.bootstrapTotalMaxChars` 为 60000 chars，并可通过 `/context` 查看 raw size 与 injected size。

Skills 的处理较克制：system prompt 中只放 name、description、location 的紧凑列表，完整 `SKILL.md` 不默认注入，模型需要时再通过 read 读取。这是一个重要的上下文节省设计。

### Context 可观测性

OpenClaw 提供多个观测入口：

- `/status`：快速查看窗口占用和 session 设置。
- `/context list`：查看注入内容和粗略大小。
- `/context detail`：查看 per-file、per-tool schema、per-skill entry、system prompt size。
- `/usage tokens`：在回复中附加 token usage footer。
- `/compact`：手动总结旧历史以释放窗口。

特别值得注意的是，OpenClaw 把 tool schema 的上下文成本暴露出来。工具不仅有 system prompt 中的工具列表文本成本，也有 JSON schema 成本，后者同样计入模型上下文。

### Context Engine：可插拔组装层

OpenClaw 默认使用内置 `legacy` context engine。Context Engine 文档确认该接口负责：

- ingest：新消息加入 session 时被调用，可存储或索引消息。
- assemble：模型运行前返回适配 token budget 的消息集合，以及可选 `systemPromptAddition`。
- compact：上下文满或用户执行 `/compact` 时总结或缩减上下文。
- after turn：run 结束后持久化状态、触发后台 compaction 或更新索引。

Plugin 可注册 `kind: "context-engine"` 并通过 `plugins.slots.contextEngine` 选为 active engine。`ownsCompaction` 控制是否由插件完全接管 compaction；如果插件不接管，也必须正确实现 `compact()` 或委托给 runtime，不能 no-op。

这个设计把“消息选择、上下文压缩、记忆提示注入”抽象成可替换 engine，而 memory plugin 仍负责检索和召回。两者解耦，但可以协作。

### Compaction 与 Pruning

Compaction 用于长会话接近 context limit 时，将旧消息总结为 compact entry：

- 旧 turns 被总结成 compact entry。
- summary 保存进 session transcript。
- 最近消息保持原样。
- 切分历史时保持 assistant tool calls 与对应 `toolResult` 成对，避免在工具块中间截断。
- Auto-compaction 默认开启；接近上下文限制或模型返回 context overflow 时触发，并可 compact 后 retry。
- Compaction 可指定专用 summarization model，也可由 plugin 注册 custom compaction provider。
- compaction 前可运行 silent memory flush，让 agent 先把重要上下文写入 memory files。

Pruning 更轻量：

- 每次 LLM 调用前从内存 prompt 中 trim 旧 tool results。
- 不改写磁盘 transcript。
- 保留普通对话文本。
- 可对 oversized results 保留 head/tail，中间插入 placeholder，再硬清理其余旧结果。
- 对 Anthropic profile 会自动启用相关默认，以减少 prompt cache 写入成本。

二者分工明确：compaction 牺牲历史细节换连续性，并持久写 transcript；pruning 只处理工具结果膨胀，不改变历史，是降低成本和延迟的辅助手段。

## 会话、并发与任务分解

### 会话并发策略

OpenClaw 通过 session key 和 lane-aware FIFO queue 控制并发：

- `runEmbeddedPiAgent` 按 session key 进入 `session:<key>` lane，保证单个 session 同时只有一个 active run。
- 每个 session run 再进入全局 lane，默认 `main`，由 `agents.defaults.maxConcurrent` 控制全局并行度。
- `subagent` 有独立 lane，默认并发 8；main lane 默认 4；未配置 lane 默认 1。
- transcript 写入还有 session write lock，避免绕过 in-process queue 的写入者破坏历史一致性。

Inbound message queue modes 包括：

- `collect`：默认模式，合并 queued messages 为一个 followup turn。
- `followup`：当前 run 结束后开启下一轮。
- `steer`：在当前 run 的下一个模型边界注入 steering。官方 agent runtime 文档指出，queued steering 会在当前 assistant turn 完成工具调用后、下一次 LLM call 前投递，不再跳过当前 assistant message 的剩余 tool calls。
- `steer-backlog`：既 steer 又保留 followup。
- `interrupt`：legacy，abort 当前 run 后运行最新消息。

这套策略用于避免 LLM/tool run 与 session 文件、CLI stdin、日志、上游限流发生冲突，同时允许不同 session 或后台任务并行。

### Sub-agents

OpenClaw 用 `sessions_spawn` 和 `/subagents` 支持任务分解：

- sub-agent 是从现有 agent run 启动的后台 agent run，运行在独立 session，session key 形如 `agent:<agentId>:subagent:<uuid>`。
- 每个 sub-agent run 被追踪为 background task。
- `sessions_spawn` 非阻塞，立即返回 `runId` 和 `childSessionKey`。
- 完成后通过 announce step 将 summary/result 推回 requester chat channel。
- 默认 sub-agent 隔离 session，可选 sandbox，默认不获得 session/system tools。
- 可配置 `maxSpawnDepth`，默认 1；设置 2 可形成 main -> orchestrator sub-agent -> worker sub-sub-agents 的模式。
- depth 1 orchestrator 在允许嵌套时获得 `sessions_spawn`、`subagents`、`sessions_list`、`sessions_history` 管理孩子；leaf 默认没有递归编排工具。
- `maxChildrenPerAgent` 和 `maxConcurrent` 用作 fan-out 安全阀。

工程取舍是：sub-agents 用独立 session 和有限工具面降低污染主上下文的风险，但 token 成本和资源消耗独立计算。官方建议对复杂或重复任务，可给 sub-agent 配置更便宜的模型。

## 亮点

1. **本地优先，但不是无边界执行**  
   OpenClaw 面向个人本机 assistant，同时明确区分 host Gateway、sandboxed tool execution、DM pairing/allowlist、tool policy、per-agent sandbox/tool overrides。这种“默认可用 + 可收紧”的设计比纯远端 SaaS 或只靠提示词安全更工程化。

2. **Tools / Skills / Plugins 职责分离**  
   Tools 是可执行能力，Skills 是操作指南，Plugins 是注册和分发容器。完整 skill 不默认进入 prompt，只放摘要，需要时读取，控制了上下文膨胀。

3. **上下文成本可观测**  
   `/context detail` 能查看 per-file、per-skill、per-tool schema size。很多 agent 系统只关注历史消息 token，OpenClaw 把 tool schema 也作为一等成本暴露给用户。

4. **Markdown 事实源 + 可重建索引**  
   长期事实不藏在不可见向量库里，而是写入可审计 Markdown 文件；检索用 SQLite、FTS、vector、hybrid search 做性能层。这利于备份、迁移和人工修正。

5. **在线召回与离线巩固分离**  
   Active Memory 是主回复前的阻塞 recall；Dreaming 是后台阈值化 promotion。两者避免把所有记忆逻辑压进主 agent 的单次推理。

6. **Compaction 和 Pruning 分工明确**  
   Compaction 持久总结对话，pruning 仅内存裁剪旧工具结果。这样既能保持 transcript 完整，又能降低工具输出对上下文和缓存成本的拖累。

7. **Session Lane + Write Lock 保证一致性**  
   per-session lane 避免同一个 session 中多个 agent run 并发踩 transcript；文件级 session write lock 进一步防止跨进程或绕过队列的写入冲突。

8. **Sub-agent 权限按深度收窄**  
   默认 sub-agent 不能再 spawn，也没有 session/system tools；只有 orchestrator depth 在明确配置下拿到有限编排工具。这是防止递归失控和上下文泄漏的实用设计。

9. **跨渠道和多 agent 路由确定性强**  
   inbound routing 通过 bindings，most-specific wins；agent 是 workspace、agentDir、session store、auth profile 的组合，避免多用户或多 persona 混状态。

## 面试问答 / 讲述要点

### 1. OpenClaw 到底是什么？

可以回答：OpenClaw 不是一个 LLM，而是本地优先的 Gateway + embedded agent runtime。Gateway 管渠道、WebSocket API、session、config、cron、canvas；agent runtime 管模型调用、工具流、上下文和持久化。它更像一个跨渠道常驻 agent 平台，而不是普通聊天机器人。

### 2. 它的工具系统有什么设计特点？

可以回答：它把能力拆成 tools、skills、plugins 三层。Tools 是 typed functions，负责真实执行；Skills 是 Markdown 运行手册，告诉 agent 什么时候、怎么用工具；Plugins 是能力注册容器。这样做的好处是扩展能力和控制上下文可以分开处理。

### 3. 工具安全靠什么保证？

可以回答：OpenClaw 不只靠系统提示。它有 `tools.allow` / `tools.deny`、tool profiles、tool groups、provider-specific restrictions、sandbox backend、exec approval、plugin hooks、DM pairing/allowlist 等机制。sandbox 不是完美安全边界，但能降低模型误操作对 host 的影响。

### 4. 记忆系统为什么强调 Markdown？

可以回答：OpenClaw 把 `MEMORY.md`、`memory/YYYY-MM-DD.md`、`DREAMS.md` 这类 Markdown 文件作为事实源。SQLite、FTS、vector search 和 hybrid search 是可重建索引层，不是唯一事实源。这样长期记忆可审计、可备份、可人工编辑。

### 5. Active Memory 和 Dreaming 的区别是什么？

可以回答：Active Memory 是在线召回，在主回复前启动一个阻塞式 memory sub-agent，只能用 `memory_search` 和 `memory_get`，用于找相关记忆；Dreaming 是后台巩固，light/REM/deep 三阶段中只有 deep 满足分数和阈值后才写入 `MEMORY.md`。前者影响回复延迟，后者影响长期记忆质量。

### 6. OpenClaw 如何管理上下文？

可以回答：它每次 run 重建 system prompt，并注入 workspace bootstrap files、skills list、工具信息、时间和运行时元数据等。完整 skill 不默认注入，只保留摘要，需要时读取。它还提供 `/context list`、`/context detail`、`/usage tokens`、`/compact` 等命令，让用户看到上下文和 token 成本。

### 7. Compaction 和 Pruning 有什么不同？

可以回答：Compaction 是持久化总结，会把旧 turns 总结成 compact entry 并写进 session transcript；Pruning 是调用模型前的轻量裁剪，主要 trim 旧 tool results，不改写磁盘 transcript。前者解决长会话连续性，后者降低工具结果膨胀带来的上下文和缓存成本。

### 8. 它如何避免会话并发冲突？

可以回答：OpenClaw 使用 per-session lane + global lane + write lock。同一 session 的 run 会串行，跨 session 可以并行；transcript 写入还有 session write lock，避免多个 run 或跨进程写入破坏历史一致性。

### 9. Sub-agent 适合怎么讲？

可以回答：OpenClaw 的 sub-agent 通过 `sessions_spawn` 启动，运行在独立 session 中，非阻塞返回 `runId` 和 `childSessionKey`，完成后 announce 结果。默认 sub-agent 的工具面和递归能力受限，可配置最大深度、最大子任务数和并发数。这是一种把复杂任务分出去、减少主上下文污染的设计。

### 10. 可以总结的 tradeoff 是什么？

可以回答：OpenClaw 选择本地可控与强执行能力，因此必须引入工具权限、sandbox、pairing/allowlist、可审计 memory、上下文可观测和会话并发控制。它比纯聊天机器人复杂，但换来跨渠道常驻 assistant 和真实操作能力。

## 参考资料

- OpenClaw GitHub README：<https://github.com/openclaw/openclaw>
- OpenClaw 官方网站：<https://openclaw.ai/>
- Gateway architecture：<https://docs.openclaw.ai/concepts/architecture>
- Agent runtime：<https://docs.openclaw.ai/concepts/agent>
- Agent loop：<https://docs.openclaw.ai/concepts/agent-loop>
- Tools and Plugins：<https://docs.openclaw.ai/tools>
- Sandboxing：<https://docs.openclaw.ai/gateway/sandboxing>
- Context：<https://docs.openclaw.ai/concepts/context>
- Context Engine：<https://docs.openclaw.ai/concepts/context-engine>
- Session management：<https://docs.openclaw.ai/concepts/session>
- Session pruning：<https://docs.openclaw.ai/concepts/session-pruning>
- Session tools：<https://docs.openclaw.ai/concepts/session-tool>
- Compaction：<https://docs.openclaw.ai/concepts/compaction>
- Memory overview：<https://docs.openclaw.ai/concepts/memory>
- Builtin memory engine：<https://docs.openclaw.ai/concepts/memory-builtin>
- Memory search：<https://docs.openclaw.ai/concepts/memory-search>
- Active Memory：<https://docs.openclaw.ai/concepts/active-memory>
- Dreaming：<https://docs.openclaw.ai/concepts/dreaming>
- Memory Wiki plugin：<https://docs.openclaw.ai/plugins/memory-wiki>
- Multi-Agent Routing：<https://docs.openclaw.ai/concepts/multi-agent>
- Command queue：<https://docs.openclaw.ai/concepts/queue>
- Sub-Agents：<https://docs.openclaw.ai/tools/subagents>
