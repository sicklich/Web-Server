/** 
 * @author: yuyuyuj1e 807152541@qq.com
 * @github: https://github.com/yuyuyuj1e
 * @csdn: https://blog.csdn.net/yuyuyuj1e
 * @date: 2023-02-27 18:25:09
 * @last_edit_time: 2023-03-16 17:12:09
 * @file_path: /CC/include/Net/TcpServer.h
 * @description: 服务器模块头文件
 */

#pragma once
#include "EventLoop.h"
#include "ThreadPool.h"
#include "Log.h"

/** 
 * @description: 服务器类
 */
class TcpServer {
private:
	int m_thread_num;  // 线程池线程数量
	EventLoop* m_main_event_loop;  // 主线程反应堆模型
	ThreadPool* m_thread_pool;  // 线程池
	int m_lfd;  // 用于监听的文件描述符
	unsigned short m_port;  // 监听端口号
	Log* m_log = Log::getInstance();  // 日志类

private:
	void setListen();  // 初始化监听器
	static int acceptConnection(void* arg);  // 建立连接

public:
	TcpServer(unsigned short port, int thread_num);
	~TcpServer() = default;

	void run();  // 启动服务器
};


