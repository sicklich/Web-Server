/** 
 * @author: yuyuyuj1e 807152541@qq.com
 * @github: https://github.com/yuyuyuj1e
 * @csdn: https://blog.csdn.net/yuyuyuj1e
 * @date: 2023-02-27 18:25:07
 * @last_edit_time: 2023-03-08 10:06:45
 * @file_path: /CC/include/HTTP/HttpRequest.h
 * @description: HttpRequest 模块头文件
 */

#pragma once
#include "Buffer.h"
#include "HttpResponse.h"
#include <map>

/** 
 * @description: 用于表示当前请求头的解析状态
 * @description: 由于解析状态只可能存在一种状态，因此无需将利用不同 bit 位进行区分
 */
enum class PrecessState :char {
    LINE,
    HEADERS,
    BODY,
    DONE
};


/** 
 * @description: 用于解析 HTTP 请求头
 */
class HttpRequest {
private:
    std::string m_method;  // 请求方式
    std::string m_url;  // 请求资源
    std::string m_version;  // HTTP 版本
    std::map<std::string, std::string> m_reqquest_headers;  // 请求头信息
    std::map<std::string, std::string> m_reqquest_body;  // 请求体信息
    PrecessState m_cur_state;  // 请求头当前状态

private:
    void reset();  // 重置对象(当一个请求头处理完毕后调用)

    char* splitLine(const char* start, const char* end, const char* sub, std::function<void(std::string)> callback);  // 拆分请求行
    bool parseLine(Buffer* read_buffer);  // 解析请求行

    bool addHeader(const std::string key, const std::string value, std::map<std::string, std::string>* map);  // 添加请求头
    std::string getHeader(const std::string key, std::map<std::string, std::string>* map);  // 根据key得到请求头的value
    bool parseHeader(Buffer* read_buffer);  // 解析请求头

    bool splitBody(char* start, int line_size);  // 拆分请求体
    bool parseBody(Buffer* read_buffer);  // 解析请求体

    int hexToDec(char c);  // 将十六进制字符转换为整形数
    std::string decodeMsg(std::string from);  // 解码字符串
    bool processRequest(HttpResponse* response);  // 处理http请求协议
    const std::string getFileType(const std::string name);
    static bool sendDir(std::string dir_name, Buffer* send_buffer, int cfd);
    static bool sendFile(std::string dir_name, Buffer* send_buffer, int cfd);
    
    inline void setMethod(std::string method);
    inline void setUrl(std::string url);
    inline void setVersion(std::string version);
    inline PrecessState getState();
    inline void setState(PrecessState state);

public:
    HttpRequest();
    ~HttpRequest() = default;
    
    // 解析http请求协议
    bool parseRequest(Buffer* read_buffer, HttpResponse* response, Buffer* send_buffer, int socket);  
};


inline void HttpRequest::setMethod(std::string method) {
    m_method = method;
}

inline void HttpRequest::setUrl(std::string url) {
    m_url = url;
}

inline void HttpRequest::setVersion(std::string version) {
    m_version = version;
}

inline PrecessState HttpRequest::getState() {
    return m_cur_state;
}

inline void HttpRequest::setState(PrecessState state) {
    m_cur_state = state;
}