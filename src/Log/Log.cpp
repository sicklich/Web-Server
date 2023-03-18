/** 
 * @author: yuyuyuj1e 807152541@qq.com
 * @github: https://github.com/yuyuyuj1e
 * @csdn: https://blog.csdn.net/yuyuyuj1e
 * @date: 2023-03-08 09:51:24
 * @last_edit_time: 2023-03-16 18:11:56
 * @file_path: /CC/src/Log/Log.cpp
 * @description: 日志模块源文件
 */

#include "Log.h"
#include <chrono>
#include <sys/stat.h>
#include <assert.h>
#include <iostream>

Log* Log::m_log = new Log("/home/ubuntu/桌面/CC/Log");

/** 
 * @param {string} log_path: 存放日志文件的目录，需要指定
 * @param {size_t} max_log_size: 日志文件的大小，超过该大小则切换另一个文件，默认为 2 M
 * @param {LogMode} mode: 日志记录方式，默认为 apt
 * @param {TimeFormat} tf: 日志记录所采用的时间格式，默认为 YYYY-MM-DD HH:MM:SS
 * @param {bool} backup: 是否进行日志备份，默认为 true
 */
Log::Log(std::string log_path, size_t max_log_size, LogMode mode, TimeFormat tf, bool backup) 
    : m_max_size(max_log_size)
    , m_path(log_path)
    , m_mode(mode)
    , m_time_format(tf)
    , m_backup(backup) 
{ 
    m_thread = nullptr;
}

Log::~Log() {
    this->close();
}

/**
 * @description: 打开日志文件
 * @param {string} name: 日志文件名称
 * @param {LogMode} mode: 日志文件打开方式
 * @return {bool}: 成功返回 true， 失败返回 false
 */
bool Log::open(std::string name) {
    // 关闭之前打开的日志文件
    this->close();

    // 打开日志文件
    m_name = name;
    std::string full_path = m_path + "/" + m_name + ".txt";

    // 判断日志写入方式
    if (m_mode == LogMode::ADD) {
        m_fp.open(full_path, std::ofstream::app);
    }
    else {
        m_fp.open(full_path, std::ofstream::out);
    }

    if (!m_fp) {  // 如果打开失败
        return false;
    }
    return true;
}

/**
 * @description: 关闭日志文件
 */
void Log::close() {
    if (m_fp) {
        m_fp.close();
        m_name = std::string("log");
        m_mode = LogMode::ADD;
        m_time_format = TimeFormat::MODEA;
    }
}

/**
 * @description: 备份日志文件
 * @return {bool}: 成功返回 true，返回 false
 */
bool Log::backup() {
    if (m_backup == false) {  // 判断是否需要进行备份
        return false;
    }

    /* 获取文件属性 */
	struct stat st;
    std::string full_path = m_path + "/" + m_name + ".txt"; 
	stat(full_path.data(), &st);  // 提供文件名字符串，获得文件属性结构体
	size_t file_size = st.st_size;  // 获取文件大小
    std::cout << file_size << " " << m_max_size * 1024 * 1024 << std::endl;
    /* 备份文件 */
    if (file_size < m_max_size * 1024 * 1024) {  // 判断是否需要进行备份
        return false;
    }

    /* 配置时间 */
    std::string now_t = getCurrentTime();

    /* 重命名 */
    m_fp.close();
    std::string new_name = m_path + "/" + now_t + ".txt";
    rename(full_path.data(), new_name.data());

    return open(m_name);
}

/**
 * @description: 获取指定时间格式的当前时间字符串
 * @return {std::string}: 指定时间格式的字符串
 */
std::string Log::getCurrentTime() {
    /* 获取当前时间戳 */
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::time_t now_t = std::chrono::system_clock::to_time_t(now);

    /* 转换时间戳 */
    struct tm now_st = *localtime(&now_t);
    now_st.tm_year = now_st.tm_year + 1900;
    ++now_st.tm_mon;

    char ctime[20];
    if (this->m_time_format == TimeFormat::MODEB) {
        snprintf(ctime, 20, "%04d/%02d/%02d %02d:%02d:%02d",
            now_st.tm_year, now_st.tm_mon, now_st.tm_mday, now_st.tm_hour, now_st.tm_min, now_st.tm_sec
        );
    }
    else {
        snprintf(ctime, 20, "%04d-%02d-%02d %02d:%02d:%02d",
            now_st.tm_year, now_st.tm_mon, now_st.tm_mday, now_st.tm_hour, now_st.tm_min, now_st.tm_sec
        );
    }

    return std::string(ctime);
}

/**
 * @description: 带时间写入日志
 * @param {string} str: 写入日志的内容
 */
void Log::writeTime(std::string str) {
    open(m_name);
    backup();  // 尝试备份
    std::string now_t = getCurrentTime();  // 获取时间 
    m_fp << now_t << " --->  " << str << '\n';  // 写入内容
}


/**
 * @description: 不带时间写入日志
 * @param {string} str: 写入日志的内容
 */
void Log::write(std::string str) {
    open(m_name);
    backup();  // 尝试备份
    m_fp << " --->  " << str << '\n';  // 写入内容
}



/** 
 * @description: 外部调用，向任务队列添加任务，静态函数
 * @param {string} str: 需要记录的日志内容
 * @param {int} flag: 是否记录时间，当数值给定数值大于 0 时记录时间，否则不记录时间，默认记录时间
 * @param {void*} arg: 指向 Log 对象的指针
 */
void Log::addTaskStatic(std::string str, int flag, void* arg) {
    Log* log = static_cast<Log*>(arg);
    assert(log->m_start);  // 必须启动
    log->m_mutex.lock();
    log->m_taskQ.push(std::make_pair(str, flag));  // 将任务加入工作队列中
    log->m_mutex.unlock();
}


/**
 * @description: 外部调用，向任务队列添加任务
 * @param {string} str: 需要记录的日志内容，默认值为 1
 * @param {int} flag: 是否记录时间，当数值给定数值大于 0 时记录时间，否则不记录时间，默认记录时间
 */
void Log::addTask(std::string str, int flag) {
    assert(m_start);  // 必须启动
    m_mutex.lock();
    m_taskQ.push(std::make_pair(str, flag));  // 将任务加入工作队列中
    m_mutex.unlock();
}


/**
 * @description: 日志对象工作函数
 */
void Log::working() {
    while (m_start || !m_taskQ.empty()) {
        while (!m_taskQ.empty()) {
            m_mutex.lock();  // 加锁，保护共享资源
            std::pair<std::string, int> pair = m_taskQ.front();
            m_taskQ.pop();
            m_mutex.unlock();  // 解锁

            if (pair.second > 0) {  // 如果标志大于 0，调用带时间的
                writeTime(pair.first);
            }
            else {
                write(pair.first);
            }
        }
    }
}


/** 
 * @description: 日志文件启动函数
 */
void Log::run() {
    assert(!m_start);  // 保证日志类没有被启动
    m_start = true;  // 启动日志类
    perror("start");
    m_thread = new std::thread(&Log::working, this);  // 构造线程
}