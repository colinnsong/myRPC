#include "friend.pb.h"
#include "myrpc.hpp"
#include "rpcprovider.hpp"
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
        int userid = request->userid();
        vector<string> friends = GetFriendList(userid);

        ResultCode *rc = response->mutable_result();
        rc->set_errcode(0);
        rc->set_errmsg("");
        for (string name : friends) {
            string *str = response->add_friends();
            *str = name;
        }

        int size = response->friends_size();
        for (int i = 0; i < size; ++i) {
            cout << "index:" << (i + 1) << " name:" << response->friends(i) << endl;
        }

        done->Run();
    }

private:
    vector<string> GetFriendList(int userid) {
        cout << "do GetFriendsList service! userid:" << userid << endl;
        vector<string> friends;
        friends.push_back("zhangsan");
        friends.push_back("lisi");
        friends.push_back("wangwu");
        return friends;
    }
};

int main(int argc, char **argv) {
    MyRpcApplication::getInstance()->Init(argc, argv);

    RpcProvider provider;
    provider.NotifyService(new FriendService());
    provider.Run();

    return 0;
}