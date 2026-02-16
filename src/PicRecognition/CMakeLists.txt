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

# 添加第三方库的库文件路径
if(APPLE)
    set(OpenCV_LIBRARY_DIR ${OPENCV_ROOT_DIR}/ext/lib/opencv2/Mac)
    set(SPDLOG_LIBRARY_DIR ${OPENCV_ROOT_DIR}/ext/lib/spdlog/Mac)
    message(STATUS "OpenCV_LIBRARY_DIR Directory: ${OpenCV_LIBRARY_DIR}")
    # 创建导入库目标
    add_library(opencv_calib3d SHARED IMPORTED)
    set_target_properties(opencv_calib3d PROPERTIES
        IMPORTED_LOCATION ${OpenCV_LIBRARY_DIR}/libopencv_calib3d.4.14.0.dylib
    )
    
    add_library(opencv_core SHARED IMPORTED)
    set_target_properties(opencv_core PROPERTIES
        IMPORTED_LOCATION ${OpenCV_LIBRARY_DIR}/libopencv_core.4.14.0.dylib
    )
    
    add_library(opencv_dnn SHARED IMPORTED)
    set_target_properties(opencv_dnn PROPERTIES
        IMPORTED_LOCATION ${OpenCV_LIBRARY_DIR}/libopencv_dnn.4.14.0.dylib
    )
    
    add_library(opencv_face SHARED IMPORTED)
    set_target_properties(opencv_face PROPERTIES
        IMPORTED_LOCATION ${OpenCV_LIBRARY_DIR}/libopencv_face.4.14.0.dylib
    )
    
    add_library(opencv_flann SHARED IMPORTED)
    set_target_properties(opencv_flann PROPERTIES
        IMPORTED_LOCATION ${OpenCV_LIBRARY_DIR}/libopencv_flann.4.14.0.dylib
    )
    
    add_library(opencv_highgui SHARED IMPORTED)
    set_target_properties(opencv_highgui PROPERTIES
        IMPORTED_LOCATION ${OpenCV_LIBRARY_DIR}/libopencv_highgui.4.14.0.dylib
    )
    
    add_library(opencv_imgcodecs SHARED IMPORTED)
    set_target_properties(opencv_imgcodecs PROPERTIES
        IMPORTED_LOCATION ${OpenCV_LIBRARY_DIR}/libopencv_imgcodecs.4.14.0.dylib
    )
    
    add_library(opencv_imgproc SHARED IMPORTED)
    set_target_properties(opencv_imgproc PROPERTIES
        IMPORTED_LOCATION ${OpenCV_LIBRARY_DIR}/libopencv_imgproc.4.14.0.dylib
    )
    
    add_library(opencv_objdetect SHARED IMPORTED)
    set_target_properties(opencv_objdetect PROPERTIES
        IMPORTED_LOCATION ${OpenCV_LIBRARY_DIR}/libopencv_objdetect.4.14.0.dylib
    )
    
    add_library(opencv_videoio SHARED IMPORTED)
    set_target_properties(opencv_videoio PROPERTIES
        IMPORTED_LOCATION ${OpenCV_LIBRARY_DIR}/libopencv_videoio.4.14.0.dylib
    )
    
    add_library(spdlog STATIC IMPORTED)
    set_target_properties(spdlog PROPERTIES
        IMPORTED_LOCATION ${SPDLOG_LIBRARY_DIR}/libspdlog.a
    )
    
    # 链接导入的库
    target_link_libraries(PicRecognition PRIVATE
        opencv_calib3d
        opencv_core
        opencv_dnn
        opencv_face
        opencv_flann
        opencv_highgui
        opencv_imgcodecs
        opencv_imgproc
        opencv_objdetect
        opencv_videoio
        spdlog
    )
else()
    # 设置库文件路径
    set(OpenCV_LIBRARY_DIR ${OPENCV_ROOT_DIR}/ext/lib/opencv2/Win64)
    message(STATUS "OpenCV_LIBRARY_DIR Directory: ${OpenCV_LIBRARY_DIR}")
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