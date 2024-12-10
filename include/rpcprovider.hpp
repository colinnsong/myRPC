#pragma once
#include <functional>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/service.h>
#include <iostream>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpServer.h>
#include <string>
#include <unordered_map>
using namespace std;
using namespace muduo;
using namespace muduo::net;

// 框架中的rpc服务网络发布类
class RpcProvider {
public:
    // 框架提供的发布rpc方法的函数接口
    void NotifyService(google::protobuf::Service *service);

    // 启动rpc服务节点提供rpc服务
    void Run();

private:
    EventLoop _loop;

    // 单个rpc服务类型信息
    struct ServiceInfo {
        google::protobuf::Service *_service;                                          // 具体的rpc服务对象指针
        unordered_map<string, const google::protobuf::MethodDescriptor *> _methodMap; // rpc服务<方法名, 方法描述>映射表
    };
    // <服务名, 服务信息>映射表
    unordered_map<string, ServiceInfo> _serviceMap;

    // 专门处理用户连接和断开事件的回调函数
    void OnConnection(const TcpConnectionPtr &);
    // 专门处理用户读写事件的回调函数
    void OnMessage(const TcpConnectionPtr &, Buffer *, Timestamp);
    // rpc方法执行完之后的回调函数
    void OnRpcResponse(const TcpConnectionPtr &, google::protobuf::Message *);
};