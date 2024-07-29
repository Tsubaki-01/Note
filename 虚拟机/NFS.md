## 服务端

**安装**

```bash
sudo apt-get install nfs-kernel-server
```

**创建共享文件夹**

选择在`/home`下创建

**修改配置文件**

在配置文件`/etc/exports`中添加以下行：

```
/home/nfs *(rw,sync,no_root_squash,no_subtree_check)
```

**重新启动 NFS 服务**

保存并关闭文件后，重新启动 NFS 服务以应用更改：

```
sudo exportfs -a
sudo systemctl restart nfs-kernel-server
```

## 客户端

在客户端机器上，挂载这个 NFS 导出的目录

**创建挂载点**：

   ```bash
   sudo mkdir -p /mnt/nfs
   ```

**挂载 NFS 目录**：

   ```bash
   sudo mount -t nfs <NFS服务器IP>:/home/nfs /mnt/nfs
   ```

例如，如果 NFS 服务器的 IP 地址是 `192.168.1.100`：

   ```bash
   sudo mount -t nfs 192.168.1.100:/home/nfs /mnt/nfs
   ```

**验证挂载**

挂载完成后，可以验证是否成功挂载：

```bash
df -h /mnt/nfs
```

或者查看挂载的内容：

```bash
ls /mnt/nfs
```