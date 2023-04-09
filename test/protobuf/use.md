### protobuf的使用

protobuf是一个用于序列化/反序列化的工具。

仍然使用聊天服务器的例子，网络间的主机通信需要实现约定好消息的格式。       
比如登录消息就是“name，pwd”，这样接受发反序列化后才知道如何获取数据。

这里的例子就主要写一下如何序列化/反序列化即可。

使用：     
* 定义proto文件：标明需要序列化/反序列化的消息格式
* 使用protoc工具生成相应的文件：`protoc test.proto --cpp_out=./`
* 现在即可使用，注意链接protobuf库

protobuf同样可以序列化单个数据，列表，映射表     

* message消息类的使用
* service服务类的使用


以我们聊天服务器的业务来，看看如何把服务转化为分布式的rpc服务。

#### v1序列化简单的结构体
掌握基本类的的序列化/反序列化。     
其实主要的操作也就是序列化对象/结构体；从接收到的数据反序列化出对象/结构体。

#### v2序列化列表
把重复的proto字段，写入一个消息体。      

列表的使用稍微麻烦，学习如何对列表进行增加/修改/删除
```C++
// 使用起来还是挺麻烦的

GetFriendListsResponse response;

// 获取列表的大小
inline int GetFriendListsResponse::friend_list_size() const {
  return _internal_friend_list_size();
}

// 返回的是指针参数，可进行修改，也可以获取所有的列表元素，具体看pb.h文件
inline ::testV2::User* GetFriendListsResponse::mutable_friend_list(int index) {
  // @@protoc_insertion_point(field_mutable:testV2.GetFriendListsResponse.friend_list)
  return friend_list_.Mutable(index);
}
// 返回的const引用，用于访问
inline const ::testV2::User& GetFriendListsResponse::friend_list(int index) const {
  // @@protoc_insertion_point(field_get:testV2.GetFriendListsResponse.friend_list)
  return _internal_friend_list(index);
}

```

#### v3实现调用方法service序列化
如何把本地的一样方法转化为线上的rpc方法？
供对方调用，你得把对方的请求给序列化啊，protobuf的用途就是序列化/反序列化。
我们的目的是写一个框架，供其他人调用。
这里引入的是一个业务背景我们的上个项目“聊天服务器”，以这个为例讲解如何把本地服务发布为rpc服务。     
不要想的太复杂（rpc相互调用的场景，先思考一个），上面的message类型序列化/反序列化非常的容易理解。
这不过这里序列化的是调用方法，可能有点难以理解。


service格式消息的学习
```proto
service UserServiceRpc
{
    rpc Login(LoginRequest) returns(LoginResponse);
    rpc GetFriendLists(GetFriendListsRequest) returns(GetFriendListsResponse);
}
```
会生成两个类：`UserServiceRpc  UserServiceRpc_Stub`      



一个重要的概念：RpcChannel；UserServiceRpc_Stub没有默认构造使用的时候需要传入一个channel。
而底层函数的调用也就是调用了channel而已。然而channel是什么？去service文件里面看看。
```C++
// 具体方法的实现全部转交给了channel
void UserServiceRpc_Stub::Login(::PROTOBUF_NAMESPACE_ID::RpcController* controller,
                              const ::testV3::LoginRequest* request,
                              ::testV3::LoginResponse* response,
                              ::google::protobuf::Closure* done) {
  channel_->CallMethod(descriptor()->method(0),
                       controller, request, response, done);
}
void UserServiceRpc_Stub::GetFriendLists(::PROTOBUF_NAMESPACE_ID::RpcController* controller,
                              const ::testV3::GetFriendListsRequest* request,
                              ::testV3::GetFriendListsResponse* response,
                              ::google::protobuf::Closure* done) {
  channel_->CallMethod(descriptor()->method(1),
                       controller, request, response, done);
}
```
// Abstract interface for an RPC channel.  An RpcChannel represents a
// communication line to a Service which can be used to call that Service's
// methods.  The Service may be running on another machine.  Normally, you
// should not call an RpcChannel directly, but instead construct a stub Service
// wrapping it.  Example:
//   RpcChannel* channel = new MyRpcChannel("remotehost.example.com:1234");
//   MyService* service = new MyService::Stub(channel);
//   service->MyMethod(request, &response, callback);

现在让我迷惑的是，调用方需要stub类，而服务的提供方需要做哪些呢？感觉服务提供方应该简单
只需要处理请求，发送回应即可。而调用方需要将本地请求==》rpc请求。
但是服务器，如何知道这是给哪个方法的请求呢？



#### 本地方法如何发布为rpc
以login方法为例简要介绍。以业务为驱动，但是我们要写的是框架

1、服务的提供方要把本地==》rpc需要怎么做？显然需要继承+override
2、调用方的请求如何序列化==》stub类，需要channel。


