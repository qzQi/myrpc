#include "rpcchannel.h"
#include "rpcheader.pb.h"
#include<string>
#include<muduo/base/Logging.h>

//for socket，或者直接使用muduo的tcpClient
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<unistd.h>

#include "zkutil.h"

using namespace std;
using namespace mprpc;//for rpcHeader
using namespace muduo;

/*
UserServiceRpc_Stub stub(new MpRpcChannel());

stub.Login(controller,request,response);
==>channel_->CallMethod(descriptor(),controller,request,response)
在CallMethod里面统一做rpc方法调用的数据数据序列化和网络发送 
*/
// 所有通过stub代理对象调用的rpc方法，最终都走到这里了，
// 统一做rpc方法调用的数据数据序列化和网络发送 
void MpRpcChannel::CallMethod(
    const protobuf::MethodDescriptor *method,
    protobuf::RpcController *controller,
    const protobuf::Message *request,
    protobuf::Message *response,
    protobuf::Closure *done) {

    const protobuf::ServiceDescriptor* pserviceDes=method->service();
    const string& serviceName=pserviceDes->name();
    const string& methodName=method->name();

    uint32_t args_size=0;
    string argsStr;
    if(request->SerializeToString(&argsStr)){
        args_size=argsStr.size();
    }else{
        // 这个channel主要是给客户端使用，不要打印运行信息，
        // 稍后可以把调用失败信息写入controller。
        // LOG_DEBUG<<"serialize request error";
        controller->SetFailed("serialize request error");
        return;
    }

    // 定义rpcHeader
    RpcHeader header;
    header.set_service_name(serviceName);
    header.set_method_name(methodName);
    header.set_args_size(args_size);
    // 构造需要发送给rpc节点的信息：payload+rpcHeader+argsStr
    uint32_t header_size=0;
    string headerStr;
    if(header.SerializeToString(&headerStr)){
        header_size=headerStr.size();
    }else{
        // 稍后可以把调用失败信息写入controller。然后调用者也可以通过controller获取调用信息
        // LOG_DEBUG<<"serialize rpcheader error";
        controller->SetFailed("serialize rpcheader error");
        return;
    }
    // 开始组装发送的数据，headerSize（4B）+headerStr+argsStr
    // string的copy和 assign:copy从字符流里面读取固定字节数据，assign负责将给定字节数写入string
    string sendStr;
    sendStr.assign((char*)&header_size,4);//headerSize==>payload
    sendStr.append(headerStr);// header_str==>(parse) RpcHeader
    sendStr.append(argsStr);// args

    // debug信息，
    LOG_INFO<<"debug info: call rpc "<<serviceName<<" "<<methodName;

    // 现在所有的都完成可以向rpc服务器发送请求了，使用网络发送
    // 然后阻塞等待响应数据。如何获取rpc服务节点的信息：ip+port
    
    // 实现的rpc框架，要考虑服务的注册与发现，我们使用的是zookeeper
    // 后面我们在添加从zookeeper获取服务信息。现在先测试功能使用直连。
    // 如何获取zookeeper的相关信息，这个服务的发布与调用都需要使用，
    // 直接作为框架的一部分，提供一个框架的初始化工作，来进行相关的配置。

    // 编写tcp客户端，完成rpc方法的远程调用，现在就差实现网络部分了
    int sockfd=socket(AF_INET,SOCK_STREAM,0);
    if(-1==sockfd){
        controller->SetFailed("socket() error ");
        return;
    }
    // 这个rpc节点的ip与port不应该由用户提供，从服务注册中心获取
    // 当前是在测试环境，直接直连
    // 新增从zk获取节点的配置信息，IP+Port
    string znodePath="/"+serviceName+"/"+methodName;
    ZkClient zkCli;
    zkCli.Start();
    string addrStr=zkCli.GetData(znodePath.c_str());
    if(addrStr==""){
        // 给用户使用的框架代码不要cout，可以使用日志，但是这里使用controller更好
        controller->SetFailed(znodePath+"is not exist");
        return;
    }
    size_t idx=addrStr.find(":");
    if(idx==string::npos){
        controller->SetFailed("valid addr from zookeeper");
        return;
    }
    string ip=addrStr.substr(0,idx);//idx的下标是从0开始的
    string port=addrStr.substr(idx+1,addrStr.size()-idx-1); //default npos==>end

    // 写死仅仅是为了测试，我们写的是框架代码，从服务注册中心获取信息
    // string ip="127.0.0.1";
    // uint16_t port=12345;
    sockaddr_in server_addr;
    server_addr.sin_family=AF_INET;
    server_addr.sin_port=htons(atoi(port.c_str()));
    server_addr.sin_addr.s_addr=inet_addr(ip.c_str());
    // connect
    // 连接rpc服务节点
    if (-1 == connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)))
    {
        close(sockfd);
        controller->SetFailed("connect() error");
        return;
    }
    // send
    if(-1==send(sockfd,sendStr.c_str(),sendStr.size(),0)){
        close(sockfd);
        // 这就要求调用者必须传入一个controller，测试环境下没有运行到这所有没有使用空指针
        controller->SetFailed("send() error");
        return;
    }
    // receive
    // 接收rpc请求的响应值
    char recv_buf[1024] = {0};
    int recv_size = 0;
    if (-1 == (recv_size = recv(sockfd, recv_buf, 1024, 0)))
    {
        close(sockfd);
        controller->SetFailed("recv error");
        return;
    }
    // parse
    // close
    close(sockfd);

    // for debug，发布后需要删除，rpc服务的调用方我们框架可不能输出这些运行日志信息
    LOG_INFO<<"debug info: stub: rpc call success!";
}