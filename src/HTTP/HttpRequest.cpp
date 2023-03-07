/** 
 * @author: yuyuyuj1e 807152541@qq.com
 * @github: https://github.com/yuyuyuj1e
 * @csdn: https://blog.csdn.net/yuyuyuj1e
 * @date: 2023-02-27 18:25:07
 * @last_edit_time: 2023-03-07 09:42:04
 * @description: HttpResponse 模块源文件
 */

#include "HttpRequest.h"
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include "TcpConnection.h"
#include <string.h>
#include <iostream>

HttpRequest::HttpRequest() {
    reset();
}

/** 
 * @description: 重置 HttpRequest 对象
 * @description: 当一个请求头被解析完成后，便调用该函数，以便解析后续请求
 */
void HttpRequest::reset() {
    m_cur_state = PrecessState::LINE;
    m_method = std::string();
    m_url = std::string();
    m_version = std::string();
    m_reqquest_headers.clear();
}

/** 
 * @description: 外部调用该函数解析 HTTP 请求
 * @param {Buffer*} read_buffer: 存放请求数据的缓冲区
 * @param {HttpResponse*} response: 组织回复数据的对象指针
 * @param {Buffer*} send_buffer: 发送数据的缓冲区
 * @param {int} cfd: 通信文件描述符
 * @return {bool} 解析成功返回 true，解析失败返回 false
 */
bool HttpRequest::parseRequest(Buffer* read_buffer, HttpResponse* response, Buffer* send_buffer, int cfd) {
    bool flag = true;
    // 循环条件——请求数据尚未处理完毕，且前序处理都成功
    while (m_cur_state != PrecessState::DONE && flag == true) {
        switch (m_cur_state) {
        case PrecessState::LINE:  // 处理请求行
            flag = parseLine(read_buffer);
            break;
        case PrecessState::HEADERS:  // 处理请求头
            flag = parseHeader(read_buffer);
            break;
        case PrecessState::BODY:  // 处理 POST 发送的数据
            flag = parseBody(read_buffer);
            break;
        default:
            break;
        }
    }

    // 如果解析完毕, 准备回复的数据
    if (m_cur_state == PrecessState::DONE) {
        processRequest(response);  // 1. 根据解析出的原始数据, 对客户端的请求做出处理
        response->prepareHeadMsg(send_buffer, cfd);  // 2. 组织响应头数据并发送给客户端
    }

    reset();   // 请求处理完毕，还原初始状态, 保证还能继续处理第二条及以后的请求
    return flag;
}

/** 
 * @description: 拆分请求行
 * @param {char*} start: 字符串起始位置
 * @param {char*} end: 字符串结束位置
 * @param {char*} stop_str: 结束的字串，请求行的格式一般为  GET xxx/xxx.jgp HTTP1.1，因此 stop_str 可以是空格
 * @param {function<void(std::string)>} callback: 用于设置的回调函数
 * @return {char*} 指向处理过后的下一个位置的指针
 */
char* HttpRequest::splitLine(const char* start, const char* end, const char* stop_str, std::function<void(std::string)> callback) {
    // 如果设置了停止子串 space = 停止子串的起始位置，否则的话是字符串的最后一个位置
    char* space = stop_str == nullptr ? const_cast<char*>(end) : static_cast<char*>(memmem(start, end - start, stop_str, strlen(stop_str)));
    if (space == nullptr) {
        perror("split line");
    }

    int length = space - start;
    callback(std::string(start, length));  // 将 start, length 这段字串传递给回调函数
    return space + 1;
}

/** 
 * @description: 处理请求行数据
 * @param {Buffer*} read_buffer: 存放请求数据的缓冲区
 * @return {bool} 解析成功返回 true，解析失败返回 false
 */
bool HttpRequest::parseLine(Buffer* read_buffer) {
    char* end = read_buffer->findCRLF();  // 请求行结束地址
    if (end == nullptr) {
        return false;
    }
    char* start = read_buffer->readPos();  // 请求行起始地址
    int line_size = end - start;  // 请求行总长度

    // 解析 method 并存储
    auto methodFunc = std::bind(&HttpRequest::setMethod, this, std::placeholders::_1);
    start = splitLine(start, end, " ", methodFunc);
    // 解析 url 并存储
    auto urlFunc = std::bind(&HttpRequest::setUrl, this, std::placeholders::_1);
    start = splitLine(start, end, " ", urlFunc);
    // 解析 version 并存储
    auto versionFunc = std::bind(&HttpRequest::setVersion, this, std::placeholders::_1);
    splitLine(start, end, nullptr, versionFunc);

    // 更新状态
    read_buffer->readPosIncrease(line_size + 2);  // 跳过 /r/n
    setState(PrecessState::HEADERS);  // 修改处理状态
    return true;
}


