cmake_minimum_required(VERSION 3.16)
project(MyProject LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 先添加 communication 库
add_subdirectory(communication)

# 添加 GUI 模块
add_subdirectory(gui_manager)

# 添加 multimedia 模块
add_subdirectory(multimedia)
