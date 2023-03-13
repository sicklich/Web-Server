/** 
 * @author: yuyuyuj1e 807152541@qq.com
 * @github: https://github.com/yuyuyuj1e
 * @csdn: https://blog.csdn.net/yuyuyuj1e
 * @date: 2023-02-27 18:25:09
 * @last_edit_time: 2023-03-09 16:33:10
 * @file_path: /CC/src/Net/TcpConnection.cpp
 * @description: TcpConnection 模块源文件
 */

#include "TcpConnection.h"
#include "HttpRequest.h"
#include "DebugLog.h"


int TcpConnection::processRead(void* arg) {
	TcpConnection* conn = static_cast<TcpConnection*>(arg);
	// 接受数据
	int socket = conn->m_channel->getSocket();
	int count = conn->m_read_buffer->readData(socket);

	Debug("receive http request data: %s", conn->m_read_buffer->readPos());
	Log::addTaskStatic(conn->m_name + '\n' + conn->m_read_buffer->readPos(), 1, conn->m_log);
	
	if (count > 0) {
		// 接收到了 http 请求，解析 http 请求
		bool flag = conn->m_request->parseRequest(conn->m_read_buffer, conn->m_response, conn->m_write_buffer, socket);
		
		if (!flag) {
			// 解析失败
			std::string err_msg = "Http/1.1 400 Bad Request\r\n\r\n";
			conn->m_write_buffer->appendData(err_msg);
			Log::addTaskStatic(conn->m_name + '\n' + "400 Bad Request", 1, conn->m_log);
		}
	}
	// 断开连接
	conn->m_event_loop->addTask(conn->m_channel, ElemType::DELETE);
	Log::addTaskStatic(conn->m_name + '\n' + "closed", 0, conn->m_log);
	return 0;
}

int TcpConnection::processWrite(void* arg) {
	TcpConnection* conn = static_cast<TcpConnection*>(arg);
	// 发送数据
	int count = conn->m_write_buffer->sendData(conn->m_channel->getSocket());
	if (count > 0) {
		// 判断数据是否全部发送
		if (conn->m_write_buffer->readableSize() == 0) {
			// 1. 不再检测写事件 —— 修改 channel 中保存的事件
			conn->m_channel->writeEventEnable(false);
			// 2. 修改 dispathcer 检测的集合 —— 添加任务节点
			conn->m_event_loop->addTask(conn->m_channel, ElemType::MODIFY);
			// 3. 删除节点 —— 断开链接
			conn->m_event_loop->addTask(conn->m_channel, ElemType::DELETE);
		}
	}
	return 0;
}

int TcpConnection::destroy(void* arg) {
	TcpConnection* conn = static_cast<TcpConnection*>(arg);
	if (conn != nullptr) {
		delete conn;
	}
	return 0;
}

TcpConnection::TcpConnection(int fd, EventLoop* event_loop, Log* log) {
	m_event_loop = event_loop;
	m_read_buffer = new Buffer(10240);
	m_write_buffer = new Buffer(10240);
	// http
	m_request = new HttpRequest;
	m_response = new HttpResponse;
	m_name = "Connection-" + std::to_string(fd);
	m_channel = new Channel(fd, FDEvent::READEVENT, processRead, processWrite, destroy, this);
	event_loop->addTask(m_channel, ElemType::ADD);

	m_log = log;
}

TcpConnection::~TcpConnection() {
	if (m_read_buffer && m_read_buffer->readableSize() == 0 && m_write_buffer && m_write_buffer->readableSize() == 0) {
		delete m_read_buffer;
		delete m_write_buffer;
		delete m_request;
		delete m_response;
		m_event_loop->freeChannel(m_channel);
	}
}
