/** 
 * @author: yuyuyuj1e 807152541@qq.com
 * @github: https://github.com/yuyuyuj1e
 * @csdn: https://blog.csdn.net/yuyuyuj1e
 * @date: 2023-02-27 18:25:09
 * @last_edit_time: 2023-02-28 16:03:23
 * @file_path: /Cpp-Web-Server/SelectDispatcher.h
 * @description: SelectDispatcher 头文件
 */

#pragma once
#include <string>
#include <sys/select.h>
#include "Channel.h"
#include "EventLoop.h"
#include "Dispatcher.h"

/** 
 * @description: 继承自抽象类 Dispatcher，底层通信模型为 select
 * @description: select 检测以线性方式进行，由于会频繁在用户区和内核区进行拷贝，其开销会随着文件描述符数量的增加而增大
 * @description: select 检测上限是 1024，及时超过上限内核也会强制设置为 1024
 * @description: select 可以跨平台使用
 */
class SelectDispatcher : public Dispatcher {
private:
	fd_set m_read_set;  // 文件描述符集合，只检测该集合中的读缓冲区，传入传出参数
	fd_set m_write_set;  // 文件描述符集合，只检测该集合中的写缓冲区，传入传出参数
	const int m_max_size = 1024;  // 最多可检测文件描述符的数量

private:
	void setFdSet();  // 添加文件描述符
	void clearFdSet();  // 移除文件描述符

public:
	SelectDispatcher(EventLoop* evLoop);
	~SelectDispatcher() = default;  // 关闭 fd 或者释放内存

	int add() override;  // 添加
	int remove() override;  // 删除
	int modify() override;  // 修改
	int dispatch(int timeout = 2) override;  // 事件检测 timeout: 单位 s
};