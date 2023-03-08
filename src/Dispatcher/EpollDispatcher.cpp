/** 
 * @author: yuyuyuj1e 807152541@qq.com
 * @github: https://github.com/yuyuyuj1e
 * @csdn: https://blog.csdn.net/yuyuyuj1e
 * @date: 2023-02-27 18:25:07
 * @last_edit_time: 2023-03-08 10:02:02
 * @file_path: /CC/src/Dispatcher/EpollDispatcher.cpp
 * @description: EpollDispatcher 源文件
 */

#include "Dispatcher.h"
#include <unistd.h>
#include "EpollDispatcher.h"

/** 
 * @description: 
 * @param {int} op: 委托 epoll 检测的事件，EPOLLIN 读事件、EPOLLOUT 写事件、EPOLLERR 异常事件
 * @return {int} 成功返回 0；失败返回 -1
 */
int EpollDispatcher::epollCtl(int op) {
	// 获取对应事件
	struct epoll_event ev;
	ev.data.fd = m_channel->getSocket();  // 获取对应的文件描述符

	int events = 0;
	if (m_channel->getEvent() & static_cast<int>(FDEvent::READEVENT)) {  // 判断是否监听读事件
		events |= EPOLLIN;
	}
	if (m_channel->getEvent() & static_cast<int>(FDEvent::WRITEEVENT)) {  // 判断是否监听写事件
		events |= EPOLLOUT;
	}
	ev.events = events;
	
	// 管理红黑树上的文件描述符(添加、删除、修改)
	int ret = epoll_ctl(m_epfd, op, m_channel->getSocket(), &ev);
	return ret;
}

EpollDispatcher::EpollDispatcher(EventLoop* event_loop) : Dispatcher(event_loop) {
	// 创建 epfd 实例，通过一颗红黑树管理待检测集合
	m_epfd = epoll_create(1);  // epoll_create 参数被抛弃，只需要提供大于 0 的数字即可

	if (m_epfd == -1) {
		perror("epoll_create");
		exit(0);
	}
	m_events = new struct epoll_event[m_max_node];
	m_name = "Epoll";
}

EpollDispatcher::~EpollDispatcher() {
	delete[] m_events;
	close(m_epfd);
}

/** 
 * @description: 将文件描述符添加到 epfd 中，即添加到红黑树上
 * @return {int} 成功返回 0；失败返回 -1
 */
int EpollDispatcher::add() {
	int ret = epollCtl(EPOLL_CTL_ADD);
	if (ret == -1) {
		perror("epoll_ctl add");
		exit(0);
	}

	return ret;
}

/** 
 * @description: 将文件描述符从 epfd 中删除
 * @return {int} 成功返回 0；失败返回 -1
 */
int EpollDispatcher::remove() {
	int ret = epollCtl(EPOLL_CTL_DEL);
	if (ret == -1) {
		perror("epoll_ctl del");
		exit(0);
	}
	// 通过 channel 释放对应的 TcpConnection 资源
	m_channel->destroyCallback(const_cast<void*>(m_channel->getArg()));
	return ret;
}

/** 
 * @description: 修改 epfd 中文件描述符的检测事件
 * @return {int} 成功返回 0；失败返回 -1
 */
int EpollDispatcher::modify() {
	int ret = epollCtl(EPOLL_CTL_MOD);
	if (ret == -1) {
		perror("epoll_ctl mod");
		exit(0);
	}

	return ret;
}

/** 
 * @description: 检测 epoll 实例中就绪的文件描述符，并执行相应操作
 * @param {int} timeout: 阻塞时长，0 不阻塞，大于 0 如果没有已就绪的文件描述符阻塞相应秒数后返回，-1 一直阻塞直至有已就绪的文件描述符
 * @return {int} 成功检测到已就绪的文件描述符个数；函数超时阻塞被强制接触返回 0；失败返回 -1
 */
int EpollDispatcher::dispatch(int timeout) {
	// 检测就绪文件描述符 event 为传入传出参数，存储了已就绪的文件描述符信息，m_max_node 表示前者元素个数
	int count = epoll_wait(m_epfd, m_events, m_max_node, timeout * 1000);

	// 处理就绪的文件描述符
	for (int i = 0; i < count; ++i) {
		int events = m_events[i].events;
		int fd = m_events[i].data.fd;
		// 如果出现异常直接将该文件描述符从 epoll 树删除
		if (events & EPOLLERR || events & EPOLLHUP) {
			// ERR 对端断开连接，HUP 对端断开连接后继续发送数据
			remove();
		}
		// 处理对应读事件
		if (events & EPOLLIN) {
			m_event_loop->eventActive(fd, (int)FDEvent::READEVENT);
		}
		// 处理对应写事件
		if (events & EPOLLOUT) {
			m_event_loop->eventActive(fd, (int)FDEvent::WRITEEVENT);
		}
	}
	return count;
}
