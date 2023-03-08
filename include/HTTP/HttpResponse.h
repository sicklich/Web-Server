/** 
 * @author: yuyuyuj1e 807152541@qq.com
 * @github: https://github.com/yuyuyuj1e
 * @csdn: https://blog.csdn.net/yuyuyuj1e
 * @date: 2023-02-27 18:25:07
 * @last_edit_time: 2023-03-08 10:06:36
 * @file_path: /CC/include/HTTP/HttpResponse.h
 * @description: HttpResponse 模块头文件
 */

#pragma once
#include "Buffer.h"
#include <map>
#include <functional>

/** 
 * @description: 状态码枚举类，用于表示对于请求的响应状态
 */
enum class StatusCode {
	UNKNOWN,
	OK = 200,
	MOVEDPERMANMENTLY = 301,
	MOVEDTEMPORARILY = 302,
	BADREQUEST = 400,
	NOTFOUND = 404
};

/** 
 * @description: 用于组织回复客户端的数据
 */
class HttpResponse {
private:
	// 状态码和描述的对应关系
	const std::map<int, std::string> m_info = {
		{200, "OK"},
		{301, "MovedPermanmently"},
		{302, "MovedTemporarily"},
		{400, "BadRequest"},
		{404, "NotFound"}
	};
	// 状态行：状态码 描述 版本
	StatusCode m_status_code;  // 状态码
	std::string m_file_name;  // 响应文件
	std::map<std::string, std::string> m_headers;  // 响应头 —— 键值对

	std::function<void(std::string, Buffer*, int)> sendDataFunc;

public:
	HttpResponse();
	~HttpResponse() = default;

	void addHeader(const std::string key, const std::string value);  // 添加响应头
	void prepareHeadMsg(Buffer* send_buffer, int socket);  // 组织 http 响应头数据
	
	inline void setFileName(std::string name);
	inline void setStatusCode(StatusCode code);
	inline void setFunc(std::function<void(std::string, Buffer*, int)> func);
};

inline void HttpResponse::setFileName(std::string name) { 
	m_file_name = name; 
}

inline void HttpResponse::setStatusCode(StatusCode code) {
	m_status_code = code; 
}

inline void HttpResponse::setFunc(std::function<void(std::string, Buffer*, int)> func) {
	sendDataFunc = func;
}