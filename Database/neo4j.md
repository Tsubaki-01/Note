# neo4j数据库导入导出

## docker中neo4j社区版导入dump格式数据库

### 问题介绍

社区版不允许在运行neo4j容器时单独停止数据库，无法正常进行数据库导入（要导入需要先停止数据库，停了数据库就相当于把容器停了）

### 解决方法

在社区版中，由于不支持多数据库管理，`STOP DATABASE` 和 `START DATABASE` 这些管理命令是**不可用**的。在社区版里，数据库的状态与容器（进程）的状态是绑定在一起的：容器开着，数据库就一定在运行并锁定文件。

由于 `neo4j-admin load` 必须在文件未被占用的情况下进行，你不能在容器运行时对自己执行加载。以下是解决这个问题的标准流程：

假设你的容器数据卷挂载在宿主机的路径（或者使用 Docker Volume），你可以停止当前容器，运行一个临时容器来执行 Load 操作：

1. **停止当前运行的容器：**

   ```bash
   docker stop silver_neo4j
   ```

2. **执行加载命令（通过临时挂载）：** 你需要确保路径映射正确。假设你的数据卷名为 `neo4j_data`，备份文件在宿主机的 `/path/to/backups`：

   ```bash
   docker run --rm \
       --volumes-from silver_neo4j \
       -v /path/to/backups:/backups \
       neo4j:latest \
       neo4j-admin database load --from-path=/backups --overwrite-destination=true neo4j
   ```

   *注：`--volumes-from` 会自动挂载旧容器的所有路径，确保文件写入位置一致。*

3. **重新启动原容器：**

   ```bash
   docker start silver_neo4j
   ```

## neo4j导出数据库

使用`apoc`插件

```bash
# 允许导出到文件
apoc.export.file.enabled=true
# (可选) 允许从文件导入
apoc.import.file.enabled=true
```

> apoc.conf

```c++
CALL apoc.export.csv.all("full_export.csv", {})
```