// 进行框架的初始化工作，主要就是读取一些配置信息
// 比如zk的ip+port，服务的注册中心，rpc的发布与调用方都需要使用
// 目前配置文件的位置得放在和可执行文件那里

#pragma once

// 这个文件服务的发布与调用都需要，把所有的框架头文件都包含来简化使用

#include "rpcchannel.h"
#include "rpccontroller.h"
#include "rpcprovider.h"

#include<string>

class MpRpcInit{
public:
    static MpRpcInit& instance();
    // 返回zkHost
    std::string getHost(){
        return zkHost;
    }
    // 返回zkPort
    std::string getPort(){
        return zkPort;
    }
private:
    // 构造函数负责从conf文件读取相关的配置，然后进行存储
    MpRpcInit();
    MpRpcInit(const MpRpcInit&)=delete;
    MpRpcInit(MpRpcInit&&)=delete;
    std::string zkHost;
    std::string zkPort;
};