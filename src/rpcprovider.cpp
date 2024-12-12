#include "rpcprovider.hpp"
#include "logger.hpp"
#include "myrpc.hpp"
#include "rpcheader.pb.h"
using namespace fixbug;

void RpcProvider::NotifyService(google::protobuf::Service *service, bool longconn) {
    // 获取rpc服务对象的描述信息
    const google::protobuf::ServiceDescriptor *serviceDesc = service->GetDescriptor();
    // 获取rpc服务对象的名字
    string serviceName = serviceDesc->name();
    LOG_INFO("notify service %s", serviceName.c_str());
    // 获取rpc服务对象中rpc方法的数量
    int methodCnt = serviceDesc->method_count();

    _serviceMap[serviceName]._service = service;
    for (int i = 0; i < methodCnt; i++) {
        const google::protobuf::MethodDescriptor *methodDesc = serviceDesc->method(i);
        string methodName = methodDesc->name();
        LOG_INFO("notify method %s", methodName.c_str());
        _serviceMap[serviceName]._methodMap.insert({methodName, methodDesc});
    }

    _islongconn = longconn;
}

void RpcProvider::Run() {
    string ip = MyRpcApplication::getInstance()->GetConfig().Load("rpcserverip");
    uint16_t port = atoi(MyRpcApplication::getInstance()->GetConfig().Load("rpcserverport").c_str());
    InetAddress addr(ip, port);
    // 创建TcpServer对象
    TcpServer _server(&_loop, addr, "RpcProvider");
    // 注册用户连接和断开事件的回调函数
    _server.setConnectionCallback(bind(&RpcProvider::OnConnection, this, _1));
    // 注册用户读写事件回调函数
    _server.setMessageCallback(bind(&RpcProvider::OnMessage, this, _1, _2, _3));
    // 设置线程数量: 1个I/O线程, 3个工作线程
    _server.setThreadNum(4);

    // 连接zookeeper并将rpc方法所在的节点信息注册到zookeeper上, 实现rpc服务发现
    ZkClient zkclient;
    zkclient.Start();
    for (auto &sp : _serviceMap) {
        // 注册名为"/service_name"的节点
        string service_path = "/" + sp.first;
        zkclient.Create(service_path.c_str(), nullptr, 0, 0);
        for (auto &mp : sp.second._methodMap) {
            // 注册名为"/service_name/method_name"的节点, 其数据为rpc服务所在的主机ip+port
            string method_path = service_path + "/" + mp.first;
            char data[128];
            sprintf(data, "%s:%d", ip.c_str(), port);
            // ZOO_EPHEMERAL表示znode是一个临时性的节点
            zkclient.Create(method_path.c_str(), data, strlen(data), ZOO_EPHEMERAL);
        }
    }
    LOG_INFO("rpcserver start service at %s:%d", ip.c_str(), port);

    // 启动服务监听
    _server.start();
    // epoll_wait()
    _loop.loop();
}

void RpcProvider::OnConnection(const TcpConnectionPtr &conn) {
    // 连接成功
    if (conn->connected()) {
        cout << conn->peerAddress().toIpPort() << " connected..." << endl;
    } else {
        cout << conn->peerAddress().toIpPort() << " disconnected..." << endl;
        conn->shutdown();
    }
}

/*
    在rpc网络通信时, 请求方和服务方需要先协商好通信用的protobuf数据格式:
        service_name + method_name + args_size
    为了解决粘包, 在网络传输时的数据包格式为:
        请求头长度(header_size) + 请求头(service_name + method_name + args_size) + 请求体(args)
*/
void RpcProvider::OnMessage(const TcpConnectionPtr &conn, Buffer *buffer, Timestamp time) {
    // 拿到接收缓冲区中的字符流数据并转换成string
    string buf = buffer->retrieveAllAsString();
    // 请求头长度约定4个字节
    int32_t header_size = 0;
    buf.copy((char *)&header_size, 4, 0);
    // 根据请求头长度获取请求头, 反序列化数据拿到service_name + method_name + args_size + args
    string header = buf.substr(4, header_size);
    RpcHeader rpcHeader;
    if (!rpcHeader.ParseFromString(header)) {
        LOG_ERROR("parse header error!");
        return;
    }
    string service_name = rpcHeader.service_name();
    string method_name = rpcHeader.method_name();
    int32_t args_size = rpcHeader.args_size();
    string request_str = buf.substr(4 + header_size, args_size); // args参数就是request的内容

    // 根据service_name获取本机服务对象的所有信息
    auto it = _serviceMap.find(service_name);
    if (it == _serviceMap.end()) {
        LOG_ERROR("service %s is not exist!", service_name.c_str());
        return;
    }
    auto mit = it->second._methodMap.find(method_name);
    if (mit == it->second._methodMap.end()) {
        LOG_ERROR("method %s:%s is not exist!", service_name.c_str(), method_name.c_str());
        return;
    }
    google::protobuf::Service *service = it->second._service;
    const google::protobuf::MethodDescriptor *method = mit->second;

    // 生成请求方法的request和response
    google::protobuf::Message *request = service->GetRequestPrototype(method).New();
    if (!request->ParseFromString(request_str)) {
        LOG_ERROR("parse request error!");
        return;
    }
    google::protobuf::Message *response = service->GetResponsePrototype(method).New();

    // 在框架上调用rpc方法
    auto done = google::protobuf::NewCallback<RpcProvider, const muduo::net::TcpConnectionPtr &, google::protobuf::Message *>(this, &RpcProvider::OnRpcResponse, conn, response);
    service->CallMethod(method, nullptr, request, response, done);
    LOG_INFO("run rpc method %s and returned...", method_name.c_str());
}

void RpcProvider::OnRpcResponse(const TcpConnectionPtr &conn, google::protobuf::Message *response) {
    string response_str = "";
    if (!response->SerializePartialToString(&response_str)) {
        LOG_ERROR("serialize response error!");
        return;
    }
    conn->send(response_str);
    // 如果当前服务是短链接, 由rpc服务提供方断开连接
    if (!_islongconn)
        conn->shutdown();
}