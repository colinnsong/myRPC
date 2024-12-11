#include "friendopr.hpp"
#include "myrpc.hpp"
#include "rpc.pb.h"
#include "rpcprovider.hpp"
#include "useropr.hpp"
#include <iostream>
#include <string>
#include <vector>
using namespace std;
using namespace fixbug;

class FriendService : public FriendServiceRpc {
public:
    void GetFriendList(::google::protobuf::RpcController *controller,
                       const ::fixbug::GetFriendListRequest *request,
                       ::fixbug::GetFriendListResponse *response,
                       ::google::protobuf::Closure *done) override {
        int id = request->userid();
        GetFriendList(id, response);

        done->Run();
    }

private:
    void GetFriendList(int id, ::fixbug::GetFriendListResponse *response) {
        FriendOpr _friendopr;
        vector<User> friends = _friendopr.query(id);
        ResultCode *rc = response->mutable_result();
        if (friends.size() != 0) {
            rc->set_errcode(0);
            rc->set_errmsg("获取好友列表成功");
            for (User &user : friends) {
                string *str = response->add_friends();
                *str = to_string(user.getId()) + " " + user.getName() + " " + user.getState();
            }
        } else {
            rc->set_errcode(1);
            rc->set_errmsg("好友列表为空");
        }
    }
};

int main(int argc, char **argv) {
    MyRpcApplication::getInstance()->Init(argc, argv);

    RpcProvider provider;
    provider.NotifyService(new FriendService());
    provider.Run();

    return 0;
}