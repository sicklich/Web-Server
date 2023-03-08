/** 
 * @author: yuyuyuj1e 807152541@qq.com
 * @github: https://github.com/yuyuyuj1e
 * @csdn: https://blog.csdn.net/yuyuyuj1e
 * @date: 2023-02-27 18:25:07
 * @last_edit_time: 2023-03-08 10:08:18
 * @file_path: /CC/include/Base/Buffer.h
 * @description: Buffer 模块头文件
 */

#pragma once
#include <string>

/** 
 * @description: 缓冲区类，可以当作读缓冲区，和写缓冲区使用
 */
class Buffer {
private:
	char* m_data; // 指向内存的指针
	int m_capacity;  // 缓冲区容量
	int m_read_pos = 0;  // 读位置
	int m_write_pos = 0;  // 写位置

public:
	Buffer(int size);
	~Buffer();

	int extendRoom(int size);  // 扩容
	inline int writeableSize();  // 得到剩余可写的内存容量
	inline int readableSize();  // 得到剩余可读的内存容量
	inline char* readPos(); // 得到读数据的起始位置
	inline int readPosIncrease(int count);  // 更新度位置

	int appendData(const char* data, int size);  // 向缓冲区添加数据
	int appendData(const char* data);  // 向缓冲区添加数据
	int appendData(const std::string data);  // 向缓冲区添加数据

	int readData(int fd);  // 接收数据
	int sendData(int fd);  // 发送数据

	char* findCRLF();  // 根据 \r\n 取出请求行，找到在数据块中的位置，返回该位置
	char* findCRLF(int i);  // 根据 & 取出 POST 请求体
};


/** 
 * @description: 得到剩余可写的内存容量
 * @return {int} 剩余当前可写容量（只关注 m_write_pos）
 */
inline int Buffer::writeableSize() {
	return m_capacity - m_write_pos;
}

inline int Buffer::readableSize() {
	return m_write_pos - m_read_pos;
}

inline char* Buffer::readPos() {
	return m_data + m_read_pos;
}

inline int Buffer::readPosIncrease(int count) {
	m_read_pos += count;
	return m_read_pos;
}