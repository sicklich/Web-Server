# 设置 CMake 最低版本要求
cmake_minimum_required(VERSION 3.0) 

# 设置项目名称
project(CppWebServer LANGUAGES CXX)

# 设置 C++11 标准
set(CMAKE_CXX_STANDARD 11)

# 引入头文件目录
include_directories(
    ${PROJECT_SOURCE_DIR}/include/Base
    ${PROJECT_SOURCE_DIR}/include/Dispatcher
    ${PROJECT_SOURCE_DIR}/include/HTTP
    ${PROJECT_SOURCE_DIR}/include/Net
    ${PROJECT_SOURCE_DIR}/include/Log
)

# 添加源文件目录
add_subdirectory(${PROJECT_SOURCE_DIR}/src)
