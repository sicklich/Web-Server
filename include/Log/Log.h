/** 
 * @author: yuyuyuj1e 807152541@qq.com
 * @github: https://github.com/yuyuyuj1e
 * @csdn: https://blog.csdn.net/yuyuyuj1e
 * @date: 2023-03-08 09:40:44
 * @last_edit_time: 2023-03-09 16:25:42
 * @file_path: /CC/include/Log/Log.h
 * @description: 日志模块头文件
 */ 

#pragma once
#include <mutex>
#include <fstream>
#include <queue>
#include <thread>

enum class LogMode : char {
    ADD = 1L << 0,  // std::ofstream::app
    WRITEONLY = 1L << 1, // std::ofstream::out
};

enum class TimeFormat : char {
    MODEA = 1L << 0,  // YYYY-MM-DD HH:MM:SS
    MODEB = 1L << 1,  // YYYY/MM/DD HH:MM:SS
};

/** 
 * @description: 日志文件类
 * @description: 可以记录日志时，可以选择是否记录当前时间
 * @description: 当日志文件过大时，会自动备份
 */
class Log {
private:
    std::fstream m_fp = std::fstream();  // 日志文件
    std::string m_path;  // 日志文件路径
    std::string m_name = std::string("log");  // 日志文件名称
    size_t m_max_size;  // 日志文件大小
    std::mutex m_mutex;  // 日志文件互斥锁
    LogMode m_mode;  // 日志文件打开方式
    TimeFormat m_time_format;  // 时间格式
    bool m_backup;  // 是否备份日志文件
    std::queue<std::pair<std::string, int>> m_taskQ;  // 任务队列
    bool m_start = false;  // 判断日志类是否已经启动
    std::thread* m_thread;  // 日志类线程

private:
    bool backup();  // 备份日志文件
    bool open(std::string name);  // 打开日志文件
    void close();  // 关闭日志文件
    std::string getCurrentTime();  // 获取当前时间

    void working();  // 线程工作函数
    void write(std::string str);  // 不带时间
    void writeTime(std::string str);  // 带时间

public:
    Log(size_t max_log_size = 2, std::string log_path = "/home/ubuntu/桌面/CC/Log",
        LogMode mode = LogMode::ADD, TimeFormat tf = TimeFormat::MODEA, bool backup = true);
    ~Log();
    
    static void addTaskStatic(std::string str, int flag, void* arg);  // 向任务队列添加任务
    void addTask(std::string str, int flag = 1);  // 向任务队列添加任务
    void run();  // 日志类启动函数
};