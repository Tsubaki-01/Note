# Git学习笔记

分布式版本控制系统

## 初始化设置

`git config [缺省(--local)/--global/--system] user.name "your name"`	配置用户名

`git config [缺省(--local)/--global/--system] user.email "mail@example.com"`	配置邮箱

`git config [缺省(--local)/--global/--system] credential.helper store`	存储配置

`git config [缺省(--local)/--global/--system] --list`

`省略（(Local)`：本地配置,只对本地仓库有效

`--global` ：全局配置，所有仓库生效

`--system` ：系统配置,对所有用户生效

## 特殊文件与特殊指针

![file-special](E:\Git\img\file-special.png)

`main/master`	主分支

`origin`	默认远程仓库

`HEAD`	当前分支

`HEAD^`	上一个版本

`HEAD~n`	上n个版本

## 创建仓库

`git init <project-name>`  	创建一个新的本地仓库 （省略project-name则在当前目录创建。）

`git clone <url>` 	克隆一个远程仓库

## 四个区域

![regions](E:\Git\img\regions.png)

![4-regions](E:\Git\img\4-regions.png)

## 文件状态

![file-status](E:\Git\img\file-status.png)

![file-status-2](E:\Git\img\file-status-2.png)

## 查看仓库状态或差异

`git status [-s]`	查看仓库状态， 列出还未提交的新的或修改的文件

`git log [--oneline]`	查看提交历史， --oneline表示简介模式（只有ID和时间）

`git reflog`	可以查看所有分支的所有操作记录（包括commit和reset的操作），包括已经被删除的commit记录

`git diff`	查看未暂存的文件更新了哪些部分

`git diff  <commit -id> <commit -id> <filename>`	查看两个提交之间的差异

## 添加文件与提交文件

`git add <file>`	添加一个文件到**暂存区**， 比如git add . 就表示添加所有文件到暂存区

`git commit -m "message"`	提交所有**暂存区**的文件到本地仓库

`git commit -am "message"`	提交所有**已修改**的文件到本地仓库

不加-m/-am的话会进入一个交互式界面，输入提交信息

`git restore --staged <file>`	撤销暂存区的文件， 重新放回工作区 （git add的反向操作）

## 修改撤销与恢复

`git reset [--mixed/--soft/--hard] <commit -id>`	重置当前分支的HEAD为之前的某个提交， 并且删除所有之后的提交

`--hard`参数表示重置工作区和暂存区

 `--soft`参数表示原样回退，不会删除东西

`--mixed`参数表示重置暂存区

![git-reset](E:\Git\img\git-reset.png)

## 删除文件

`git rm <filename>`	从工作区和暂存区删除一个文件， 并且将这次删除放入暂存区

`git rm --cached <filename>`	从索引/暂存区中删除文件， 但是本地工作区文件还在， 只是不希望这个文件被版本控制

`git rm -r *`	删除某个目录下所有文件（删除后需要提交）

`git mv  <filename> <new-file>`	移动文件到新的位置

`git checkout <file> <commit-id>`	恢复文件到之前的版本

`git revert <commit-id>`	创建一个新的提交， 用来撤销指定的提交， 后者的所有变化将被前者抵消， 并且应用到当前分支

## Stash

`git stash save "message"`	Stash操作可以把当前工作现场 “储藏”起来， 等以后恢复现场后继续工作。

`-u` 参数表示把所有未跟踪的文件也一并存储

`-a` 参数表示把所有未跟踪的文件和忽略的文件也一并存储

`save`参数表示存储的信息， 可以不写

`git stash list`	查看所有stash

`git stash pop`	恢复最近一次stash

`git stash pop stash@{2}`	恢复指定的stash， stash@{2}表示第三个stash， stash@{0}表示最近的stash

`git stash apply`	重新接受最近一次stash

`git stash drop stash@{2}`	pop和apply的区别是， pop会把stash内容删除， 而apply不会。 可以使用git stash drop 来删除stash

`git stash clear`	删除所有stash

## 远程仓库

`git remote add <remote-name> <remote-url>`  	添加远程仓库

`git remote -v`	查看远程仓库

`git remote rm <remote-name>`	删除远程仓库

`git remote rename <old-name> <new-name>`	重命名远程仓库

`git pull <remote-name> <branch-name>`	从远程仓库拉取代码。 默认拉取远程仓库名origin的master或者main分支

`git pull --rebase`	将本地改动的代码rebase到远程仓库的最新代码上 （为了有一个干净、 线性的提交历史）

`git push <remote-name> <branch-name>`	推送代码到远程仓库 （然后再发起pull request）

`git fetch <remote-name>`	获取所有远程分支

`git branch -r`	查看远程分支

`git fetch <remote-name> <branch-name>`	Fetch某一个特定的远程分支

## 分支

`git branch`	查看所有本地分支， 当前分支前面会有一个星号*， -r查看远程分支， -a查看所有分支

`git branch <branch-name>`	创建一个新的分支

`git switch <branch-name>`	切换分支

`git branch -d <branch-name>`	删除一个已经合并的分支

`git branch -D <branch-name>`	删除一个分支， 不管是否合并

`git tag <tag-name>`	给当前的提交打上标签， 通常用于版本发布

`git squash <branch-name>`	合并&挤压 （squash） 所有提交到一个提交

`git merge [--no-ff/-ff] -m message <branch-name>`	合并分支

`--no-ff` 参数表示禁用Fast Forward模式， 合并后的历史有分支， 能看出曾经做过合并（默认）

`-ff` 参数表示使用FastForward模式， 合并后的历史会变成一条直线

`git rebase <main>`	合并分支

rebase 操作可以把本地未push的分叉提交历史整理成直线， 看起来更加直观。 但是， 如果多人协作时， 不要对已经推送到远程的分支执行rebase操作。 

rebase不会产生新的提交， 而是把当前分支的每一个提交都 “复制”到目标分支上， 然后再把当前分支指向目标分支， 而merge会产生一个新的提交， 这个提交有两个分支的所有修改。

**强制推送**

强制推送到远程仓库 `git push --force origin master` 
