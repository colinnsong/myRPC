#pragma once
#include <condition_variable>
#include <iostream>
#include <queue>
#include <string>
#include <thread>
#include <time.h>
using namespace std;

// 日志队列(后序可以使用Kafka作为日志队列)
template <typename T>
class LogQueue {
public:
    // 多个epoll的工作线程都会调用Push方法
    void Push(const T &data) {
        lock_guard<mutex> lock(_mtx);
        _que.push(data);
        _cv.notify_one();
    }
    // 只有写日志线程才会调用Pop方法
    T Pop() {
        unique_lock<mutex> lock(_mtx);
        while (_que.empty()) {
            // 日志队列如果为空则写日志线程wait
            _cv.wait(lock);
        }
        T data = _que.front();
        _que.pop();
        return data;
    }

private:
    queue<T> _que;
    mutex _mtx;
    condition_variable _cv;
};

enum LogLevel {
    INFO,
    ERROR
};

// 异步日志类
class Logger {
public:
    static Logger *getInstance();
    void SetLevel(LogLevel level);
    void Log(string msg);

private:
    Logger();
    Logger(const Logger &) = delete;
    Logger(Logger &&) = delete;
    LogLevel _loglevel;
    LogQueue<string> _logque;
};

// 定义宏函数
#define LOG_INFO(msg, ...)                       \
    {                                            \
        Logger *logger = Logger::getInstance();  \
        logger->SetLevel(INFO);                  \
        char str[1024];                          \
        snprintf(str, 1024, msg, ##__VA_ARGS__); \
        logger->Log(str);                        \
    }

#define LOG_ERROR(msg, ...)                      \
    {                                            \
        Logger *logger = Logger::getInstance();  \
        logger->SetLevel(ERROR);                 \
        char str[1024];                          \
        snprintf(str, 1024, msg, ##__VA_ARGS__); \
        logger->Log(str);                        \
    }
