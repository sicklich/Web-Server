/** 
 * @author: yuyuyuj1e 807152541@qq.com
 * @github: https://github.com/yuyuyuj1e
 * @csdn: https://blog.csdn.net/yuyuyuj1e
 * @date: 2023-02-27 18:25:09
 * @last_edit_time: 2023-03-08 10:03:38
 * @file_path: /CC/src/Base/WokerThread.cpp
 * @description: 工作线程模块源文件
 */

#include "WokerThread.h"

/** 
 * @description: 子线程工作函数
 */
void WorkerThread::running() {
	m_mutex.lock();  // 等待主线程 wait 释放锁，
	m_event_loop = new EventLoop(m_name);  // 创建子线程的反应堆模型
	m_mutex.unlock();

	m_cond.notify_one();  // 唤醒主线程

	m_event_loop->run();  // 运行子反应堆模型
}

WorkerThread::WorkerThread(int index) {
	m_event_loop = nullptr;
	m_thread = nullptr;
	m_threadID = std::thread::id();
	m_name = "SubThread-" + std::to_string(index);
}

WorkerThread::~WorkerThread() {
	if (m_thread != nullptr) {
		delete m_thread;
	}
}

/** 
 * @description: 启动工作线程函数，工作线程只有在 run 之后才会创建子线程
 */
void WorkerThread::run() {
	// 创建子线程
	m_thread = new std::thread(&WorkerThread::running, this);  // 将类成员函数当作线程工作函数时需要传入 this 指针，表明这个函数是属于哪个对象的
	// 阻塞主线程，等待子线程创建反应堆模型
	std::unique_lock<std::mutex> locker(m_mutex);  // unique_lock 构造出来后就锁定互斥锁
	while (m_event_loop == nullptr) {
		m_cond.wait(locker);  // 主线程在此处被阻塞，同时解开 locker （也释放了 mutex）让程序继续进行（因此设置了 while 循环），等待被唤醒（唤醒后加锁 locker）
	}
	// 自动解锁
}
