# Hermes Agent 调研报告

## 摘要

本文中的 Hermes 指 **NousResearch/hermes-agent**，即 Nous Research 开源的 Hermes Agent 系统，而不是 Nous 的 Hermes 系列语言模型或其他同名项目。

Hermes Agent 的定位是面向本地/终端环境的 AI coding 与 automation agent。公开资料显示，它围绕 agent loop 组织任务执行：接收用户输入，组装上下文，调用模型，执行工具，记录结果，并进入下一轮推理。其核心能力包括工具调用、文件编辑、shell 命令、浏览器交互、MCP 集成、长期记忆、技能系统、会话持久化、上下文压缩与缓存。

需要保留的事实边界是：官方资料在不同页面对内置工具数量的表述存在版本差异，例如架构页提到 “61 tools across 52 toolsets”，其他工具页/参考页可能出现 “68 built-in tools” 等说法。因此，本报告不把具体工具数量作为稳定结论，只讨论工具机制、能力类别和工程设计。

## 项目定位

Hermes Agent 不是单纯的聊天机器人，而是一个具备本地执行能力的 agent 系统。它的公开文档覆盖用户指南和开发者指南，主题包括 Tools Runtime、Agent Loop、Prompt Assembly、Context Compression、Session Storage、Memory、Skills 等，说明其关注点是 agent 系统实现，而不仅是模型调用封装。

可以把 Hermes Agent 理解为一个本地任务执行框架：模型负责规划和决策，工具运行时负责执行动作，记忆、技能、会话存储和上下文管理负责维持长期任务中的状态与可复用知识。

同名 “Hermes” 还可能指 Nous 的 Hermes 语言模型、其他聊天 UI、社区项目或无关工具。这些资料不以 agent 系统实现为核心，因此不作为本次研究对象。

## 核心结论

1. Hermes 的核心不是“模型聊天”，而是围绕 agent loop 的工具执行系统。模型可以返回结构化 tool call，Tools Runtime 执行后将结果回灌到下一轮上下文。
2. Hermes 将工具按 toolset 分域管理，例如 shell、filesystem、browser、MCP、memory、skills 等，而不是维护一个无边界的全局函数表。
3. Hermes 区分 session storage、long-term memory、context files、skills、context compression 等不同状态层，避免把所有历史和知识都塞进聊天上下文。
4. Prompt Assembly 是关键工程层，负责把系统指令、用户请求、会话历史、工具定义、上下文文件、记忆检索结果、技能信息等统一组装给模型。
5. MCP 是 Hermes 的重要扩展机制，使外部系统可以通过标准协议接入，而不必把每个第三方服务硬编码进 agent core。
6. Hermes 的 self-improving 方向在公开资料中主要体现在 Memory 和 Skills：前者保存长期知识，后者保存可复用流程。但公开资料不足以确认其自动自改进闭环、安全审查和评估机制已经完整实现。

## 工具调用

### 工具体系分层

公开文档显示，Hermes 的工具能力是分层组织的：

- **Toolsets**：按能力域组织工具，例如 shell、filesystem、browser、MCP、memory、skills。
- **Tools**：具体可调用动作，例如运行命令、读写文件、浏览网页、编辑文件、调用 MCP server。
- **Tools Runtime**：注册工具、向模型暴露工具 schema、执行工具调用，并把结果返回 agent loop。
- **Agent Loop**：接收模型返回的 tool call，调用 runtime 执行，再把 observation/result 作为下一轮上下文。

这种设计的价值在于工具能力被按领域隔离。新增、禁用或控制工具时，可以以 toolset 为单位管理，有利于权限控制、上下文预算和维护。

### 模型侧调用流程

Hermes 依赖模型的 tool-calling 能力驱动工具执行。公开资料可确认的基本流程是：

1. Prompt Assembly 将系统提示、会话历史、上下文文件、记忆、技能信息和可用工具定义组装给模型。
2. 模型返回普通文本或结构化 tool call。
3. Tools Runtime 根据 tool name 和 arguments 找到对应工具并执行。
4. 工具结果写入会话历史，并作为下一轮 agent loop 的上下文。
5. 模型继续决定调用工具、追问用户，或生成最终答复。

