# Claude Code / OpenClaw / Hermes Agent 系统调研总结

## 1. 标题与摘要

本文合并三份最终调研报告：Claude Code、OpenClaw 和 Hermes Agent。目标不是逐段拼接，而是从学习和面试表达角度，提炼三类 agent 系统在工具调用、记忆与状态、上下文管理、任务分解和工程安全边界上的共性与差异。

一句话概括：

- **Claude Code** 更像一个运行在开发者终端中的受控 coding agent，重点是“本地项目理解 + 工具执行 + 权限控制 + 显式项目记忆”。
- **OpenClaw** 更像一个本地优先、跨渠道常驻的 Gateway + embedded agent runtime，重点是“消息路由 + 强工具执行 + 可审计记忆 + 上下文成本观测 + 会话并发控制”。
- **Hermes Agent** 更像一个本地/终端 coding 与 automation agent 框架，重点是“agent loop + tools runtime + prompt assembly + memory providers + skills”。

事实边界：本文只使用三份报告中已经整理出的公开资料结论。凡是报告明确标注“未从公开资料确认”的内部 prompt、工具选择策略、压缩算法、存储 schema、provider 内部索引/召回细节、安全审查闭环等内容，本文均不写成确定事实。

## 2. 一页速览：三套 agent 系统的定位差异

| 系统 | 更准确的定位 | 最核心的工程问题 | 最突出的设计抓手 | 面试中最适合强调 |
| --- | --- | --- | --- | --- |
| Claude Code | 面向开发者终端的 agentic coding 工具 | 如何让模型在本地代码库中安全执行开发动作 | 工具调用与权限系统分层、`CLAUDE.md` 显式记忆、Hooks、MCP、subagent 上下文隔离 | “模型规划 + 工具执行 + 权限控制”的终端 coding agent |
| OpenClaw | 本地优先的 Gateway + embedded agent runtime | 如何让 agent 跨渠道常驻运行，同时保持工具安全、记忆可审计、上下文可观测 | Tools/Skills/Plugins 三层、Markdown 记忆事实源、Active Memory、Dreaming、context engine、session lane | “本地常驻 + 多渠道路由 + 强工具执行”的 agent 平台 |
| Hermes Agent | 本地/终端 coding 与 automation agent 框架 | 如何把工具、记忆、会话、技能和上下文组装成稳定 agent loop | Tools Runtime、Prompt Assembly、Memory Providers、Session Storage、Context Compression/Caching、Skills | “agent loop + tools runtime + prompt assembly”的模块化执行框架 |

最简对比：

- **Claude Code** 的强项是开发者工作流落地：读代码、改文件、跑命令、做 git 工作流，并用权限与 Hooks 控制风险。
- **OpenClaw** 的强项是平台化常驻：不同聊天渠道、会话路由、并发队列、sandbox、memory、context observability 都进入系统设计。
- **Hermes** 的强项是架构分层：工具运行时、prompt 组装、记忆 provider、session storage、skills、压缩/缓存各司其职。

## 3. 横向对比表

