cmake_minimum_required(VERSION 3.16)

project(PicPlayer VERSION 0.1 LANGUAGES CXX)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置源码目录
set(SOURCE_DIR ${CMAKE_SOURCE_DIR}/PicPlayer/src)
# 新增源码目录
set(EVENTLOOP_DIR ${CMAKE_SOURCE_DIR}/PicPlayer/src/EventLoop)
set(UTILS_DIR ${CMAKE_SOURCE_DIR}/PicPlayer/src/Utils)
set(PLAYERINS_DIR ${CMAKE_SOURCE_DIR}/PicPlayer/src/PlayerInstance)
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/PicPlayer/src/Imgui)
set(RENDER_DIR ${CMAKE_SOURCE_DIR}/PicPlayer/src/Render)
set(NODES_DIR ${CMAKE_SOURCE_DIR}/PicPlayer/src/NodeDataDef)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 定义第三方库路径
set(EXT_DIR ${CMAKE_SOURCE_DIR}/ext)

# 查找OpenGL库
find_package(OpenGL REQUIRED)
if(NOT OPENGL_FOUND)
    message(FATAL_ERROR "OpenGL library not found")
endif()

# 添加第三方库的头文件路径
include_directories(${EXT_DIR}/include
                    ${EXT_DIR}/include/GL
                    ${EXT_DIR}/include/GLFW
                    ${OPENGL_INCLUDE_DIR})
# 添加内部头文件路径
include_directories(${EVENTLOOP_DIR}
                    ${UTILS_DIR}
                    ${PLAYERINS_DIR}
                    ${IMGUI_DIR}
                    ${RENDER_DIR}
                    ${NODES_DIR})
# 添加第三方库的库文件路径
link_directories(
    ${EXT_DIR}/lib/glew
    ${EXT_DIR}/lib/glfw
    ${OPENGL_LIBRARY_DIRS}
)

# 使用 file(GLOB ...) 自动收集源文件
file(GLOB PROJECT_SOURCES
    "${SOURCE_DIR}/*.cpp"
    "${SOURCE_DIR}/*.h"
    "${EVENTLOOP_DIR}/*.cpp"
    "${EVENTLOOP_DIR}/*.h"
    "${UTILS_DIR}/*.cpp"
    "${UTILS_DIR}/*.h"
    "${PLAYERINS_DIR}/*.cpp"
    "${PLAYERINS_DIR}/*.h"
    "${IMGUI_DIR}/*.cpp"
    "${IMGUI_DIR}/*.h"
    "${RENDER_DIR}/*.cpp"
    "${RENDER_DIR}/*.h"
    "${NODES_DIR}/*.cpp"
    "${NODES_DIR}/*.h"
)

if(APPLE)
    # 添加 PicPlayerWindowForMac.mm 文件
    list(APPEND PROJECT_SOURCES "${PLAYERINS_DIR}/PicPlayerWindowForMac.mm")
endif()

# 使用 file(GLOB_RECURSE ...) 递归收集 IMGUI_DIR 中的所有 .cpp /.c 和 .h 文件
file(GLOB_RECURSE IMGUI_SOURCES
    "${IMGUI_DIR}/*.cpp"
    "${IMGUI_DIR}/*.c"
    "${IMGUI_DIR}/*.h"
)
# 将 IMGUI_SOURCES 添加到 PROJECT_SOURCES
list(APPEND PROJECT_SOURCES ${IMGUI_SOURCES})

# 创建共享库
add_library(PicPlayer SHARED ${PROJECT_SOURCES})

# 添加第三方库
target_link_libraries(PicPlayer PRIVATE
    glfw3
    GLEW
    OpenGL::GL
)

# 在 macOS 上添加 CoreGraphics 和 CoreFoundation 库
if(APPLE)
    # 查找CoreFoundation库
    find_library(COREFOUNDATION_LIBRARY CoreFoundation REQUIRED)
    # 查找其他必要的系统库
    find_library(COCOA_LIBRARY Cocoa REQUIRED)
    find_library(IOKIT_LIBRARY IOKit REQUIRED)
    find_library(COREVIDEO_LIBRARY CoreVideo REQUIRED)
    find_library(COREGRAPHICS_LIBRARY CoreGraphics REQUIRED)
    find_library(FOUNDATION_LIBRARY Foundation REQUIRED)
    target_link_libraries(PicPlayer PRIVATE
        ${COREFOUNDATION_LIBRARY}
        ${COCOA_LIBRARY}
        ${IOKIT_LIBRARY}
        ${COREVIDEO_LIBRARY}
        ${COREGRAPHICS_LIBRARY}
        ${FOUNDATION_LIBRARY}
    )
endif()

# 设置目标属性
set_target_properties(PicPlayer PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

# 安装目标
include(GNUInstallDirs)
install(TARGETS PicPlayer
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
