#pragma once
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
using namespace std;

// 框架中的配置文件加载类
class RpcConfig {
public:
    // 解析并加载配置文件的方法
    void LoadConfigFile(const string &config_file);
    // 查询配置信息
    string Load(const string &key);

private:
    unordered_map<string, string> _configMap;
};