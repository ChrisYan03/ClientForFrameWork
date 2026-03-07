cmake_minimum_required(VERSION 3.16)

project(ClientForFrameWork VERSION 0.1 LANGUAGES CXX)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 定义第三方库路径
set(EXT_DIR ${CMAKE_SOURCE_DIR}/../ext)
# 配置spdlog和通用头文件路径
include_directories(
    ${EXT_DIR}/include
    ${EXT_DIR}/include/spdlog
    ${CMAKE_SOURCE_DIR}/LogUtil
    ${CMAKE_SOURCE_DIR}/Common
)

# 输出基目录（放在工程根下��� target）
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

# macOS 统一配置：Qt 路径、避免 AGL 框架（可执行与 dylib 均使用过滤链接器）
if(APPLE)
  set(CMAKE_PREFIX_PATH "/usr/local/Qt-6.8.1")
  set(QT_MAC_OPENGL_IMPLEMENTATION "CoreProfile")
  set(AGL_FILTER_SCRIPT "${CMAKE_SOURCE_DIR}/filter_agl_linker.sh")
  if(EXISTS "${AGL_FILTER_SCRIPT}")
    execute_process(COMMAND chmod +x "${AGL_FILTER_SCRIPT}")
    set(CMAKE_CXX_LINK_EXECUTABLE "${AGL_FILTER_SCRIPT} <CMAKE_CXX_COMPILER> <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
    # 共享库必须带 -dynamiclib，否则会按可执行文件链接并报 _main 未定义
    set(CMAKE_CXX_CREATE_SHARED_LIBRARY "${AGL_FILTER_SCRIPT} <CMAKE_CXX_COMPILER> <CMAKE_SHARED_LIBRARY_CXX_FLAGS> -dynamiclib -Wl,-headerpad_max_install_names <FLAGS> <LINK_FLAGS> -o <TARGET> <OBJECTS> <LINK_LIBRARIES>")
    message(STATUS "Mac: AGL filter enabled for executable and shared library linking")
  endif()
endif()

# 添加子项目：先构建组件（含 PicPlayer、PicRecognition），再构建主框架
add_subdirectory(Component/PicMatch_Component)
add_subdirectory(ClientForFrame)

message("project setup complete. Output will be in: ${OUTPUT_DIR}")