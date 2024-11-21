#include "rpccontroller.hpp"

RpcController::RpcController() {
    _failed = false;
    _errText = "";
}

void RpcController::Reset() {
    _failed = false;
    _errText = "";
}

bool RpcController::Failed() const {
    return _failed;
}

std::string RpcController::ErrorText() const {
    return _errText;
}

void RpcController::SetFailed(const std::string &reason) {
    _failed = true;
    _errText = reason;
}

// 目前未实现具体的功能
void RpcController::StartCancel() {}
bool RpcController::IsCanceled() const { return false; }
void RpcController::NotifyOnCancel(google::protobuf::Closure *callback) {}