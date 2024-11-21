#include "rpcconfig.hpp"

void RpcConfig::LoadConfigFile(const string &config_file) {
    ifstream input(config_file);
    if (!input.is_open()) {
        cout << "config file open failed!" << endl;
        exit(1);
    }
    string line = "";
    while (getline(input, line)) {
        // 如果是注释行或者空行则跳过
        if (line[0] == '#' || line.empty())
            continue;
        int idx = line.find("=");
        // 如果不是正确的配置信息行则跳过
        if (idx == string::npos)
            continue;
        string key = line.substr(0, idx);
        string value = line.substr(idx + 1, line.size() - idx);
        _configMap.insert({key, value});
    }
    input.close();
}

string RpcConfig::Load(const string &key) {
    auto it = _configMap.find(key);
    if (it == _configMap.end())
        return "";
    return it->second;
}