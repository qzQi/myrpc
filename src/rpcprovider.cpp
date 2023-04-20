#include "rpcprovider.h"

#include "rpcheader.pb.h"

#include <muduo/base/Logging.h>

#include "mprpcinit.h" //获取zk的ip与port
#include "zkutil.h"    // 使用zk znode进行服务的注册
using namespace muduo;
using namespace std::placeholders;

// 框架提供给调用者使用来发布rpc服务，我们直到protobuf的service类型是继承自service
void RpcProvider::NotifyService(protobuf::Service *service) {
    // 设计一个数据结构来存储相关信息，调用者会发送服务名字，方法名字
    ServiceInfo serInfo;

    // 获取service对象的详细描述，比如服务的名字，服务的方法个数
    const protobuf::ServiceDescriptor *pserviceDes = service->GetDescriptor();
    const string &serviceName = pserviceDes->name();
    int methodCnt = pserviceDes->method_count();
    LOG_INFO << "Service " << serviceName << " has " << methodCnt << " methods";
    serInfo.pservice = service;

    // 来获取这个rpc服务每个rpc方法的信息，比如方法名字和描述信息
    for (int i = 0; i < methodCnt; i++) {
        const protobuf::MethodDescriptor *pmethodDes = pserviceDes->method(i);
        const string &methodName = pmethodDes->name();
        LOG_INFO << "method name " << methodName;
        // 存储服务名字及其描述信息
        serInfo.methodMap.insert({methodName, pmethodDes});
    }

    serviceMap.insert({serviceName, serInfo});
}

// 调用Server的start并开始eventloop，注册相应的事件回调
// 并把自己的提供的rpc进行注册，写入znode
void RpcProvider::Run() {
    LOG_INFO << "rpc node begin running!";

    // 设置相应的回调操作
    server_.setConnectionCallback(
        std::bind(&RpcProvider::OnConnection, this, _1));
    server_.setMessageCallback(
        std::bind(&RpcProvider::OnMessage, this, _1, _2, _3));

    // 向服务注册中心注册自己的服务信息，ip+port，这个稍后再写
    // 已经完成，使用zookeeper完成服务的注册
    ZkClient zkCli;
    zkCli.Start();
    // 向znode写入，发布在这个rpc节点上面的所有的服务；
    // eg: /UserServiceRpc/Login ==>127.0.0.1:12345
    string addr = listenAddr_.toIpPort();
    for (const auto &spEntry : serviceMap) {
        string servicePath = "/" + spEntry.first;
        // zk的znode不能递归创建，必须先创建上一级，创建的永久性节点
        zkCli.Create(servicePath.c_str(), nullptr, 0, 0);

        const auto &methodMap = spEntry.second.methodMap;
        // 获取这个methodMap对应的所有方法
        for (const auto &mpEntry : methodMap) {
            string methodPath=servicePath+"/"+mpEntry.first;
            // 创建的临时性znode节点
            zkCli.Create(methodPath.c_str(),addr.c_str(),addr.size(),ZOO_EPHEMERAL);
        }
    }
    LOG_INFO<<"publish all methods on "<<addr;
    // 启动网络服务
    server_.setThreadNum(3);
    server_.start();
    loop_->loop();
}

// muduo库的回调，我们的rpc采用的短链接，
void RpcProvider::OnConnection(const TcpConnectionPtr &conn) {
    if (!conn->connected()) { conn->shutdown(); }
}

