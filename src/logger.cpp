#include "logger.hpp"

Logger::Logger() {
    // 启动写日志线程
    thread write([&]() {
        for (;;) {
            time_t t = time(nullptr);
            tm *now = localtime(&t);
            // 文件名
            char file_name[128];
            sprintf(file_name, "../log/%d-%d-%d-log.txt", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday);
            // 打开文件
            FILE *outfile = fopen(file_name, "a+");
            if (outfile == nullptr) {
                cout << "log file:" << file_name << " open failed!" << endl;
                exit(1);
            }
            // 组装日志内容
            string msg = _logque.Pop();
            char time_buf[128];
            sprintf(time_buf, "%d:%d:%d[%s]: ", now->tm_hour, now->tm_min, now->tm_sec, (_loglevel == INFO ? "INFO" : "ERROR"));
            msg.insert(0, time_buf);
            msg.append("\n");
            // 写入日志文件
            fprintf(outfile, "%s", msg.c_str());
            // 关闭文件
            fclose(outfile);
        }
    });
    write.detach();
}

Logger *Logger::getInstance() {
    static Logger logger;
    return &logger;
}

void Logger::SetLevel(LogLevel level) {
    _loglevel = level;
}

void Logger::Log(string msg) {
    _logque.Push(msg);
}