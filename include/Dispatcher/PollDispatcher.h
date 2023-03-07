/** 
 * @author: yuyuyuj1e 807152541@qq.com
 * @github: https://github.com/yuyuyuj1e
 * @csdn: https://blog.csdn.net/yuyuyuj1e
 * @date: 2023-02-27 18:25:07
 * @last_edit_time: 2023-02-28 15:35:57
 * @file_path: /Cpp-Web-Server/PollDispatcher.h
 * @description: PollDispatcher 头文件
 */

#pragma once
#include <string>
#include <sys/poll.h>
#include "Channel.h"
#include "EventLoop.h"
#include "Dispatcher.h"

/** 
 * @description: 继承自抽象类 Dispatcher，底层通信模型为 poll 
 * @description: poll 检测以线性方式进行，由于会频繁在用户区和内核区进行拷贝，其开销会随着文件描述符数量的增加而增大
 * @description: poll 没有最大文件描述符数量限制
 * @description: poll 只能在 Linux 平台使用
 */
class PollDispatcher : public Dispatcher {
private:
	int m_max_fd_sub;  // 最大有效文件描述复下标
	struct pollfd* m_fds;  // 每个委托 poll 检测的 fd 都对应一个 pollfd 结构体
	const int m_max_node = 1024;  //  检测数量

public:
	PollDispatcher(EventLoop* evLoop);
	~PollDispatcher();  // 关闭 fd 或者释放内存

	int add() override;  // 添加
	int remove() override;  // 删除
	int modify() override;  // 修改
	int dispatch(int timeout = 2) override;  // 事件检测 timeout: 单位 s
};