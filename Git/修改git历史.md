## 核心原则：Git 历史不可变，只能“重写”
所有修改历史的操作都是**创建新的提交对象并替换旧的**，因此哈希值会改变。若历史已推送，需**强制推送**，并通知所有协作者。

---

## 1. 修改最近一次提交
### 只改提交信息
```bash
git commit --amend -m "新信息"
```
或打开编辑器修改：
```bash
git commit --amend
```

### 修改文件内容（补充遗漏或修正错误）
```bash
# 修改文件后
git add <文件>                 # 也可用 git add -A
git commit --amend --no-edit  # 不动提交信息
```
若要同时改信息，去掉 `--no-edit`。

### 修改作者身份或时间
- **修改作者姓名/邮箱（保留 Author Date）**：
  ```bash
  GIT_AUTHOR_NAME="新名字" GIT_AUTHOR_EMAIL="新邮箱" git commit --amend --no-edit
  ```
- **修改作者日期**：
  ```bash
  git commit --amend --date="2026-06-10 15:30:00 +0800"
  ```
- **全部重置为当前用户和时间**：
  ```bash
  git commit --amend --reset-author
  ```
- **修改提交者（Committer）信息**：
  ```bash
  GIT_COMMITTER_NAME="新名" GIT_COMMITTER_EMAIL="新邮箱" git commit --amend --no-edit
  ```
  若需要同时保留原作者不变，可以结合 `--author` 参数。

---

## 2. 修改历史上多个提交：`git rebase -i`
### 启动交互式变基
```bash
git rebase -i <基准提交>        # 例如 HEAD~5 或某个提交的哈希^
```

### 常用命令列表
| 命令 | 简写 | 作用 |
|------|------|------|
| `pick` | p | 保留该提交（默认） |
| `reword` | r | 保留提交，但修改**提交信息** |
| `edit` | e | 暂停，允许你**修改内容**（文件），也可接着改信息 |
| `squash` | s | 将该提交**合并到前一个提交**，并合并提交信息 |
| `fixup` | f | 合并到前一个提交，但**丢弃该提交的信息** |
| `drop` | d | **删除**该提交及其变更 |
| `break` | b | 在此处暂停（方便调试） |

### 使用 `reword` 修改多个提交信息
在待办清单中把 `pick` 改为 `reword`，保存退出。Git 会依次打开编辑器让你重写信息。

### 使用 `edit` 修改文件内容或拆分提交
1. 标记为 `edit`，保存退出。
2. Git 会在该提交处暂停。此时可修改文件、`git add`，然后执行：
   ```bash
   git commit --amend          # 修改当前提交的内容和/或信息
   # 或继续完成
   git rebase --continue
   ```
3. 如需拆分提交：
   ```bash
   git reset HEAD^             # 回退提交，保留文件在工作区
   # 然后多次 git add + git commit 重新拆成多个提交
   git rebase --continue
   ```

### 使用 `squash` / `fixup` 合并提交
- `squash`：标记后会弹出编辑器让你合并提交信息。
- `fixup`：自动丢弃该提交信息，使用前一个提交的信息。  
  可配合 `git commit --fixup=<commit>` 预先创建 fixup 提交，然后用 `git rebase -i --autosquash` 自动排列。

### 调整提交顺序
在待办清单中**移动行**即可，Git 会按新顺序重放提交。

### 删除提交
将对应行的 `pick` 改为 `drop`，或直接删除那一行。

---

## 3. 修改提交的作者、提交者、时间戳（在变基过程中）

### 在 `edit` 暂停时修改 Author / Committer
- **只修改作者（Author）**：
  ```bash
  GIT_AUTHOR_NAME="名" GIT_AUTHOR_EMAIL="邮箱" git commit --amend --no-edit
  ```
- **只修改提交者（Committer）**：
  ```bash
  GIT_COMMITTER_NAME="名" GIT_COMMITTER_EMAIL="邮箱" git commit --amend --no-edit
  ```
