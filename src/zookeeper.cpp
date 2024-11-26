#include "zookeeper.hpp"
#include "logger.hpp"
#include "myrpc.hpp"
#include <iostream>
#include <semaphore.h>
using namespace std;

// 全局的watcher观察器   zkserver给zkclient的通知
void global_watcher(zhandle_t *zh, int type, int state, const char *path, void *watcherCtx) {
    // 回调的消息类型是和会话相关的消息类型
    if (type == ZOO_SESSION_EVENT) {
        // zkclient和zkserver连接成功
        if (state == ZOO_CONNECTED_STATE) {
            sem_t *sem = (sem_t *)zoo_get_context(zh);
            sem_post(sem);
        }
    }
}

ZkClient::ZkClient() : _zhandle(nullptr) {}

ZkClient::~ZkClient() {
    if (_zhandle != nullptr) {
        // 关闭连接释放资源
        zookeeper_close(_zhandle);
    }
}

void ZkClient::Start() {
    string ip = MyRpcApplication::getInstance()->GetConfig().Load("zookeeperip");
    string port = MyRpcApplication::getInstance()->GetConfig().Load("zookeeperport");
    string addr = ip + ":" + port;
    /*
        创建客户端连接使用的是多线程异步依赖库zookeeper_mt, 分别创建以下三个线程:
        1.API调用线程, 也就是调用zookeeper_init, 能够返回表示能够创建zk客户端的句柄, 不表示能够连接zkserver;
        2.网络I/O线程, 连接zkserver, poll多路复用, 心跳机制
        3.watcher回调线程, 调用global_watcher, 如果连接成功唤醒主线程
    */
    _zhandle = zookeeper_init(addr.c_str(), global_watcher, 30000, nullptr, nullptr, 0);
    if (nullptr == _zhandle) {
        cout << "zookeeper_init error!" << endl;
        exit(1);
    }

    sem_t sem;
    sem_init(&sem, 0, 0);
    zoo_set_context(_zhandle, &sem);

    sem_wait(&sem);
    cout << "zookeeper_init success!" << endl;
}

void ZkClient::Create(const char *path, const char *data, int datalen, int state) {
    int flag;
    // 先判断path表示的znode节点是否存在, 如果存在就不再重复创建了
    flag = zoo_exists(_zhandle, path, 0, nullptr);
    // 表示path的znode节点不存在
    if (ZNONODE == flag) {
        // 创建指定path的znode节点
        char path_buffer[64];
        int bufferlen = sizeof(path_buffer);
        /*
            state用来控制创建临时节点还是永久节点:
            zookeeper在和节点保持心跳超时后会删除临时节点但是不会删除永久节点
        */
        flag = zoo_create(_zhandle, path, data, datalen,
                          &ZOO_OPEN_ACL_UNSAFE, state, path_buffer, bufferlen);
        if (flag == ZOK) {
            LOG_INFO("znode create success, path:%s", path);
        } else {
            LOG_ERROR("znode create fail, path:%s, flag:%d", path, flag);
            exit(1);
        }
    }
}

string ZkClient::GetData(const char *path) {
    char data[128];
    int datalen = sizeof(data);
    int flag = zoo_get(_zhandle, path, 0, data, &datalen, nullptr);
    if (flag != ZOK) {
        cout << "get znode error, path:" << path << endl;
        return "";
    } else
        return data;
}