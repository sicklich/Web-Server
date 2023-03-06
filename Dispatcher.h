/** 
 * @author: yuyuyuj1e 807152541@qq.com
 * @github: https://github.com/yuyuyuj1e
 * @csdn: https://blog.csdn.net/yuyuyuj1e
 * @date: 2023-02-27 18:25:07
 * @last_edit_time: 2023-03-01 11:08:47
 * @file_path: /Cpp-Web-Server/Dispatcher.h
 * @description: 分发器模块头文件
 */
 
#pragma once
#include <string>
#include "Channel.h"

class EventLoop;  // 声明，表示该类存在

/** 
 * @description: Dispatcher 是抽象类，派生出 EpollDispatcher、PollDispatcher、SelectDispatcher 三个子类
 * @description: 调用时统一由 Dispatcher 指针操作，虽然三个子类具体操作不同，但是操作的结果是统一的
 */
class Dispatcher {
protected:
	Channel* m_channel;
	EventLoop* m_event_loop;
	std::string m_name = std::string();
public:
	Dispatcher(EventLoop* event_loop);
	virtual ~Dispatcher() = default;  // 关闭 fd 或者释放内存

	virtual int add() = 0;  // 添加
	virtual int remove() = 0;  // 删除
	virtual int modify() = 0;  // 修改
	virtual int dispatch(int timeout = 2) = 0;  // 事件检测 timeout: 单位 s

	inline void setChannel(Channel* channel);
};

inline void Dispatcher::setChannel(Channel* channel) {
	m_channel = channel;
}