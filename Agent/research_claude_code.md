# Claude Code Agent 系统调研报告

## 摘要

Claude Code 是 Anthropic 面向开发者终端的 agentic coding 工具。它更接近一个在本地项目目录中工作的命令行 agent，而不是 IDE 中的代码补全器：它可以读取代码、维护任务状态、调用工具执行 shell 命令或编辑文件，并通过权限系统控制高风险操作。

从公开资料看，Claude Code 的外部可见机制可以拆成四层：

- **Agent 循环**：模型根据用户目标和当前上下文规划下一步，选择工具，观察工具结果，并继续迭代。
- **工具与权限层**：内置文件读写、搜索、Bash、Web、Todo、Notebook 等工具，并用 allow/deny/ask 风格的权限配置、命令行参数、目录信任等机制降低误操作风险。
- **项目记忆层**：通过 `CLAUDE.md`、用户级 memory、项目级 memory、`/memory`、`#` 快捷记忆等方式，把偏好、架构说明、常用命令持久化为显式文本。
- **上下文管理层**：通过 `@` 文件引用、按需读取、任务列表、subagent 隔离上下文、自动/手动 compact、resume/continue 会话恢复来支撑长任务。

需要明确边界：Claude Code 的核心 agent prompt、工具选择策略、压缩摘要算法、内部状态格式、模型侧 planner 的具体实现，未从公开资料确认。公开资料足以确认外部机制和用户可配置边界，但不足以反推完整内部实现。

## 核心结论

1. **Claude Code 的关键不是“会写代码”，而是“能在受控权限下执行开发动作”**。它把搜索、编辑、测试、提交等动作放到工具层，通过权限系统决定哪些动作可以真正执行。
2. **记忆系统以显式 Markdown 为主**。`CLAUDE.md` 和不同作用域的 memory 让团队可以审查、版本化和共享项目知识；公开资料不支持断言它依赖黑盒向量长期记忆。
3. **上下文管理是组合拳**。Claude Code 不只是依赖更大的上下文窗口，而是结合文件引用、搜索后读取、Todo、subagent、compact 和 resume 管理长任务。
4. **Hooks 是工程化亮点**。它把 formatter、lint、安全阻断、审计日志等确定性流程放到工具调用生命周期中，而不是完全依赖模型自觉。
5. **MCP 体现平台化工具生态**。Claude Code 通过 Model Context Protocol 接入外部服务和本地工具，降低核心系统复杂度，但工具质量、权限和认证边界仍需要治理。
6. **不能把外部机制误讲成内部实现**。面试或学习时可以讲清外部设计，但不应声称知道内部 prompt、压缩算法、工具选择模型或会话 schema。

## 工具调用

### 工具族与执行层

Claude Code 的工具调用围绕“让模型在本地开发环境中完成可验证动作”设计。公开文档和命令帮助中可见的工具/能力包括：

- 文件与代码库工具：读取文件、列目录、搜索文件、搜索文本、编辑文件、写文件。
- 终端工具：通过 Bash 执行命令，常用于运行测试、lint、构建、git 命令等。
- 任务管理工具：`TodoWrite` 一类工具用于在长任务中维护 checklist。
- Web 工具：在需要时搜索或抓取网页资料。
- Notebook 工具：面向 notebook 文件的读写。
- Subagent / Task 工具：把独立任务派发给独立上下文的 subagent。
- MCP 工具：通过 Model Context Protocol 接入外部服务和本地工具。

官方文档的 Claude Code overview 和 Interactive mode 都强调，它可以理解代码库、编辑文件、运行命令、修复测试、处理 git 工作流。这说明工具调用不是附属能力，而是 Claude Code agent 工作流的执行层。

### 权限模型

Claude Code 的工具调用不是默认无限制执行。公开资料中可确认的权限机制包括：

- **工具权限配置**：通过 `settings.json` 的 `permissions.allow`、`permissions.ask`、`permissions.deny` 控制允许、询问或拒绝的工具/命令模式；CLI 也提供 `--allowedTools`、`--disallowedTools` 等临时覆盖参数。
- **权限规则优先级**：设置可来自企业策略、命令行参数、本地项目设置、共享项目设置、用户设置等多个来源；更高优先级配置覆盖低优先级配置。
- **目录信任**：Claude Code 会区分是否信任当前工作目录，以降低在不可信项目中自动执行危险操作的风险。
- **交互式确认**：对文件修改、命令执行、网络访问等操作，Claude Code 可要求用户确认，也可通过配置或 flag 进入更自动化的模式。

这个设计的核心取舍是：Claude Code 把“模型能做什么”和“模型被允许做什么”分离。模型可以提出调用 Bash 或编辑文件，但权限层决定是否拦截、询问或拒绝。这比只依赖 prompt 更稳健，因为权限是模型外部的控制面。

