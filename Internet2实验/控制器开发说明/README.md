# Internet2实验 控制器开发说明

##### 概述

提供对Internet2拓扑上的SDN实验环境的控制平面支持。使用OpenDaylight控制器（以下简称ODL），并根据实验需求，进行二次开发。主要实现的功能模块有：

 	1. 拓扑发现模块
 	2. 主机发现模块
 	3. 路由模块
 	4. 流表格式
 	5. PacketIn信息记录模块

##### OpenDaylight控制器

ODL 主要有RPC、DataTreeChangeListener、notification三种**消息传递机制**。其中RPC是远程过程调用；notification是类似一对多的事件广播机制；DataTreeChangeListener也是一种消息广播机制，它会在ODL的数据库某个数据节点被修改后，通知监听者。

ODL的架构和如何开发的细节详见官网的开发文档，接下来主要描述各个功能模块的工作流程和流程的算法。

## 模块实现

### 1 拓扑发现模块

目的：形成本地的交换机拓扑视图

实现原理：监听LLDP包，形成本地交换机拓扑视图

工作流程：

  * 监听ODL和OVS链接建立事件，数据库里会有节点产生事件
  * 收到ODL和OVS的链接后下发LLDP流表，让OVS将LLDP数据包上送控制器
  * ODL收到LLDP数据包后，数据库里会有链路产生事件
  * 监听数据库节点和链路修改事件，构造一个本地的交换机拓扑视图

技术实现：

 * 通过注册DataTreeChangeListener监听ODL和OVS链接建立事件
 * 通过注册DataTreeChangeListener监听拓扑数据节点修改事件
 * 本地拓扑的代码实现视图使用了Jung —— 一个java的图库

### 2 主机发现模块

目的：形成主机和边缘交换机的映射

实现原理：

​	最先开始使用的是基于ARP的主机发现方式，即主机通信前先发ARP包  ->  程序根据收到ARP包判断主机MAC地址和交换机边缘端口的映射。

​	后来由于实验需求改变，主机会发送非常多的自定义包——源目MAC地址和IP地址都会改变。所以不能使用ARP来映射MAC地址和交换机端口，改为采用静态的映射方式，在控制器程序里事先写好所有主机IP地址和交换机边缘端口的映射。

技术实现：

* 改为采用静态的映射方式，在控制器程序里事先写好所有主机IP地址和交换机边缘端口的映射Map。
* 例如 {"10.0.0.1","openflow:1:4"}，就表示IP10.0.0.1的主机连接着交换机s1的端口4上。
* 例如 {"1.0.0.0/8-50.0.0.0/8","openflow:1:4"}，就表示将IP域1.0.0.0/8-50.0.0.0/8里的所有主机都映射到交换机s1的端口4上

工作流程：

* 控制器收到PacketIn后，解析源目IP，根据源目IP找到源主机所在边缘端口，目的主机所在边缘端口，然后可以根据路由模块下相应的流表

### 3 路由模块

目的：收到IP-PacketIn后，下发流表指导ovs转发。本实验主要需求有最短路路由和固定路径路由。固定路径通过静态映射方式写到程序里，最短路则根据本地拓扑视图找到。

实现原理：

* 最短路路由：根据IP包的源目地址，找到两台边缘交换机，然后查询本地拓扑视图构建一条最短路径，根据最短路给当前ovs下流表。（每次收到一个PacketIn上送，就给上送PacketIn的交换机下一个流表）
* 固定路径路由：使用Map映射两台ovs和它们的固定路径交换机序列。例如{ "openflow:7-openflow:15" ： "7,8,17,16,15" } 就表示交换机7给交换机15的包，需要通过固定路径s7、s8、s17、s16、s15。

工作流程：

* 路由模块监听PacketIn的通知，收到消息后处理PacketIn
* 解析IP包的源目的IP地址，根据主机发现模块，找到源目IP映射的源目边缘交换机。
* 根据源目的边缘交换机，查看映射，是否使用固定路径，需要的话根据固定路径找到出端口，否则使用最短路找到出端口。
* 使用RPC下PacketOut，使用RPC下IP域流表。

技术实现：

* 注册模块为PacketProcessingListener，让模块成为PacketIn通知消息的监听者。即能够收到PacketIn包。然后在onPacketReceived接口里写处理函数。
* 处理好之后，使用ODL的RPC发送PacketOut。ODL实现了下发PacketOut的远程过程调用——PacketProcessingService，
* 处理好之后，使用ODL的RPC下发流表。ODL实现了下发流表的远程过程调用——SalFlowService

### 4 流表格式

根据实验需求，下聚合流的流表。原理：我们主要通过给流表的IP加掩码的方式来表示聚合流，通过这种方式就表示了一个网段到一个网段的数据流路由。

如下所示是一条实验过程中产生的流表：设置流表空闲超时300ms

	cookie=0x2a000000000011f0, duration=45.815s, table=0, n_packets=419, n_bytes=430909, idle_timeout=300, idle_age=0, priority=10,ip,in_port=4,nw_src=4.0.0.0/8,nw_dst=192.0.0.0/8 actions=output:1

其中匹配项为：ip协议、入端口、源IP/掩码、目的IP/掩码、出端口

```
ip,in_port=4,nw_src=4.0.0.0/8,nw_dst=192.0.0.0/8 actions=output:1
```

### 5 PacketIn信息记录模块

目的：是本实验数据统计的一部分，需要统计相关交换机上送PacketIn的情况

工作流程：

* 记录模块监听PacketIn的通知，收到消息后处理PacketIn
* 处理流程就是提取PacketIn的各个属性写到文件，包括：到达时间、源IP、目的IP、源TCP/UDP端口、目TCP/UDP端口、字节数

技术实现:

* 注册模块为PacketProcessingListener，让模块成为PacketIn通知消息的监听者。即能够收到PacketIn包。然后在onPacketReceived接口里写处理函数。
* 收到包后产生一条日志，写入文件。





### PS 关于流表信息统计是在数据平面上统计的

在ovs宿主机上使用脚本定期获取ovs流表统计信息。脚本如下：

```
#!/bin/bash
				
FILE=$(date '+%Y-%m-%d')"_rtxpacket.txt"
				
				
num=10*60*60 #set last time
ovs=br1	   #set one ovs name
				
				
for ((i=1;i<=num;i++))
	do
		cur_sec_and_ns=`date '+%s-%N'`
		cur_sec=${cur_sec_and_ns%-*}
		cur_ns=${cur_sec_and_ns##*-}
		cur_timestamp=$((cur_sec*1000+10#$cur_ns/1000000))
		echo $cur_timestamp >> $FILE
		ovs-ofctl dump-flows $ovs >> $FILE
		sleep 1
done	

```

说明：运行linux的bash脚本，在一定时间范围内（例如20小时），每秒钟执行一次ovs-ofctl dump-flows 命令获取流表统计情况然后写入日志文件中。

### 