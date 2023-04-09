#include "../test.pb.h"
#include <string>
#include <iostream>

using namespace std;
using namespace testV3;

// 在单机里面的userservice类，提供本地服务的调用：Login/GetFriendLists
// 如何把本地的服务改为支持rpc调用的方法？
// 1、先把要调用的方法使用proto声明一下

class UserService : public UserServiceRpc {
  public:
    // 本地的方法,用户登录需要name/pwd
    // 如何让远程的用户，像调用本地方法一杨调用这个login？
    bool Login(const string &name, const string &pwd) {
        cout << "name: " << name << endl;
        cout << "pwd: " << pwd << endl;
        return true;
    }

    // override基类的login方法（我们proto文件指明的）
    // 把服务改为rpc调用，需要把调用者的请求进行反序列化
    // 这个代码是由框架直接调用的,我们这里仅仅是以业务实例一下
    void Login(
        ::google::protobuf::RpcController *controller,
        const ::testV3::LoginRequest *request,
        ::testV3::LoginResponse *response,
        ::google::protobuf::Closure *done) {
        // 可以看到request是const的，但是response是可改变的
        done->Run();
    }
};