/** 
 * @author: yuyuyuj1e 807152541@qq.com
 * @github: https://github.com/yuyuyuj1e
 * @csdn: https://blog.csdn.net/yuyuyuj1e
 * @date: 2023-02-27 18:25:07
 * @last_edit_time: 2023-03-05 16:05:59
 * @file_path: /Cpp-Web-Server/EventLoop.cpp
 * @description: EventLoop 模块源文件
 */

#include "EventLoop.h"
#include <assert.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include "SelectDispatcher.h"
#include "EpollDispatcher.h"
#include "PollDispatcher.h"


/** 
 * @description: 主线程调用，用于唤醒子线程函数，通过本地通信向子线程发送一个消息，解除子线程阻塞
 */
void EventLoop::taskWakeup() {
	const char* msg = "wake up";
	write(m_socket_pair[0], msg, strlen(msg));
}

EventLoop::EventLoop() : EventLoop(std::string()) { }  // 委托构造函数

EventLoop::EventLoop(const std::string thread_name) {
	m_quit = true;  // 默认没有启动
	m_threadID = std::this_thread::get_id();  // 获取控制该反应堆模型的线程 ID
	m_thread_name = thread_name == std::string() ? "MainThread" : thread_name;
	m_dispatcher = new EpollDispatcher(this);  // 设置底层实现模型
	m_channel_map.clear();

	// 创建一对用于本地通信的套接字，用于激活被阻塞的线程
	int ret = socketpair(AF_UNIX, SOCK_STREAM, 0, m_socket_pair);
	if (ret == -1) {
		perror("socketpair");
		exit(0);
	}
	auto read_callback = std::bind(&EventLoop::readLocalMessage, this);  // 将读取本地信息的函数当作 raeacallback
	Channel* channel = new Channel(m_socket_pair[1], FDEvent::READEVENT, read_callback, nullptr, nullptr, this);
	// 将用于本地通信的 channel 添加到任务队列
	addTask(channel, ElemType::ADD);
}


/** 
 * @description: 启动反应堆模型，持续检测就绪文件描述符
 * @return {int} 成功返回 0；失败返回 -1
 */
int EventLoop::run() {
	// 比较线程 ID 是否正常, 非所有者
	if (m_threadID != std::this_thread::get_id()) {
		return -1;
	}

	// 启动反应堆模型
	m_quit = false;

	// 循环处理事件，检测并处理就绪文件描述符
	while (!m_quit) {
		m_dispatcher->dispatch(2);  // 阻塞函数，主线程调用唤醒函数后，子线程从此处解除阻塞
		processTaskQ();  // 此处是主线程调用唤醒函数后，子线程处理主线程给子线程添加的任务的动作，这个任务就是本地通信
	}
	return 0;
}

/** 
 * @description: 处理文件描述符对应事件
 * @param {int} fd: 文件描述符（对应其在 channel_map 中的下标）
 * @param {int} event: 处理事件
 * @return {int} 成功返回 0；失败返回 -1
 */
int EventLoop::eventActive(int fd, int event) {
	if (fd < 0) {
		return -1;
	}

	// 取出 channel
	Channel* channel = m_channel_map[fd];  // 通过 fd 找到 channel
	assert(channel->getSocket() == fd);

	// 处理文件描述符对应事件
	if (event & (int)FDEvent::READEVENT && channel->readCallback != NULL) {
		channel->readCallback(const_cast<void*>(channel->getArg()));  // 处理文件描述符的读事件
	}
	if (event & (int)FDEvent::WRITEEVENT && channel->writeCallback != NULL) {
		channel->writeCallback(const_cast<void*>(channel->getArg()));  // 处理文件描述符的写事件
	}
	return 0;
}

/** 
 * @description: 向线程任务队列（处理文件描述符）添加任务
 * @param {Channel*} channel: 封装了待操作文件描述符的对象 
 * @param {ElemType} type: 处理文件描述符的动作
 * @return {int} 成功返回 0；失败返回 -1
 */
int EventLoop::addTask(Channel* channel, ElemType type) {
	// step 1：添加任务
	m_mutex.lock();  // 加锁，保护共享资源

	// 创建新节点
	ChannelElement* task = new ChannelElement;
	task->channel = channel;
	task->type = type;
	m_taskQ.push(task);

	m_mutex.unlock();  // 解锁

	// step2：处理任务
	/*
	* 任务可能由当前线程添加，也可能由其他线程添加
	* 1. 由当前线程添加
	*	1). 子线程对通信文件描述符的操作
	*   2). 主线程建立新的连接，通过监听文件描述符获取通信文件描述符
	* 2. 由其他线程（主线程）添加
	*	1). 主线程将通信描述符添加给子线程，将通信工作交给子线程
	*/
	if (m_threadID == std::this_thread::get_id()) {  // 由当前线程添加
		processTaskQ();  // 线程自己处理任务
	}
	else {  // 非当前线程添加
		// 这种情况主要是由主线程给子线程添加，但是主线程无法知道子线程当前状态
		// 1. 子线程正在工作：无影响，只是往任务队列添加了一个不重要的任务
		// 2. 子线程被阻塞：唤醒子线程，如果不唤醒子线程，子线程阻塞在等待就绪文件描述符处（防止一直阻塞或者阻塞时间设置较长的情况）
		taskWakeup();  // 该函数就是发送一个不重要的消息，以触发文件描述符的读事件
	}
	return 0;
}

