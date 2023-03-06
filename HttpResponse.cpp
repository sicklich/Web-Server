/** 
 * @author: yuyuyuj1e 807152541@qq.com
 * @github: https://github.com/yuyuyuj1e
 * @csdn: https://blog.csdn.net/yuyuyuj1e
 * @date: 2023-02-27 18:25:07
 * @last_edit_time: 2023-03-05 16:09:14
 * @file_path: /Cpp-Web-Server/HttpResponse.cpp
 * @description: HttpResponse 模块源文件
 */

#include "HttpResponse.h"

HttpResponse::HttpResponse() {
	m_status_code = StatusCode::UNKNOWN;
	m_headers.clear();
	m_file_name = std::string();
	sendDataFunc = nullptr;
}

/** 
 * @description: 添加响应头
 * @param {string} key: 响应头 key 值
 * @param {string} value: 响应头 value 值
 */
void HttpResponse::addHeader(const std::string key, const std::string value) {
	if (key.empty() || value.empty()) {
		return;
	}

	m_headers.insert(std::make_pair(key, value));
}

/** 
 * @description: 组织响应头并发送，并调用发送数据的函数
 * @param {Buffer*} send_buffer: 存储待发送数据的缓冲区
 * @param {int} socket: 和客户端通信的文件描述符
 */
void HttpResponse::prepareMsg(Buffer* send_buffer, int socket) {
	char tmp[1024] = { 0 };

	// 组织响应行
	int code = static_cast<int>(m_status_code);
	sprintf(tmp, "HTTP/1.1 %d %s\r\n", code, m_info.at(code).data());
	send_buffer->appendData(tmp);

	// 组织响应头
	for (auto it = m_headers.begin(); it != m_headers.end(); ++it) {
		sprintf(tmp, "%s: %s\r\n", it->first.data(), it->second.data());
		send_buffer->appendData(tmp);
	}

	// 组织空行
	send_buffer->appendData("\r\n");

	// 发送响应头
	send_buffer->sendData(socket);

	// 发送数据
	sendDataFunc(m_file_name, send_buffer, socket);
}
