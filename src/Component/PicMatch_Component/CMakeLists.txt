# PicMatch 特性组件：依赖共性组件 PicPlayer + 本地 PicRecognition + 本组件 UI
# 共性组件 PicPlayer 成果物（头文件）由 post-build 脚本拷贝到 deps/PicPlayer
cmake_minimum_required(VERSION 3.16)

# 组件依赖 Qt Widgets / Qml / Quick（与主框架一致）
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Qml Quick)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Qml Quick LinguistTools)

set(PICMATCH_COMPONENT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# 仅构建本地子组件 PicRecognition（不依赖 Qt，不启用 MOC/UIC）
add_subdirectory(${PICMATCH_COMPONENT_DIR}/SubComponent/PicRecognition ${CMAKE_BINARY_DIR}/PicRecognition)

# 仅对后续的 PicMatchComponent 启用 MOC/UIC（含 Q_OBJECT 的 Qt 代码）
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

# 本组件的 UI 与桥接代码
set(PICMATCH_SOURCE_DIR ${PICMATCH_COMPONENT_DIR}/src)
set(PICMATCH_RESOURCE_DIR ${PICMATCH_COMPONENT_DIR}/resource)
# PicPlayer 头文件从 deps/PicPlayer（由 post-build 从 Common 拷贝）引入
set(PICPLAYER_INCLUDE_DIR ${PICMATCH_COMPONENT_DIR}/deps/PicPlayer/include)
set(PICRECOGNITION_SRC_DIR ${PICMATCH_COMPONENT_DIR}/SubComponent/PicRecognition/src)

include_directories(
    ${PICMATCH_SOURCE_DIR}
    ${PICMATCH_SOURCE_DIR}/Common
    ${PICMATCH_SOURCE_DIR}/StbImage
    ${PICMATCH_SOURCE_DIR}/QmlBridge
    ${PICMATCH_SOURCE_DIR}/Core
    ${PICPLAYER_INCLUDE_DIR}
    ${PICRECOGNITION_SRC_DIR}
    ${CMAKE_SOURCE_DIR}/Framework/interface
)
include_directories(${CMAKE_SOURCE_DIR}/LogUtil)

file(GLOB PICMATCH_SOURCES
    "${PICMATCH_SOURCE_DIR}/StbImage/*.cpp"
    "${PICMATCH_SOURCE_DIR}/StbImage/*.h"
    "${PICMATCH_SOURCE_DIR}/Core/*.cpp"
    "${PICMATCH_SOURCE_DIR}/Core/*.h"
    "${PICMATCH_SOURCE_DIR}/QmlBridge/*.cpp"
    "${PICMATCH_SOURCE_DIR}/QmlBridge/*.h"
    "${PICMATCH_SOURCE_DIR}/PicMatchComponentGlobal.h"
    "${PICMATCH_SOURCE_DIR}/PicMatchComponent.cpp"
    "${PICMATCH_SOURCE_DIR}/PicMatchComponent.h"
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
    Framework
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
    ${PICPLAYER_INCLUDE_DIR}
    ${PICRECOGNITION_SRC_DIR}
)

# 组件成果物输出到 exe 同盘下的 Component/PicMatch/bin
set(COMPONENT_PICMATCH_BIN "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Component/PicMatch/bin")
set_target_properties(PicMatchComponent PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    RUNTIME_OUTPUT_DIRECTORY "${COMPONENT_PICMATCH_BIN}"
    LIBRARY_OUTPUT_DIRECTORY "${COMPONENT_PICMATCH_BIN}"
    ARCHIVE_OUTPUT_DIRECTORY "${COMPONENT_PICMATCH_BIN}"
)
# PicRecognition 作为组件依赖，DLL 也输出到同一目录
set_target_properties(PicRecognition PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${COMPONENT_PICMATCH_BIN}"
    LIBRARY_OUTPUT_DIRECTORY "${COMPONENT_PICMATCH_BIN}"
    ARCHIVE_OUTPUT_DIRECTORY "${COMPONENT_PICMATCH_BIN}"
)

# VS 多配置生成器下，主程序位于 .../bin/<Config>，组件也必须落在同级目录：
#   .../bin/<Config>/Component/PicMatch/bin
# 否则会出现新 DLL 生成在 .../bin/Component/.../<Config>，运行时却加载到旧 DLL 的问题。
if(CMAKE_CONFIGURATION_TYPES)
    foreach(cfg ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER "${cfg}" cfg_upper)
        set(component_cfg_bin "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${cfg}/Component/PicMatch/bin")
        set_target_properties(PicMatchComponent PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY_${cfg_upper} "${component_cfg_bin}"
            LIBRARY_OUTPUT_DIRECTORY_${cfg_upper} "${component_cfg_bin}"
            ARCHIVE_OUTPUT_DIRECTORY_${cfg_upper} "${component_cfg_bin}"
        )
        set_target_properties(PicRecognition PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY_${cfg_upper} "${component_cfg_bin}"
            LIBRARY_OUTPUT_DIRECTORY_${cfg_upper} "${component_cfg_bin}"
            ARCHIVE_OUTPUT_DIRECTORY_${cfg_upper} "${component_cfg_bin}"
        )
    endforeach()
endif()

if(APPLE)
    target_link_libraries(PicMatchComponent PUBLIC
        "-framework OpenGL"
        "-framework AppKit"
        "-framework ImageIO"
        "-framework Metal"
    )
endif()

# ========== post-build: 将共性组件 PicPlayer 的接口头文件拷贝到 deps/PicPlayer ==========
# 触发时机：每次构建前
# 结果：PicPlayerApi.h + PicPlayerDataDef.h 被复制到 deps/PicPlayer/include，
#      使 PicMatch_Component 可以通过 deps/PicPlayer/include 引入 PicPlayer 接口
add_custom_command(TARGET PicMatchComponent PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${PICMATCH_COMPONENT_DIR}/deps/PicPlayer/include"
    COMMENT "Ensure deps/PicPlayer/include directory exists"
)
add_custom_command(TARGET PicMatchComponent PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/Component/Common/PicPlayer/src/PicPlayerApi.h" "${PICMATCH_COMPONENT_DIR}/deps/PicPlayer/include/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/Component/Common/PicPlayer/src/PicPlayerDataDef.h" "${PICMATCH_COMPONENT_DIR}/deps/PicPlayer/include/"
)

# 复制资源文件（图标、皮肤）到输出目录，供运行时加载
add_custom_command(TARGET PicMatchComponent POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:PicMatchComponent>/resource/icons"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PICMATCH_RESOURCE_DIR}/icons/start.svg" "$<TARGET_FILE_DIR:PicMatchComponent>/resource/icons/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PICMATCH_RESOURCE_DIR}/icons/stop.svg" "$<TARGET_FILE_DIR:PicMatchComponent>/resource/icons/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PICMATCH_RESOURCE_DIR}/icons/settings.svg" "$<TARGET_FILE_DIR:PicMatchComponent>/resource/icons/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PICMATCH_RESOURCE_DIR}/icons/settings_dark.svg" "$<TARGET_FILE_DIR:PicMatchComponent>/resource/icons/"
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:PicMatchComponent>/resource/themes"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PICMATCH_RESOURCE_DIR}/themes/light.json" "$<TARGET_FILE_DIR:PicMatchComponent>/resource/themes/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PICMATCH_RESOURCE_DIR}/themes/dark.json" "$<TARGET_FILE_DIR:PicMatchComponent>/resource/themes/"
    COMMENT "Copy PicMatch icons and theme JSON to output directory"
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
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:PicMatchComponent>/../meta_info/runtime_info"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PICMATCH_RESOURCE_DIR}/icons/face_recognition.svg" "$<TARGET_FILE_DIR:PicMatchComponent>/../meta_info/runtime_info/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PICMATCH_COMPONENT_DIR}/resource/meta_info/manifest.json" "$<TARGET_FILE_DIR:PicMatchComponent>/../meta_info/"
    COMMENT "Copy PicMatch meta_info (icon + manifest) to Component/PicMatch/meta_info"
)

# 复制 OpenCV 人脸检测模型到组件输出目录（仅非 Windows：该路径来自 Mac 侧 install 树，Windows 无此目录且运行时可不依赖此拷贝）
if(NOT WIN32)
    set(OPENCV_HAARCASCADES_DIR "${CMAKE_SOURCE_DIR}/../ext/OrginPackage/opencv/install_with_qt681/share/opencv4/haarcascades")
    add_custom_command(TARGET PicMatchComponent POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:PicMatchComponent>/data/haarcascades"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${OPENCV_HAARCASCADES_DIR}/haarcascade_frontalface_default.xml" "$<TARGET_FILE_DIR:PicMatchComponent>/data/haarcascades/"
        COMMENT "Copy OpenCV haarcascade files to component output directory"
    )
endif()

message(STATUS "PicMatch_Component: built (deps PicPlayer + PicRecognition + UI) -> Component/PicMatch/bin, meta_info -> Component/PicMatch/meta_info")

# 翻译文件配置
set(PICMATCH_TRANSLATIONS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/translations")
set(PICMATCH_TS_FILES
    "${PICMATCH_TRANSLATIONS_DIR}/picmatch_zh_CN.ts"
    "${PICMATCH_TRANSLATIONS_DIR}/picmatch_en_US.ts"
)

# 预先编译所有.ts文件为.qm
set(PICMATCH_QM_FILES_LIST)
foreach(TS_FILE ${PICMATCH_TS_FILES})
    get_filename_component(TS_NAME ${TS_FILE} NAME_WE)
    list(APPEND PICMATCH_QM_FILES_LIST "${CMAKE_CURRENT_BINARY_DIR}/${TS_NAME}.qm")
endforeach()

# 单独的目标用于生成翻译文件（在主目标之前执行）
add_custom_target(picmatch_generate_translations ALL
    DEPENDS ${PICMATCH_QM_FILES_LIST}
    COMMENT "Generating PicMatch translation files"
)

# 生成.qm文件
foreach(TS_FILE ${PICMATCH_TS_FILES})
    get_filename_component(TS_NAME ${TS_FILE} NAME_WE)
    add_custom_command(
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${TS_NAME}.qm"
        COMMAND Qt6::lrelease ${TS_FILE} -qm "${CMAKE_CURRENT_BINARY_DIR}/${TS_NAME}.qm"
        DEPENDS ${TS_FILE}
        COMMENT "Generating PicMatch translation: ${TS_NAME}.qm"
        VERBATIM
    )
endforeach()

# 复制翻译文件到组件输出目录（在组件链接后执行POST_BUILD）
add_custom_command(TARGET PicMatchComponent POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:PicMatchComponent>/../meta_info/runtime_info/language"
    COMMENT "Creating language directory"
)

foreach(TS_FILE ${PICMATCH_TS_FILES})
    get_filename_component(TS_NAME ${TS_FILE} NAME_WE)
    add_custom_command(TARGET PicMatchComponent POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/${TS_NAME}.qm" "$<TARGET_FILE_DIR:PicMatchComponent>/../meta_info/runtime_info/language/${TS_NAME}.qm"
        COMMENT "Copying PicMatch translation: ${TS_NAME}.qm"
        VERBATIM
    )
endforeach()

message(STATUS "PicMatch translation files: ${PICMATCH_TS_FILES}")