- **修改时间**：
  ```bash
  git commit --amend --date="YYYY-MM-DD HH:MM:SS +TZ"
  # 或同时改提交者日期
  GIT_COMMITTER_DATE="YYYY-MM-DD HH:MM:SS +TZ" git commit --amend --no-edit
  ```
  注意：只加 `--date` 修改的是 **Author Date**；`GIT_COMMITTER_DATE` 修改 **Commit Date**。

### 批量修改所有提交的身份（使用 `--exec`）
```bash
git rebase -i --exec 'GIT_COMMITTER_NAME="新名" GIT_COMMITTER_EMAIL="新邮箱" git commit --amend --no-edit -m "new msg" --author="..."'
```
更可靠的方式是使用 `git filter-branch` 或 `git filter-repo`（见后）。

---

## 4. 历史重写工具（大规模修改）

### `git filter-branch`（已不推荐，但很多参考资料仍提及）
```bash
# 修改所有提交的作者/提交者
git filter-branch --env-filter '
  export GIT_AUTHOR_NAME="新名"
  export GIT_AUTHOR_EMAIL="新邮箱"
  export GIT_COMMITTER_NAME="新名"
  export GIT_COMMITTER_EMAIL="新邮箱"
' --tag-name-filter cat -- --all
```

### `git filter-repo`（官方推荐，更安全快速）
```bash
# 安装：pip install git-filter-repo
# 修改身份
git filter-repo --commit-callback '
  commit.author_name = b"新名"
  commit.author_email = b"新邮箱"
  commit.committer_name = b"新名"
  commit.committer_email = b"新邮箱"
'
```

---

## 5. 恢复误操作的历史

### 变基过程中放弃
```bash
git rebase --abort
```

### 变基完成后想回到之前的状态
使用 `git reflog` 找到变基前的 HEAD，然后：
```bash
git reset --hard <变基前HEAD哈希>
```

### 撤销已经推送的重写历史
唯一的方法是**重写回旧历史**（再用 `reflog` 找到旧提交，reset 后强制推送）。

---

## 6. 推送重写后的历史

任何重写历史的操作后，若分支已存在于远程，需使用**强制推送**：

```bash
git push --force-with-lease
```

**最佳实践**：
- 推送前先 `git fetch` 更新远程跟踪分支，让保护生效。
- 如果是为了合并别人的提交，先确保自己已包含远程最新，或与团队沟通。
- 如果分支是受保护的（如 GitHub 上的 `main`），可能需要临时解除保护。

---

## 7. 重要提醒

- **修改历史会改变提交哈希**，导致所有协作者必须强制同步（他们需要执行 `git fetch` + `git rebase origin/分支` 或 `git reset --hard`）。
- **永远不要修改已经公开的稳定分支（如 main/master）的历史**，除非是紧急情况（如泄露密码）并且与团队协调完成。
- 使用 `--force-with-lease` 代替 `--force`，能防止覆盖他人的新提交。
- 如果历史中曾存在敏感文件，仅删除文件是不够的，必须使用 `filter-branch` 或 `filter-repo` 彻底清除历史，并更换泄露的密钥。

---

## 速查表：何时用什么？

| 需求 | 命令/操作 |
|------|-----------|
| 改最近一次提交的信息 | `git commit --amend` |
| 改最近一次提交的内容 | 修改文件后 `git add` + `git commit --amend` |
| 改最近几次的提交信息 | `git rebase -i HEAD~n` + `reword` |
| 修改历史某个提交的内容 | `git rebase -i <父提交>` + `edit` |
| 合并多个提交 | `rebase -i` + `squash` / `fixup` |
| 删除某个提交 | `rebase -i` + `drop` 或删除行 |
| 调整提交顺序 | `rebase -i` 中移动行 |
| 批量修改作者/提交者 | `git filter-repo`（或 `filter-branch`） |
| 修改 Author Date | `git commit --amend --date="..."` 或在 `edit` 后执行 |
| 修改 Committer Date | 设置 `GIT_COMMITTER_DATE` 环境变量 |
| 强制推送但保护他人工作 | `git push --force-with-lease` |
| 恢复变基前的状态 | `git reflog` + `git reset --hard <HASH>` |
