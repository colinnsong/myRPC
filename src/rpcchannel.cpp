#include "rpcchannel.hpp"
#include "myrpc.hpp"
#include "rpcheader.pb.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
using namespace fixbug;

/*
    在rpc网络通信时, 请求方和服务方需要先协商好通信用的protobuf数据格式:
        service_name + method_name + args_size
    为了解决粘包, 在网络传输时的数据包格式为:
        请求头长度(header_size) + 请求头(service_name + method_name + args_size) + 请求体(args)
*/
void MyRpcChannel::CallMethod(const google::protobuf::MethodDescriptor *method,
                              google::protobuf::RpcController *controller,
                              const google::protobuf::Message *request,
                              google::protobuf::Message *response,
                              google::protobuf::Closure *done) {
    // 拿到service_name和method_name
    string service_name = method->service()->name();
    string method_name = method->name();

    // request即为请求体args, 序列化并拿到请求体长度args_size
    string request_str = "";
    if (!request->SerializePartialToString(&request_str)) {
        controller->SetFailed("serialize request error!");
        return;
    }
    int32_t args_size = request_str.size();

    // 生成请求头并序列化, 拿到请求头长度
    RpcHeader rpcHeader;
    rpcHeader.set_args_size(args_size);
    rpcHeader.set_method_name(method_name);
    rpcHeader.set_service_name(service_name);
    string header = "";
    if (!rpcHeader.SerializePartialToString(&header)) {
        controller->SetFailed("serialize header error!");
        return;
    }
    int32_t header_size = header.size();

    // 生成网络传输的字符数据
    string rpc_str = "";
    rpc_str.insert(0, string((char *)&header_size, 4));
    rpc_str += header;
    rpc_str += request_str;

    // 网络传输
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == clientfd) {
        char error[1024] = {0};
        sprintf(error, "socket create error, errno:%d", errno);
        controller->SetFailed(error);
        return;
    }
    // 这里我们直接指定了rpc服务节点的ip和port, 实际上应该使用zookeeper去管理服务和节点信息
    // string ip = MyRpcApplication::getInstance()->GetConfig().Load("rpcserverip");
    // uint16_t port = atoi(MyRpcApplication::getInstance()->GetConfig().Load("rpcserverport").c_str());

    // 连接并查询zookeeper中目标rpc方法所在节点的信息
    ZkClient zkclient;
    zkclient.Start();
    string method_path = "/" + service_name + "/" + method_name;
    std::string data = zkclient.GetData(method_path.c_str());
    if (data == "") {
        controller->SetFailed(method_path + " is not exist!");
        return;
    }
    int idx = data.find(":");
    if (idx == string::npos) {
        controller->SetFailed(method_path + " address is invalid!");
        return;
    }
    std::string ip = data.substr(0, idx);
    uint16_t port = atoi(data.substr(idx + 1, data.size() - idx).c_str());

    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ip.c_str());
    if (connect(clientfd, (sockaddr *)&server, sizeof(sockaddr_in)) == -1) {
        close(clientfd);
        char error[1024] = {0};
        sprintf(error, "connect error, errno:%d", errno);
        controller->SetFailed(error);
        return;
    }
    if (send(clientfd, rpc_str.c_str(), rpc_str.size(), 0) == -1) {
        close(clientfd);
        char error[1024] = {0};
        sprintf(error, "send error, errno:%d", errno);
        controller->SetFailed(error);
        return;
    }

    // 接收rpc请求响应数据
    char buffer[1024] = {0};
    int recv_size = recv(clientfd, buffer, 1024, 0);
    if (recv_size == -1) {
        close(clientfd);
        char error[1024] = {0};
        sprintf(error, "recive error, errno:%d", errno);
        controller->SetFailed(error);
        return;
    }

    // 反序列化并构造response对象
    // 从字符数组初始化成string可能会出现'\0'截断而造成字符串反序列化失败
    // 由于muduo网络库可以正确地将字符数组初始化成string, 故框架的服务端直接使用ParseFromString即可
    // string response_str(buffer, 0, recv_size);
    // if (!response->ParseFromString(response_str))
    if (!response->ParseFromArray(buffer, recv_size)) {
        close(clientfd);
        controller->SetFailed("parse response error!");
        return;
    }

    close(clientfd);
}