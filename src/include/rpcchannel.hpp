#include "google/protobuf/descriptor.h"
#include "google/protobuf/message.h"
#include "google/protobuf/service.h"
#include <iostream>
#include <string>
using namespace std;

class MyRpcChannel : public google::protobuf::RpcChannel {
public:
    void CallMethod(const google::protobuf::MethodDescriptor *method,
                    google::protobuf::RpcController *controller,
                    const google::protobuf::Message *request,
                    google::protobuf::Message *response,
                    google::protobuf::Closure *done);
};