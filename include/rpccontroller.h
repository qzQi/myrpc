#pragma once

/*
void UserServiceRpc_Stub::Login(::google::protobuf::RpcController* controller,
                              const ::example::LoginRequest* request,
                              ::example::LoginResponse* response,
                              ::google::protobuf::Closure* done) {
  channel_->CallMethod(descriptor()->method(0),
                       controller, request, response, done);
}

服务的调用方通过stub类来调用我们的服务的时候，这个服务究竟是调用成功还是失败？
我们不应该在框架里给调用方输出这些信息（调试时候自己看看还行）
这些rpc调用信息都可以通过RpcController来返回，

服务端当然不需要，服务端直接使用日志库来输出相关的信息；但是服务的调用方（客户端）不要给用户显示

RpcController是一个纯基类
也就是只是定义了一组接口
*/

#include<google/protobuf/service.h>
#include<string>

using std::string;

using namespace google;

class MpRpcController:public protobuf::RpcController{
public:
   MpRpcController():flag(false){

   }
  // Client-side methods ---------------------------------------------
  // These calls may be made from the client side only.  Their results
  // are undefined on the server side (may crash).

  // Resets the RpcController to its initial state so that it may be reused in
  // a new call.  Must not be called while an RPC is in progress.
  virtual void Reset() override{
    //框架里的channel和controller对象是可以重复使用的
    flag=false;
    errMsg="";
  }

  // After a call has finished, returns true if the call failed.  The possible
  // reasons for failure depend on the RPC implementation.  Failed() must not
  // be called before a call has finished.  If Failed() returns true, the
  // contents of the response message are undefined.
  virtual bool Failed() const override{
    //const方法不是只有const能调用吗？ 还是说const对象只能调用const版本
    return flag;
  }

  // If Failed() is true, returns a human-readable description of the error.
  virtual string ErrorText() const override{
    return errMsg;
  }

  // Advises the RPC system that the caller desires that the RPC call be
  // canceled.  The RPC system may cancel it immediately, may wait awhile and
  // then cancel it, or may not even cancel the call at all.  If the call is
  // canceled, the "done" callback will still be called and the RpcController
  // will indicate that the call failed at that time.
  virtual void StartCancel() override{

  };

  // Server-side methods ---------------------------------------------
  // These calls may be made from the server side only.  Their results
  // are undefined on the client side (may crash).

  // Causes Failed() to return true on the client side.  "reason" will be
  // incorporated into the message returned by ErrorText().  If you find
  // you need to return machine-readable information about failures, you
  // should incorporate it into your response protocol buffer and should
  // NOT call SetFailed().
  virtual void SetFailed(const string& reason) override{
    errMsg=reason;
  }

  // If true, indicates that the client canceled the RPC, so the server may
  // as well give up on replying to it.  The server should still call the
  // final "done" callback.
  virtual bool IsCanceled() const override{
    // 未实现
    return true;
  };

  // Asks that the given callback be called when the RPC is canceled.  The
  // callback will always be called exactly once.  If the RPC completes without
  // being canceled, the callback will be called after completion.  If the RPC
  // has already been canceled when NotifyOnCancel() is called, the callback
  // will be called immediately.
  //
  // NotifyOnCancel() must be called no more than once per request.
  virtual void NotifyOnCancel(protobuf::Closure* callback) override{

  }
private:
    bool flag;
    std::string errMsg;
};