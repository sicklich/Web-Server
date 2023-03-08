/** 
 * @author: yuyuyuj1e 807152541@qq.com
 * @github: https://github.com/yuyuyuj1e
 * @csdn: https://blog.csdn.net/yuyuyuj1e
 * @date: 2023-02-27 18:25:07
 * @last_edit_time: 2023-03-08 10:06:08
 * @file_path: /CC/include/Net/Channel.h
 * @description: Channel模块头文件
 */

#pragma once
#include <functional>

// 定义文件描述符的读写事件
enum class FDEvent : char {
	TIMEOUT = 1 << 0,
	READEVENT = 1 << 1,
	WRITEEVENT = 1 << 2
};

/** 
 * @description: Channel类，用于封装文件描述符，记录所需监听的事件及其响应回调函数
 */
class Channel {
private:
	int m_fd; // 文件描述符（通信/监听）
	int m_events;  // 事件（读/写）
	void* m_arg;  // 回调函数的参数

public:
	// 可调用对象包装器可打包 1. 函数指针 2. 可调用对象（可以像函数一样使用），最后得到的是地址
	// using handleFunc = int(*)(void*) <==> typedef int(*handleFunc)(void* arg)
	using handleFunc = std::function<int(void*)>;
	Channel(int fd, FDEvent events, handleFunc readFunc, handleFunc writeFunc, handleFunc destroyFunc, void* arg);
	~Channel() = default;
	
	void writeEventEnable(bool flag);  // 修改 fd 的写事件（检测 or 不检测）
	bool isWriteEventEnable();  // 判断是否需要检测文件描述符的写事件

	// 取出私有成员的值
	inline int getEvent();
	inline int getSocket();
	inline const void* getArg();  // 返回地址，但是不让用户修改地址里的数据，因此添加 const，但是 Callback 参数并非只读，因此后续需要去掉 const 属性
	/* const_cast 注意事项
	 * const int a = 10;
	 * int *p = const_cast<int*>(&a);
	 * *p = 20;
	 * 注意此时，*p 的值为 20，但是 a 的值依旧是 10
	 * 这是因为变量 a 被 const 修饰后，编译器会将变量 a 存放到寄存器中，这样就无需从内存取值（为了提升效率）
	 * *p 修改 a 的之后，内存响应的值被修改，但是编译器依旧从寄存器中读取数据，因此 a 的值依旧是 10
	 * 此时可以通过 volatile 变量，即定义 volatile const int a = 10，强制 CPU 每次都内存读取数据，此时 a 的值为 20
	 */

public:
	// 下述三个函数指针需要在外部使用，因此不能是私有的
	handleFunc readCallback;  // 读回调函数
	handleFunc writeCallback;  // 写回调函数
	handleFunc destroyCallback;  // 销毁回调函数
};

inline int Channel::getEvent() {
	return m_events;
}

inline int Channel::getSocket() {
	return m_fd;
}

inline const void* Channel::getArg() {
	return m_arg;
}
