#include <iostream>
#include <string>

#include "test.pb.h"

using namespace std;
using namespace testV2;

void testList() {
    GetFriendListsRequest request;
    request.set_userid(1);

    // 对请求str进行解析
    GetFriendListsRequest parseReq;
    parseReq.ParseFromString(request.SerializeAsString());
    cout << parseReq.userid() << endl;

    // 构造response
    GetFriendListsResponse response;
    ResultCode *result = response.mutable_result();
    result->set_errcode(1);
    result->set_errmsg("ok");
    // 开始添加列表元素了
    // 内部开辟好了空间，返回一个指针，进行添加即可
    User *user = response.add_friend_list();
    user->set_name("qzy");
    user->set_age(22);
    user->set_sex(User_Sex::User_Sex_MAN);
    // 为何cout显示总是缺失？看看能不能解析，直接cout会看不出来，但是解析没有问题
    cout << response.SerializeAsString() << endl;
    for (int i = 0; i < 3; i++) {
        user=response.add_friend_list();//需要先开辟空间才能添加
        user->set_name("qzy");
        user->set_age(22);
        user->set_sex(User::MAN);
    }

    cout << "对响应进行解析" << endl;
    // 客户端收到响应str，进行解析
    GetFriendListsResponse parseResp;
    parseResp.ParseFromString(response.SerializeAsString());
    // protobuf放入列表也支持range-base-for
    for (const User &u : parseResp.friend_list()) {
        cout << u.name() << endl;
        cout << u.age() << endl;
        if (!u.sex()) { cout << "male" << endl; }
    }
    // 当然可以通过下标访问
    cout<<"通过下标访问列表"<<endl;
    for(int i=0;i<parseResp.friend_list_size();i++){
        const User& u=parseResp.friend_list(i);
        cout << u.name() << endl;
        cout << u.age() << endl;
        if (u.sex()==User::MAN) { cout << "male" << endl; }
    }
}

int main() {
    testList();

    /*
    // 简单的测试，里面有一个复合类型
    LoginRequest request1;
    request1.set_name("qiZhiYun");
    request1.set_pwd("123456");

    string str1=request1.SerializeAsString();
    cout<<str1<<endl;

    // respon消息内部有一个复合类型,对内部复合类型的访问需要获取它的指针
    LoginResponse response1;
    // 释放，当然不是想释放它
    // response1.release_result();
    ResultCode* result;
    result=response1.mutable_result();
    result->set_errcode(0);
    result->set_errmsg("ok");
    response1.set_success(true);

    string str2=response1.SerializeAsString();
    cout<<str2<<endl;

    // 进行反序列化测试
    LoginResponse test1;
    if(test1.ParseFromString(str2)){
        if(test1.success()){
            cout<<"response ok"<<endl;
        }
        cout<<test1.result().errcode()<<endl;
        cout<<test1.result().errmsg()<<endl;
    }
    */
}
