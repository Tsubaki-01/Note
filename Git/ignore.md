## Git 个人配置方法

### 1. **本地排除文件** `.git/info/exclude`

```bash
# 位置：<repo>/.git/info/exclude
# 作用：忽略文件，仅对当前仓库生效，不会被提交
.codestable/
my-notes.md
```

**适用场景**：个人临时文件、笔记、测试数据等

### 2. **全局 gitignore** `~/.gitignore_global`

```bash
# 配置全局忽略文件
git config --global core.excludesfile ~/.gitignore_global

# 在 ~/.gitignore_global 中添加
.DS_Store
*.swp
.idea/
*-local.*
```

**适用场景**：跨所有项目的个人习惯文件（编辑器临时文件、系统文件等）

### 3. **本地 Git 配置** `.git/config`

```bash
# 设置仅对当前仓库生效的配置
git config --local user.email "personal@example.com"
git config --local core.editor "vim"
git config --local alias.st "status"
```

**适用场景**：个人 Git 行为定制（别名、编辑器、用户信息等）

### 4. **环境变量覆盖**

```bash
# 在 ~/.zshrc 或项目特定的 .envrc 中
export SOME_API_KEY="your-key"
export DEBUG=true
```

**适用场景**：个人开发环境配置，配合 `.env` 文件（已在 .gitignore 中）

### 5. **Git Hooks 本地化**

```bash
# .git/hooks/ 目录下的钩子脚本不会被提交
# 创建个人的 pre-commit hook
.git/hooks/pre-commit
```

**适用场景**：个人开发流程自动化（提交前检查、自动格式化等）

---

**优先级对比**：

- `.git/info/exclude` > `.gitignore` > `~/.gitignore_global`
- 本地配置 `--local` > 全局配置 `--global` > 系统配置 `--system`

**最佳实践**：

- 项目共享的忽略规则 → `.gitignore`
- 个人在当前项目的忽略 → `.git/info/exclude`
- 个人跨项目的忽略 → `~/.gitignore_global`