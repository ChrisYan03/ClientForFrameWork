cmake_minimum_required(VERSION 3.16)

project(PicPlayer VERSION 0.1 LANGUAGES CXX)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
    set(CMAKE_CXX_FLAGS_RELEASE /Zi)   #设置C/C++调试信息格式
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE /DEBUG) #生成exe时生成调试信
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE /DEBUG) #生成dll时生成调试信
    add_compile_options(-Od)
    # 添加UTF-8编译选项以支持spdlog
    add_compile_options(/utf-8)
elseif(APPLE)
    # 查找OpenGL库
    find_package(OpenGL REQUIRED)
    if(NOT OPENGL_FOUND)
        message(FATAL_ERROR "OpenGL library not found")
    endif()
endif()

# 设置源码目录（按当前目录结构：本 CMakeLists 位于 SubComponent/PicPlayer）
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(LOG_DIR ${CMAKE_SOURCE_DIR}/LogUtil)
message("LOG_DIR: ${LOG_DIR}")
set(EVENTLOOP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/EventLoop)
set(UTILS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/Utils)
set(PLAYERINS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/PlayerInstance)
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/Imgui)
set(RENDER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/Render)
set(NODES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/NodeDataDef)

# 添加第三方库的头文件路径
include_directories(${EXT_DIR}/include
                    ${EXT_DIR}/include/GL
                    ${EXT_DIR}/include/GLFW
                    ${OPENGL_INCLUDE_DIR})
# 添加内部头文件路径（含 SOURCE_DIR 以便包含 PicPlayerLog.h）
include_directories(${SOURCE_DIR}
                    ${LOG_DIR}
                    ${EVENTLOOP_DIR}
                    ${UTILS_DIR}
                    ${PLAYERINS_DIR}
                    ${IMGUI_DIR}
                    ${RENDER_DIR}
                    ${NODES_DIR})
# 添加第三方库的库文件路径
if(APPLE)
    link_directories(
        ${EXT_DIR}/lib/glew/Mac
        ${EXT_DIR}/lib/glfw/Mac
        ${EXT_DIR}/lib/spdlog/Mac
        ${OPENGL_LIBRARY_DIRS}
    )
else()
    link_directories(
        ${EXT_DIR}/lib/glew/Win64
        ${EXT_DIR}/lib/glfw/Win64
        ${EXT_DIR}/lib/spdlog/Win64
        ${OPENGL_LIBRARY_DIRS}
    )
endif()

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
# 使用 file(GLOB_RECURSE ...) 递归收集 RENDER_DIR 中的所有 .cpp 和 .h 文件
file(GLOB_RECURSE RENDER_SOURCES
    "${RENDER_DIR}/*.cpp"
    "${RENDER_DIR}/*.h"
)
# 将 IMGUI_SOURCES 和 RENDER_SOURCES添加到 PROJECT_SOURCES
list(APPEND PROJECT_SOURCES ${IMGUI_SOURCES})
list(APPEND PROJECT_SOURCES ${RENDER_SOURCES})

# 创建共享库
add_library(PicPlayer SHARED ${PROJECT_SOURCES})

if(APPLE)
    # 添加第三方库
    target_link_libraries(PicPlayer PRIVATE
        glfw3
        GLEW
        spdlog
        "-framework OpenGL"
    )
else()
    target_compile_definitions(PicPlayer PRIVATE PICPLAYER_EXPORTS)
    target_link_libraries(PicPlayer PRIVATE 
        ${EXT_DIR}/lib/glew/Win64/glew32.lib
        ${EXT_DIR}/lib/glfw/Win64/glfw3.lib
        ${EXT_DIR}/lib/spdlog/Win64/spdlog.lib
        opengl32
    )        
endif()

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

# 确保输出到统一的 bin 目录而不是配置子目录
set_target_properties(PicPlayer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
)

message("PicPlayer output run_directory: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message("PicPlayer output lib_directory: ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")