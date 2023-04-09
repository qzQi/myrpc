/*
继承protobuf的RpcChannel，并override callMthod方法

这是我们使用我们框架的，rpc服务调用者主要关心的类

比如：调用者在初始化对象的时候所主要关注的参数
UserServiceRpc_Stub stub(new MpRpcChannel());

至于这个callMethod，service和rpcchannel里面都有，
在实现框架的发布部分时候，主要关注的是service的callMethod
*/
#pragma once

#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>

using namespace google;

class MpRpcChannel:public protobuf::RpcChannel{
public:
// 所有通过stub代理对象调用的rpc方法，最终都走到这里了，统一做rpc方法调用的数据数据序列化和网络发送 
virtual void CallMethod(const protobuf::MethodDescriptor* method,
                          protobuf::RpcController* controller,
                          const protobuf::Message* request,
                          protobuf::Message* response,
                          protobuf::Closure* done)override;
};