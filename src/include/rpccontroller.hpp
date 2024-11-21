#pragma once
#include <google/protobuf/service.h>
#include <iostream>
#include <string>
using namespace std;

class RpcController : public google::protobuf::RpcController {
public:
    RpcController();
    void Reset();
    bool Failed() const;
    string ErrorText() const;
    void SetFailed(const std::string &reason);

    // 未实现具体的功能
    void StartCancel();
    bool IsCanceled() const;
    void NotifyOnCancel(google::protobuf::Closure *callback);

private:
    bool _failed;    // rpc方法执行过程中的状态
    string _errText; // rpc方法执行过程中的错误信息
};