### Bash 与命令执行

Bash 是 Claude Code 的重要工具，带来两个直接效果：

- **优点**：可以复用项目已有脚本、测试、lint、构建、git 工作流，不需要为每种开发动作设计专用 API。
- **风险**：shell 是高权限、强副作用接口，因此必须配合权限规则、目录信任、命令模式限制和用户确认。

公开资料确认了 Bash 工具、权限配置以及 Hooks 可监听工具调用，但没有公开确认 Claude Code 内部如何对任意 shell 命令做语义级风险分类。若存在更细粒度的内部风险评分，未从公开资料确认。

### Hooks：工具调用生命周期扩展点

Claude Code 提供 Hooks，用确定性的本地命令挂接 agent 生命周期。公开文档列出的事件包括：

- `PreToolUse`：工具调用前触发，可用于阻止或校验某些操作。
- `PostToolUse`：工具调用后触发，可用于格式化、记录、运行检查。
- `UserPromptSubmit`：用户 prompt 提交后触发。
- `Notification`、`Stop`、`SubagentStop`、`PreCompact` 等事件。

Hooks 的设计亮点，是把不适合交给模型自由发挥的工程约束外置为确定性脚本。例如：每次编辑后自动运行 formatter；在 Bash 执行前拒绝 `rm -rf`；compact 前保存上下文快照。这类控制点适合团队把本地规范制度化。

### MCP：外部工具与上下文接入

Claude Code 支持 Model Context Protocol，可连接本地或远程 MCP server，把外部系统暴露为工具或资源。公开文档显示，Claude Code 可管理 MCP server，并把 MCP 能力接入 Claude Code 会话。

工程取舍：

- MCP 把工具生态从 Claude Code 核心中解耦，避免把每个外部系统都内置进主程序。
- MCP server 边界让权限、认证、审计可以在外部服务侧实现。
- 代价是工具质量、延迟、错误处理会受外部 server 影响；Claude Code 核心无法完全控制第三方工具语义。

### Subagents：独立上下文中的任务工具

Claude Code 支持自定义 subagent。公开文档说明 subagent 具有：

- 独立的上下文窗口。
- 可定制 system prompt。
- 可限制可用工具。
- 可按任务类型自动或显式调用。

这说明 Claude Code 的 Task/subagent 不是简单函数调用，而是把一部分任务放进隔离上下文执行，再把结果回传主会话。其收益是降低主上下文污染，适合代码搜索、专项审查、文档调研等边界明确的独立任务。内部调度策略和结果压缩方式未从公开资料确认。

## 记忆系统

### `CLAUDE.md`：显式项目记忆

Claude Code 的主要公开记忆机制是 `CLAUDE.md`。官方文档建议把以下内容写入 `CLAUDE.md`：

- 常用 bash 命令。
- 代码风格指南。
- 测试说明。
- 仓库结构说明。
- 项目约定和开发工作流。

从实现方式看，这是一种“文本即记忆”的设计，而不是黑盒向量记忆。它的优势是可审查、可版本化、可团队共享；缺点是需要人工维护，且内容过长会占用上下文预算。

### 多层 memory 范围

公开文档显示 Claude Code 支持不同作用域的 memory，包括项目级、用户级、本地项目级等形式。典型用法是：

- **项目级 memory**：放在仓库中，团队共享。
- **用户级 memory**：放在用户目录中，表达个人偏好。
- **本地项目 memory**：表达不适合提交到仓库的本机偏好或临时说明。

这种分层的好处是：团队共识进入项目 memory，个人习惯进入用户 memory，避免把个人偏好污染仓库。代价是多层 memory 可能产生冲突。公开资料没有确认 Claude Code 是否有复杂冲突解析；可确认的是它会按文档定义加载不同来源。

### `/memory` 与 `#` 快捷记忆

Claude Code 提供 `/memory` 命令查看或编辑 memory；交互中也支持用 `#` 快捷地把信息加入 memory。

这个设计降低了“把隐性偏好固化为显性上下文”的摩擦。用户不需要打开文件手动编辑，也能把“以后都这样做”转成长期可见文本。

### Import 机制

`CLAUDE.md` 支持通过 `@path/to/import` 引入额外文件。工程视角下，这允许团队把大项目上下文拆分成多个文档，例如：

- 根目录 `CLAUDE.md` 放全局规则。
- 子模块文档放模块约定。
- 共享命令文档放测试/部署命令。

但导入规模需要控制。公开资料没有确认 Claude Code 对 memory import 的内部裁剪算法；可确认的是 import 会把更多文本纳入可用上下文，过度使用会消耗上下文预算。

### 未公开或未确认部分

以下内容未从公开资料确认：

