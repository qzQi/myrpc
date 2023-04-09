#include<muduo/net/TcpServer.h>
#include<muduo/base/Logging.h>
#include<muduo/net/EventLoop.h>
#include<functional>

using namespace muduo::net;
using namespace muduo;
using namespace std;
using namespace std::placeholders;

class EchoServer{
public:
    EchoServer(InetAddress& listenAddr,EventLoop* loop):_server(loop,listenAddr,"echoServer"){
        _server.setConnectionCallback(std::bind(&EchoServer::onConnection,this,_1));
        _server.setMessageCallback(std::bind(&EchoServer::onMessage,this,_1,_2,_3));
        _server.setThreadNum(4);
    }
    void start(){
        // _server.setThreadNum(4);
        _server.start();
    }
private:
    TcpServer _server;
    EventLoop* _loop;
    void onConnection(const TcpConnectionPtr&conn){
        if(conn->disconnected()){
            LOG_INFO<<"remove a conn"<<conn->name();
        }
    }
    void onMessage(const TcpConnectionPtr&conn,Buffer* buff,Timestamp t){
        string str=buff->retrieveAllAsString();
        LOG_INFO<<"get msg from"<<conn->name();
        conn->send(str);
    }
};

int main(){
    InetAddress listenAddr("127.0.0.1",8000);
    EventLoop loop;
    EchoServer server(listenAddr,&loop);
    server.start();
    loop.loop();
    // loop.quit();
}