| 维度 | Claude Code | OpenClaw | Hermes Agent |
| --- | --- | --- | --- |
| 工具调用 | 内置文件读写、搜索、Bash、Web、Notebook、Todo、Task/subagent、MCP 等；工具执行受权限配置约束 | typed tools 覆盖 runtime、fs、browser/web、message、cron、sessions、memory、media 等；由 tools/skills/plugins 分层组织 | tools 按 toolset 分域；Tools Runtime 注册工具、暴露 schema、执行 tool call 并回灌 agent loop |
| 权限与安全 | `allow` / `ask` / `deny`、CLI 覆盖、目录信任、交互确认、Hooks | `tools.allow` / `tools.deny`、tool profiles、provider restrictions、sandbox backend、exec approval、plugin hooks、DM pairing/allowlist | 公开资料确认工具/能力管理方向；更细粒度安全策略、沙箱边界、危险命令防护未完全确认 |
| 记忆系统 | 以 `CLAUDE.md` 和多层 memory 为主，显式 Markdown 可审查；未确认黑盒向量长期记忆 | `MEMORY.md`、`memory/YYYY-MM-DD.md`、`DREAMS.md` 是事实源；SQLite/FTS/vector/hybrid search 是可重建索引层 | Memory 与 session history 区分；Memory 后端抽象为 providers；provider 内部索引与召回细节未完全确认 |
| 状态持久化 | 支持 `/memory`、`#` 快捷记忆、会话 resume/continue；内部会话 schema 未公开确认 | Gateway 拥有 session store 和 transcript；DM/group/cron/webhook 有不同 session 路由策略 | Session Storage 单独成层，用于保存会话状态或历史；完整 session 不等于完整 prompt |
| 上下文管理 | `@` 文件引用、按需读取、Todo、subagent 隔离、`/compact`、`resume` | system prompt、bootstrap files、skills list、tool schema、history、attachments、compaction summaries、pruning artifacts 都计入 context；提供 `/context`、`/usage tokens` 等观测入口 | Prompt Assembly 统一组装系统指令、历史、工具定义、context files、memory、skills；支持 compression/caching |
| Sub-agent / 任务分解 | subagent 有独立上下文、可定制 prompt、可限制工具；内部调度和结果压缩未公开确认 | `sessions_spawn` 启动独立 session 的 sub-agent，非阻塞返回 runId；默认工具面和递归能力受限 | 报告主要强调 agent loop、tools runtime、skills；未将 sub-agent 作为最突出公开机制 |
| 亮点 | 权限系统外置、Hooks 生命周期、显式项目记忆、MCP、compact 生命周期 | 常驻 Gateway、跨渠道路由、Markdown 事实源、Active Memory/Dreaming、上下文成本可观测、session lane | Prompt Assembly 独立成层、Memory/Session/Skills/Context 分层清晰、MCP 扩展 |
| 借鉴场景 | 开发者 CLI agent、代码库修改、测试/构建/git 自动化 | 私有化个人 assistant、跨渠道 agent、需要长期运行与会话治理的系统 | 自研 agent runtime、工具注册框架、上下文组装层、记忆 provider 抽象 |

## 4. 工具调用设计模式总结

三套系统的共同点是：**工具不是附属功能，而是 agent 能做真实工作的执行层**。模型负责规划和选择，工具层负责执行，工具结果再进入下一轮上下文。

可提炼出四个模式：

1. **模型能力与执行权限分离**

Claude Code 最明显：模型可以请求 Bash、编辑文件或调用 MCP，但权限层决定允许、询问还是拒绝。OpenClaw 也通过 allow/deny、profile、sandbox、provider restrictions 等实现类似分层。这个模式的重点是：不要只靠 prompt 让模型“别做危险事”，应把 policy enforcement 放到模型外部。

2. **工具按能力域组织，而不是无限全局函数表**

OpenClaw 用 tools/skills/plugins 三层拆分：Tools 是可执行 API，Skills 是操作规程，Plugins 是注册和分发边界。Hermes 用 toolsets 管理 shell、filesystem、browser、MCP、memory、skills 等能力域。Claude Code 虽然报告中没有用 toolset 这个词描述，但也能看到文件、Bash、Web、Todo、Notebook、Task、MCP 等工具族。

3. **生命周期钩子补足模型不可靠性**

Claude Code 的 Hooks 是典型例子：`PreToolUse` 可拦截危险操作，`PostToolUse` 可运行 formatter、lint 或记录审计，`PreCompact` 可在压缩前保存关键状态。OpenClaw 也有 plugin hooks，可在 `before_tool_call` / `after_tool_call` 处理工具参数和结果。

4. **MCP/插件化减少核心膨胀**

Claude Code、OpenClaw、Hermes 都把 MCP 或插件化作为外部工具接入机制的一部分。核心思路是：通用本地能力由内置工具覆盖，外部系统通过协议或插件注册进来，避免 agent core 为每个第三方服务硬编码集成。

面试表达可以这样讲：

