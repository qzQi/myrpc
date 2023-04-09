#include "mprpcinit.h"
#include "example.pb.h"

using namespace example;

#include <iostream>
#include <string>

using namespace std;

#include <muduo/net/InetAddress.h>
#include <muduo/net/EventLoop.h>
using namespace muduo::net;

class UserService : public UserServiceRpc {
  public:
    bool login(uint32_t uid, const string &pwd) {
        cout << "uid " << uid << " pwd " << pwd << endl;
        return true;
    }
    virtual void Login(
        ::google::protobuf::RpcController *controller,
        const ::example::LoginRequest *request,
        ::example::LoginResponse *response,
        ::google::protobuf::Closure *done) override {
        // 获取请求信息
        uint32_t uid = request->uid();
        string pwd = request->pwd();
        // 做本地业务
        login(uid, pwd);
        // 设置响应参数
        ResultCode *res = response->mutable_result();
        res->set_errcoed(0);
        res->set_errmsg("");
        response->set_success(true);
        // 框架所提供的回调，完成调用后发送响应消息
        done->Run();
    }

    virtual void Regist(
        ::google::protobuf::RpcController *controller,
        const ::example::RegistRequest *request,
        ::example::RegistResponse *response,
        ::google::protobuf::Closure *done) override {
        // 直接省略一些
        ResultCode *res = response->mutable_result();
        res->set_errcoed(0);
        res->set_errmsg("");
        response->set_success(true);
        response->set_uid(1);
        done->Run();
    }
};

int main(int argc, char **argv) {
    string ip = argv[1];
    uint16_t port = atoi(argv[2]);
    InetAddress listenAddr(ip, port);
    EventLoop loop;

    RpcProvider provider(&loop, listenAddr, "UserService");
    provider.NotifyService(new UserService{});

    provider.Run();
}