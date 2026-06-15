这里是 **Git 子模块的常用命令速查**，按使用场景分类。基本格式假设主仓库是公开项目，子模块是私有仓库，目录名为 `private_dir`。

---

## 1. 添加子模块
```bash
# 添加一个子模块（默认跟踪主分支的最新提交）
git submodule add <仓库URL> <路径>
```
**例子**：
```bash
git submodule add https://github.com/user/private-repo.git private_dir
```
添加后会在根目录生成 `.gitmodules` 文件，同时新增一个子模块提交记录。

---

## 2. 克隆包含子模块的项目
普通 `git clone` 不会自动拉取子模块内容，需要额外操作。
```bash
# 方法一：克隆时直接递归拉取子模块
git clone --recurse-submodules <主仓库URL>

# 方法二：先克隆主仓库，再初始化并拉取
git clone <主仓库URL>
cd <主仓库>
git submodule update --init --recursive
```
如果只需要拉取部分子模块，可用 `git submodule init <路径>` 然后再 `update`。

---

## 3. 更新子模块
### 拉取子模块远程的最新提交
```bash
# 进入子模块目录拉取最新，然后回到主仓库提交指针
cd private_dir
git pull origin main
cd ..
git add private_dir
git commit -m "更新子模块到最新"
```

### 将子模块更新到主仓库记录的版本
```bash
git submodule update
```
这会把子模块重置到主仓库 `.gitmodules` 中记录的提交（即 `git submodule status` 显示的哈希）。

### 更新到远程分支的最新提交（而不手动进入子模块）
```bash
git submodule update --remote
```
默认跟踪子模块仓库的 `main` 或 `master` 分支（可在 `.gitmodules` 中配置 `branch = xxx`）。

---

## 4. 查看子模块状态与信息
```bash
# 查看所有子模块当前哈希、是否有未推送提交等
git submodule status

# 查看子模块日志
git diff --submodule

# 查看子模块的远程 URL
git submodule foreach git remote -v
```

---

## 5. 进入子模块直接操作
```bash
cd private_dir
# 此时你在一个独立的 Git 仓库内，可以做任何 Git 操作
git checkout -b feature-branch
git add .
git commit -m "修改私有文件"
git push origin feature-branch
```
回到主仓库后，需要 `git add private_dir` 更新子模块指针。

---

## 6. 配置子模块跟踪的远程分支
编辑 `.gitmodules` 添加 `branch` 字段，或直接命令：
```bash
git config -f .gitmodules submodule.private_dir.branch main
```
这样 `git submodule update --remote` 就会拉取 `main` 分支的最新提交。

---

## 7. 修改子模块的远程 URL
有时仓库地址变了，需要更新 URL。
```bash
# 方法一：直接编辑 .gitmodules 然后同步
git config -f .gitmodules submodule.private_dir.url <新URL>
git submodule sync
# 方法二：用 set-url（会同时改配置和子模块实际的 origin）
git submodule set-url <路径> <新URL>
```
更新后执行 `git submodule sync` 确保本地配置一致。

---

## 8. 移动或重命名子模块
```bash
git mv <旧路径> <新路径>
```
Git 会自动更新 `.gitmodules` 和子模块在工作区的位置。

---

## 9. 删除子模块
现代 Git（≥1.8.5）推荐：
```bash
# 从工作区和暂存区移除，并删除 .gitmodules 中的条目
git submodule deinit -f <路径>
# 从暂存区中删除（实际上就是删除目录的跟踪）
git rm -f <路径>
# 此时可以手动删除 .git/modules/<路径> 中的内部仓库数据（可选）
rm -rf .git/modules/<路径>
```
或者一步到位：
```bash
git rm <路径>
git commit -m "删除子模块"
```
`git rm` 会自动删除目录和 `.gitmodules` 记录，但可能留下 `.git/modules` 里的缓存，可手动清理。

---

## 10. 递归操作（嵌套子模块）
如果子模块内还有子模块：
```bash
# 递归添加子模块
git submodule add --recursive <URL> <路径>

# 递归拉取所有嵌套子模块
git submodule update --init --recursive

# 递归更新到远程分支
git submodule update --remote --recursive

# 对所有子模块（及其子子模块）执行任意命令
git submodule foreach --recursive 'git status'
```

---

## 11. 主仓库切换分支时子模块的处理
如果主仓库切换到旧分支，子模块记录可能变化，需要重新对齐：
```bash
git checkout <其他分支>
git submodule update --recursive
```
为了避免手动执行，可以设置自动更新：
```bash
git config --global submodule.recurse true
```

---

## 12. 查看子模块的差异
```bash
# 查看主仓库中子模块指针的具体变化
git diff --cached --submodule
# 或者直接看子模块内部的详细 diff
git diff --submodule=diff
```

---

## 13. 临时只读子模块？
通常拉取的子模块会处于 detached HEAD 状态，是因为它指向一个具体的 commit。如果想在分支上工作，需要进入子模块目录后手动 `git checkout` 到分支名，然后修改、推送。

---

## 14. 一些别名建议
在 `.gitconfig` 中设置快捷命令：
```ini
[alias]
    # 更新所有子模块到最新远程提交
    subup = submodule update --remote --recursive
    # 初始化并递归拉取
    subinit = submodule update --init --recursive
```

---

## 常见工作流示例
**初次设置其他设备**：
```bash
git clone --recurse-submodules <公开仓库URL>
```

**日常更新私有文件**：
```bash
cd private_dir
git pull origin main
cd ..
git add private_dir
git commit -m "更新私有配置"
git push
```

**换了一台新电脑，私有文件怎么同步**：
```bash
git clone <公开仓库>
cd public-project
git submodule update --init --recursive   # 输入私有仓库凭证
```

如果需要记住的只有两条核心命令：`git submodule add`（添加）和 `git submodule update --init --recursive`（拉取），其他按需查阅即可。