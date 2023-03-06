/** 
 * @author: yuyuyuj1e 807152541@qq.com
 * @github: https://github.com/yuyuyuj1e
 * @csdn: https://blog.csdn.net/yuyuyuj1e
 * @date: 2023-02-27 18:25:07
 * @last_edit_time: 2023-03-01 16:33:23
 * @file_path: /Cpp-Web-Server/EventLoop.h
 * @description: EventLoop 模块头文件
 */

#pragma once
#include "Channel.h"
#include <thread>
#include <queue>
#include <map>
#include <mutex>

class Dispatcher;  // 声明

// 处理节点中的 channel 的方式
enum class ElemType:char {
	ADD,
	DELETE,
	MODIFY
};

// 定义任务队列的节点
struct ChannelElement {
	ElemType type;  // 如何处理节点中的 channel
	Channel* channel;
};


/** 
 * @description: 一个线程控制一个事件循环模型（反应堆模型），事件循环主要分为两类：主反应堆模型和子反应堆模型
 * @description: 由主线程控制的主事件循环模型（主反应堆模型）主要负责，监听连接请求，与客户端建立连接，并将随后通信任务交给子线程
 * @description: 由子线程控制的子反应堆模型，主要负责与客户端的通信，在断开连接时需要处理关闭连接的操作
 * @description: 一个反应堆模型可以与多个客户端进行通信，每次需要通过 m_taskQ 取出一个需要操作的 Channel 对象，该对象封装了一个文件描述符
 */
class EventLoop {
private:
	// 该指针指向子类的实例 poll epoll select
	Dispatcher* m_dispatcher;  // 底层实现方式

	std::queue<ChannelElement*> m_taskQ;  // 任务队列
	// std::queue<std::map<ElemTypem, Channel*>> m_taskQ;
	std::map<int, Channel*> m_channel_map;  // map

	// 线程相关
	std::thread::id m_threadID;  // 线程 ID
	std::string m_thread_name;  // 线程名称
	std::mutex m_mutex;  // 互斥锁

	int m_socket_pair[2];  // 存储本地通信的 fd，通过 socketpair 初始化
	bool m_quit;  // 退出标志

private:
	void taskWakeup();  // 唤醒线程处理任务

public:
	EventLoop();
	EventLoop(const std::string thread_name);
	~EventLoop() = default;

	int run();  // 启动反应堆模型
	int eventActive(int fd, int event);  // 处理激活的文件描述符
	int addTask(Channel* channel, ElemType type);  // 添加任务到任务队列
	int processTaskQ();  // 处理任务队列的任务

	// 处理 dispatcher 中的节点
	int add(Channel* channel);
	int remove(Channel* channel);
	int modify(Channel* channel);

	int freeChannel(Channel* channel);  // 释放 channel

	static int readLocalMessage(void* arg);  // 类静态函数，无需实例化对象也存在
	
	// 获取成员变量
	inline std::thread::id getThreadID();
};

inline std::thread::id EventLoop::getThreadID() {
	return m_threadID;
}