- Claude Code 是否使用向量数据库保存长期记忆。
- Claude Code 是否对用户偏好做自动 embedding 检索。
- Claude Code 是否把历史会话自动抽取成长期偏好。
- Claude Code 内部如何决定 memory 与当前任务的优先级。

公开资料可确认的主路径仍是显式 Markdown memory，而不是不可见的自动长期记忆系统。官方 settings 文档还明确说明：不同于 claude.ai，Claude Code 的内部 system prompt 不在网站上公开；用户应通过 `CLAUDE.md` 或 `--append-system-prompt` 添加自定义指令。

## 上下文管理

### 文件引用与按需加载

Claude Code 支持用 `@` 引用文件或目录，把相关上下文加入对话。这种方式避免启动时一次性塞入整个仓库。用户或 agent 可以先搜索，再选择性读取关键文件。

工程上，这是大型代码库 agent 的必要取舍：上下文窗口再大，也不适合无差别承载完整仓库。

### Todo 与中间状态

Claude Code 会在复杂任务中维护 todo/checklist。公开资料和工具说明显示，`TodoWrite` 用于计划和跟踪步骤。其工程收益包括：

- 把长任务拆成可验证中间目标。
- 在上下文压缩或会话恢复后保留任务骨架。
- 让用户看到 agent 当前意图，减少黑盒感。

Todo 的内部存储位置、是否进入会话历史或压缩摘要，未从公开资料确认。

### 自动与手动压缩

Claude Code 支持上下文压缩。公开命令中有 `/compact`，用于压缩当前会话上下文；Hooks 中存在 `PreCompact` 事件，说明压缩是 Claude Code agent 生命周期中的一等事件。

公开资料可确认：

- 用户可以手动触发 compact。
- Claude Code 在接近上下文限制时会进行上下文管理。
- Hooks 可在 compact 前触发。

以下内容未从公开资料确认：

- compact 摘要 prompt 的具体内容。
- compact 保留或丢弃信息的算法。
- compact 后工具观察、文件 diff、todo、用户约束分别如何排序。
- 是否存在可配置的压缩策略。

工程取舍也很清楚：压缩让长任务不因上下文窗口耗尽而中断，但压缩必然损失细节。因此关键约束应写入 `CLAUDE.md`、todo、文件或显式计划，而不是只留在历史聊天中。`PreCompact` Hook 则提供了一个实用扩展点，可在压缩前把重要状态外写或注入摘要提醒。

### 会话恢复

Claude Code 支持恢复历史会话。CLI 文档中有 `--continue`、`--resume` 等参数；交互命令中也有 `/resume`。

公开资料可确认：

- 可以继续最近会话。
- 可以选择并恢复历史会话。
- 会话恢复是 CLI/交互模式的正式能力。

以下内容未从公开资料确认：

- 会话历史在本地磁盘上的确切 schema。
- 恢复时是完整回放历史，还是加载压缩摘要和关键状态。
- 会话恢复和 `CLAUDE.md` memory 的合并顺序。

### Subagent 上下文隔离

Subagents 官方文档明确提到独立上下文窗口。这是一项重要的上下文管理手段：主 agent 不必把所有搜索过程、失败路径、子任务推理都塞进主上下文；它可以派发任务，只接收结果。

代价是子 agent 可能缺少主会话中的隐性细节，因此更适合边界明确的任务。

## 亮点与可学习设计

### 1. 显式文本记忆优先

Claude Code 没有把公开记忆机制设计成不可见黑盒，而是把项目规则、偏好、命令固化为 `CLAUDE.md`。这对工程团队很重要：

- 可 code review。
- 可随仓库演进。
- 可让新人和 agent 共享同一份项目知识。
- 出错时可以直接修改源文本，而不是调试不可见记忆。

可学习点：对开发 agent 而言，先做可审计记忆，再考虑自动检索记忆。

### 2. 权限系统独立于模型能力

Claude Code 允许模型请求执行强力工具，但用权限、配置、目录信任、确认流程控制实际执行。这比“告诉模型不要做危险事”更工程化。

可学习点：agent 系统应把 policy enforcement 放在模型外部，尤其是 shell、文件写入、网络、凭据访问这类高风险能力。

### 3. Hooks 提供确定性约束

Hooks 让团队把 formatter、lint、审计、阻断规则接到工具生命周期上。它的价值不是“让模型更聪明”，而是把确定性工程流程放回确定性代码中。

可学习点：agent 平台需要 lifecycle hooks，而不是只给一个 prompt 输入框。

### 4. MCP 解耦工具生态

MCP 让 Claude Code 不必内置所有外部系统。工具通过协议接入，Claude Code 负责调用和上下文编排。

可学习点：工具协议化可以降低核心系统复杂度，但要配套权限、认证和错误边界。