未从公开资料确认的细节包括：每个工具 schema 的完整内部生成方式、工具参数校验是否统一依赖某个特定 schema 库、工具执行的超时/重试/并发策略的完整代码路径。

### 内置工具类别

官方资料可确认的工具类别包括：

- **Shell/command execution**：执行本地命令，用于构建、测试、搜索和自动化。
- **Filesystem/file editing**：读取、写入和编辑文件。
- **Browser/web interaction**：打开页面、抓取或交互网页，辅助调研和 UI 验证。
- **MCP integration**：通过 Model Context Protocol 接入外部工具和数据源。
- **Memory tools**：写入、读取和搜索长期记忆。
- **Skills tools**：创建、管理或使用技能，使 agent 能复用流程知识。

工程取舍也很明确：Hermes 把本地执行能力做成 first-class tools，因此能完成开发任务，而不是只给出建议；代价是权限、安全边界和危险操作防护会变得更重要。公开文档确认存在工具/能力管理，但更细粒度的安全策略、沙箱边界和危险命令防护细节未从公开资料完全确认。

### MCP 扩展机制

Hermes 支持 MCP，这是工具扩展上的关键设计。

MCP 的意义在于：agent core 不需要硬编码所有外部系统集成；外部服务可以通过 MCP server 暴露工具；Hermes 再把这些工具纳入可调用集合。这样可以把核心 agent 逻辑与外部集成面解耦。

可学习点是：内置工具负责通用本地能力，MCP 负责外部系统能力。相比“每接一个服务就写一个专用插件”，这种标准协议更适合长期扩展。

## 记忆与状态持久化

### 长期记忆

Hermes 的 Memory 功能定位为跨会话持久化信息，用于保存用户偏好、项目约定、重要事实和可复用知识。

公开文档确认：Hermes 有 memory feature，支持 memory provider，Memory 可以在后续会话中被检索和使用，并且 Memory 与 session history 不同。session history 记录当前或历史会话过程，Memory 更像长期、可检索的知识层。

这个区分很重要。会话记录适合还原过程，长期记忆适合抽取稳定事实。如果直接把所有历史消息塞进上下文，会快速消耗 token，也会引入大量噪声。

### Memory Providers

官方 Memory Providers 文档显示，Hermes 将记忆后端抽象为 provider。其工程意义是：记忆系统不绑定单一存储实现，可以按部署场景选择本地、外部服务或其他 provider；agent 核心只依赖 provider 接口，而不是依赖具体存储。

未从公开资料完全确认的内容包括：各 provider 的具体数据结构、索引字段、embedding 模型选择、召回排序算法；是否所有 provider 都支持语义搜索、关键字搜索和 metadata 过滤；写入 memory 时是否有统一的去重、合并或过期策略。

### Session Storage

Hermes 的开发者文档单独说明 Session Storage，说明它把会话作为一类需要持久化的状态处理。

公开资料确认的方向是：Session Storage 负责保存会话状态或历史，使 agent 能恢复或查看之前的对话和执行过程。同时，它与上下文管理相关：不是所有 session 内容都会原样进入模型上下文，模型上下文仍需要经过组装、选择和压缩。

这里的设计取舍是：完整 session 有利于审计、恢复和调试；但完整 session 不等于完整 prompt。Hermes 因此需要同时使用 session storage、memory 和 context compression，而不是只依赖一种历史记录。

## 上下文管理

### Prompt Assembly

Hermes 将 Prompt Assembly 作为开发者文档中的独立主题，说明它的上下文不是简单拼接用户消息。

公开资料可确认，Prompt Assembly 会整合多类信息：

- 系统/开发者指令。
- 当前用户请求。
- 会话历史。
- 可用工具定义。
- Context files。
- 记忆检索结果。
- 技能或 agent 行为相关指令。

从工程视角看，Prompt Assembly 的价值是集中管理上下文选择。工具系统、记忆系统、文件上下文和技能系统不需要各自随意修改 prompt，而是由统一入口决定本轮给模型看什么。

未从公开资料确认的细节包括：各类上下文的优先级、token budget 分配公式，以及上下文超限时的裁剪顺序。

### Context Files

