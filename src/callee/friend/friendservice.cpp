#include "friendopr.hpp"
#include "groupopr.hpp"
#include "myrpc.hpp"
#include "rpc.pb.h"
#include "rpcprovider.hpp"
#include "useropr.hpp"
#include <iostream>
#include <string>
#include <vector>
using namespace std;
using namespace fixbug;

class FriendAndGroupService : public FriendAndGroupServiceRpc {
public:
    void GetFriendList(::google::protobuf::RpcController *controller,
                       const ::fixbug::GetFriendListRequest *request,
                       ::fixbug::GetFriendListResponse *response,
                       ::google::protobuf::Closure *done) {
        int id = request->userid();
        GetFriendList(id, response);

        done->Run();
    }

    void GetGroupList(::google::protobuf::RpcController *controller,
                      const ::fixbug::GetGroupListRequest *request,
                      ::fixbug::GetGroupListResponse *response,
                      ::google::protobuf::Closure *done) {
        int id = request->userid();
        GetGroupList(id, response);

        done->Run();
    }

private:
    void GetFriendList(int id, ::fixbug::GetFriendListResponse *response) {
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

    void GetGroupList(int id, ::fixbug::GetGroupListResponse *response) {
        vector<Group> groups = _groupopr.queryGroups(id);
        ResultCode *rc = response->mutable_result();
        if (groups.size() != 0) {
            rc->set_errcode(0);
            rc->set_errmsg("获取好友列表成功");
            for (Group &group : groups) {
                string *str = response->add_groups();
                *str = to_string(group.getId()) + " " + group.getName() + " " + group.getDesc();
            }
        } else {
            rc->set_errcode(1);
            rc->set_errmsg("群组列表为空");
        }
    }

    FriendOpr _friendopr;
    GroupOpr _groupopr;
};

int main(int argc, char **argv) {
    MyRpcApplication::getInstance()->Init(argc, argv);

    RpcProvider provider;
    provider.NotifyService(new FriendAndGroupService());
    provider.Run();

    return 0;
}