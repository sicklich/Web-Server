/** 
 * @author: yuyuyuj1e 807152541@qq.com
 * @github: https://github.com/yuyuyuj1e
 * @csdn: https://blog.csdn.net/yuyuyuj1e
 * @date: 2023-03-08 09:40:44
 * @last_edit_time: 2023-03-08 09:54:45
 * @file_path: /Cpp-Web-Server/include/Log/mLog.h
 * @description: 日志模块头文件
 */ 

#pragma once
#include <mutex>
#include <fstream>

enum class LogOpenMode {
    ADDTO = 1L << 0,  // std::ofstream::app
    WRITEONLY = 1L << 2, // std::ofstream::out
    NONE = 1L << 3
};

enum class TimeFormat {
    FULLA = 1L << 0,  // YYYY-MM-DD HH:MM:SS
    FULLB = 1L << 1,  // YYYY/MM/DD HH:MM:SS
    YMDA = 1L << 2,  // YYYY-MM-DD
    YMDB = 1L << 3,  // YYYY/MM/DD
    TIMEONLY = 1L << 4  // HH:MM:SS
};

class Log {
private:
    std::fstream m_log_fp;  // 日志文件
    std::string m_log_path;  // 日志文件路径
    std::string m_log_name;  // 日志文件名称
    size_t m_max_log_size;  // 日志文件大小
    std::recursive_mutex m_log_mutex;  // 日志文件互斥递归锁
    LogOpenMode m_log_open_mode;  // 日志文件打开方式
    TimeFormat m_time_format;  // 时间格式
    bool m_backup;  // 备份日志文件

public:
    Log(const size_t max_log_size = 2, const std::string log_path = "./Log", const TimeFormat tf = TimeFormat::FULLA, bool backup = true);
    ~Log();

    /* 接口 */
    void writeLog(const std::string);  // 不带时间
    void writeLog(const char*);  // 不带时间
    void writeLogWithTIme(const std::string);  // 带时间
    void writeLogWithTIme(const char*);  // 带时间
    std::string getCurrentTime();  // 获取当前时间
    void setTimeFormat(TimeFormat);  // 设置时间格式
    bool backupLog();  // 备份日志文件
    bool openLog(std::string name, const LogOpenMode mode = LogOpenMode::ADDTO);  // 打开日志文件
    void closeLog();  // 关闭日志文件
};