> 一个成熟 agent 的工具系统，通常不是“模型随便调用函数”。它至少需要工具注册、schema 暴露、参数校验、执行隔离、权限控制、结果回灌、审计/Hook，以及外部工具扩展协议。Claude Code、OpenClaw 和 Hermes 的差异在产品定位，但都围绕这个执行闭环设计。

## 5. 记忆与状态设计模式总结

三份报告都在强调一件事：**记忆、会话历史、项目上下文、技能流程不是同一种东西**。

1. **显式文本事实源优先**

Claude Code 的 `CLAUDE.md` 和 OpenClaw 的 `MEMORY.md` / daily memory 都体现了“文本即记忆”的设计。优点是可审查、可版本化、可备份、可人工修正。报告没有支持把 Claude Code 记忆描述成黑盒向量长期记忆；OpenClaw 也明确把 Markdown 作为事实源，SQLite/FTS/vector/hybrid search 是索引和检索层。

2. **长期记忆与会话历史分离**

Hermes 明确区分 Memory 和 Session Storage。OpenClaw 也区分 memory 文件与 session transcript。这个区分很关键：会话历史适合恢复过程和审计，长期记忆适合保存稳定事实、偏好和项目决策。把所有历史直接塞进 prompt 会带来 token 成本和噪声。

3. **在线召回与离线巩固分离**

OpenClaw 的 Active Memory 和 Dreaming 是最完整的公开例子。Active Memory 在主回复前阻塞召回相关记忆；Dreaming 在后台用 light/REM/deep 阶段做长期巩固，并且只有 deep 满足阈值后才写入 `MEMORY.md`。这说明记忆不只是“存进去”，还要考虑何时召回、何时提升、如何避免低质量内容污染长期记忆。

4. **Skills 是过程型记忆**

OpenClaw 和 Hermes 都强调 skills。可以把 Memory 理解为事实型记忆，把 Skills 理解为过程型记忆。事实型记忆保存“用户偏好、项目约定、重要事实”；过程型记忆保存“做某类任务的步骤和工具使用方式”。这比把所有经验都写成聊天摘要更可控。

事实边界：Hermes 的 Memory Provider 内部数据结构、embedding 选择、召回排序、去重合并策略未从公开资料完全确认；Claude Code 是否使用向量数据库保存长期记忆也未从公开资料确认。

## 6. 上下文管理设计模式总结

三套系统都没有把长任务问题简化为“上下文窗口越大越好”。更准确的共识是：**上下文是每次模型调用前被选择、组装、裁剪、压缩和恢复出来的工作集**。

1. **按需加载优于全量注入**

Claude Code 用 `@` 文件引用和搜索后读取来避免一次性塞入整个仓库。OpenClaw 的 skills 只默认注入 name、description、location，完整 `SKILL.md` 需要时再读。Hermes 的 context files 也强调显式提供项目背景，而不是让模型猜。

2. **Prompt Assembly / Context Engine 是核心控制面**

Hermes 把 Prompt Assembly 作为独立层，统一组装系统指令、用户请求、历史、工具定义、context files、memory 和 skills。OpenClaw 进一步把 context engine 做成可插拔接口，负责 ingest、assemble、compact、after turn。这个设计适合自研 agent：不要让各模块直接拼 prompt，而要有统一上下文入口。

3. **Compaction 和 Pruning 解决不同问题**

Claude Code 有 `/compact` 和 `PreCompact`；OpenClaw 明确区分 compaction 与 pruning：compaction 会把旧 turns 总结成 compact entry 并写入 transcript，pruning 只是模型调用前裁剪旧工具结果，不改写磁盘 transcript。Hermes 也有 context compression/caching，但具体触发阈值、摘要格式和缓存层细节未从公开资料确认。

4. **上下文成本需要可观测**

OpenClaw 在这方面最突出，报告提到 `/context list`、`/context detail`、`/usage tokens`，并把 tool schema 的上下文成本也暴露出来。这个点很适合借鉴：agent 系统不应只统计聊天历史 token，工具 schema、系统提示、bootstrap files、附件和压缩摘要也都是成本。

## 7. 面试回答框架：如何讲 agent 系统架构

