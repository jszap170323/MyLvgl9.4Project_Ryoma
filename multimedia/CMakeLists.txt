
project(multimedia) 


set(ENV{PKG_CONFIG_SYSROOT_DIR} ${CMAKE_SYSROOT})
set(ENV{PKG_CONFIG_LIBDIR} ${CMAKE_SYSROOT}/usr/lib/pkgconfig:${CMAKE_SYSROOT}/usr/share/pkgconfig)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
find_package(PkgConfig REQUIRED)

# 查找 GStreamer（现在会在 sysroot 中查找）
pkg_check_modules(GST REQUIRED gstreamer-1.0)

# 检查是否找到了正确的路径
message(STATUS "GST_INCLUDE_DIRS: ${GST_INCLUDE_DIRS}")
message(STATUS "GST_LIBRARIES: ${GST_LIBRARIES}")


#find_package(PkgConfig REQUIRED)

#pkg_check_modules(GST REQUIRED 
#  gstreamer-1.0 
#)
include_directories(
	${GST_INCLUDE_DIRS}
)


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc/
                    ${CMAKE_CURRENT_SOURCE_DIR}/common/inc/
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/pipeline/inc/)

file(GLOB_RECURSE SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/common/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
)

add_executable(multimedia ${SOURCES})


# 添加到add_executable之后
target_link_libraries(multimedia
		${GST_LIBRARIES} 
    communication
	gstapp-1.0 
	gstreamer-1.0
    gobject-2.0
    glib-2.0
	pthread
	rt
)
