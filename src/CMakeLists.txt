cmake_minimum_required(VERSION 3.16)

project(ClientForFrameWork VERSION 0.1 LANGUAGES CXX)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 定义第三方库路径
set(EXT_DIR ${CMAKE_SOURCE_DIR}/../ext)
# 配置spdlog和通用头文件路径
include_directories(
    ${EXT_DIR}/include
    ${EXT_DIR}/include/spdlog
    ${CMAKE_SOURCE_DIR}/LogUtil
)

# 输出基目录（放在工程根下的 target）
set(OUTPUT_BASE_DIR ${CMAKE_SOURCE_DIR}/../target)
message("project setup ing... CMAKE_SOURCE_DIR will be in: ${CMAKE_SOURCE_DIR}")
# 根据平台选择子目录（区分 mac 与 win64）
if(APPLE)
  set(TARGET_PLATFORM_DIR Mac)
elseif(WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(TARGET_PLATFORM_DIR Win64)
else()
  message(FATAL_ERROR "Unsupported platform")
endif()

# 最终输出目录
set(OUTPUT_DIR ${OUTPUT_BASE_DIR}/${TARGET_PLATFORM_DIR})

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR}/lib)

# 添加子项目
add_subdirectory(ClientForFrame)
add_subdirectory(PicPlayer)

message("project setup complete. Output will be in: ${OUTPUT_DIR}")