可以用一个稳定框架回答：**入口层、上下文层、模型层、工具层、状态层、安全层、观测层**。

1. **入口层：请求从哪里来？**

Claude Code 主要面向终端/本地项目；OpenClaw 面向多聊天渠道和 Gateway；Hermes 面向本地/终端 coding 与 automation。先讲入口，能避免把所有 agent 都说成聊天机器人。

2. **上下文层：本轮模型看什么？**

说明系统如何组装 system prompt、用户请求、历史、文件、工具 schema、记忆、skills 和压缩摘要。这里可以重点讲 Hermes 的 Prompt Assembly、OpenClaw 的 Context Engine、Claude Code 的 `@` 引用和 compact。

3. **模型层：模型负责什么？**

模型负责基于上下文规划下一步，输出文本或结构化 tool call。不要过度声称知道内部 prompt 或 planner；三份报告都有未确认边界。

4. **工具层：动作如何执行？**

讲工具注册、schema 暴露、执行、结果回灌。Claude Code 可讲 Bash/Edit/Web/Todo/MCP 与权限；OpenClaw 可讲 typed tools、tools/skills/plugins、sandbox；Hermes 可讲 Tools Runtime 和 toolsets。

5. **状态层：什么信息跨轮或跨会话保留？**

区分 session history、long-term memory、context files、skills、daily notes、compact summaries。强调长期事实不应只依赖聊天历史。

6. **安全层：如何限制强执行能力？**

重点是模型外部控制：allow/deny/ask、tool profile、sandbox、hooks、目录信任、provider restrictions、DM allowlist 等。Hermes 的细粒度安全细节未完全确认，回答时应保持边界。

7. **观测层：如何知道系统在做什么、花了多少上下文？**

OpenClaw 的 `/context detail`、`/usage tokens` 是好例子；Claude Code 的 Todo 和 Hooks 也增加可见性；Hermes 的 Session Storage 有利于恢复和调试。

可直接用于面试的一段话：

> 我会把 agent 系统拆成“上下文组装、模型决策、工具执行、状态持久化、安全控制和观测”几层。Claude Code 更偏终端 coding agent，它把文件、Bash、MCP 等能力放到工具层，再用权限和 Hooks 控制风险；OpenClaw 更像常驻 agent 平台，有 Gateway、多渠道 session、sandbox、Markdown memory 和 context engine；Hermes 更像模块化 runtime，Tools Runtime、Prompt Assembly、Memory Providers、Session Storage、Skills 分层清楚。三者共同点是都不只靠大上下文，而是通过按需加载、记忆检索、压缩和会话恢复管理长期任务。

## 8. 可迁移到自己项目的设计清单

- **工具注册表**：每个工具有名称、描述、参数 schema、执行函数、权限标签和结果大小限制。
- **权限外置**：把 allow/deny/ask、危险命令拦截、网络/文件写入权限放在模型之外。
- **工具生命周期 Hook**：至少支持 before tool call、after tool call、before compact，用于审计、格式化、阻断和状态保存。
- **显式项目记忆**：用 Markdown 文件保存项目约定、常用命令、用户偏好和重要决策。
- **记忆索引可重建**：如果做向量或全文检索，让 Markdown/文件成为事实源，索引只是性能层。
- **Session 与 Memory 分离**：session 保存过程，memory 保存稳定事实，不把完整历史直接当长期记忆。
- **Prompt Assembly 单点化**：所有上下文来源都通过一个组装层进入模型，避免模块各自拼 prompt。
- **按需加载 Skills**：系统提示中只放技能摘要，需要时再读取完整技能说明，控制上下文膨胀。
- **工具结果裁剪**：长工具输出需要 head/tail、截断、摘要或持久化引用，避免污染下一轮 prompt。
- **压缩可审计**：compact 结果应写入 transcript 或可见状态；关键约束应保存在文件或显式计划中。
- **上下文成本观测**：统计 system prompt、工具 schema、文件、skills、history、attachments、summary 的 token 或近似大小。
- **Sub-agent 隔离**：独立任务用独立上下文和受限工具面执行，只把结论汇回主会话。
- **并发写入保护**：同一 session 的 run 串行，transcript 写入加锁，避免历史被并发破坏。
- **MCP/插件化扩展**：外部服务通过协议或插件接入，不要把所有第三方集成写进 agent core。