/** 
 * @description: 向 Header 数组中添加相应键值对
 * @param {string} key: Header key
 * @param {string} value: Header value
 * @param {map<std::string, std::string>*} map: 需要操作的集合
 * @return {bool} 成功返回 true，失败返回 false
 */
bool HttpRequest::addHeader(const std::string key, const std::string value, std::map<std::string, std::string>* map) {
    if (key.empty() || value.empty()) {
        return false;
    }
    map->insert(std::make_pair(key, value));
    return true;
}

/** 
 * @description: 根据指定 Header key 值获取其相应 value 值
 * @description: 可以根据某些 value 判断是否允许其进行访问
 * @param {string} key: Header key
 * @param {map<std::string, std::string>*} map: 需要操作的集合
 * @return {string} Header value
 */
std::string HttpRequest::getHeader(const std::string key, std::map<std::string, std::string>* map) {
    auto item = map->find(key);
    if (item == map->end()) {
        return std::string();
    }
    return item->second;
}


/** 
 * @description: 解析请求头
 * @description: 该函数可能会被调用多次，因为请求头可能会拥有多条信息，需要多次提取
 * @param {Buffer*} read_buffer: 存放请求数据的缓冲区
 * @return {bool} 解析成功返回 true，解析失败返回 false
 */
bool HttpRequest::parseHeader(Buffer* read_buffer) {
    char* end = read_buffer->findCRLF();  // 请求头结束地址
    if (end == nullptr) {
        return false;
    }
    char* start = read_buffer->readPos();  // 请求头起始地址
    int line_size = end - start;  // 请求头总长度
    
    // 搜索字符串  请求头格式 xxxx: xxxx 中间被 冒号和空格 分开
    char* middle = static_cast<char*>(memmem(start, line_size, ": ", 2));
    if (middle != nullptr) {
        int key_len = middle - start;
        int value_len = end - (middle + 2);
        if (key_len > 0 && value_len > 0) {
            std::string key(start, key_len);
            std::string value(middle + 2, value_len);
            addHeader(key, value, &m_reqquest_headers);
        }
        // 更新位置
        read_buffer->readPosIncrease(line_size + 2);  // 跳过 /r/n
    }
    else {  // 此时解析到了空行，如果是 GET 请求，则直接结束，如果是 POST 请求则后续是请求体
        if (strcasecmp(m_method.data(), "get") == 0 || strcasecmp(m_method.data(), "head") == 0) {  // GET 请求或 HEAD 请求
            read_buffer->readPosIncrease(2);  // 跳过空行
            setState(PrecessState::DONE);  // 修改解析状态
        }
        else if (strcasecmp(m_method.data(), "post") == 0) {  // POST 请求
            read_buffer->readPosIncrease(2);  // 跳过空行
            setState(PrecessState::BODY);  // 修改解析状态
        }
    }
    return true;
}

bool HttpRequest::splitBody(char* start, int line_size) {
     // 搜索字符串  请求头格式 xxx=yyy&xxx=yyy 中间被 & 分开
    char* split = static_cast<char*>(memmem(start, line_size, "=", 1));
    int sub_len = line_size - (split + 1 - start);  // 获取 value 的长度

    // 添加键值对
    std::string key = std::string(start, split);
    std::string value = std::string(split + 1, sub_len);
    addHeader(key, value, &m_reqquest_body);
    return true;
}

/** 
 * @description: 解析请求体
 * @param {Buffer*} read_buffer: 存放请求数据的缓冲区
 * @return {bool} 解析成功返回 true，解析失败返回 false
 */
bool HttpRequest::parseBody(Buffer* read_buffer)  {
    auto item = m_reqquest_headers.find("Content-Length");  // 取出存放请求体长度的容器
    int remain_len = std::stoi(item->second);  // 获取剩余长度
    int line_size = remain_len;  // 默认这次取出的长度是剩余长度（为最后一个键值对做准备）
    
    char* end = read_buffer->findCRLF(1);  // 请求头结束地址
    char* start = read_buffer->readPos();  // 请求头起始地址

    if (end == nullptr && remain_len <= 0) {  // 没有找到 & 并且 Content-Length 也全部取出
        return false;
    }
    else if (end != nullptr) {  // 当找到 &，更新本次取出的长度
        line_size = end - start;  // 请求头总长度
    }
    
    // 拆分并添加字符串
    splitBody(start, line_size);
    
    // 更新状态
    remain_len -= line_size;  // 本次已经取出的内容
    read_buffer->readPosIncrease(line_size);
    if (end != nullptr) {  // & 符号
        remain_len -= 1;  
        read_buffer->readPosIncrease(1); 
    }
    item->second = std::to_string(remain_len);  // 更新 Content-Length
    
    if (remain_len == 0) {  // 更新解析状态
        setState(PrecessState::DONE);  
    }
    
    return true;
}

