## [Raft的可视化](https://thesecretlivesofdata.com/raft/)

## Raft的重要组件

==状态机==：raft的上层应用，可以是k-v数据库（本项目）

==日志、log、entry==：

1. 日志log：raft保存的外部命令是以日志保存
2. entry：日志有很多，可以看成一个连续的数组，而其中的的一个称为entry

==提交日志commit==：raft保存日志后，经过复制同步，才能真正应用到上层状态机，这个“应用”的过程称为提交

==节点身份==：`follower、Candidate、Leader` ：raft集群中不同节点的身份

==term==：也称任期，是raft的逻辑时钟

==选举==：follower变成leader需要选举

==领导人==：就是leader

==日志的term==：在日志提交的时候，会记录这个日志在什么“时候”（哪一个term）记录的，用于后续日志的新旧比较

==心跳、日志同步==：leader向follower发送心跳（AppendEntryRPC）用于告诉follower自己的存在以及通过心跳来携带日志以同步