/** 
 * @description: 处理任务队列中的任务（添加、修改、删除）文件描述符
 * @return {int} 成功返回 0；失败返回 -1
 */
int EventLoop::processTaskQ() {
	while (!m_taskQ.empty()) {
		m_mutex.lock();  // 加锁，保护共享资源
		ChannelElement* node = m_taskQ.front();  // 取出节点
		m_taskQ.pop();
		m_mutex.unlock();  // 解锁

		Channel* channel = node->channel;
		// 处理动作
		if (node->type == ElemType::ADD) {  // 添加
			add(channel);
		}
		else if (node->type == ElemType::DELETE) {  // 删除
			remove(channel);
		}
		else if (node->type == ElemType::MODIFY) {  // 修改
			modify(channel);
		}

		delete node;
	}
	return 0;
}

/** 
 * @description: 添加文件描述符
 * @param {Channel*} channel: 封装文件描述符的管道
 * @return {int} 成功返回 0；失败返回 -1
 */
int EventLoop::add(Channel* channel) {
	int fd = channel->getSocket();  // 获取封装的文件描述符

	// 如果之前没有存储过该文件描述符，存储该文件描述符
	if (m_channel_map.find(fd) == m_channel_map.end()) {
		m_channel_map.insert(std::make_pair(fd, channel));  // 往 channel_map 添加该文件描述符
		m_dispatcher->setChannel(channel);  // 设置 dispatcher 的 channel，由于一个反应堆模型只有一个 dispatcher 因此需要告诉 dispatcher 需要操作哪个 channel
		int ret = m_dispatcher->add();  // 将文件描述符添加到对应的检测集合中
		return ret;
	}
	return -1;
}

/** 
 * @description: 移除文件描述符
 * @param {Channel*} channel: 封装文件描述符的管道
 * @return {int} 成功返回 0；失败返回 -1
 */
int EventLoop::remove(Channel* channel) {
	int fd = channel->getSocket();  // 获取文件描述符

	// 如果文件描述符不在记录的文件描述符映射中，移除失败
	if (m_channel_map.find(fd) == m_channel_map.end()) {
		return -1;
	}

	m_dispatcher->setChannel(channel);  // 设置 dispatcher 的 channel
	int ret = m_dispatcher->remove();  // 将文件描述符从对应检测集合中移除
	return ret;
}

/** 
 * @description: 修改文件描述符
 * @param {Channel*} channel: channel: 封装文件描述符的管道
 * @return {int} 成功返回 0；失败返回 -1
 */
int EventLoop::modify(Channel* channel) {
	int fd = channel->getSocket();  // 获取文件描述符

	// 检测是否存在 fd 和 channel 的键值对
	if (m_channel_map.find(fd) == m_channel_map.end()) {
		return -1;
	}

	m_dispatcher->setChannel(channel);  // 设置 dispatcher 的 channel
	int ret = m_dispatcher->modify();  // 修改检测集合中文件描述符检测事件
	return ret;
}

/** 
 * @description: 断开连接后，关闭文件描述符的操作
 * @param {Channel*} channel: channel: 封装文件描述符的管道
 * @return {int} 成功返回 0；失败返回 -1
 */
int EventLoop::freeChannel(Channel* channel) {
	auto it = m_channel_map.find(channel->getSocket());
	if (it == m_channel_map.end()) {
		return -1;
	}

	m_channel_map.erase(it);  // 删除 channel 和 fd 的对应关系
	close(channel->getSocket());  // 关闭文件描述符
	delete channel;  // 释放资源

	return 0;
}

/** 
 * @description: 读取本地通信文件描述符发送的信息
 * @param {void*} arg: 反应堆模型实例
 * @return {int} 成功返回 0；失败返回 -1
 */
int EventLoop::readLocalMessage(void* arg) {
	EventLoop* evLoop = static_cast<EventLoop*>(arg);
	char buf[256];
	int ret = read(evLoop->m_socket_pair[1], buf, sizeof(buf));
	return ret;
}