/** 
 * @description: 将十六进制字符转换为整形数
 * @param {char} c: 十六进制字符
 * @return {int} 十六进制字符对应的整形数
 */
int HttpRequest::hexToDec(char c) {
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;

    return 0;
}

/** 
 * @description: 解码特殊字符
 * @description: HTTP GET 请求的请求行不支持特殊字符，如果有特殊字符就会自动进行转换成 UTF-8（三个字符），如 “%EF%9B%BD”
 * @description: 可以通过 (unicode 国 --> e5 9b bd) 这种方法来查看特殊字符的 UTF-8 编码值
 * @param {string} msg: 待解码的字符串
 * @return {string} 解码成含有特殊字符的字符串
 */
std::string HttpRequest::decodeMsg(std::string msg) {
    std::string str = std::string();
    const char* from = msg.data();
    for (; *from != '\0'; ++from) {
        // isxdigit -> 判断字符是不是16进制格式, 取值在 0-f
        if (from[0] == '%' && isxdigit(from[1]) && isxdigit(from[2])) {  // 判断格式 --> %EF%9B%BD
            // 将三个字符, 变成一个字符, 这个字符就是原始数据
            str.append(1, hexToDec(from[1]) * 16 + hexToDec(from[2]));
            from += 2;  // 跳过 from[1] 和 from[2]
        }
        else {  // 非特殊字符直接使用
            str += *from;
        }
    }

    str += "\0";
    return str;
}

/** 
 * @description: 获取文件的 Content-type 类型
 * @description: Content-type 类型可以参考 https://tool.oschina.net/commons 有详细的展示列表
 * @param {string} name: 文件名称
 * @return {string} Content-type 类型
 */
const std::string HttpRequest::getFileType(const std::string name) {
    const char* dot = strrchr(name.data(), '.');  // 自右向左查找 . 字符，以解析文件后缀

    if (dot == NULL) return "text/plain; charset=utf-8";
    else if (strcmp(dot, ".html") == 0 || strcmp(dot, ".htm") == 0) return "text/html; charset=utf-8";
    else if (strcmp(dot, ".jpg") == 0 || strcmp(dot, ".jpeg") == 0) return "image/jpeg";
    else if (strcmp(dot, ".gif") == 0) return "image/gif";
    else if (strcmp(dot, ".png") == 0) return "image/png";
    else if (strcmp(dot, ".css") == 0) return "text/css";
    else if (strcmp(dot, ".js") == 0) return "	application/x-javascript";
    else if (strcmp(dot, ".pdf") == 0) return "application/pdf";
    else if (strcmp(dot, ".au") == 0) return "audio/basic";
    else if (strcmp(dot, ".wav") == 0) return "audio/wav";
    else if (strcmp(dot, ".avi") == 0) return "video/x-msvideo";
    else if (strcmp(dot, ".mov") == 0 || strcmp(dot, ".qt") == 0) return "video/quicktime";
    else if (strcmp(dot, ".mpeg") == 0 || strcmp(dot, ".mpe") == 0) return "video/mpeg";
    else if (strcmp(dot, ".vrml") == 0 || strcmp(dot, ".wrl") == 0) return "model/vrml";
    else if (strcmp(dot, ".midi") == 0 || strcmp(dot, ".mid") == 0) return "audio/midi";
    else if (strcmp(dot, ".mp3") == 0) return "audio/mpeg";
    else if (strcmp(dot, ".ogg") == 0) return "application/ogg";
    else if (strcmp(dot, ".pac") == 0) return "application/x-ns-proxy-autoconfig";

    return "text/plain; charset=utf-8";
}

/** 
 * @description: 处理 HTTP 请求
 * @param {HttpResponse*} response: 指向组织相应数据块对象的指针
 * @return {bool} 成功返回 true，失败返回 false
 */
