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
    // 重写父类UserServiceRpc的Login方法
    void Login(::google::protobuf::RpcController *controller,
               const ::fixbug::LoginRequest *request,
               ::fixbug::LoginResponse *response,
               ::google::protobuf::Closure *done) {
        // 获取框架上报的请求参数执行本地业务
        int id = request->userid();
        string pwd = request->password();
        Login(id, pwd, response);

        // 执行回调
        done->Run();
    }

    void Regist(::google::protobuf::RpcController *controller,
                const ::fixbug::RegistRequest *request,
                ::fixbug::RegistResponse *response,
                ::google::protobuf::Closure *done) {
        string name = request->username();
        string pwd = request->password();
        Regist(name, pwd, response);

        done->Run();
    }

    // 重写父类UserServiceRpc的Loginout方法
    virtual void Loginout(::google::protobuf::RpcController *controller,
                          const ::fixbug::LoginoutRequest *request,
                          ::fixbug::LoginoutResponse *response,
                          ::google::protobuf::Closure *done) {
        int id = request->userid();
        Loginout(id, response);

        done->Run();
    }

private:
    // 本地的Login方法
    void Login(int id, string pwd, ::fixbug::LoginResponse *response) {
        // 写入响应数据(登录结果)
        ResultCode *rc = response->mutable_result();
        // 查询数据库
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

                FriendAndGroupServiceRpc_Stub stub(new MyRpcChannel());

                // 调用远程GetFriendList方法
                GetFriendListRequest friend_request;
                friend_request.set_userid(id);
                GetFriendListResponse friend_response;

                RpcController friend_controller;
                stub.GetFriendList(&friend_controller, &friend_request, &friend_response, nullptr);
                if (!friend_controller.Failed()) {
                    GetFriendListResponse *res = response->mutable_getfriendlistresponse();
                    *res = friend_response;
                }

                // 调用远程GetGroupList方法
                GetGroupListRequest group_request;
                group_request.set_userid(id);
                GetGroupListResponse group_response;

                RpcController group_controller;
                stub.GetGroupList(&group_controller, &group_request, &group_response, nullptr);
                if (!group_controller.Failed()) {
                    GetGroupListResponse *res2 = response->mutable_getgrouplistresponse();
                    *res2 = group_response;
                }

                ChatServiceRpc_Stub stub2(new MyRpcChannel());

                // 调用远程GetOfflineMsg方法
                GetOfflineMsgRequest offmsg_request;
                offmsg_request.set_userid(id);
                GetOfflineMsgResponse offmsg_response;

                RpcController offmsg_controller;
                stub2.GetOfflineMsg(&offmsg_controller, &offmsg_request, &offmsg_response, nullptr);
                if (!offmsg_controller.Failed()) {
                    GetOfflineMsgResponse *res3 = response->mutable_getofflinemsgresponse();
                    *res3 = offmsg_response;
                }

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
    // 本地的Regist方法
    void Regist(string name, string pwd, ::fixbug::RegistResponse *response) {
        ResultCode *rc = response->mutable_result();
        User user;
        user.setName(name);
        user.setPassword(pwd);
        bool state = _useropr.insert(user);
        if (state) {
            rc->set_errcode(0);
            rc->set_errmsg("注册成功");
            response->set_userid(user.getId());
        } else {
            rc->set_errcode(1);
            rc->set_errmsg("注册失败");
        }
    }
    // 本地的Loginout方法
    void Loginout(int id, ::fixbug::LoginoutResponse *response) {
        ResultCode *rc = response->mutable_result();
        User user;
        user.setId(id);
        user.setState("offline");
        bool state = _useropr.updateState(user);
        if (state) {
            rc->set_errcode(0);
            rc->set_errmsg("用户已注销");
        } else {
            rc->set_errcode(1);
            rc->set_errmsg("注销失败");
        }
    }

    UserOpr _useropr;
};

int main(int argc, char **argv) {
    // 框架的初始化
    MyRpcApplication::getInstance()->Init(argc, argv);

    RpcProvider provider;
    provider.NotifyService(new UserService());
    provider.Run();

    return 0;
}