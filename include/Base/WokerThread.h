/** 
 * @author: yuyuyuj1e 807152541@qq.com
 * @github: https://github.com/yuyuyuj1e
 * @csdn: https://blog.csdn.net/yuyuyuj1e
 * @date: 2023-02-27 18:25:09
 * @last_edit_time: 2023-03-08 10:07:51
 * @file_path: /CC/include/Base/WokerThread.h
 * @description: 工作线程模块头文件
 */

#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>
#include "EventLoop.h"

/** 
 * @description: 工作线程，即子线程，由主线程统一管理，主要负责与客户端的通信；
 * @description: 一个工作线程负责一个反应堆模型，需要注意的是一个反应堆模型可以与多个客户端交流，与客户端交流的操作在 TcpConnection 模块中
 */
class WorkerThread {
private:
	std::thread* m_thread;  // 线程地址
	std::thread::id m_threadID;  // 线程 ID
	std::string m_name;  // 线程名
	std::mutex m_mutex;  // 互斥锁
	std::condition_variable m_cond;  // 条件变量
	EventLoop* m_event_loop;  // 反应堆模型

private:
	void running();

public:
	WorkerThread() = delete;
	WorkerThread(int index);
	~WorkerThread();

	void run();  // 启动子线程
	inline EventLoop* getEventLoop();  // 获取子线程的反应堆模型
};

inline EventLoop* WorkerThread::getEventLoop() {
	return m_event_loop;
}