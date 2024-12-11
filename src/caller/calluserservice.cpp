#include "myrpc.hpp"
#include "rpc.pb.h"
#include "rpcchannel.hpp"
#include <iostream>
using namespace std;
using namespace fixbug;

int main(int argc, char **argv) {
    // 框架的初始化
    MyRpcApplication::getInstance()->Init(argc, argv);

    // rpc请求参数
    LoginRequest request;
    request.set_userid(13);
    request.set_password("123456");
    // rpc请求响应
    LoginResponse response;

    // 调用远程rpc方法
    UserServiceRpc_Stub stub(new MyRpcChannel());
    RpcController controller;
    // stub.Login实际上调用的是RpcChannel::callMethod, 集中来做所有rpc方法调用的参数序列化和网络发送
    stub.Login(&controller, &request, &response, nullptr);

    // controller保存远程rpc调用的结果
    if (controller.Failed())
        cout << controller.ErrorText() << endl;
    else {
        // 读取rpc请求的响应数据
        cout << response.result().errmsg() << endl;
        if (!response.result().errcode()) {
            if (response.getfriendlistresponse().result().errcode())
                cout << response.getfriendlistresponse().result().errmsg() << endl;
            else {
                int size = response.getfriendlistresponse().friends_size();
                for (int i = 0; i < size; ++i) {
                    cout << response.getfriendlistresponse().friends(i) << endl;
                }
            }
        }
    }

    return 0;
}