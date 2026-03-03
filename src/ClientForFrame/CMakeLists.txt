cmake_minimum_required(VERSION 3.16)

project(ClientForFrame VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(APPLE)
    set(CMAKE_PREFIX_PATH "/usr/local/Qt-6.8.1")
    # 解决AGL框架找不到的问题 - M1芯片终极方案
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum OS X deployment version")
    set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "Build architectures")
    
    # 关键改进
    # 方法1：使用链接器标志忽略未解析的符号
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-dead_strip_dylibs -Wl,-undefined,dynamic_lookup")
    
    # 设置Qt使用Cocoa而不是Carbon
    set(QT_MAC_OPENGL_IMPLEMENTATION "CoreProfile")

    # 创建链接器过滤脚本的完整路径
    set(LINKER_FILTER_SCRIPT "${CMAKE_SOURCE_DIR}/filter_agl_linker.sh")
    
    # 使脚本可执行
    execute_process(COMMAND chmod +x ${LINKER_FILTER_SCRIPT})
    
    # 设置自定义链接器
    set(CMAKE_CXX_LINK_EXECUTABLE "${LINKER_FILTER_SCRIPT} <CMAKE_CXX_COMPILER> <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
    
else()
    set(CMAKE_PREFIX_PATH "D:/Qt/6.5.3/msvc2019_64/lib/cmake")
    set(CMAKE_CXX_FLAGS_RELEASE /Zi)   #设置C/C++调试信息格式
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE /DEBUG) #生成exe时生成调试信
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE /DEBUG) #生成dll时生成调试信
    add_compile_options(-Od)
    # 添加UTF-8编译选项以支持spdlog
    add_compile_options(/utf-8)
endif()

# 可选：设为 ON 使用 QML 界面，OFF 使用原有 Qt Widgets 界面
set(USE_QML ON CACHE BOOL "Use QML for UI instead of Qt Widgets")

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

if(USE_QML)
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Quick QuickControls2)
endif()

message("ClientForFrame qt_directory: Qt${QT_VERSION_MAJOR} found at ${Qt${QT_VERSION_MAJOR}_DIR}")
# 设置源码目录与资源目录
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(RESOURCE_DIR ${SOURCE_DIR}/resource)
include_directories(${SOURCE_DIR})

# 定义源码文件
# 使用 file(GLOB ...) 自动收集源文件
file(GLOB_RECURSE PROJECT_SOURCES_RECURSIVE
    "${SOURCE_DIR}/*.cpp"
    "${SOURCE_DIR}/*.h"
    "${RESOURCE_DIR}/*.qrc"
)
list(APPEND PROJECT_SOURCES ${PROJECT_SOURCES_RECURSIVE})
if(USE_QML)
    list(REMOVE_ITEM PROJECT_SOURCES "${SOURCE_DIR}/main.cpp")
else()
    list(REMOVE_ITEM PROJECT_SOURCES "${SOURCE_DIR}/main_qml.cpp"
        "${SOURCE_DIR}/QmlBridge/AppController.cpp"
        "${SOURCE_DIR}/QmlBridge/AppController.h"
        "${SOURCE_DIR}/QmlBridge/PlayerHostItem.cpp"
        "${SOURCE_DIR}/QmlBridge/PlayerHostItem.h")