Hermes 支持 context files，用于把项目相关文件或说明纳入 agent 上下文。

可确认的设计意图是：用户可以显式提供项目背景，而不是让模型自己猜测。Context files 适合放项目约定、架构说明、任务背景等稳定信息，能降低重复解释成本，也能让 agent 在多轮开发任务中更一致。

其取舍是：显式 context files 比“自动读全仓库”更可控；但如果这些文件过期，agent 也可能继承错误假设。

### Context Compression and Caching

官方开发者文档有 Context Compression and Caching 页面，说明 Hermes 对长会话上下文有专门处理。

公开资料可确认的方向包括：长会话不会无限追加原始消息；Hermes 使用压缩机制降低上下文长度；也使用缓存机制优化重复上下文的成本或延迟。

这对 agent loop 很关键，因为工具结果往往很长，原样保留会快速污染上下文。压缩机制用于保留任务进展、关键决策和必要状态，同时丢弃低价值细节；缓存则适合系统提示、工具定义、稳定上下文文件等重复内容。

未从公开资料确认的细节包括：压缩摘要的触发阈值、摘要格式、是否使用分层摘要；缓存使用的是具体模型供应商的 prompt caching，还是 Hermes 自身缓存层。

### Skills 与任务分解

Hermes 的 Skills 是明显的上下文和能力复用机制。

公开资料确认：Skills 是可复用的 agent 行为或流程知识；agent 可以通过技能执行特定类型任务，而不是每次都从零推理工作流；Skills 与 self-improving 定位相关，因为系统可以积累可复用技能。

可以把 Skills 理解为“过程型记忆”，而 Memory 更接近“事实型记忆”。对复杂 coding agent 来说，这个区分很重要：流程经验应该以结构化技能复用，用户偏好和项目事实则更适合放入长期记忆。

未从公开资料完全确认的内容包括：Skill 的文件格式、加载时机、冲突处理、版本管理策略；agent 是否能自动创建或修改 skills，以及自动修改时的安全审查机制。

### Agent Loop 会话策略

Hermes 的 agent loop 可以概括为：

1. 收集用户输入和当前状态。
2. 通过 Prompt Assembly 生成模型输入。
3. 模型输出文本或 tool call。
4. 如果是 tool call，Tools Runtime 执行并返回结果。
5. 记录 session state。
6. 根据上下文窗口和策略进行压缩或缓存。
7. 重复循环，直到模型给出最终答复或任务停止。

这是一种典型 tool-using agent 架构。Hermes 的特点在于 memory、skills、MCP、context compression、session storage 都进入了这个循环，而不是作为旁路功能存在。

## 亮点

### 分层清楚

Hermes 没有把所有上下文都归为“记忆”，而是拆成多个职责：

- Tools：执行动作。
- Memory：保存长期事实和偏好。
- Session Storage：保存过程和历史。
- Context Files：提供显式项目背景。
- Skills：保存可复用工作流。
- Context Compression：处理长会话的信息密度。

这个分层值得学习。很多 agent 项目会把所有内容都塞进聊天历史，导致上下文不可控、成本高、行为漂移。Hermes 的设计更像是在区分“执行能力、事实知识、过程历史、项目背景、流程经验”。

### 扩展边界合理

Hermes 通过 MCP 扩展外部工具能力，避免 agent core 被越来越多第三方集成拖复杂。核心内置通用本地能力，外部系统通过标准协议接入，再由统一的 tool registry/runtime 执行模型发出的调用。

### Prompt Assembly 独立成层

把 prompt 组装抽成独立层，是一个重要工程设计。上下文来源变多后，如果没有统一入口，工具定义、记忆、文件、会话历史很容易互相污染。Prompt Assembly 让排序、裁剪、压缩和缓存有了集中的控制点。

### 长会话不是只靠摘要

Hermes 同时使用 session storage、memory providers、context files、compression/caching 和 skills，说明它把长会话问题拆成多个子问题：恢复历史靠 session storage，记住稳定事实靠 memory，提供项目背景靠 context files，控制 token 靠 compression，复用工作流靠 skills。

这种拆解比单一“总结历史消息”更稳。

### Self-improving 的落地点