/*
这种注释不会出现在提示信息里面
在框架内部，RpcProvider和RpcConsumer协商好之间通信用的protobuf数据类型,也就是我们的rpcHeader
service_name method_name
args,但是发送的整个包不是仅有protobuf的内容，而是需要有一些payload
payload表示rpcHeader的大小，然后从收到的数据里面截取rpcHeader大小的数据来序列化
|---payLoad:4bytes--RpcHeader:(payload define)---args(size defined by
rpcHeader)---|
*/
// muduo库的回调，网络数据的读取与处理，进行分发rpc服务
void RpcProvider::OnMessage(
    const TcpConnectionPtr &conn, Buffer *buff, muduo::Timestamp t) {
    LOG_DEBUG << "recvive call request from" << conn->name();
    string bufStr = buff->retrieveAllAsString();

    // 从字符流读取四字节数据，来表示rpcHeader的大小
    uint32_t header_size = 0;
    // 读取四字节数据，并表示为uint
    bufStr.copy((char *)(&header_size), 4, 0);

    // 根据header_size读取数据头的原始字符流，反序列化数据，得到rpc请求的详细信息
    string rpcHeaderStr = bufStr.substr(4, header_size);
    string serviceName, methodName;
    uint32_t args_size = 0;
    mprpc::RpcHeader header;
    if (header.ParseFromString(rpcHeaderStr)) {
        serviceName = header.service_name();
        methodName = header.method_name();
        args_size = header.args_size();
    } else {
        LOG_ERROR << "parse rpcHeader error! ";
        return;
    }

    // 根据args_size从字符流里面获取request参数数据（request参数也是一个经过protobuf序列化的messgae类型）
    string argsStr = bufStr.substr(4 + header_size, args_size);

    LOG_INFO << "receive RpcRequest :" << serviceName << " " << methodName;

    // 可是获取了参数的数据，如何知道用哪个类型来反序列化呢？
    // 上面已经获取了调用服务+方法的名字，从serviceMap找出描述信息，protobuf为我们保存了rpc方法的参数信息
    // 找出相应的服务及方法
    // serviceName,methodName
    auto sMapIt = serviceMap.find(serviceName);
    // 在serviceMap里找到相应的服务信息
    if (sMapIt != serviceMap.end()) {
        const ServiceInfo &serInfo = sMapIt->second;
        auto methodMapIt = serInfo.methodMap.find(methodName);
        // 在这个服务所在的methodMap里面找到相应的method描述
        if (methodMapIt != serInfo.methodMap.end()) {
            // 获取相应的service以及methodDescriptor
            protobuf::Service *pservice = serInfo.pservice;
            const protobuf::MethodDescriptor *pmethodDes = methodMapIt->second;

            // 现在已经获取了相应的service以及method的描述，
            //  protobuf就可以告诉我们相应的rpc的参数及返回值
            protobuf::Message *request =
                pservice->GetRequestPrototype(pmethodDes).New();
            protobuf::Message *response =
                pservice->GetResponsePrototype(pmethodDes).New();

            // 现在可以对参数argsStr进行解析了
            if (request->ParseFromString(argsStr)) {
                // 现在参数解析完成，就可以调用相应的方法了

                // 看看UserServiceRpc::Login是如何调用的，会发现方法调用完成后需要调用closure的
                // done->Run() 读一读callback.h里面的NewCallback
                // protobuf::Closure*
                // done=protobuf::NewCallback(std::bind(RpcProvider::SendRpcResponse,this,conn,response));
                // 通过阅读callback：65line发现无法传递引用参数
                protobuf::Closure *done = protobuf::NewCallback(
                    this, &RpcProvider::SendRpcResponse, conn, response);

                // 这个closure肯定是不能传nullptr的，点进去看看，callMethod
                // callMethod会转而调用相关的method
                pservice->CallMethod(
                    pmethodDes, nullptr, request, response, done);

            } else {
                LOG_ERROR << "parse args error " << methodName;
                return;
            }

        } else {
            LOG_ERROR << "can't find " << methodName;
            return;
        }
    } else {
        LOG_ERROR << "can't find " << serviceName;
        return;
    }
}

// 向调用者发送response的回调，google::protobuf::Closure* done， done->Run();
//
void RpcProvider::SendRpcResponse(
    const TcpConnectionPtr conn, protobuf::Message *response) {
    // info: run in SendRpcResponse
    LOG_INFO << "finish method call,now send rpc response ";
    conn->send(response->SerializeAsString());
    // conn->shutdown();这个不应该由服务端来调用，
    // 我们是框架的实现者，自己决定在哪调用，这个应该给客户端调用，
    // rpc的调用者每发起一次rpc请求后主动断开连接
}
