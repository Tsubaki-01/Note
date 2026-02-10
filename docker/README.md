#### Error response from daemon: ports are not available: exposing port TCP : listen tcp : bind: An attempt was made to access a socket in a way forbidden by its access permissions.

> [!NOTE]
>
> 你运行 `net stop winnat` 和 `net start winnat` 后容器端口绑定恢复正常，核心原因是 **WinNAT 服务（Windows 网络地址转换服务）出现了端口占用 / 分配异常**，重启该服务重置了 Windows 的端口分配规则，解决了 7687 端口的绑定权限问题。
>
> #### 1. WinNAT 服务的核心作用
>
> WinNAT 是 Windows 系统内置的核心网络服务，主要负责：
>
> - 网络地址转换（NAT）：比如 WSL、Docker 容器、虚拟机等虚拟网络环境与宿主机的网络转发。
> - 端口分配与管理：控制宿主机端口的占用、释放和绑定权限，尤其是虚拟网络（如 Docker 的桥接网络）与宿主机的端口映射。
>
> #### 2. 为什么会出现端口绑定失败？
>
> WinNAT 服务运行过程中可能因以下原因出现异常：
>
> - Docker 容器异常退出、强制终止，导致 WinNAT 未正确释放已分配的端口（比如 7687），系统误以为该端口仍被占用 / 锁定。
> - Windows 系统更新、网络配置变更后，WinNAT 的端口分配缓存出错，导致正常的端口绑定请求被判定为 “权限禁止”。
> - WSL、Hyper-V（Docker 依赖的虚拟化技术）与 WinNAT 之间的端口分配冲突。
>
> 此时端口并未被具体的进程占用（用 `netstat` 可能查不到占用 PID），而是被 WinNAT 服务的 “逻辑锁” 占用，普通的终止进程操作无效，必须重启 WinNAT 来重置端口状态。
>
> #### 3. `net stop/start winnat` 的作用
>
> - `net stop winnat`：强制停止 WinNAT 服务，清空其端口分配缓存、释放所有被它锁定的端口。
> - `net start winnat`：重新启动 WinNAT 服务，重建端口分配规则，此时系统会重新识别端口状态，7687 端口恢复为 “未占用”，Docker 就能正常绑定。

