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
# 查找CoreFoundation库
find_library(COREFOUNDATION_LIBRARY CoreFoundation REQUIRED)
# 查找其他必要的系统库
find_library(COCOA_LIBRARY Cocoa REQUIRED)
find_library(IOKIT_LIBRARY IOKit REQUIRED)
find_library(COREVIDEO_LIBRARY CoreVideo REQUIRED)

# 添加第三方库的头文件路径
include_directories(${EXT_DIR}/include
                    ${EXT_DIR}/include/GL
                    ${EXT_DIR}/include/GLFW)
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
)

# 定义源码文件
set(PROJECT_SOURCES
    ${SOURCE_DIR}/PicPlayerApi.cpp
    ${SOURCE_DIR}/PicPlayerApi.h
    ${SOURCE_DIR}/PicPlayerGlobal.h
    ${SOURCE_DIR}/PicPlayerDataDef.h
    ${EVENTLOOP_DIR}/CallableObject.h
    ${EVENTLOOP_DIR}/EventMessage.h
    ${EVENTLOOP_DIR}/EventLoop.h
    ${EVENTLOOP_DIR}/EventLoop.cpp
    ${UTILS_DIR}/IDGenerator.h
    ${UTILS_DIR}/HandleRegister.h
    ${UTILS_DIR}/Singleton.h
    ${PLAYERINS_DIR}/PicPlayer.h
    ${PLAYERINS_DIR}/PicPlayer.cpp
    ${PLAYERINS_DIR}/PicPlayerCtrlBase.h
    ${PLAYERINS_DIR}/PicPlayerCtrlBase.cpp
    ${PLAYERINS_DIR}/PicPlayerCtrlDelegate.h
    ${PLAYERINS_DIR}/PicPlayerCtrlDelegate.cpp
    ${PLAYERINS_DIR}/PicPlayerGui.h
    ${PLAYERINS_DIR}/PicPlayerGui.cpp
    ${PLAYERINS_DIR}/PicPlayerHandleManager.h
    ${PLAYERINS_DIR}/PicPlayerHandleManager.cpp
    ${PLAYERINS_DIR}/PicPlayerShowWindow.h
    ${PLAYERINS_DIR}/PicPlayerShowWindow.cpp
    ${IMGUI_DIR}/glad/glad.h
    ${IMGUI_DIR}/glad/glad.c
    ${IMGUI_DIR}/glad/KHR/khrplatform.h
    ${IMGUI_DIR}/imconfig.h
    ${IMGUI_DIR}/imstb_textedit.h
    ${IMGUI_DIR}/imgui.h
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_impl_opengl3.h
    ${IMGUI_DIR}/imgui_impl_opengl3.cpp
    ${IMGUI_DIR}/imgui_impl_glfw.h
    ${IMGUI_DIR}/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/imgui_impl_opengl3_loader.h
    ${IMGUI_DIR}/imgui_internal.h
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${RENDER_DIR}/PicPlayerRender.h
    ${RENDER_DIR}/PicPlayerRender.cpp
    ${RENDER_DIR}/PicPlayerScene.h
    ${RENDER_DIR}/PicPlayerScene.cpp
    ${RENDER_DIR}/PicPlayerRenderSync.h
    ${RENDER_DIR}/PicPlayerRenderSync.cpp
    ${NODES_DIR}/NodesData.h
    ${NODES_DIR}/NodesData.cpp
)

# 创建共享库
add_library(PicPlayer SHARED ${PROJECT_SOURCES})

# 添加第三方库
target_link_libraries(PicPlayer PRIVATE
    glfw3
    GLEW
    ${COREFOUNDATION_LIBRARY}
    ${COCOA_LIBRARY}
    ${IOKIT_LIBRARY}
    ${COREVIDEO_LIBRARY}
)
# 设置目标属性
set_target_properties(PicPlayer PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

# 安装目标
include(GNUInstallDirs)
install(TARGETS PicPlayer
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