### 5. Subagent 隔离上下文

子 agent 的独立上下文窗口是长任务和复杂代码库中的实用设计。它解决的不是并发本身，而是上下文污染和注意力竞争。

可学习点：把“探索型工作”隔离出去，只把结论汇入主上下文，有助于控制 token 成本和主线任务清晰度。

### 6. Compact 作为正式生命周期事件

`/compact` 和 `PreCompact` 表明上下文压缩不是临时补丁，而是被纳入 agent 生命周期。

可学习点：长上下文 agent 必须承认压缩会发生，并提供用户可控和程序可控的入口。

## 面试问答 / 讲述要点

### Q1：Claude Code 和普通代码补全工具有什么区别？

可以这样讲：

> Claude Code 更像终端里的开发 agent，而不是单纯补全器。它不只是返回代码片段，还会读取项目、搜索文件、编辑代码、运行测试、处理 git 工作流。它的核心是“模型规划 + 工具执行 + 权限控制”的循环。

讲述重点：不要只说“更智能”，要强调它能在本地开发环境中执行可验证动作。

### Q2：Claude Code 的工具调用机制怎么理解？

可以概括为：

> 模型决定下一步要调用什么工具，比如 Read、Search、Edit、Bash；工具层执行具体动作；权限系统决定这个动作是允许、询问还是拒绝。

讲述重点：工具能力和权限控制是分层的。这个分层降低了 agent 误操作风险。

### Q3：Claude Code 的记忆系统有什么值得学？

可以这样回答：

> 它最值得学的是透明性。公开资料显示，Claude Code 主要通过 `CLAUDE.md` 和不同作用域的 memory 保存项目规则、命令、偏好。这些内容是显式文本，可以 review、版本化、回滚，也能被团队共享。

需要保留边界：公开资料不支持断言 Claude Code 使用向量数据库或自动 embedding 检索长期记忆。

### Q4：它如何管理长上下文任务？

可以从组合机制讲：

> Claude Code 不是简单依赖大上下文窗口。它通过 `@` 文件引用、搜索后按需读取、Todo、subagent 隔离、compact 压缩、resume 会话恢复来管理长任务。

讲述重点：大型代码库 agent 的关键不是“把所有文件塞进 prompt”，而是持续选择、压缩和恢复关键上下文。

### Q5：Hooks 的价值是什么？

可以这样讲：

> Hooks 把确定性工程约束放到 agent 生命周期里。例如工具调用前做安全阻断，工具调用后跑 formatter 或 lint，compact 前保存关键状态。这样可以减少对模型自觉性的依赖。

讲述重点：Hooks 不是让模型更聪明，而是让工程流程更可控。

### Q6：MCP 在 Claude Code 中扮演什么角色？

可以这样回答：

> MCP 是外部工具和上下文接入协议。Claude Code 不必把所有系统都内置进核心，而是通过 MCP server 接入外部能力。这样有利于扩展生态，但也要求处理好权限、认证、审计和错误边界。

### Q7：哪些部分不能乱讲？

可以明确说：

> 公开资料不能确认 Claude Code 的内部 agent prompt、工具选择策略、压缩摘要算法、内部状态格式和会话恢复 schema。因此只能讲外部可见机制和公开文档确认的行为，不能把推测说成事实。

这类回答能体现技术严谨性。

## 参考资料

- [Claude Code overview - Anthropic Docs](https://docs.anthropic.com/en/docs/claude-code/overview)
- [Claude Code interactive mode - Anthropic Docs](https://docs.anthropic.com/en/docs/claude-code/interactive-mode)
- [Claude Code CLI reference - Anthropic Docs](https://docs.anthropic.com/en/docs/claude-code/cli-reference)
- [Claude Code settings - Anthropic Docs](https://docs.anthropic.com/en/docs/claude-code/settings)
- [Claude Code memory - Anthropic Docs](https://docs.anthropic.com/en/docs/claude-code/memory)
- [Claude Code slash commands - Anthropic Docs](https://docs.anthropic.com/en/docs/claude-code/slash-commands)
- [Claude Code hooks - Anthropic Docs](https://docs.anthropic.com/en/docs/claude-code/hooks)
- [Claude Code MCP - Anthropic Docs](https://docs.anthropic.com/en/docs/claude-code/mcp)
- [Claude Code subagents - Anthropic Docs](https://docs.anthropic.com/en/docs/claude-code/sub-agents)
- [Claude Code security - Anthropic Docs](https://docs.anthropic.com/en/docs/claude-code/security)
- [Claude Code common workflows - Anthropic Docs](https://docs.anthropic.com/en/docs/claude-code/common-workflows)
- [anthropics/claude-code - GitHub](https://github.com/anthropics/claude-code)
