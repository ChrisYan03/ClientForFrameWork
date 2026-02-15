cmake_minimum_required(VERSION 3.16)

project(PicRecognition VERSION 0.1 LANGUAGES CXX)

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
endif()

# 设置 OpenCV 路径
set(OPENCV_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../..)
message(STATUS "OpenCV Root Directory: ${OPENCV_ROOT_DIR}")
# 设置 OpenCV 头文件目录
set(OpenCV_INCLUDE_DIRS ${OPENCV_ROOT_DIR}/ext/include)
# 添加第三方库的头文件路径
include_directories(${OpenCV_INCLUDE_DIRS})

# 设置源码目录
set(PicPlayer_DIR ${CMAKE_SOURCE_DIR}/PicPlayer/src)
set(SOURCE_DIR ${CMAKE_SOURCE_DIR}/PicRecognition/src)
# 新增源码目录
set(LOG_DIR ${CMAKE_SOURCE_DIR}/LogUtil)
message("LOG_DIR: ${LOG_DIR}")
# 添加内部头文件路径
include_directories(${PicPlayer_DIR}
                    ${LOG_DIR}
                    ${SOURCE_DIR})

# 使用 file(GLOB ...) 自动收集源文件
file(GLOB_RECURSE PROJECT_SOURCES_RECURSIVE
    "${SOURCE_DIR}/*.cpp"
    "${SOURCE_DIR}/*.h"
)
list(APPEND PROJECT_SOURCES ${PROJECT_SOURCES_RECURSIVE})
list(REMOVE_DUPLICATES PROJECT_SOURCES)
# 创建共享库
add_library(PicRecognition SHARED ${PROJECT_SOURCES})
# 设置库文件路径
set(OpenCV_LIBRARY_DIR ${OPENCV_ROOT_DIR}/ext/lib/opencv2/Win64)
message(STATUS "OpenCV_LIBRARY_DIR Directory: ${OpenCV_LIBRARY_DIR}")
# 添加第三方库的库文件路径
if(APPLE)
    link_directories(
        ${EXT_DIR}/lib/glew/Mac
        ${EXT_DIR}/lib/glfw/Mac
        ${EXT_DIR}/lib/spdlog/Mac
        ${OPENGL_LIBRARY_DIRS}
    )
    # 添加第三方库
    target_link_libraries(PicRecognition PRIVATE
        glfw3
        GLEW
        spdlog
        OpenGL::GL
    )
else()
    link_directories(
        ${OpenCV_LIBRARY_DIR}
        ${EXT_DIR}/lib/spdlog/Win64
    )
    target_compile_definitions(PicRecognition PRIVATE PICRECOGNITION_EXPORTS)
    target_link_libraries(PicRecognition PRIVATE
        ${OpenCV_LIBRARY_DIR}/opencv_calib3d4140.lib
        ${OpenCV_LIBRARY_DIR}/opencv_core4140.lib
        ${OpenCV_LIBRARY_DIR}/opencv_dnn4140.lib
        ${OpenCV_LIBRARY_DIR}/opencv_face4140.lib
        ${OpenCV_LIBRARY_DIR}/opencv_flann4140.lib
        ${OpenCV_LIBRARY_DIR}/opencv_highgui4140.lib
        ${OpenCV_LIBRARY_DIR}/opencv_imgcodecs4140.lib
        ${OpenCV_LIBRARY_DIR}/opencv_imgproc4140.lib
        ${OpenCV_LIBRARY_DIR}/opencv_objdetect4140.lib
        ${OpenCV_LIBRARY_DIR}/opencv_videoio4140.lib
        ${EXT_DIR}/lib/spdlog/Win64/spdlog.lib
    )  
endif()
    
# 设置目标属性
set_target_properties(PicRecognition PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
)

message("PicRecognition output run_directory: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message("PicRecognition output lib_directory: ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")