endif()
list(REMOVE_DUPLICATES PROJECT_SOURCES)

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
    # USE_QML 时用 add_executable 避免 qt_finalize_executable 触发的 QML 导入扫描（在部分环境会失败）
    if(USE_QML)
        add_executable(ClientForFrame ${PROJECT_SOURCES})
        target_link_libraries(ClientForFrame PRIVATE
            Qt${QT_VERSION_MAJOR}::Core
            Qt${QT_VERSION_MAJOR}::Gui
            Qt${QT_VERSION_MAJOR}::Widgets
            Qt${QT_VERSION_MAJOR}::Quick
            Qt${QT_VERSION_MAJOR}::QuickControls2
            PicPlayer
            PicRecognition
        )
        if(APPLE)
            target_link_libraries(ClientForFrame PRIVATE
                "-framework OpenGL"
                "-framework AppKit"
                "-framework ImageIO"
                "-framework Metal"
                "-framework IOKit"
                "-framework DiskArbitration"
                "-framework UniformTypeIdentifiers"
            )
            set_target_properties(ClientForFrame PROPERTIES
                LINK_FLAGS "-Wl,-framework,OpenGL -Wl,-framework,AppKit -Wl,-framework,ImageIO -Wl,-framework,Metal -Wl,-framework,IOKit -Wl,-framework,DiskArbitration -Wl,-framework,UniformTypeIdentifiers -Wl,-undefined,dynamic_lookup"
            )
            set_target_properties(ClientForFrame PROPERTIES
                INSTALL_RPATH "@executable_path/../Frameworks"
            )
        endif()
    else()
        qt_add_executable(ClientForFrame
            MANUAL_FINALIZATION
            ${PROJECT_SOURCES}
        )
        # M1芯片Mac的特殊处理
        if(APPLE)
            # 先链接我们的库
            target_link_libraries(ClientForFrame PRIVATE PicPlayer PicRecognition)
            # 手动指定Qt库，避免自动添加AGL
            target_link_libraries(ClientForFrame PRIVATE 
                ${Qt${QT_VERSION_MAJOR}Widgets_LIBRARIES}
                ${Qt${QT_VERSION_MAJOR}Gui_LIBRARIES}
                ${Qt${QT_VERSION_MAJOR}Core_LIBRARIES}
            )
            # 显式指定需要的框架，但排除AGL
            target_link_libraries(ClientForFrame PRIVATE 
                "-framework OpenGL"
                "-framework AppKit" 
                "-framework ImageIO"
                "-framework Metal"
                "-framework IOKit"
                "-framework DiskArbitration"
                "-framework UniformTypeIdentifiers"
            )
            set_target_properties(ClientForFrame PROPERTIES
                LINK_FLAGS "-Wl,-framework,OpenGL -Wl,-framework,AppKit -Wl,-framework,ImageIO -Wl,-framework,Metal -Wl,-framework,IOKit -Wl,-framework,DiskArbitration -Wl,-framework,UniformTypeIdentifiers -Wl,-undefined,dynamic_lookup"
            )
            set_target_properties(ClientForFrame PROPERTIES
                INSTALL_RPATH "@executable_path/../Frameworks"
            )
        else()
            target_link_libraries(ClientForFrame PRIVATE Qt${QT_VERSION_MAJOR}::Widgets PicPlayer PicRecognition)
        endif()
        qt_finalize_executable(ClientForFrame)
    endif()
else()
    if(ANDROID)
        add_library(ClientForFrame SHARED ${PROJECT_SOURCES})
        target_link_libraries(ClientForFrame PRIVATE Qt${QT_VERSION_MAJOR}::Widgets PicPlayer PicRecognition)
    else()
        add_executable(ClientForFrame ${PROJECT_SOURCES})
        target_link_libraries(ClientForFrame PRIVATE Qt${QT_VERSION_MAJOR}::Widgets PicPlayer PicRecognition)
    endif()
endif()

# 添加 Qt 资源（qrc），图标等从 :/icons/xxx 加载（FILES 必须为相对路径，相对 CMAKE_CURRENT_SOURCE_DIR）
qt_add_resources(ClientForFrame "resources"
    PREFIX "/"
    BASE "${CMAKE_CURRENT_SOURCE_DIR}"
    FILES "src/resource/resource.qrc"
)
if(USE_QML)
    qt_add_resources(ClientForFrame "qml_resources"
        PREFIX "/"
        BASE "${CMAKE_CURRENT_SOURCE_DIR}"
        FILES "src/resource/qml.qrc"
    )
endif()
if(WIN32)
    target_link_libraries(ClientForFrame PRIVATE
        ${CMAKE_SOURCE_DIR}/../target/Win64/lib/PicPlayer.lib
        ${CMAKE_SOURCE_DIR}/../target/Win64/lib/PicRecognition.lib
        dwmapi
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
