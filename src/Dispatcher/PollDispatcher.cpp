/** 
 * @author: yuyuyuj1e 807152541@qq.com
 * @github: https://github.com/yuyuyuj1e
 * @csdn: https://blog.csdn.net/yuyuyuj1e
 * @date: 2023-02-27 18:25:07
 * @last_edit_time: 2023-03-08 10:01:46
 * @file_path: /CC/src/Dispatcher/PollDispatcher.cpp
 * @description: PollDispatcher 源文件
 */

#include "Dispatcher.h"
#include <sys/poll.h>
#include <unistd.h>
#include "PollDispatcher.h"

PollDispatcher::PollDispatcher(EventLoop* event_loop) : Dispatcher(event_loop) {
	m_max_fd_sub = 0;
	m_fds = new struct pollfd[m_max_node];
	for (int i = 0; i < m_max_node; ++i) {
		m_fds[i].fd = -1;
		m_fds[i].events = 0;
		m_fds[i].revents = 0;
	}
	m_name = "Poll";
}

PollDispatcher::~PollDispatcher() {
	delete[] m_fds;
}

/** 
 * @description: 将文件描述符添加到待检测集合中
 * @return {int} 成功返回 0；失败返回 -1
 */
int PollDispatcher::add() {
	// 获取对应事件
	int events = 0;
	if (m_channel->getEvent() & static_cast<int>(FDEvent::READEVENT)) {  // 判断是否监听读事件
		events |= POLLIN;
	}
	if (m_channel->getEvent() & static_cast<int>(FDEvent::WRITEEVENT)) {  // 判断是否监听写事件
		events |= POLLOUT;
	}

	// 找到空闲的位置放置 fd
	int i = 0;
	for (; i < m_max_node; ++i) {
		if (m_fds[i].fd == -1) {
			m_fds[i].fd = m_channel->getSocket();
			m_fds[i].events = events;
			m_max_fd_sub = i > m_max_fd_sub ? i : m_max_fd_sub;  // 更新最大有效下标
			break;
		}
	}

	// 如果没有空闲位置，添加文件描述符失败，返回 -1
	if (i >= m_max_node) {
		return -1;
	}

	return 0;
}

/** 
 * @description: 将文件描述符从待检测集合中删除
 * @return {int} 成功返回 0；失败返回 -1
 */
int PollDispatcher::remove() {
	int i = 0;
	for (; i < m_max_node; ++i) {
		if (m_fds[i].fd == m_channel->getSocket()) {
			m_fds[i].fd = -1;
			m_fds[i].events = 0;
			m_fds[i].revents = 0;
			break;
		}
	}
	// 通过 channel 释放对应的 TcpConnection 资源
	m_channel->destroyCallback(const_cast<void*>(m_channel->getArg()));

	// 未找到对应文件描述符
	if (i >= m_max_node) {
		return -1;
	}

	return 0;
}

/** 
 * @description: 修改待检测集合中的文件描述符的检测事件
 * @return {int} 成功返回 0；失败返回 -1
 */
int PollDispatcher::modify() {
	// 获取对应事件
	int events = 0;
	if (m_channel->getEvent() & static_cast<int>(FDEvent::READEVENT)) {  // 判断是否监听读事件
		events |= POLLIN;
	}
	if (m_channel->getEvent() & static_cast<int>(FDEvent::WRITEEVENT)) {  // 判断是否监听写事件
		events |= POLLOUT;
	}

	// 找到对应文件描述符
	int i = 0;
	for (; i < m_max_node; ++i) {
		if (m_fds[i].fd == m_channel->getSocket()) {
			m_fds[i].events = events;
			break;
		}
	}

	// 为找到文件描述符，返回 -1
	if (i >= m_max_node) {
		return -1;
	}

	return 0;
}


/** 
 * @description: 检测 poll 实例中就绪的文件描述符，并执行相应操作
 * @param {int} timeout: 阻塞时长，0 不阻塞检测集合中没有就绪文件描述符立马返回，大于 0 如果没有已就绪的文件描述符阻塞相应秒数后返回，-1 一直阻塞直至有已就绪的文件描述符
 * @return {int} 失败返回 -1，成功返回检测集合中就绪的文件描述符个数
 */
int PollDispatcher::dispatch(int timeout) {
	// poll 失败返回 -1，成功返回检测集合中就绪的文件描述符个数
	int count = poll(m_fds, m_max_fd_sub + 1, timeout * 1000);
	if (count == -1) {
		perror("poll");
		exit(0);
	}

	// 处理激活的文件描述符
	for (int i = 0; i < m_max_fd_sub + 1; ++i) {
		if (m_fds[i].fd == -1) {
			continue;
		}
		// 处理对应读事件
		if (m_fds[i].revents & POLLIN) {
			m_event_loop->eventActive(m_fds[i].fd, (int)FDEvent::READEVENT);
		}
		// 处理对应写事件
		if (m_fds[i].revents & POLLOUT) {
			m_event_loop->eventActive(m_fds[i].fd, (int)FDEvent::WRITEEVENT);
		}
	}

	return count;
}
