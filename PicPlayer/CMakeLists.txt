cmake_minimum_required(VERSION 3.16)

project(PicPlayer VERSION 0.1 LANGUAGES CXX)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置源码目录
set(SOURCE_DIR ${CMAKE_SOURCE_DIR}/PicPlayer/src)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 定义第三方库路径
set(EXT_DIR ${CMAKE_SOURCE_DIR}/ext)

# 添加第三方库的头文件路径
include_directories(${EXT_DIR}/include)

# 定义源码文件
set(PROJECT_SOURCES
    ${SOURCE_DIR}/PicPlayerApi.cpp
    ${SOURCE_DIR}/PicPlayerApi.h
    ${SOURCE_DIR}/PicPlayer_global.h
)

# 创建共享库
add_library(PicPlayer SHARED ${PROJECT_SOURCES})

# 添加第三方库
#target_link_libraries(PicPlayer PRIVATE ${EXT_DIR}/lib/libGLEW.a)

# 设置目标属性
set_target_properties(PicPlayer PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

# 安装目标
include(GNUInstallDirs)
install(TARGETS PicPlayer
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
