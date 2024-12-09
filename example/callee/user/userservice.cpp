#include "myrpc.hpp"
#include "rpc.pb.h"
#include "rpcchannel.hpp"
#include "rpcprovider.hpp"
#include <iostream>
#include <string>
using namespace std;
using namespace fixbug;

// RPC服务的发布端(RPC服务的提供者)业务类
class UserService : public UserServiceRpc {
public:
    // 本地的Login方法
    bool Login(string name, string pwd) {
        cout << "name: " << name << endl;
        cout << "pwd: " << pwd << endl;
        return true;
    }

    // 重写父类UserServiceRpc的Login方法
    void Login(::google::protobuf::RpcController *controller, const ::fixbug::LoginRequest *request,
               ::fixbug::LoginResponse *response, ::google::protobuf::Closure *done) {
        // 获取框架上报的请求参数执行本地业务
        string name = request->name();
        string pwd = request->pwd();
        bool login_res = Login(name, pwd);

        // 请求执行远程的rpc方法
        GetFriendListRequest friend_request;
        friend_request.set_userid(0);
        GetFriendListResponse friend_response;

        FriendServiceRpc_Stub stub(new MyRpcChannel());
        RpcController friend_controller;
        stub.GetFriendList(&friend_controller, &friend_request, &friend_response, nullptr);

        // 写入响应数据
        ResultCode *rc = response->mutable_result();
        rc->set_errcode(0);
        rc->set_errmsg("no error");
        response->set_success(login_res);

        int size = friend_response.friends_size();
        for (int i = 0; i < size; ++i) {
            string *str = response->add_friends();
            *str = friend_response.friends(i);
        }

        // 执行回调
        done->Run();
    }
};

int main(int argc, char **argv) {
    // 框架的初始化
    MyRpcApplication::getInstance()->Init(argc, argv);

    RpcProvider provider;
    provider.NotifyService(new UserService());
    provider.Run();

    return 0;
}