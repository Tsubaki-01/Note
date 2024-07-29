### vscode安装

具体参照网上教程

### 安装Remote-SSH拓展

![QQ_1721210471128](./vscode+remotessh+linux.assets/QQ_1721210471128.png)

### 连接

![QQ_1721210491475](./vscode+remotessh+linux.assets/QQ_1721210491475.png)

![QQ_1721210515609](./vscode+remotessh+linux.assets/QQ_1721210515609.png)

![QQ_1721210682225](./vscode+remotessh+linux.assets/QQ_1721210682225.png)

![QQ_1721210694942](./vscode+remotessh+linux.assets/QQ_1721210694942.png)

![QQ_1721210794143](./vscode+remotessh+linux.assets/QQ_1721210794143.png)

![QQ_1721210823325](./vscode+remotessh+linux.assets/QQ_1721210823325.png)

![QQ_1721210838426](./vscode+remotessh+linux.assets/QQ_1721210838426.png)

发现连不通，提示

> 过程试图写入的管道不存在。

原因是Ubuntu没有安装ssh服务

### Ubuntu安装SSH服务

**安装软件包**

```shell
sudo apt-get install openssh-server
```

**开启SSH服务**

```shell
sudo service ssh start
```

**查看SSH服务状态**

```shell
systemctl service ssh start
```

**关闭虚拟机防火墙**

```shell
systemctl stop firewalld.service
#关闭防火墙
systemctl disable firewalld.service 
#关闭防火墙开机启动
```

主机ping虚拟机能ping通即可

### 继续连接

![img](./vscode+remotessh+linux.assets/{C8061DE6-3BC6-4B21-8B19-DAF1315B8075})

![img](./vscode+remotessh+linux.assets/{19113387-D309-4BB7-AA68-4C224477256A})

![img](./vscode+remotessh+linux.assets/{9D4DBA85-A99C-4EF0-A71E-4B4CB00E98A0})

![img](./vscode+remotessh+linux.assets/{FACC14A2-44F0-406D-8B45-3ABB7E5C52CD})

出现终端的命令行即成功

### 秘钥连接

完成以上步骤之后，每次连接都需要输入对应用户的密码，接下来使用秘钥，不需要每次都输入密码

**生成秘钥**

![QQ_1721269209459](./vscode+remotessh+Ubuntu.assets/QQ_1721269209459.png)

接下来找到公钥文件`id_rsa.pub`

将其复制到Ubuntu对应用户的`.ssh`文件夹中，并将其添加到`authorized_keys`文件中

![QQ_1721269362342](./vscode+remotessh+Ubuntu.assets/QQ_1721269362342.png)

完成
