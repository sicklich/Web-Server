/** 
 * @author: yuyuyuj1e 807152541@qq.com
 * @github: https://github.com/yuyuyuj1e
 * @csdn: https://blog.csdn.net/yuyuyuj1e
 * @date: 2023-02-27 18:25:09
 * @last_edit_time: 2023-03-05 16:05:46
 * @file_path: /Cpp-Web-Server/SelectDispatcher.cpp
 * @description: SelectDispatcher 源文件
 */

#include "Dispatcher.h"
#include "Channel.h"
#include <sys/select.h>
#include <unistd.h>
#include "SelectDispatcher.h"

/** 
 * @description: 将文件描述符添加到对应的检测集合中
 */
void SelectDispatcher::setFdSet() {
	if (m_channel->getEvent() & static_cast<int>(FDEvent::READEVENT)) {  // 判断是否监听读事件
		FD_SET(m_channel->getSocket(), &m_read_set);
	}
	if (m_channel->getEvent() & static_cast<int>(FDEvent::WRITEEVENT)) {  // 判断是否监听写事件
		FD_SET(m_channel->getSocket(), &m_write_set);
	}
}

/** 
 * @description: 将文件描述符从对应文件描述符中移除
 */
void SelectDispatcher::clearFdSet() {
	if (m_channel->getEvent() & static_cast<int>(FDEvent::READEVENT)) {  // 判断是否监听读事件
		FD_CLR(m_channel->getSocket(), &m_read_set);
	}
	if (m_channel->getEvent() & static_cast<int>(FDEvent::WRITEEVENT)) {  // 判断是否监听写事件
		FD_CLR(m_channel->getSocket(), &m_write_set);
	}
}

SelectDispatcher::SelectDispatcher(EventLoop* event_loop) : Dispatcher(event_loop) {
	// 将集合中所有文件描述符对应的标志位设置为 0，表示该集合没有添加任何文件描述符
	FD_ZERO(&m_read_set);  
	FD_ZERO(&m_write_set);
	m_name = "Select";
}


/** 
 * @description: 将文件描述符添加到对应检测集合中
 * @return {int} 成功返回 0；失败返回 -1
 */
int SelectDispatcher::add() {
	if (m_channel->getSocket() >= m_max_size) {
		return -1;
	}

	// 添加对应事件
	setFdSet();
	return 0;
}

/** 
 * @description: 将文件描述符从对应检测集合中移除
 * @return {int} 成功返回 0；失败返回 -1
 */
int SelectDispatcher::remove() {
	if (m_channel->getSocket() >= m_max_size) {
		return -1;
	}

	// 移除对应文件描述符
	clearFdSet();

	// 通过 channel 释放对应的 TcpConnection 资源
	m_channel->destroyCallback(const_cast<void*>(m_channel->getArg()));
	return 0;
}

/** 
 * @description: 修改检测集合中的文件描述符
 * @return {int} 成功返回 0；失败返回 -1
 */
int SelectDispatcher::modify() {
	setFdSet();
	clearFdSet();
	return 0;
}

/** 
 * @description: 检测 select 实例中就绪的文件描述符，并执行相应操作
 * @param {int} timeout: 阻塞时长，0 不阻塞检测集合中没有就绪文件描述符立马返回，大于 0 如果没有已就绪的文件描述符阻塞相应秒数后返回，-1 一直阻塞直至有已就绪的文件描述符
 * @return {int} 成功，返回集合中已就绪文件描述符总个数；失败返回 -1；超时，没有检测到就绪的文件描述符，返回 0
 */
int SelectDispatcher::dispatch(int timeout) {
	// 将 timeval 设置为 NULL 表示检测不到文件描述符就一直阻塞；固定时长表示指定阻塞指定长度；0 不阻塞
	struct timeval val;
	val.tv_sec = timeout;
	val.tv_usec = 0;
	fd_set rdtmp = m_read_set;
	fd_set wrtmp = m_write_set;
	int count = 0;

	if (timeout < 0) {
		count = select(m_max_size, &rdtmp, &wrtmp, NULL, NULL);
	}
	else {
		count = select(m_max_size, &rdtmp, &wrtmp, NULL, &val);
	}

	if (count == -1) {
		perror("select");
		exit(0);
	}

	// 处理激活的文件描述符
	for (int i = 0; i < m_max_size; ++i) {
		if (FD_ISSET(i, &rdtmp)) {
			m_event_loop->eventActive(i, static_cast<int>(FDEvent::READEVENT));
		}

		if (FD_ISSET(i, &wrtmp)) {
			m_event_loop->eventActive(i, static_cast<int>(FDEvent::WRITEEVENT));
		}
	}
	return count;
}
