#include "myrpc.hpp"
#include "rpc.pb.h"
#include "rpcchannel.hpp"
#include "user.hpp"
#include <functional>
#include <iostream>
#include <thread>
#include <unordered_map>
using namespace std;
using namespace fixbug;

// 记录当前系统登录的用户信息
User currentUser;
// 控制主菜单页面程序
bool isMainMenuRunning = false;
// 记录登录状态
bool isLoginSuccess = false;

// "help" command handler
void help(int, string);
// "chat" command handler
void chat(int, string);
// "addfriend" command handler
void addfriend(int, string);
// "creategroup" command handler
void creategroup(int, string);
// "addgroup" command handler
void addgroup(int, string);
// "groupchat" command handler
void groupchat(int, string);
// "refresh" command handler
void refresh(int, string);
// "loginout" command handler
void loginout(int, string);

// 系统支持的客户端命令列表
unordered_map<string, string> commandMap = {
    {"help", "显示所有支持的命令[help]"},
    {"chat", "一对一聊天[chat:friendid:message]"},
    {"addfriend", "添加好友[addfriend:friendid]"},
    {"creategroup", "创建群组[creategroup:groupname:groupdesc]"},
    {"addgroup", "加入群组[addgroup:groupid]"},
    {"groupchat", "群聊[groupchat:groupid:message]"},
    {"refresh", "刷新[refresh]"},
    {"loginout", "注销[loginout]"}};

// 注册系统支持的客户端命令处理
unordered_map<string, function<void(int, string)>> commandHandlerMap = {
    {"help", help},
    {"chat", chat},
    {"addfriend", addfriend},
    {"creategroup", creategroup},
    {"addgroup", addgroup},
    {"groupchat", groupchat},
    {"refresh", refresh},
    {"loginout", loginout}};

void doLoginResponse(RpcController &controller, LoginResponse &response) {
    if (controller.Failed()) {
        cout << controller.ErrorText() << endl;
        isLoginSuccess = false;
    } else {
        // 读取rpc请求的响应数据
        cout << response.result().errmsg() << endl;
        if (!response.result().errcode()) {
            cout << "======================================================" << endl;
            cout << "----------------------friend list---------------------" << endl;
            if (response.getfriendlistresponse().result().errcode())
                cout << response.getfriendlistresponse().result().errmsg() << endl;
            else {

                int size = response.getfriendlistresponse().friends_size();
                for (int i = 0; i < size; ++i) {
                    cout << response.getfriendlistresponse().friends(i) << endl;
                }
            }
            cout << "----------------------group list----------------------" << endl;
            if (response.getgrouplistresponse().result().errcode())
                cout << response.getgrouplistresponse().result().errmsg() << endl;
            else {
                int size = response.getgrouplistresponse().groups_size();
                for (int i = 0; i < size; ++i) {
                    cout << response.getgrouplistresponse().groups(i) << endl;
                }
            }
            cout << "======================================================" << endl;
        }
        isLoginSuccess = true;
    }
}

void doRegistResponse(RpcController &controller, RegistResponse &response) {
    if (controller.Failed()) {
        cout << controller.ErrorText() << endl;
    } else {
        // 读取rpc请求的响应数据
        cout << response.result().errmsg() << endl;
        if (!response.result().errcode()) {
            cout << "用户id账号为: " << response.userid() << endl;
        }
    }
}

void readTaskHandler() {
}

// 主聊天页面程序
void mainMenu() {
    help(1, "");
    char buffer[1024] = {0};
    while (isMainMenuRunning) {
        cin.getline(buffer, 1024);
        string commandbuf(buffer);
        string command;
        int idx = commandbuf.find(":");
        if (-1 == idx) {
            command = commandbuf;
        } else {
            command = commandbuf.substr(0, idx);
        }
        auto it = commandHandlerMap.find(command);
        if (it == commandHandlerMap.end()) {
            cerr << "invalid input command!" << endl;
            continue;
        }

        it->second(0, commandbuf.substr(idx + 1, commandbuf.size() - idx));
    }
}

