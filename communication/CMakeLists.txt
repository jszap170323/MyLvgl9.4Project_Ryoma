# 头文件目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc)

# 编译为静态库
add_library(communication STATIC
    src/ipc_shared_memory.cpp
)

# 导出头文件给其他模块使用
target_include_directories(communication PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc)