## 9. 常见追问与回答

### Q1：这三个系统最大的共同点是什么？

它们都把 agent 看成一个循环：组装上下文，调用模型，模型决定调用工具或回复，工具执行后把结果回灌，再进入下一轮。区别在于产品定位和工程边界：Claude Code 偏开发者终端，OpenClaw 偏常驻平台，Hermes 偏模块化 runtime。

### Q2：为什么不能只靠更大的上下文窗口？

因为上下文里不只有聊天历史，还包括 system prompt、工具 schema、文件、技能、附件、工具结果、压缩摘要等。三份报告都说明成熟系统会做按需读取、压缩、裁剪、记忆检索和会话恢复，而不是简单把所有东西塞进 prompt。

### Q3：工具调用最容易出问题的地方是什么？

强副作用工具，例如 shell、文件写入、网络、浏览器、外部服务调用。成熟设计会把“模型能提出调用”和“系统是否允许执行”分开，用权限、sandbox、hooks、profile、目录信任或 allowlist 限制风险。

### Q4：记忆系统为什么要用 Markdown 或显式文件？

显式文件可审查、可备份、可版本化、可人工修改。Claude Code 和 OpenClaw 都体现了这个方向。向量索引或全文索引可以提升召回，但如果把不可见数据库当唯一事实源，调试和纠错会更困难。

### Q5：Session history 和 Memory 有什么区别？

Session history 保存交互过程和工具执行轨迹，适合审计、恢复和调试；Memory 保存跨会话稳定事实，例如用户偏好、项目约定和长期决策。完整 session 不应无差别进入模型上下文。

### Q6：Skills 和 Memory 的区别怎么讲？

Memory 是事实型记忆，回答“已知什么”；Skills 是过程型记忆，回答“遇到某类任务该怎么做”。OpenClaw 和 Hermes 都体现了这种流程知识与事实知识分离的思路。

### Q7：Sub-agent 的价值是什么？

Sub-agent 的核心价值不是简单并发，而是上下文隔离。它适合搜索、审查、调研、批量子任务等边界明确的工作，避免把探索过程污染主上下文。Claude Code 和 OpenClaw 的报告都强调了独立上下文或独立 session。

### Q8：OpenClaw 为什么比普通聊天机器人复杂？

因为它要处理常驻 Gateway、多渠道路由、session store、sandbox、tools profiles、memory、context engine、compaction、pruning、sub-agent、并发 lane 等问题。它换来的是跨渠道、长期运行和强工具执行能力。

### Q9：Hermes 的 Prompt Assembly 为什么重要？

因为 agent 的上下文来源很多。如果没有统一 Prompt Assembly，各模块会分散拼接 prompt，导致上下文顺序、优先级、裁剪和缓存不可控。Hermes 把它独立成层，说明 prompt 组装本身是 agent runtime 的核心工程问题。

### Q10：哪些内容面试时不能乱讲？

不要声称知道 Claude Code 的内部 system prompt、工具选择策略、压缩摘要算法或会话 schema；不要把 OpenClaw 中未公开确认的 Pi agent core 内部工具选择、完整 JSON Schema 说成事实；不要把 Hermes 的工具数量、provider 内部索引、自动自改进闭环、安全审查机制说成稳定结论。更稳妥的说法是“公开资料确认了外部机制，但内部实现细节未确认”。

## 10. 参考资料索引

### Claude Code

