#include "myrpc.hpp"
#include "rpcchannel.hpp"
#include "user.pb.h"
#include <iostream>
using namespace std;
using namespace fixbug;

int main(int argc, char **argv) {
    // 框架的初始化
    MyRpcApplication::getInstance()->Init(argc, argv);

    // rpc请求参数
    LoginRequest request;
    request.set_name("zhang san");
    request.set_pwd("123456");
    // rpc请求响应
    LoginResponse response;

    // 调用远程rpc方法
    UserServiceRpc_Stub stub(new MyRpcChannel());
    RpcController controller;
    stub.Login(&controller, &request, &response, nullptr);

    // controller保存远程rpc调用的结果
    if (controller.Failed())
        cout << controller.ErrorText() << endl;
    else {
        // 读取rpc请求的响应数据
        cout << response.success() << endl;
        cout << response.result().errcode() << endl;
        cout << response.result().errmsg() << endl;
    }

    return 0;
}