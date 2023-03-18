/** 
 * @author: yuyuyuj1e 807152541@qq.com
 * @github: https://github.com/yuyuyuj1e
 * @csdn: https://blog.csdn.net/yuyuyuj1e
 * @date: 2023-02-27 18:25:09
 * @last_edit_time: 2023-03-16 17:16:04
 * @file_path: /CC/include/Net/TcpConnection.h
 * @description: TcpConnection 模块头文件
 */

#pragma once
#include "EventLoop.h"
#include "Buffer.h"
#include "Channel.h"
#include "HttpResponse.h"
#include "HttpRequest.h"
#include "Log.h"

/** 
 * @description: TcpConnection 主要负责与客户端进行通信，接收客户端的信息
 * @description: 通过 HttpRequest 解析请求数据，在通过 HttpResponse 组织回复数据
 * @description: 需要注意的事，反应堆实例不属于 TcpConnection，而是 TcpConnection 属于反应堆实例，一个反应堆实例可以被不同的 TcpConnection 调用
 * @description: 一个 TcpConnection 只拥有一个 Channel 对象（封装了通信文件描述符），相当于每当有一个客户端建立连接，就会产生一个 TcpConnection 对象与其通信
 */
class TcpConnection {
private:
	EventLoop* m_event_loop;  // 操作 TcpConnection 的反应堆实例
	Channel* m_channel;  // 通信用的管道
	Buffer* m_read_buffer;  // 从客户端接收的数据
	Buffer* m_write_buffer;  // 准备发送给客户端的响应数据
	std::string m_name;  // TcpConnection 名称
	// http 
	HttpRequest* m_request;  // 解析客户端请求数据
	HttpResponse* m_response;  // 组织返还客户端的数据块

	Log* m_log = Log::getInstance();  // 日志类

private:
	static int processRead(void* arg);
	static int processWrite(void* arg);
	static int destroy(void* arg);

public:
	TcpConnection(int fd, EventLoop* event_loop);
	~TcpConnection();
};