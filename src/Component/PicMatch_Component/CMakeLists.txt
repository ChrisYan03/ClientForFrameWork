# PicMatch 独立组件：包含 SubComponent(PicPlayer、PicRecognition) 及本组件 UI，由主框架按配置加载
cmake_minimum_required(VERSION 3.16)

# 组件依赖 Qt Widgets / Qml / Quick（与主框架一致）
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Qml Quick)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Qml Quick)

set(PICMATCH_COMPONENT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# 先构建依赖库（PicPlayer、PicRecognition 在 SubComponent 下，不依赖 Qt，不启用 MOC/UIC）
add_subdirectory(${PICMATCH_COMPONENT_DIR}/SubComponent/PicPlayer ${CMAKE_BINARY_DIR}/PicPlayer)
add_subdirectory(${PICMATCH_COMPONENT_DIR}/SubComponent/PicRecognition ${CMAKE_BINARY_DIR}/PicRecognition)

# 仅对后续的 PicMatchComponent 启用 MOC/UIC（含 Q_OBJECT 的 Qt 代码）
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

# 本组件的 UI 与桥接代码
set(PICMATCH_SOURCE_DIR ${PICMATCH_COMPONENT_DIR}/src)
set(PICMATCH_RESOURCE_DIR ${PICMATCH_COMPONENT_DIR}/resource)
set(PICPLAYER_SRC_DIR ${PICMATCH_COMPONENT_DIR}/SubComponent/PicPlayer/src)
set(PICRECOGNITION_SRC_DIR ${PICMATCH_COMPONENT_DIR}/SubComponent/PicRecognition/src)

include_directories(
    ${PICMATCH_SOURCE_DIR}
    ${PICMATCH_SOURCE_DIR}/Common
    ${PICMATCH_SOURCE_DIR}/StbImage
    ${PICMATCH_SOURCE_DIR}/QmlBridge
    ${PICMATCH_SOURCE_DIR}/Core
    ${PICPLAYER_SRC_DIR}
    ${PICRECOGNITION_SRC_DIR}
)
include_directories(${CMAKE_SOURCE_DIR}/LogUtil)

file(GLOB PICMATCH_SOURCES
    "${PICMATCH_SOURCE_DIR}/Common/*.cpp"
    "${PICMATCH_SOURCE_DIR}/Common/*.h"
    "${PICMATCH_SOURCE_DIR}/StbImage/*.cpp"
    "${PICMATCH_SOURCE_DIR}/StbImage/*.h"
    "${PICMATCH_SOURCE_DIR}/Core/*.cpp"
    "${PICMATCH_SOURCE_DIR}/Core/*.h"
    "${PICMATCH_SOURCE_DIR}/QmlBridge/*.cpp"
    "${PICMATCH_SOURCE_DIR}/QmlBridge/*.h"
    "${PICMATCH_SOURCE_DIR}/PicMatchComponentApi.cpp"
    "${PICMATCH_SOURCE_DIR}/PicMatchComponentApi.h"
)
# 显式加入 FaceListModel，避免 GLOB 在 configure 时未包含新文件导致链接缺失
list(APPEND PICMATCH_SOURCES
    "${PICMATCH_SOURCE_DIR}/QmlBridge/FaceListModel.cpp"
    "${PICMATCH_SOURCE_DIR}/QmlBridge/FaceListModel.h"
    "${PICMATCH_SOURCE_DIR}/QmlBridge/FaceImageProvider.cpp"
    "${PICMATCH_SOURCE_DIR}/QmlBridge/FaceImageProvider.h"
)

add_library(PicMatchComponent SHARED ${PICMATCH_SOURCES})

if(WIN32)
    target_compile_definitions(PicMatchComponent PRIVATE PICMATCHCOMPONENT_EXPORTS)
endif()

target_link_libraries(PicMatchComponent PUBLIC
    PicPlayer
    PicRecognition
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Qml
    Qt${QT_VERSION_MAJOR}::Quick
)
target_include_directories(PicMatchComponent PUBLIC
    ${PICMATCH_SOURCE_DIR}
    ${PICPLAYER_SRC_DIR}
    ${PICRECOGNITION_SRC_DIR}
)

# 组件成果物输出到 exe 同盘下的 Component/PicMatch/bin（多配置下带 Release/Debug 子目录）
set(COMPONENT_PICMATCH_BIN "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/Component/PicMatch/bin")
set_target_properties(PicMatchComponent PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    RUNTIME_OUTPUT_DIRECTORY "${COMPONENT_PICMATCH_BIN}"
    LIBRARY_OUTPUT_DIRECTORY "${COMPONENT_PICMATCH_BIN}"
    ARCHIVE_OUTPUT_DIRECTORY "${COMPONENT_PICMATCH_BIN}"
)
# PicPlayer、PicRecognition 作为组件依赖，DLL 也输出到同一目录
set_target_properties(PicPlayer PicRecognition PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${COMPONENT_PICMATCH_BIN}"
    LIBRARY_OUTPUT_DIRECTORY "${COMPONENT_PICMATCH_BIN}"
    ARCHIVE_OUTPUT_DIRECTORY "${COMPONENT_PICMATCH_BIN}"
)

if(APPLE)
    target_link_libraries(PicMatchComponent PUBLIC
        "-framework OpenGL"
        "-framework AppKit"
        "-framework ImageIO"
        "-framework Metal"
    )
endif()

# 复制资源文件（图标）到输出目录，供运行时加载
add_custom_command(TARGET PicMatchComponent POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:PicMatchComponent>/resource/icons"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PICMATCH_RESOURCE_DIR}/icons/start.svg" "$<TARGET_FILE_DIR:PicMatchComponent>/resource/icons/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PICMATCH_RESOURCE_DIR}/icons/stop.svg" "$<TARGET_FILE_DIR:PicMatchComponent>/resource/icons/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PICMATCH_RESOURCE_DIR}/icons/settings.svg" "$<TARGET_FILE_DIR:PicMatchComponent>/resource/icons/"
    COMMENT "Copy PicMatch icons to output directory"
)

# 组件 QML 页与子组件复制到 bin/qml/，供主框架按 manifest.qmlPage 加载
add_custom_command(TARGET PicMatchComponent POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:PicMatchComponent>/qml/components"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PICMATCH_COMPONENT_DIR}/resource/qml/PicMatchPage.qml" "$<TARGET_FILE_DIR:PicMatchComponent>/qml/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PICMATCH_COMPONENT_DIR}/resource/qml/components/FaceShowPanel.qml" "$<TARGET_FILE_DIR:PicMatchComponent>/qml/components/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PICMATCH_COMPONENT_DIR}/resource/qml/components/PicMatchToolBar.qml" "$<TARGET_FILE_DIR:PicMatchComponent>/qml/components/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PICMATCH_COMPONENT_DIR}/resource/qml/components/PicMatchConfigPanel.qml" "$<TARGET_FILE_DIR:PicMatchComponent>/qml/components/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PICMATCH_COMPONENT_DIR}/resource/qml/components/qmldir" "$<TARGET_FILE_DIR:PicMatchComponent>/qml/components/"
    COMMENT "Copy PicMatch QML page and components to bin/qml for framework load"
)

# meta_info：图标与 manifest 复制到 exe 同盘下的 Component/PicMatch/meta_info（主程序据此加载 DLL 与图标）
add_custom_command(TARGET PicMatchComponent POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:PicMatchComponent>/../meta_info"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PICMATCH_RESOURCE_DIR}/icons/face_recognition.svg" "$<TARGET_FILE_DIR:PicMatchComponent>/../meta_info/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PICMATCH_COMPONENT_DIR}/resource/meta_info/manifest.json" "$<TARGET_FILE_DIR:PicMatchComponent>/../meta_info/"
    COMMENT "Copy PicMatch meta_info (icon + manifest) to Component/PicMatch/meta_info"
)

message(STATUS "PicMatch_Component: built PicMatchComponent (SubComponent PicPlayer + PicRecognition + UI) -> Component/PicMatch/bin, meta_info -> Component/PicMatch/meta_info")
