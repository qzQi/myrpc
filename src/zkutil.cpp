#include "zkutil.h"
#include <iostream>

#include "mprpcinit.h"

#include <muduo/base/Logging.h>
using namespace muduo;
// 使用日志库进行打印日志，不要cout，使用muduo可以输出到文件

// 全局的watcher观察器   zkserver给zkclient的通知
// server创建session是一个异步的过程，等创建好session后进行回调
void global_watcher(
    zhandle_t *zh, int type, int state, const char *path, void *watcherCtx) {
    if (type == ZOO_SESSION_EVENT) // 回调的消息类型是和会话相关的消息类型
    {
        if (state == ZOO_CONNECTED_STATE) // zkclient和zkserver连接成功
        {
            sem_t *sem = (sem_t *)zoo_get_context(zh);
            sem_post(sem);
        }
    }
}

ZkClient::ZkClient() : m_zhandle(nullptr) {
}

ZkClient::~ZkClient() {
    if (m_zhandle != nullptr) {
        zookeeper_close(m_zhandle); // 关闭句柄，释放资源  MySQL_Conn
    }
}

// 连接zkserver
void ZkClient::Start() {
    // 获取zk的相关配置，这个应该写在配置文件里面
    // MpRpcInit instance=MpRpcInit::instance();
    MpRpcInit &instance = MpRpcInit::instance(); // 单例
    std::string host = instance.getHost();
    std::string port = instance.getPort();
    std::string connstr = host + ":" + port;

    /*
    zookeeper_mt：多线程版本
    zookeeper的API客户端程序提供了三个线程
    API调用线程
    网络I/O线程  pthread_create  poll
    watcher回调线程 pthread_create
    */
    // 需要链接zookeeper_mt库，
    m_zhandle = zookeeper_init(
        connstr.c_str(), global_watcher, 30000, nullptr, nullptr, 0);
    if (nullptr == m_zhandle) {
        LOG_ERROR << "zookeeper init error !";
        exit(EXIT_FAILURE);
    }

    sem_t sem;
    sem_init(&sem, 0, 0);
    zoo_set_context(m_zhandle, &sem);

    // CountDownLatch，等待zk的session进行建立
    sem_wait(&sem);
    LOG_INFO << "zk init success !";
}

void ZkClient::Create(
    const char *path, const char *data, int datalen, int state) {
    char path_buffer[128];
    int bufferlen = sizeof(path_buffer);
    int flag;
    // 先判断path表示的znode节点是否存在，如果存在，就不再重复创建了
    flag = zoo_exists(m_zhandle, path, 0, nullptr);
    if (ZNONODE == flag) // 表示path的znode节点不存在
    {
        // 创建指定path的znode节点了
        flag = zoo_create(
            m_zhandle, path, data, datalen, &ZOO_OPEN_ACL_UNSAFE, state,
            path_buffer, bufferlen);
        if (flag == ZOK) {
            LOG_INFO << "znode create success... path:" << path;
        } else {
            LOG_ERROR<<"znode create error... path:"<<path;
            exit(EXIT_FAILURE);
        }
    }
}


// 根据指定的path，获取znode节点的值，服务的发现
// 成功返回正确的值，失败返回空字符串
std::string ZkClient::GetData(const char *path)
{
    char buffer[64];
	int bufferlen = sizeof(buffer);
	int flag = zoo_get(m_zhandle, path, 0, buffer, &bufferlen, nullptr);
	if (flag != ZOK)
	{
		LOG_ERROR<<"get znode error... path:"<<path;
		return "";
	}
	else
	{
		return buffer;
	}
}