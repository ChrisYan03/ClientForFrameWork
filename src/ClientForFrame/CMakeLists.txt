cmake_minimum_required(VERSION 3.16)

project(ClientForFrame VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(APPLE)
    set(CMAKE_PREFIX_PATH "/usr/local/Qt-6.8.1")
else()
    set(CMAKE_PREFIX_PATH "D:/Qt/6.5.3/msvc2019_64/lib/cmake")
    set(CMAKE_CXX_FLAGS_RELEASE /Zi)   #设置C/C++调试信息格式
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE /DEBUG) #生成exe时生成调试信
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE /DEBUG) #生成dll时生成调试信
    add_compile_options(-Od)
    # 添加UTF-8编译选项以支持spdlog
    add_compile_options(/utf-8)
endif()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

message("ClientForFrame qt_directory: Qt${QT_VERSION_MAJOR} found at ${Qt${QT_VERSION_MAJOR}_DIR}")
# 设置源码目录
set(SOURCE_DIR ${CMAKE_SOURCE_DIR}/ClientForFrame/src)
set(PICMATCH_DIR ${CMAKE_SOURCE_DIR}/ClientForFrame/src/PicMatchComponent)
set(TITLE_DIR ${CMAKE_SOURCE_DIR}/ClientForFrame/src/TitleComponent)
set(STBIMAGE_DIR ${CMAKE_SOURCE_DIR}/ClientForFrame/src/PicMatchComponent/StbImage)
message("TITLE_DIR source_dir: ${TITLE_DIR}")

# 定义源码文件
# 使用 file(GLOB ...) 自动收集源文件
file(GLOB PROJECT_SOURCES
    "${TITLE_DIR}/*.cpp"
    "${TITLE_DIR}/*.h"
    "${SOURCE_DIR}/*.cpp"
    "${SOURCE_DIR}/*.h"
    "${PICMATCH_DIR}/*.cpp"
    "${PICMATCH_DIR}/*.h"
    "${STBIMAGE_DIR}/*.cpp"
    "${STBIMAGE_DIR}/*.h"
)

# 添加 PicPlayer 库路径
set(PICPLAYER_DIR ${CMAKE_SOURCE_DIR}/PicPlayer)
message("picplayer_dir: ${PICPLAYER_DIR}")
# 添加 PicPlayer 库的头文件路径
include_directories(${PICPLAYER_DIR}/src)
# 添加 PicRecognition 库路径
set(PICRECOGNITION_DIR ${CMAKE_SOURCE_DIR}/PicRecognition)
message("picrecognition_dir: ${PICRECOGNITION_DIR}")
# 添加 PicRecognition 库的头文件路径
include_directories(${PICRECOGNITION_DIR}/src)
set(OpenCV_LIBRARY_DIR ${CMAKE_SOURCE_DIR}/../ext/lib/opencv2/Win64)
message(STATUS "OpenCV_LIBRARY_DIR Directory: ${OpenCV_LIBRARY_DIR}")
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(ClientForFrame
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
    target_link_libraries(ClientForFrame PRIVATE Qt${QT_VERSION_MAJOR}::Widgets PicPlayer PicRecognition)
    qt_finalize_executable(ClientForFrame)
else()
    if(ANDROID)
        add_library(ClientForFrame SHARED ${PROJECT_SOURCES})
        target_link_libraries(ClientForFrame PRIVATE Qt${QT_VERSION_MAJOR}::Widgets PicPlayer PicRecognition)
    else()
        add_executable(ClientForFrame ${PROJECT_SOURCES})
        target_link_libraries(ClientForFrame PRIVATE Qt${QT_VERSION_MAJOR}::Widgets PicPlayer PicRecognition)
    endif()
endif()

if(WIN32)
    target_link_libraries(ClientForFrame PRIVATE
        ${CMAKE_SOURCE_DIR}/../target/Win64/lib/PicPlayer.lib
        ${CMAKE_SOURCE_DIR}/../target/Win64/lib/PicRecognition.lib
    )
endif()

# 设置目标属性
set_target_properties(ClientForFrame PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)
# 确保输出到统一的 bin 目录而不是配置子目录
set_target_properties(ClientForFrame PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
)

message("ClientForFrame output run_directory: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message("ClientForFrame output lib_directory: ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")