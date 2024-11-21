#pragma once
#include "rpcconfig.hpp"
#include "rpccontroller.hpp"
#include <iostream>
#include <string>
#include <unistd.h>
using namespace std;

// myrpc框架的基础类
class MyRpcApplication {
public:
    void Init(int argc, char **argv);
    RpcConfig &GetConfig();
    static MyRpcApplication *getInstance();

private:
    MyRpcApplication() {}
    MyRpcApplication(const MyRpcApplication &) = delete;
    MyRpcApplication(MyRpcApplication &&) = delete;

    RpcConfig _config;
};