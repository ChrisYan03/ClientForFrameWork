cmake_minimum_required(VERSION 3.16)

project(ClientForFrame VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# 设置源码目录
set(SOURCE_DIR ${CMAKE_SOURCE_DIR}/ClientForFrame/src)
set(PICMATCH_DIR ${CMAKE_SOURCE_DIR}/ClientForFrame/src/PicMatchComponent)
set(STBIMAGE_DIR ${CMAKE_SOURCE_DIR}/ClientForFrame/src/PicMatchComponent/StbImage)

# 定义源码文件
# 使用 file(GLOB ...) 自动收集源文件
file(GLOB PROJECT_SOURCES
    "${SOURCE_DIR}/*.cpp"
    "${SOURCE_DIR}/*.h"
    "${PICMATCH_DIR}/*.cpp"
    "${PICMATCH_DIR}/*.h"
    "${STBIMAGE_DIR}/*.cpp"
    "${STBIMAGE_DIR}/*.h"
)

# 添加 PicPlayer 库路径
set(PICPLAYER_DIR ${CMAKE_SOURCE_DIR}/PicPlayer)

# 添加 PicPlayer 库的头文件路径
include_directories(${PICPLAYER_DIR}/src)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(ClientForFrame
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
    target_link_libraries(ClientForFrame PRIVATE Qt${QT_VERSION_MAJOR}::Widgets PicPlayer)
    qt_finalize_executable(ClientForFrame)
else()
    if(ANDROID)
        add_library(ClientForFrame SHARED ${PROJECT_SOURCES})
        target_link_libraries(ClientForFrame PRIVATE Qt${QT_VERSION_MAJOR}::Widgets PicPlayer)
    else()
        add_executable(ClientForFrame ${PROJECT_SOURCES})
        target_link_libraries(ClientForFrame PRIVATE Qt${QT_VERSION_MAJOR}::Widgets PicPlayer)
    endif()
endif()

# 设置目标属性
set_target_properties(ClientForFrame PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# 安装目标
include(GNUInstallDirs)
install(TARGETS ClientForFrame
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
