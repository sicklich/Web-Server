# 添加源文件到自定义的变量中
aux_source_directory(. SRC_LIST)
aux_source_directory(./Base BASE_LIST)
aux_source_directory(./Dispatcher DISPATCHER_LIST)
aux_source_directory(./HTTP HTTP_LIST)
aux_source_directory(./Net NET_LIST)
aux_source_directory(./Log LOG_LIST)

# 设置可执行文件存放路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 指定生成可执行文件
add_executable(server ${SRC_LIST} ${BASE_LIST} ${DISPATCHER_LIST} ${HTTP_LIST} ${NET_LIST} ${LOG_LIST})

# 指定链接到目标文件所需的库
target_link_libraries(server PRIVATE pthread)