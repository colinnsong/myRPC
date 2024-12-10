#include "myrpc.hpp"
#include "rpc.pb.h"
#include "rpcchannel.hpp"
#include "rpcprovider.hpp"
#include "useropr.hpp"
#include <iostream>
#include <string>
using namespace std;
using namespace fixbug;

// RPC服务的发布端(RPC服务的提供者)业务类
class UserService : public UserServiceRpc {
public:
    // 本地的Login方法
    void Login(int id, string pwd, ::fixbug::LoginResponse *response) {
        // 写入响应数据(登录结果)
        ResultCode *rc = response->mutable_result();
        // 查询数据库
        UserOpr _useropr;
        User user = _useropr.query(id);
        if (user.getId() != -1 && user.getPassword() == pwd) {
            if (user.getState() == "online") {
                // 重复登录
                rc->set_errcode(1);
                rc->set_errmsg("请勿重复登录");
                return;
            } else {
                // 登录成功, 更新用户的状态
                user.setState("online");
                _useropr.updateState(user);
                rc->set_errcode(0);
                rc->set_errmsg("登录成功");
                return;
            }
        } else {
            if (user.getId() == -1) {
                // 用户名不存在
                rc->set_errcode(1);
                rc->set_errmsg("用户不存在");
                return;
            } else {
                // 密码错误
                rc->set_errcode(1);
                rc->set_errmsg("密码错误");
                return;
            }
        }
    }

    // 重写父类UserServiceRpc的Login方法
    void Login(::google::protobuf::RpcController *controller, const ::fixbug::LoginRequest *request,
               ::fixbug::LoginResponse *response, ::google::protobuf::Closure *done) {
        // 获取框架上报的请求参数执行本地业务
        int id = request->userid();
        string pwd = request->password();
        Login(id, pwd, response);

        // 请求执行远程的rpc方法
        GetFriendListRequest friend_request;
        friend_request.set_userid(0);
        GetFriendListResponse friend_response;

        FriendServiceRpc_Stub stub(new MyRpcChannel());
        RpcController friend_controller;
        stub.GetFriendList(&friend_controller, &friend_request, &friend_response, nullptr);

        // 写入响应数据(好友列表)
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