int main(int argc, char **argv) {
    // 启动子线程
    // thread readTask(readTaskHandler);
    // readTask.detach();

    // 框架的初始化
    MyRpcApplication::getInstance()->Init(argc, argv);

    // 主线程用于接收用户输入
    while (true) {
        // 显示首页面菜单:登录、注册、退出
        cout << "========================" << endl;
        cout << "1. login" << endl;
        cout << "2. register" << endl;
        cout << "3. quit" << endl;
        cout << "========================" << endl;
        cout << "choice:";
        int choice = 0;
        cin >> choice;
        cin.get(); // 读掉缓冲区残留的回车

        switch (choice) {
        // 登录业务
        case 1: {
            int id;
            string pwd;
            cout << "userid:";
            cin >> id;
            cin.get();
            cout << "password:";
            getline(cin, pwd);

            // rpc请求参数
            LoginRequest request;
            request.set_userid(id);
            request.set_password(pwd);
            // rpc请求响应
            LoginResponse response;

            // 调用远程rpc方法
            UserServiceRpc_Stub stub(new MyRpcChannel());
            RpcController controller;
            // stub.Login实际上调用的是RpcChannel::callMethod, 集中来做所有rpc方法调用的参数序列化和网络发送
            stub.Login(&controller, &request, &response, nullptr);

            doLoginResponse(controller, response);
            if (isLoginSuccess) {
                currentUser.setId(id);
                // 进入聊天主菜单页面
                isMainMenuRunning = true;
                mainMenu();
            }

            break;
        }

        // 注册业务
        case 2: {
            string name;
            string pwd;
            getline(cin, name);
            getline(cin, pwd);

            // rpc请求参数
            RegistRequest request;
            request.set_username(name);
            request.set_password(pwd);
            // rpc请求响应
            RegistResponse response;
            // 调用远程rpc方法
            UserServiceRpc_Stub stub(new MyRpcChannel());
            RpcController controller;
            stub.Regist(&controller, &request, &response, nullptr);

            doRegistResponse(controller, response);

            break;
        }

        case 3: {
            exit(0);
        }

        default:
            cerr << "invalid input!" << endl;
            break;
        }
    }
}

// "help" command handler
void help(int fd = 0, string str = "") {
    for (auto &p : commandMap) {
        cout << p.first << ">>" << p.second << endl;
    }
    cout << endl;
}

// "addfriend" command handler
void addfriend(int clientfd, string str) {
}

// "chat" command handler
void chat(int clientfd, string str) {
}

// "creategroup" command handler  groupname:groupdesc
void creategroup(int clientfd, string str) {
}

// "addgroup" command handler groupid
void addgroup(int clientfd, string str) {
}

// "groupchat" command handler   groupid:message
void groupchat(int clientfd, string str) {
}

// "refresh" command handler
void refresh(int clientfd, string str) {
}

// "loginout" command handler
void loginout(int clientfd, string str) {
    // rpc请求参数
    LoginoutRequest request;
    request.set_userid(currentUser.getId());
    // rpc响应数据
    LoginoutResponse response;
    // 调用远程rpc方法
    UserServiceRpc_Stub stub(new MyRpcChannel());
    RpcController controller;
    stub.Loginout(&controller, &request, &response, nullptr);

    if (controller.Failed()) {
        cout << controller.ErrorText() << endl;
    } else {
        // 读取rpc请求的响应数据
        cout << response.result().errmsg() << endl;
    }

    isMainMenuRunning = false;
}

// 获取系统时间（聊天信息需要添加时间信息）
string getCurrentTime() {
    auto tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct tm *ptm = localtime(&tt);
    char date[60] = {0};
    sprintf(date, "%d-%02d-%02d %02d:%02d:%02d",
            (int)ptm->tm_year + 1900, (int)ptm->tm_mon + 1, (int)ptm->tm_mday,
            (int)ptm->tm_hour, (int)ptm->tm_min, (int)ptm->tm_sec);
    return std::string(date);
}