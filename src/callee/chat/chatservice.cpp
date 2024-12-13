#include "myrpc.hpp"
#include "offmsgopr.hpp"
#include "rpc.pb.h"
#include "rpcprovider.hpp"
using namespace std;
using namespace fixbug;

class ChatService : public ChatServiceRpc {
public:
    void FriendChat(::google::protobuf::RpcController *controller,
                    const ::fixbug::FriendChatRequest *request,
                    ::fixbug::FriendChatResponse *response,
                    ::google::protobuf::Closure *done) {
        int id = request->friendid();
        string msg = request->message();
        FriendChat(id, msg, response);

        done->Run();
    }

    void GetOfflineMsg(::google::protobuf::RpcController *controller,
                       const ::fixbug::GetOfflineMsgRequest *request,
                       ::fixbug::GetOfflineMsgResponse *response,
                       ::google::protobuf::Closure *done) {
        int id = request->userid();
        GetOfflineMsg(id, response);

        done->Run();
    }

private:
    void FriendChat(int id, string msg, ::fixbug::FriendChatResponse *response) {
    }

    void GetOfflineMsg(int id, ::fixbug::GetOfflineMsgResponse *response) {
        vector<string> offlinemsg = _offmsgopr.query(id);
        ResultCode *rc = response->mutable_result();
        if (offlinemsg.size() != 0) {
            rc->set_errcode(0);
            rc->set_errmsg("获取离线消息成功");
            for (string &msg : offlinemsg) {
                string *str = response->add_offlinemsg();
                *str = msg;
            }
        } else {
            rc->set_errcode(1);
            rc->set_errmsg("离线消息为空");
        }
    }

    OffMsgOpr _offmsgopr;
};

int main(int argc, char **argv) {
    MyRpcApplication::getInstance()->Init(argc, argv);

    RpcProvider provider;
    provider.NotifyService(new ChatService(), true);
    provider.Run();

    return 0;
}