Hermes 的 self-improving 不只是口号。公开资料中能看到两个落地方向：Memory 保存长期知识，Skills 保存可复用流程。

但这里必须谨慎表述：公开资料没有充分确认其自动自改进的完整闭环、安全审查和评估机制，因此不能断言它已经实现了完善的自动学习系统。

## 面试问答/讲述要点

**Q1：Hermes Agent 和 Hermes LLM 是一回事吗？**

不是。这里讨论的是 NousResearch 的 `hermes-agent`，依据是其 GitHub 仓库和官方 agent 文档都围绕工具、记忆、上下文、会话、技能等 agent 系统实现展开，而不是单纯介绍模型能力。

**Q2：Hermes 的工具调用机制是什么？**

可以概括为 agent loop 加 tools runtime。Prompt Assembly 先把工具 schema 和上下文组装给模型；模型返回 tool call；runtime 根据工具名和参数执行；结果进入下一轮上下文；模型再决定继续调用工具还是输出最终答复。它的特点是工具按 toolset 分域管理，并支持 MCP 扩展。

**Q3：Hermes 如何处理长期记忆？**

Hermes 区分 session history 和 Memory。session history 保存会话过程，Memory 保存跨会话可检索的长期知识，例如用户偏好、项目约定和稳定事实。Memory 后端被抽象为 provider，但具体 provider 的索引结构、召回算法、去重合并策略等未从公开资料完全确认。

**Q4：为什么 Prompt Assembly 很关键？**

因为 agent 的上下文来源很多，包括系统指令、用户请求、会话历史、工具定义、context files、记忆检索结果和 skills。如果没有统一的 Prompt Assembly 层，这些信息会分散拼接，导致上下文不可控。Hermes 将其独立出来，有利于统一排序、裁剪、压缩和缓存。

**Q5：Skills 和 Memory 有什么区别？**

Memory 更像事实型记忆，保存偏好、项目约定和稳定知识；Skills 更像过程型记忆，保存可复用工作流。这个区分对 coding agent 很重要，因为“怎么做一类任务”和“用户/项目的事实”不应该混在同一堆聊天摘要里。

**Q6：Hermes 的设计亮点是什么？**

亮点是模块边界清楚：工具负责动作，memory 负责长期事实，session storage 负责过程，context files 负责显式背景，skills 负责流程复用，compression/caching 负责长会话成本控制。MCP 也让外部系统扩展有更清晰的边界。

**Q7：Hermes 的风险或未确认点是什么？**

主要风险在工具权限、安全边界、危险命令防护、自动记忆质量和技能自修改机制。公开资料确认了工具和能力管理的大方向，但没有完全披露细粒度安全策略、沙箱边界、自动自改进闭环和评估机制，因此面试中应避免过度断言。

## 参考资料

- NousResearch/hermes-agent GitHub 仓库：https://github.com/NousResearch/hermes-agent
- Hermes Agent 官方文档首页：https://hermes-agent.nousresearch.com/docs/
- Architecture：https://hermes-agent.nousresearch.com/docs/developer-guide/architecture/
- Agent Loop：https://hermes-agent.nousresearch.com/docs/developer-guide/agent-loop/
- Tools Runtime：https://hermes-agent.nousresearch.com/docs/developer-guide/tools-runtime/
- Tools 用户指南：https://hermes-agent.nousresearch.com/docs/user-guide/features/tools/
- MCP Integration：https://hermes-agent.nousresearch.com/docs/user-guide/features/mcp/
- Memory：https://hermes-agent.nousresearch.com/docs/user-guide/features/memory/
- Memory Providers：https://hermes-agent.nousresearch.com/docs/user-guide/features/memory-providers/
- Session Storage：https://hermes-agent.nousresearch.com/docs/developer-guide/session-storage/
- Prompt Assembly：https://hermes-agent.nousresearch.com/docs/developer-guide/prompt-assembly/
- Context Compression and Caching：https://hermes-agent.nousresearch.com/docs/developer-guide/context-compression-and-caching/
- Context Files：https://hermes-agent.nousresearch.com/docs/user-guide/features/context-files/
- Skills：https://hermes-agent.nousresearch.com/docs/user-guide/features/skills/