bool HttpRequest::processRequest(HttpResponse* response) {
    // 只接受 get post head 请求
    if (!(strcasecmp(m_method.data(), "get") == 0 
        || strcasecmp(m_method.data(), "post") == 0 
        || strcasecmp(m_method.data(), "head") == 0 )) 
    {  // 忽略大小写
        return false;
    }

    m_url = decodeMsg(m_url);  // 将含 UTF-8 编码的字符串解码成含有特殊字符的字符串
    const char* file = NULL;  // 处理客户端请求的静态资源(目录或者文件)

    // 确定文件此时的路径
    if (strcmp(m_url.data(), "/") == 0) {  // 相对路径
        file = "./";
    }
    else {  // 当前目录下
        file = m_url.data() + 1;
    }

    // 初始化获取文件/目录属性的结构体
    struct stat st;
    int ret = stat(file, &st);

    if (ret == -1) {  // 文件/目录不存在 -- 回复404
        response->setFileName("skydash-free-bootstrap-admin-template-main/template/pages/samples/error-404.html");  // 待发送文件的文件名
        response->setStatusCode(StatusCode::NOTFOUND);  // 响应状态
        response->addHeader("Content-type", getFileType(".html"));  // 响应头
        response->setFunc(sendFile);  // 发送 404 文件
    }
    // 可以添加 else if 以控制某些文件不允许访问，组织 303 等
    else {  // 文件/目录存在
        response->setFileName(file);  // 待发送文件的文件名
        response->setStatusCode(StatusCode::OK);  // 响应状态
        
        // 判断文件类型
        if (S_ISDIR(st.st_mode)) {  // 目录
            response->addHeader("Content-type", getFileType(".html"));  // 响应头
            response->setFunc(sendDir);
        }
        else {  // 文件
            response->addHeader("Content-type", getFileType(file));  // 响应头
            response->addHeader("Content-length", std::to_string(st.st_size));  // 响应头
            response->setFunc(sendFile);  
        }
    }
    return true;
}


/** 
 * @description: 发送目录
 * @param {string} dir_name: 待发送目录的目录名 
 * @param {Buffer*} send_buffer: 存储发送数据的缓冲区
 * @param {int} cfd: 用于通信的文件描述符
 * @return {bool} 成功返回 true，失败返回 false
 */
bool HttpRequest::sendDir(std::string dir_name, Buffer* send_buffer, int cfd) {
    char buf[4096] = { 0 };
    sprintf(buf, "<html><head><title>%s</title></head><body><table>", dir_name.data());

    struct dirent** name_list;  // name_list 指向的是一个指针数组 struct dirent* tmp[]
    int num = scandir(dir_name.data(), &name_list, NULL, alphasort);  // alphasort 指定文件的排序方式
    for (int i = 0; i < num; ++i) {
        struct stat st;
        char sub_path[1024] = { 0 };
        char* name = name_list[i]->d_name;  // 提取文件名 
        sprintf(sub_path, "%s/%s", dir_name.data(), name);  // 提取文件路径
        stat(sub_path, &st);  // 提取文件属性

        if (S_ISDIR(st.st_mode)) {  // 如果目标是目录
            sprintf(buf + strlen(buf), "<tr><td><a href=\"%s/\">%s</a></td><td>%ld</td></tr>", name, name, st.st_size);
        }
        else {  // 如果目标是文件
            sprintf(buf + strlen(buf), "<tr><td><a href=\"%s\">%s</a></td><td>%ld</td></tr>", name, name, st.st_size);
        }
        send_buffer->appendData(buf);  // 向缓冲区存储数据
        send_buffer->sendData(cfd);  // 发送数据

        // 重置
        memset(buf, 0, sizeof(buf));
        free(name_list[i]);
    }
    sprintf(buf, "</table></body></html>");
    send_buffer->appendData(buf);
    send_buffer->sendData(cfd);

    free(name_list);
    return true;
}

/** 
 * @description: 发送文件
 * @param {string} file_name: 待发送文件的文件名 
 * @param {Buffer*} send_buffer: 存储发送数据的缓冲区
 * @param {int} cfd: 用于通信的文件描述符
 * @return {bool} 成功返回 true，失败返回 false
 */
bool HttpRequest::sendFile(std::string file_name, Buffer* send_buffer, int cfd) {
    // 1. 打开文件
    int fd = open(file_name.data(), O_RDONLY);
    if (fd <= 0) {
        perror("open file");
        close(fd);
        return false;
    }

    // 2. 发送文件
    while (true) {
        char buf[1024];
        int len = read(fd, buf, sizeof buf);
        if (len > 0) {  // 发送
            send_buffer->appendData(buf, len);
            send_buffer->sendData(cfd);
        }
        else if (len == 0) {  // 发送完毕
            break;
        }
        else {  // 出错
            close(fd);
            perror("read");
            return false;
        }
    }

    close(fd);
    return true;
}