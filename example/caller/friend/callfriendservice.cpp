#include "myrpc.hpp"
#include "rpc.pb.h"
#include "rpcchannel.hpp"
#include <iostream>
#include <string>
using namespace std;
using namespace fixbug;

int main(int argc, char **argv) {
    MyRpcApplication::getInstance()->Init(argc, argv);

    GetFriendListRequest request;
    request.set_userid(0);
    GetFriendListResponse response;

    FriendServiceRpc_Stub stub(new MyRpcChannel());
    RpcController controller;
    stub.GetFriendList(&controller, &request, &response, nullptr);

    if (controller.Failed()) {
        cout << controller.ErrorText() << endl;
    } else {
        if (0 == response.result().errcode()) {
            int size = response.friends_size();
            for (int i = 0; i < size; ++i) {
                cout << "index:" << (i + 1) << " name:" << response.friends(i) << endl;
            }
        } else {
            cout << "rpc GetFriendsList response error : " << response.result().errmsg() << endl;
        }
    }
}