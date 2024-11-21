#include "myrpc.hpp"

void ShowArgsHelp() {
    cout << "correct format: command -i <configfile>" << endl;
}

void MyRpcApplication::Init(int argc, char **argv) {
    if (argc < 2) {
        ShowArgsHelp();
        exit(1);
    }
    int c = 0;
    string config_file;
    while ((c = getopt(argc, argv, "i:")) != -1) {
        switch (c) {
        case 'i':
            config_file = optarg;
            break;
        case '?':
            ShowArgsHelp();
            exit(1);
        case ':':
            ShowArgsHelp();
            exit(1);
        default:
            break;
        }
    }

    // 开始加载配置文件
    _config.LoadConfigFile(config_file.c_str());
    cout << "rpcserverip:" << _config.Load("rpcserverip") << endl;
    cout << "rpcserverport:" << _config.Load("rpcserverport") << endl;
    cout << "zookeeperip:" << _config.Load("zookeeperip") << endl;
    cout << "zookeeperport:" << _config.Load("zookeeperport") << endl;
}

RpcConfig &MyRpcApplication::GetConfig() {
    return _config;
}

MyRpcApplication *MyRpcApplication::getInstance() {
    static MyRpcApplication app;
    return &app;
}