## 1. 查看提交列表 — `git log`

### 基础输出
```bash
git log                      # 详细列表（提交哈希、作者、日期、信息）
git log --oneline            # 一行一个提交（哈希 + 标题）
git log --graph --oneline    # 带 ASCII 分支图
```

### 按提交者/作者/时间过滤
```bash
git log --author="Alice"      # 按作者筛选
git log --committer="Bob"     # 按提交者筛选
git log --since="2026-06-01" --until="2026-06-15"
git log --after="1 week ago"
```

### 按文件/内容过滤
```bash
git log -- <file>                    # 查看某个文件的历史
git log -p <file>                    # 查看该文件每次改了什么（含 diff）
git log --follow <file>              # 追踪文件重命名之前的历史
git log -S "关键字"                  # 搜索增加/删除了该字符串的提交（pickaxe）
git log -G "正则"                    # 搜索 diff 里匹配正则的变更
git log -L :函数名:文件              # 查看某一函数范围的变化历史
```

### 按合并/分支过滤
```bash
git log --merges            # 只看合并提交
git log --no-merges         # 排除合并提交
git log branchA ^branchB    # 在 branchA 中但不在 branchB 中的提交
```

### 控制输出格式
```bash
git log --format=fuller    # 同时显示 Author 和 Committer 信息
git log --format=raw       # 最完整的元数据（含树哈希、父哈希等）
git log --pretty=format:"%h %an %ad : %s" --date=short  # 自定义格式
```

**常用占位符**：
- `%H` 完整哈希，`%h` 短哈希
- `%an` 作者名，`%ae` 作者邮箱，`%ad` 作者日期
- `%cn` 提交者名，`%ce` 提交者邮箱，`%cd` 提交者日期
- `%aI` / `%cI` 符合 ISO 8601 的作者/提交者日期
- `%s` 提交标题，`%b` 提交正文

**比如只看作者日期和提交者日期的区别**：
```bash
git log --format="%h  authored: %ad  committed: %cd" --date=iso
```

---

## 2. 查看单个提交详情 — `git show`

```bash
git show <commit-hash>                 # 查看该提交的元数据和完整 diff
git show --stat <commit-hash>          # 只看修改了哪些文件及行数统计
git show --format=fuller <commit-hash> # 同时显示 Author 和 Committer 信息（不展示 diff 加 -s）
```

---

## 3. 查看文件/代码的每一行是谁改的 — `git blame`

```bash
git blame <file>                   # 显示每一行的最后修改者、提交哈希和时间
git blame -L 10,20 <file>          # 只看第10到20行
git blame <commit> <file>          # 从指定提交开始查看（更早的变更会被忽略）
git blame -w -C -C -C <file>       # 忽略空白变更，并深度追踪代码移动/拷贝
```

---

## 4. 查看引用日志（操作历史） — `git reflog`

```bash
git reflog                          # 查看 HEAD 的变迁（所有 checkout、reset、rebase 等）
git reflog show <branch>           # 查看特定分支的历史引用变更
```

> 即使历史被 rebase 或 reset，`reflog` 仍可看到之前的提交（只要垃圾回收未执行）。

---

## 5. 查看差异 — `git diff`

```bash
git diff                           # 工作区 vs 暂存区
git diff --staged                  # 暂存区 vs 最新提交
git diff <commit1>..<commit2>      # 两个提交之间的差异
git diff <branch1>..<branch2>      # 两个分支顶端差异
git diff --stat                    # 只显示文件变更统计
git diff --name-only               # 只显示变更文件名
git diff --submodule               # 显示子模块的具体变更（而不只是哈希指针）
```

---

## 6. 查看子模块的历史

```bash
git log --submodule                # 显示子模块指向的变更（类似 git log 但会展示子模块内部提交）
git diff --submodule               # 查看工作区子模块变更的详情
git submodule summary              # 汇总子模块的更新状态
```

---

## 7. 查看分支/标签的整体历史图

```bash
git log --oneline --graph --all --decorate   # 全览所有分支的提交图
gitk --all                                   # 图形化工具（如果装了）
git log --oneline --graph --date-order       # 按提交者日期排序
git log --oneline --graph --author-date-order # 按作者日期排序
```

---

## 快速参考表

| 想了解什么 | 命令 |
|-----------|------|
| 提交列表（含作者和提交者日期） | `git log --format=fuller` |
| 某个文件被谁改过每一行 | `git blame <file>` |
| 某个提交具体改了什么 | `git show <commit>` |
| 某个函数的历史 | `git log -L :func:file` |
| 代码中某段文字什么时候被添加/删除 | `git log -S "text"` |
| 操作日志（找回误删提交） | `git reflog` |
| 暂存区和工作区差异 | `git diff` 和 `git diff --staged` |
| 子模块指针变化历史 | `git log --submodule` |
