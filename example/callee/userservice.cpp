#include "user.pb.h"
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
        cout << "pwd: " << pwd;
        return true;
    }

    // 重写父类UserServiceRpc的Login方法
    void Login(::google::protobuf::RpcController *controller, const ::fixbug::LoginRequest *request,
               ::fixbug::LoginResponse *response, ::google::protobuf::Closure *done) {
        // 获取框架上报的请求参数执行本地业务
        string name = request->name();
        string pwd = request->pwd();
        bool login_res = Login(name, pwd);

        // 写入响应数据
        ResultCode *rc = response->mutable_result();
        rc->set_errcode(0);
        rc->set_errmsg("");
        response->set_success(login_res);

        // 执行回调
        done->Run();
    }
};

int main() {
    return 0;
}