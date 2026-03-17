# Framework 接口库：包含 IComponent 组件接口定义
cmake_minimum_required(VERSION 3.16)

# 启用 AUTOMOC
set(CMAKE_AUTOMOC ON)

# 查找 Qt 模块
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Qml)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Qml)

set(FRAMEWORK_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(FRAMEWORK_INTERFACE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Interface)
set(FRAMEWORK_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Framework 源文件（实现放在 src）
set(FRAMEWORK_SOURCES
    ${FRAMEWORK_SRC_DIR}/ComponentApiImpl.cpp    # C 风格 API 实现
    ${FRAMEWORK_SRC_DIR}/ComponentManager.cpp     # 组件管理器实现
    ${FRAMEWORK_SRC_DIR}/ComponentManager.h       # 组件管理器头文件
    ${FRAMEWORK_SRC_DIR}/ComponentManifest.cpp    # JSON 解析实现
    ${FRAMEWORK_SRC_DIR}/ComponentService.cpp     # 组件服务统一入口
)

# Framework 接口头文件（对外暴露）
set(FRAMEWORK_INTERFACE_HEADERS
    ${FRAMEWORK_INTERFACE_DIR}/IComponentData.h     # 数据结构定义
    ${FRAMEWORK_INTERFACE_DIR}/IComponent.h        # 组件接口定义（C++）
    ${FRAMEWORK_INTERFACE_DIR}/ComponentService.h  # 组件服务统一入口
)

# 创建静态库
add_library(Framework STATIC ${FRAMEWORK_SOURCES} ${FRAMEWORK_INTERFACE_HEADERS})

# 接口头文件目录（对外暴露）
target_include_directories(Framework PUBLIC
    ${FRAMEWORK_INTERFACE_DIR}
)

# 源文件目录（仅内部使用）
target_include_directories(Framework PRIVATE
    ${FRAMEWORK_SRC_DIR}
    ${CMAKE_SOURCE_DIR}/../ext/include  # xpack 头文件路径
)

# 链接 Qt Core 和 Qml
target_link_libraries(Framework PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Qml
)

# 设置接口头文件属性，使其对使用者可见
set_target_properties(Framework PROPERTIES
    PUBLIC_HEADER "${FRAMEWORK_INTERFACE_HEADERS}"
    PRIVATE_HEADER "${FRAMEWORK_SRC_DIR}/ComponentManager.h"
)

message(STATUS "Framework: built IComponent interface library")
message(STATUS "  - Interface headers: ${FRAMEWORK_INTERFACE_DIR}")
message(STATUS "  - Implementation sources: ${FRAMEWORK_SRC_DIR}")
