#include "mprpcinit.h"
#include "json.hpp"

#include<fstream>

#include<iostream>

using std::ifstream;

using json=nlohmann::json;

// 加载conf文件并进行相关的配置读写，存储
MpRpcInit::MpRpcInit(){
    // 文件读写相关的不太会！读取文件然后存储到string
    ifstream in;
    in.open("conf.json");
    string bufStr;
    if(in.is_open()){
        // debug信息，还是建议多写log
        // std::cout<<"begin read conf.json"<<std::endl;
        string line;
        while(getline(in,line)){
            bufStr+=line;
        }
        in.close();
    }else{
        // 结果是文件都没打开
        std::cout<<"file not open"<<std::endl;
    }
    json js=json::parse(bufStr);
    // cout<<js.dump()<<endl;//使用日志输出。
    zkHost=js["zkHost"].get<string>();
    zkPort=js["zkPort"].get<string>();
}

MpRpcInit& MpRpcInit::instance(){
    // 创建一个局部的static对象，存储在bss区域，第一次用到才会进行初始化
    static MpRpcInit instance{};
    return instance;
}