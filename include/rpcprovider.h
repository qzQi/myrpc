#pragma once

#include<google/protobuf/service.h>
#include<google/protobuf/descriptor.h>
#include<muduo/net/InetAddress.h>
#include<muduo/net/TcpConnection.h>
#include<muduo/net/TcpServer.h>
#include<muduo/net/EventLoop.h>

#include<unordered_map>
#include<functional>
#include<string>

using namespace std;
using namespace muduo::net;
using namespace google;

// 发布rpc节点，也就是rpc服务器
class RpcProvider{
public:
    //这样定义的话，使用我们的框架就和使用muduo很像了
    RpcProvider(EventLoop* loop,const InetAddress&listenAddr,const string&name):
                loop_(loop),server_(loop,listenAddr,name),listenAddr_(listenAddr)
    {   

    }
    
    // protobuf的所有rpc方法都是继承自service类，里面有这个服务和method的相应描述。
    // 对外提供的接口，将用户的rpc服务进行处理，保存后使用Run进行发布
    void NotifyService(protobuf::Service* service);

    // 使用notify后统一使用run方法进行发布
    // 启动rpc服务器节点，开始提供rpc服务
    void Run();
private:
    // 使用muduo网络库来提供网络服务
    InetAddress listenAddr_;
    TcpServer server_;
    EventLoop* loop_;

    // 保存服务及其所拥有的所有的信息
    struct ServiceInfo{
        protobuf::Service* pservice;
        unordered_map<string,const protobuf::MethodDescriptor*> methodMap;
    };
    // 保存服务名字及其信息的映射
    unordered_map<string,ServiceInfo> serviceMap;

    // muduo库的回调
    void OnConnection(const TcpConnectionPtr& conn);
    // muduo库的回调，网络数据的读取与处理
    void OnMessage(const TcpConnectionPtr& conn,Buffer* buff,muduo::Timestamp t);

    // 设置给Closure的回调，done->Run(),运行这个回调函数来发送方法的response
    // 由于使用NewCallback生成的CLosure*的时候不能传递引用，所有改为非引用
    void SendRpcResponse(const TcpConnectionPtr conn,protobuf::Message* response);
};