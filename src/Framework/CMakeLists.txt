# Framework V3.0 - 支持4类组件的轻量级框架
cmake_minimum_required(VERSION 3.16)

# 启用 AUTOMOC
set(CMAKE_AUTOMOC ON)

# 查找 Qt 模块
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Qml)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Qml)

# 目录定义
set(FRAMEWORK_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(FRAMEWORK_INTERFACE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/interface)
set(FRAMEWORK_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(FRAMEWORK_LOADERS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/Loaders)

# Framework 源文件
set(FRAMEWORK_SOURCES
    # 核心实现
    ${FRAMEWORK_SRC_DIR}/ComponentManager.cpp
    ${FRAMEWORK_SRC_DIR}/ComponentLoaderFactory.cpp
    ${FRAMEWORK_SRC_DIR}/ComponentManifest.cpp
    ${FRAMEWORK_SRC_DIR}/ComponentService.cpp

    # 组件加载器
    ${FRAMEWORK_LOADERS_DIR}/NativeDllComponentLoader.cpp
    ${FRAMEWORK_LOADERS_DIR}/StandaloneExeComponentLoader.cpp
    ${FRAMEWORK_LOADERS_DIR}/WebUrlComponentLoader.cpp
    ${FRAMEWORK_LOADERS_DIR}/EmbeddedExeComponentLoader.cpp
)

# Framework 对外暴露的头文件
set(FRAMEWORK_PUBLIC_HEADERS
    # 主框架接口
    ${FRAMEWORK_INCLUDE_DIR}/ComponentService.h
    ${FRAMEWORK_INCLUDE_DIR}/ComponentTypes.h

    # 组件接口
    ${FRAMEWORK_INTERFACE_DIR}/IComponent.h
)

# 内部头文件
set(FRAMEWORK_PRIVATE_HEADERS
    ${FRAMEWORK_SRC_DIR}/ComponentManager.h
    ${FRAMEWORK_SRC_DIR}/ComponentLoaderFactory.h
    ${FRAMEWORK_LOADERS_DIR}/NativeDllComponentLoader.h
    ${FRAMEWORK_LOADERS_DIR}/StandaloneExeComponentLoader.h
    ${FRAMEWORK_LOADERS_DIR}/WebUrlComponentLoader.h
    ${FRAMEWORK_LOADERS_DIR}/EmbeddedExeComponentLoader.h
)

# 创建静态库
add_library(Framework STATIC ${FRAMEWORK_SOURCES} ${FRAMEWORK_PUBLIC_HEADERS} ${FRAMEWORK_PRIVATE_HEADERS})

# 对外暴露的include目录
target_include_directories(Framework PUBLIC
    $<BUILD_INTERFACE:${FRAMEWORK_INCLUDE_DIR}>
    $<BUILD_INTERFACE:${FRAMEWORK_INTERFACE_DIR}>
)

# 内部使用的include目录
target_include_directories(Framework PRIVATE
    ${FRAMEWORK_SRC_DIR}
    ${FRAMEWORK_LOADERS_DIR}
    ${CMAKE_SOURCE_DIR}/../ext/include  # xpack 头文件路径
)

# 链接 Qt 库
target_link_libraries(Framework PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Qml
)

# 设置头文件属性
set_target_properties(Framework PROPERTIES
    PUBLIC_HEADER "${FRAMEWORK_PUBLIC_HEADERS}"
    PRIVATE_HEADER "${FRAMEWORK_PRIVATE_HEADERS}"
    VERSION 3.0.0
    SOVERSION 3
)

# 编译定义
target_compile_definitions(Framework PRIVATE
    FRAMEWORK_VERSION_MAJOR=3
    FRAMEWORK_VERSION_MINOR=0
    FRAMEWORK_VERSION_PATCH=0
)

message(STATUS "Framework V3.0: 4类组件支持架构")
message(STATUS "  - Public headers: ${FRAMEWORK_INCLUDE_DIR}")
message(STATUS "  - Interface headers: ${FRAMEWORK_INTERFACE_DIR}")
message(STATUS "  - Source files: ${FRAMEWORK_SOURCES}")
message(STATUS "  - Loaders: NativeDll, StandaloneExe, WebUrl, EmbeddedExe")