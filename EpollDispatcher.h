/** 
 * @author: yuyuyuj1e 807152541@qq.com
 * @github: https://github.com/yuyuyuj1e
 * @csdn: https://blog.csdn.net/yuyuyuj1e
 * @date: 2023-02-27 18:25:07
 * @last_edit_time: 2023-02-28 15:36:03
 * @file_path: /Cpp-Web-Server/EpollDispatcher.h
 * @description: EpollDispatcher 头文件
 */

#pragma once
#include <string>
#include <sys/epoll.h>
#include "Channel.h"
#include "EventLoop.h"
#include "Dispatcher.h"

/** 
 * @description: 继承自抽象类 Dispatcher，底层通信模型为 epoll 
 * @description: epoll 基于红黑树来管理待检测的集合，通过回调机制，效率高，处理效率不会随着检测集合的表达而下降
 * @description: epoll 没有最大文件描述符的限制，仅受到系统中近程能打开的最大文件数目限制
 * @description: 当多路复用的文件数量庞大、IO流量多的时候，通常使用 epoll
 * @description: epoll 只能在 Linux 平台下使用
 */
class EpollDispatcher : public Dispatcher {
private:
	const int m_max_node = 1024;  // epoll_event 数量
	int m_epfd;  // 文件描述符，可以访问 epoll 实例
	struct epoll_event* m_events;  // epoll 事件，用来修饰文件描述符，指定检测该未见描述符的什么事件

private:
	int epollCtl(int op);

public:
	EpollDispatcher(EventLoop* event_loop);
	~EpollDispatcher();  // 关闭 fd 或者释放内存

	int add() override;  // 添加
	int remove() override;  // 删除
	int modify() override;  // 修改
	int dispatch(int timeout = 2) override;  // 事件检测 timeout: 单位 s
};