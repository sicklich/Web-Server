/** 
 * @author: yuyuyuj1e 807152541@qq.com
 * @github: https://github.com/yuyuyuj1e
 * @csdn: https://blog.csdn.net/yuyuyuj1e
 * @date: 2023-02-27 18:25:09
 * @last_edit_time: 2023-03-05 16:03:59
 * @file_path: /Cpp-Web-Server/TcpServer.cpp
 * @description: 服务器模块源代码
 */

#include "TcpServer.h"
#include <stdlib.h>
#include <arpa/inet.h>
#include "TcpConnection.h"
#include <stdio.h>


/** 
 * @description: 检测到连接请求后的操作函数，获取通信文件描述符后将该文件描述符发送给线程池，让子线程处理通信，主线程继续监听通信
 * @description: 由于建立连接需要调用类私有成员 m_lfd（类静态成员可以调用私有成员），且类静态函数无需实例化对象也存在（存在地址）
 * @description: 所以将该函数设置为类的静态成员函数更方便
 * @param {void*} arg: 服务器自身
 * @return {int} : 之所以需要设置返回值，是因为 Channel 设置的函数指针 function<int(void*)> 需要匹配类型
 */
int TcpServer::acceptConnection(void* arg) {
	TcpServer* server = static_cast<TcpServer*>(arg);
	// 和客户端建立链接
	int cfd = accept(server->m_lfd, NULL, NULL);

	// 从线程池中取出一个子线程的反应堆模型，处理 cfd
	EventLoop* evLoop = server->m_thread_pool->takeWorkerEventLoop();

	// 将 cfd 放到 TcpConnection 中处理
	new TcpConnection(cfd, evLoop);
	return 0;
}


TcpServer::TcpServer(unsigned short port, int thread_num) : m_port(port), m_thread_num(thread_num) {
	m_main_event_loop = new EventLoop();
	m_thread_pool = new ThreadPool(m_main_event_loop, thread_num);
	setListen();
}

/** 
 * @description: 设置监听函数，创建监听套接字，绑定端口并监听该端口是否有连接请求
 */
void TcpServer::setListen() {
	// 1. 创建用于监听的套接字
	m_lfd = socket(AF_INET, SOCK_STREAM, 0);
	if (m_lfd == -1) {
		perror("socket");
		return ;
	}

	// 2. 设置端口复用，服务器主动断开连接后，一定时长内不会释放端口，通过端口复用可以在段时间内使用该端口
	int opt = 1;
	int ret = setsockopt(m_lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt);
	if (ret == -1) {
		perror("setsockopt");
		return ;
	}

	// 3. 绑定端口
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;  // 指定为 IPV4
	addr.sin_port = htons(m_port);  // 将主机字节序转换为网络字节序，并设置端口
	// IP 范围是 0~255 对应一个 unsigned char 点分十进制 192.168.0.1 为四个 unsigned char 大小为四个字节，即一个 int 类型
	// 0.0.0.0 表示本地的任意地址，宏为 INADDR_ANY
	addr.sin_addr.s_addr = INADDR_ANY;  // 设置 IP 地址   
	ret = bind(m_lfd, (struct sockaddr*)&addr, sizeof(struct sockaddr_in));
	if (ret == -1) {
		perror("bind");
		return ;
	}

	// 4. 设置监听
	ret = listen(m_lfd, 128);  // 128 表示监听过程中一次性最多可以连接的客户端数量
	if (ret == -1) {
		perror("listen");
		return ;
	}

}


/** 
 * @description: 启动服务器程序，启动线程池，封装监听套接字与响应操作，并启动事件循环反应堆模型（主反应堆模型）
 */
void TcpServer::run() {
	// 启动线程池
	m_thread_pool->run();
	// 初始化一个 channel，封装监听套接字
	Channel* channel = new Channel(m_lfd, FDEvent::READEVENT, acceptConnection, nullptr, nullptr, this);
	// 添加检测的任务
	m_main_event_loop->addTask(channel, ElemType::ADD);
	// 启动主线程反应堆模型
	m_main_event_loop->run();
}
