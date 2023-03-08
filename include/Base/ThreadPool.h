/** 
 * @author: yuyuyuj1e 807152541@qq.com
 * @github: https://github.com/yuyuyuj1e
 * @csdn: https://blog.csdn.net/yuyuyuj1e
 * @date: 2023-02-27 18:25:09
 * @last_edit_time: 2023-03-08 10:08:00
 * @file_path: /CC/include/Base/ThreadPool.h
 * @description: 线程池头文件
 */

#pragma once
#include "EventLoop.h"
#include "WokerThread.h"
#include <vector>

/** 
 * @description: 线程池类，主要管理主反应堆模型以及子线程
 */
class ThreadPool {
private:
	EventLoop* m_main_loop;  // 主线程的反应堆模型
	bool m_start;  // 判断线程池是否启动
	int m_thread_num;  // 线程池数量
	std::vector<WorkerThread*> m_worker_threads;  // 线程数组
	int m_index;

public:
	ThreadPool(EventLoop* main_loop, int count);
	~ThreadPool();

	void run();
	// 取出线程池中的某个子线程的反应堆实例
	EventLoop* takeWorkerEventLoop();
};
