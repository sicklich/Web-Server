/** 
 * @author: yuyuyuj1e 807152541@qq.com
 * @github: https://github.com/yuyuyuj1e
 * @csdn: https://blog.csdn.net/yuyuyuj1e
 * @date: 2023-02-27 18:25:09
 * @last_edit_time: 2023-03-08 10:04:11
 * @file_path: /CC/src/Base/ThreadPool.cpp
 * @description: 线程池源文件
 */

#include "ThreadPool.h"
#include <assert.h>
#include <stdlib.h>


ThreadPool::ThreadPool(EventLoop* main_loop, int count) {
	m_index = 0;
	m_start = false;
	m_main_loop = main_loop;
	m_thread_num = count;
	m_worker_threads.clear();
}

ThreadPool::~ThreadPool() {
	for (auto item : m_worker_threads) {
		delete item;
	}
}

/** 
 * @description: 启动线程池
 */
void ThreadPool::run() {
	assert(!m_start && m_main_loop->getThreadID() == std::this_thread::get_id());  // 线程池没有被启动，并且由主线程启动

	m_start = true;  // 启动线程池


	// 构造子线程，如果设置的 m_thread_num <= 0，则表明使用单反应堆模型，即只使用主反应堆模型
	if (m_thread_num > 0) {
		for (int i = 0; i < m_thread_num; ++i) {
			WorkerThread* sub_thread = new WorkerThread(i);
			sub_thread->run();
			m_worker_threads.push_back(sub_thread);  // 加入子线程队列
		}
	}
}

/** 
 * @description: 获取一个子线程的反应堆实例，取出规则从0号开始以此取出一个，到最大数量后继续从0号取出
 * @return {EventLoop*} 返回一个子线程的反应堆实例
 */
EventLoop* ThreadPool::takeWorkerEventLoop() {
	assert(m_start);  // 线程池已经被启动
	assert(m_main_loop->getThreadID() == std::this_thread::get_id());  // 由主线程启动

	// 取出子线程的反应堆实例
	EventLoop* sub_event_loop = m_main_loop;  // 如果没有子线程，则使用主线程（此时为单反应堆模型）
	if (m_thread_num > 0) {
		sub_event_loop = m_worker_threads[m_index]->getEventLoop();
		m_index = ++m_index % m_thread_num;
	}
	return sub_event_loop;
}
