/** 
 * @author: yuyuyuj1e 807152541@qq.com
 * @github: https://github.com/yuyuyuj1e
 * @csdn: https://blog.csdn.net/yuyuyuj1e
 * @date: 2023-02-27 18:25:07
 * @last_edit_time: 2023-03-05 16:06:04
 * @file_path: /Cpp-Web-Server/Channel.cpp
 * @description: Channel模块源文件
 */

#include "Channel.h"

Channel::Channel(int fd, FDEvent events, handleFunc readFunc, handleFunc writeFunc, handleFunc destroyFunc, void* arg) {
	m_arg = arg;
	m_fd = fd;
	m_events = static_cast<int>(events);
	readCallback = readFunc;
	writeCallback = writeFunc;
	destroyCallback = destroyFunc;
}

/** 
 * @description: 修改 fd 的写事件（检测 or 不检测）
 * @param {bool} flag: true 允许写事件，否则不允许
 */
void Channel::writeEventEnable(bool flag) {
	if (flag) {
		m_events |= static_cast<int>(FDEvent::WRITEEVENT);
	}
	else {
		m_events = m_events & ~static_cast<int>(FDEvent::WRITEEVENT);
	}
}

/** 
 * @description: 判断是否需要检测文件描述符的写事件
 * @return {bool} 监听写事件返回 true，否则返回 false
 */
bool Channel::isWriteEventEnable() {
	return m_events & static_cast<int>(FDEvent::WRITEEVENT);
}
