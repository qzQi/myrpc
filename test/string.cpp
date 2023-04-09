#include<iostream>
#include<string>

using namespace std;

int main(){
    string addr="127.0.0.1:12345";
    size_t index=addr.find(":");
    string ip=addr.substr(0,index);
    string port=addr.substr(index+1,addr.size()-index-1);
    cout<<"ip: "<<ip<<" port: "<<port<<endl;
}