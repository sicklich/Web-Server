/** 
 * @author: yuyuyuj1e 807152541@qq.com
 * @github: https://github.com/yuyuyuj1e
 * @csdn: https://blog.csdn.net/yuyuyuj1e
 * @date: 2023-02-27 18:25:07
 * @last_edit_time: 2023-03-06 10:59:15
 * @file_path: /Cpp-Web-Server/Buffer.cpp
 * @description: Buffer 模块源文件
 */

#include "Buffer.h"
#include <string.h>
#include <sys/uio.h>
#include <unistd.h>
#include <sys/socket.h>


Buffer::Buffer(int size) : m_capacity(size) {
	m_data = (char*)malloc(sizeof(char) * size);  // 申请空间
	memset(m_data, 0, size);  // 初始化
}

Buffer::~Buffer() {
	if (m_data != nullptr) {
		free(m_data);
	}
}

/** 
 * @description: 扩容缓冲区
 * @param {int} size: 所需内存大小
 * @return {int} 成功返回 0；失败(或无需扩容)返回 -1
 */
int Buffer::extendRoom(int size) {
	// 1. 内存够用 —— 不需要扩容
	if (writeableSize() >= size) {
		return -1;
	}
	// 2. 内存不够用 —— 合并后够用 —— 不需要扩容
	// 剩余可写空间 + 已读空间 >= size
	else if (m_read_pos + writeableSize() >= size) {
		// 未读大小
		int readable = readableSize();
		// 移动内存，将 m_data + m_read_pos 处复制 readable 大小的内容到 m_data 
		memcpy(m_data, m_data + m_read_pos, readable);
		// 更新位置
		m_read_pos = 0;
		m_write_pos = readable;
		return -1;
	}
	// 3. 内存不够用 —— 需要扩容
	else {
		void* temp = realloc(m_data, m_capacity + size);  // 扩容
		if (temp == NULL) {
			return -1;  // 扩容失败
		}
		memset((char*)temp + m_capacity, 0, size);  // 初始化新扩容的内存
		// 更新数据
		m_data = (char*)temp;
		m_capacity += size;
	}
	return 0;
}

/** 
 * @description: 向缓冲区添加数据
 * @param {char*} data: 数据内容
 * @param {int} size: 数据大小
 * @return {int} 成功返回 0；失败返回 -1
 */
int Buffer::appendData(const char* data, int size) {
	if (m_data == nullptr || size < 0) {
		return -1;
	}

	extendRoom(size);  // 扩容
	memcpy(m_data + m_write_pos, data, size);  // 写数据
	m_write_pos += size;  // 更新

	return 0;
}

/** 
 * @description: 向缓冲区添加数据
 * @param {char*} data: 数据内容
 * @return {int} 成功返回 0；失败返回 -1
 */
int Buffer::appendData(const char* data) {
	int size = strlen(data);
	int ret = appendData(data, size);
	return ret;
}

/** 
 * @description: 向缓冲区添加数据
 * @param {string} data: 数据内容
 * @return {int} 成功返回 0；失败返回 -1
 */
int Buffer::appendData(const std::string data) {
	int ret = appendData(data.data());
	return ret;
}


/** 
 * @description: 接收指定客户端发送过来的数据
 * @param {int} fd: 通信套接字
 * @return {int} 成功返回接收数据大小；失败返回 0
 */
int Buffer::readData(int fd) {
	// read/recv 只能指定一个数组（接收数据），readv 可以指定多个数组（接收数据）
	// ssize_t readv(int fd, const struct iovec *iov, int iovcnt)
	// ssize_t writev(int fd, const struct iovec *iov, int iovcnt)
	// 使用这个方法的原因是为了 post 方法可能会接收大量数据
	
	// 初始化 iovec
	struct iovec vec[2];
	int writeable = writeableSize();
	vec[0].iov_base = m_data + m_write_pos;  // iov_base 指向一个内存地址，用于存放 readv 所接收的数据或是 writev 将要发送的数据
	vec[0].iov_len = writeable;  // iov_len 确定了接收的最大长度或者实际写入的长度。

	char* tmp_buf = (char*)malloc(40960);  // 40k
	vec[1].iov_base = tmp_buf;  // tmp_buf
	vec[1].iov_len = 40960;

	// 接收数据
	int result = readv(fd, vec, 2);
	if (result == -1) {  // 接收失败
		return -1;
	}
	else if (result <= writeable) {  // buffer 内存块足够用
		m_write_pos += result;
	}
	else {  // 不够用，写到堆内存
		// 更新位置
		m_write_pos = m_capacity;  // 此时 buffer 已经写满，剩余的数据写到了 tmp_buf 中，所以将 m_write_pos 位置更新为 m_capacity
		// 将 tmp_buf 的数据添加到 buffer 中
		appendData(tmp_buf, result - writeable);  // result - writeable 表示 tmp_buf 接收到的数据大小
	}
	free(tmp_buf);
	return result;
}

/** 
 * @description: 给指定的客户端发送数据
 * @param {int} fd: 通信套接字
 * @return {int} 成功返回发送数据大小；失败返回 0
 */
int Buffer::sendData(int fd) {
	// 判断有无数据
	int readable = readableSize();
	if (readable > 0) {
		// 当连接断开时发数据，send() 会向系统发送一个异常消息，系统会发出 BrokePipe，强迫程序会退出
		// 可以将 send() 函数的最后一个参数可以设 MSG_NOSIGNAL，禁止 send() 函数向系统发送异常消息
		int count = send(fd, m_data + m_read_pos, readable, MSG_NOSIGNAL);
		if (count > 0) {
			m_read_pos += count;
			usleep(1);
		}
		return count;
	}
	return 0;
}

/** 
 * @description: 找到 HTTP 协议特定的 /r/n 换行符
 * @return {char*} 返回 /r/n 起始位置
 */
char* Buffer::findCRLF() {
	// strstr --> 大字符串中匹配子字符串（遇到 \0 结束）
	// memmem --> 大数据块中匹配子数据块（需要指定数据块大小）
	char* ptr = (char*)memmem(m_data + m_read_pos, readableSize(), "\r\n", 2);
	return ptr;
}

/** 
 * @description: 根据 & 取出 POST 请求体
 * @param {int} i: 无实际意义，表明为 POST 取出请求体内容
 * @return {char*} 返回 & 起始位置
 */
char* Buffer::findCRLF(int i) {
	char* ptr = (char*)memmem(m_data + m_read_pos, readableSize(), "&", 1);
	return ptr;
}