- Claude Code overview - Anthropic Docs: https://docs.anthropic.com/en/docs/claude-code/overview
- Claude Code interactive mode - Anthropic Docs: https://docs.anthropic.com/en/docs/claude-code/interactive-mode
- Claude Code CLI reference - Anthropic Docs: https://docs.anthropic.com/en/docs/claude-code/cli-reference
- Claude Code settings - Anthropic Docs: https://docs.anthropic.com/en/docs/claude-code/settings
- Claude Code memory - Anthropic Docs: https://docs.anthropic.com/en/docs/claude-code/memory
- Claude Code slash commands - Anthropic Docs: https://docs.anthropic.com/en/docs/claude-code/slash-commands
- Claude Code hooks - Anthropic Docs: https://docs.anthropic.com/en/docs/claude-code/hooks
- Claude Code MCP - Anthropic Docs: https://docs.anthropic.com/en/docs/claude-code/mcp
- Claude Code subagents - Anthropic Docs: https://docs.anthropic.com/en/docs/claude-code/sub-agents
- Claude Code security - Anthropic Docs: https://docs.anthropic.com/en/docs/claude-code/security
- Claude Code common workflows - Anthropic Docs: https://docs.anthropic.com/en/docs/claude-code/common-workflows
- anthropics/claude-code - GitHub: https://github.com/anthropics/claude-code

### OpenClaw

- OpenClaw GitHub README: https://github.com/openclaw/openclaw
- OpenClaw 官方网站: https://openclaw.ai/
- Gateway architecture: https://docs.openclaw.ai/concepts/architecture
- Agent runtime: https://docs.openclaw.ai/concepts/agent
- Agent loop: https://docs.openclaw.ai/concepts/agent-loop
- Tools and Plugins: https://docs.openclaw.ai/tools
- Sandboxing: https://docs.openclaw.ai/gateway/sandboxing
- Context: https://docs.openclaw.ai/concepts/context
- Context Engine: https://docs.openclaw.ai/concepts/context-engine
- Session management: https://docs.openclaw.ai/concepts/session
- Session pruning: https://docs.openclaw.ai/concepts/session-pruning
- Session tools: https://docs.openclaw.ai/concepts/session-tool
- Compaction: https://docs.openclaw.ai/concepts/compaction
- Memory overview: https://docs.openclaw.ai/concepts/memory
- Builtin memory engine: https://docs.openclaw.ai/concepts/memory-builtin
- Memory search: https://docs.openclaw.ai/concepts/memory-search
- Active Memory: https://docs.openclaw.ai/concepts/active-memory
- Dreaming: https://docs.openclaw.ai/concepts/dreaming
- Memory Wiki plugin: https://docs.openclaw.ai/plugins/memory-wiki
- Multi-Agent Routing: https://docs.openclaw.ai/concepts/multi-agent
- Command queue: https://docs.openclaw.ai/concepts/queue
- Sub-Agents: https://docs.openclaw.ai/tools/subagents

### Hermes Agent

- NousResearch/hermes-agent GitHub 仓库: https://github.com/NousResearch/hermes-agent
- Hermes Agent 官方文档首页: https://hermes-agent.nousresearch.com/docs/
- Architecture: https://hermes-agent.nousresearch.com/docs/developer-guide/architecture/
- Agent Loop: https://hermes-agent.nousresearch.com/docs/developer-guide/agent-loop/
- Tools Runtime: https://hermes-agent.nousresearch.com/docs/developer-guide/tools-runtime/
- Tools 用户指南: https://hermes-agent.nousresearch.com/docs/user-guide/features/tools/
- MCP Integration: https://hermes-agent.nousresearch.com/docs/user-guide/features/mcp/
- Memory: https://hermes-agent.nousresearch.com/docs/user-guide/features/memory/
- Memory Providers: https://hermes-agent.nousresearch.com/docs/user-guide/features/memory-providers/
- Session Storage: https://hermes-agent.nousresearch.com/docs/developer-guide/session-storage/
- Prompt Assembly: https://hermes-agent.nousresearch.com/docs/developer-guide/prompt-assembly/
- Context Compression and Caching: https://hermes-agent.nousresearch.com/docs/developer-guide/context-compression-and-caching/
- Context Files: https://hermes-agent.nousresearch.com/docs/user-guide/features/context-files/
- Skills: https://hermes-agent.nousresearch.com/docs/user-guide/features/skills/
