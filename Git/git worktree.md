# Git Worktree 使用指南

## 核心概念

Git Worktree 允许你从同一个仓库创建多个工作目录，每个目录可以检出不同的分支，共享同一个 `.git` 目录。

**使用场景**：
- 同时开发多个功能分支，避免频繁 `git stash` 和切换
- 快速切换去修复紧急 bug，不影响当前开发进度
- 并行进行代码审查和开发工作

## 常用命令

### 1. 创建新 worktree
```bash
# 基本用法：创建新分支并关联到新目录
git worktree add ../peanut-cut-bugfix bugfix/issue-123

# 基于现有分支创建
git worktree add ../peanut-cut-feature -b feature/new-tool origin/main

# 临时 worktree（不创建分支）
git worktree add ../peanut-cut-temp --detach HEAD
```

### 2. 查看所有 worktree
```bash
git worktree list

# 输出示例：
# /Users/bilibili/peanut-cut              abc123 [main]
# /Users/bilibili/peanut-cut-bugfix       def456 [bugfix/issue-123]
# /Users/bilibili/peanut-cut-feature      789ghi [feature/new-tool]
```

### 3. 删除 worktree
```bash
# 先删除目录（或在目录外）
rm -rf ../peanut-cut-bugfix

# 清理 Git 记录
git worktree prune

# 或者一步到位
git worktree remove ../peanut-cut-bugfix

# 强制删除（即使有未提交的修改）
git worktree remove ../peanut-cut-bugfix --force
```

### 4. 移动 worktree
```bash
git worktree move ../peanut-cut-bugfix ../new-location
```

### 5. 锁定/解锁 worktree
```bash
# 锁定（防止被自动清理）
git worktree lock ../peanut-cut-feature --reason "长期开发分支"

# 解锁
git worktree unlock ../peanut-cut-feature
```

## 实际工作流示例

```bash
# 场景：正在主目录开发功能 A，突然需要修复紧急 bug

# 1. 当前在 /Users/bilibili/peanut-cut，正在开发 feature-a 分支
cd /Users/bilibili/peanut-cut

# 2. 创建新 worktree 处理 bug
git worktree add ../peanut-cut-hotfix -b hotfix/urgent-bug main

# 3. 切换到新目录修复 bug
cd ../peanut-cut-hotfix
# ... 修复 bug，提交代码
git add .
git commit -m "fix: urgent bug"
git push origin hotfix/urgent-bug

# 4. 回到原来的工作目录继续开发
cd /Users/bilibili/peanut-cut
# feature-a 分支和未提交的修改都还在，无需 stash

# 5. bug 修完后，清理 worktree
git worktree remove ../peanut-cut-hotfix
```

## 注意事项

1. **同一分支不能在多个 worktree 中同时检出**
   ```bash
   # 会报错
   git worktree add ../another main  # main 已在主目录检出
   ```

2. **共享 Git 对象和引用**
   - 所有 worktree 共享 `.git` 目录
   - 在任何 worktree 中 fetch/pull，其他 worktree 都能看到
   - 提交、分支创建等操作全局可见

3. **`.git` 文件替代目录**
   - worktree 目录下的 `.git` 是一个文件（不是目录）
   - 指向主仓库的 `.git/worktrees/<name>`

4. **清理孤立 worktree**
   ```bash
   # 手动删除了目录但忘记用 git worktree remove
   git worktree prune  # 清理过期记录
   ```

## 与 Claude Code 的结合

在项目中，可以这样用：
```bash
# 实验性开发不想影响主分支
git worktree add ../project-experiment -b experiment/new-idea main

# 用 Claude Code 在新目录中工作
cd ../project-experiment
# Claude Code 会识别这是一个 worktree，共享主仓库的 Git 历史
```