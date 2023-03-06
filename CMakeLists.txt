cmake_minimum_required(VERSION 3.0) 

project(CppWebServer)

set(SOURCES
    Buffer.cpp
    Channel.cpp
    Dispatcher.cpp
    EpollDispatcher.cpp
    EventLoop.cpp
    HttpRequest.cpp
    HttpResponse.cpp
    main.cpp
    PollDispatcher.cpp
    SelectDispatcher.cpp
    TcpConnection.cpp
    TcpServer.cpp
    ThreadPool.cpp
    WokerThread.cpp
)

add_executable(server ${SOURCES})

target_include_directories(server PRIVATE include)

target_link_libraries(server PRIVATE pthread)
