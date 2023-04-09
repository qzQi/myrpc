#include<string>
#include"test.pb.h"
#include<iostream>

using namespace testProto;
using namespace std;

int main(){
    LoginRequest req;
    req.set_name("qzy");
    req.set_pwd("123456");

    string str;
    // 对login数据进行序列化
    // if(req.SerializeToString(&str)){
    //     cout<<str<<endl;
    // }
    str=req.SerializeAsString();
    cout<<str<<endl;

    // 对获取的数据进行反序列化,ParseFrom...
    LoginRequest getReq;
    if(getReq.ParseFromString(str)){
        // 向使用结构体一样使用，非常的方便
        cout<<getReq.name()<<endl;
        cout<<getReq.pwd()<<endl;
    }

    cout<<"对响应数据进行序列化/反序列化"<<endl;

    LoginResponse response;
    response.set_errcode(1);
    response.set_errmsg("no err");
    response.set_success(true);

    string respStr=response.SerializeAsString();
    cout<<respStr<<endl;

    LoginResponse parseResp;
    if(parseResp.ParseFromString(respStr)){
        cout<<parseResp.errcode()<<" "<<parseResp.errmsg()<<" "<<parseResp.success()<<endl;
    }
    
}