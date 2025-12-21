cmake_minimum_required(VERSION 3.16)


set(CMAKE_SYSTEM_NAME Linux) #设置目标系统名字 
set(CMAKE_SYSTEM_PROCESSOR arm) #设置目标处理器架构 
  
 
# 指定编译器的 sysroot 路径 
#set(TOOLCHAIN_DIR /home/alientek/sysroot_imx6ull) 
#set(CMAKE_SYSROOT /home/alientek/sysroot_imx6ull) 
 
# 指定交叉编译器 arm-linux-gcc 和 arm-linux-g++ 
#set(CMAKE_C_COMPILER   /usr/local/arm/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc)

#set(CMAKE_CXX_COMPILER /usr/local/arm/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-g++)
#set(CMAKE_CROSSCOMPILING TRUE)
 
# 为编译器添加编译选项 
 
# 指定编译器的 sysroot 路径 
set(TOOLCHAIN_DIR /opt/fsl-imx-x11/4.1.15-2.1.0/sysroots) 
set(CMAKE_SYSROOT ${TOOLCHAIN_DIR}/cortexa7hf-neon-poky-linux-gnueabi) 
 
# 指定交叉编译器 arm-linux-gcc 和 arm-linux-g++ 
set(CMAKE_C_COMPILER ${TOOLCHAIN_DIR}/x86_64-pokysdk-linux/usr/bin/arm-poky-linux-gnueabi/arm-poky-linux-gnueabi-gcc)

set(CMAKE_CXX_COMPILER ${TOOLCHAIN_DIR}/x86_64-pokysdk-linux/usr/bin/arm-poky-linux-gnueabi/arm-poky-linux-gnueabi-g++)

 
# 为编译器添加编译选项 
set(CMAKE_C_FLAGS "-march=armv7ve -mfpu=neon -mfloat-abi=hard -mcpu=cortex-a7") 
set(CMAKE_CXX_FLAGS "-march=armv7ve -mfpu=neon -mfloat-abi=hard -mcpu=cortex-a7") 
 
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER) 
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY) 
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY) 

################################# 
# end 
##################################

project(MyProject LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/common/inc/)

# 先添加 communication 库
add_subdirectory(communication)

# 添加 GUI 模块
add_subdirectory(gui_manager)

# 添加 multimedia 模块
add_subdirectory(multimedia)
