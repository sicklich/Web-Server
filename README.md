# Cpp-Web-Server

## 一、项目说明

### 1.1 项目描述
本项目在 `Linux` 系统下实现了 `C++` 版本的**基于事件循环模型的多线程 `Web` 服务器**，可接收 `GET`、`POST`、`HEAD` 不同请求，并可根据实习需求使用日志记录信息。

### 1.2 应用技术
`Linux`、`C++`、`Tcp`、套接字编程(`Socket`)、`I/O`多路复用(`Epoll` + `Poll` + `Select`)、线程池、反应堆模型

### 1.3 项目特点
- **底层分别实现 `Epoll + LT`、`Poll`、`Select` 三种模式的 `I/O` 复用模型**，可以通过在初始化 `EventLoop` 对象时，给定不同的 `Dispatcher` 对象来切换；
- **采用主从反应堆模型**，主反应堆模型只负责监听并建立新连接，将新连接分配给从反应堆模型，不涉及业务处理采用单线程处理。从反应堆模型运行在独立的线程中，可以并行处理分配给自己的事件，从而提高并发性能；
- **采用 `One loop per thread` 并发编程模式，并通过线程池进行管理**，可以有效减轻频繁创建/销毁线程带来的性能影响；
- **拥有大量的回调函数**，可以减少代码复用，使代码看起来更加精简，且更具灵活性和扩展性
- **通过强类型枚举作为各类事件的通知描述符**，方便更高效的派发事件，也可以通过位运算判断某一事件是否为组合事件，同时避免了隐式类型转换。

## 二、项目配置
### 2.1 环境及软件版本
- **操作系统**: `Ubuntu 22.04.1 LTS`
- **cmake**: `3.22.1`
- **gcc/g++**: `11.3.0`
- **gdb**: `12.1`

### 2.2 启动方法
1. `Visual Studio`
如果是通过 `Visual Studio` 需要设置库依赖项，然后将头文件和源文件放在一个项目中，运行即可
![库依赖项](images/image1.png)

2. `bash`
- 构建项目: ```bash build.sh```
- 运行项目: ```bash run.sh```

## 三、项目文件结构
```
├── build.sh
├── CMakeLists.txt
├── images
│   └── image1.png
├── include
│   ├── Base
│   │   ├── Buffer.h
│   │   ├── ThreadPool.h
│   │   └── WokerThread.h
│   ├── Dispatcher
│   │   ├── Dispatcher.h
│   │   ├── EpollDispatcher.h
│   │   ├── PollDispatcher.h
│   │   └── SelectDispatcher.h
│   ├── HTTP
│   │   ├── HttpRequest.h
│   │   └── HttpResponse.h
│   ├── Log
│   │   └── Log.h
│   └── Net
│       ├── Channel.h
│       ├── EventLoop.h
│       ├── TcpConnection.h
│       └── TcpServer.h
├── LICENSE
├── README.md
├── run.sh
└── src
    ├── Base
    │   ├── Buffer.cpp
    │   ├── ThreadPool.cpp
    │   └── WokerThread.cpp
    ├── CMakeLists.txt
    ├── Dispatcher
    │   ├── Dispatcher.cpp
    │   ├── EpollDispatcher.cpp
    │   ├── PollDispatcher.cpp
    │   └── SelectDispatcher.cpp
    ├── HTTP
    │   ├── HttpRequest.cpp
    │   └── HttpResponse.cpp
    ├── main.cpp
    └── Net
        ├── Channel.cpp
        ├── EventLoop.cpp
        ├── TcpConnection.cpp
        └── TcpServer.cpp
```
