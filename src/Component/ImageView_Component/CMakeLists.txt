cmake_minimum_required(VERSION 3.16)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Qml Quick Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Qml Quick Widgets)

set(IMAGEVIEW_COMPONENT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(IMAGEVIEW_SOURCE_DIR ${IMAGEVIEW_COMPONENT_DIR}/src)
set(IMAGEVIEW_RESOURCE_DIR ${IMAGEVIEW_COMPONENT_DIR}/resource)
set(IMAGEVIEW_DEPS_DIR ${IMAGEVIEW_COMPONENT_DIR}/deps/PicPlayer/include)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

include_directories(
    ${IMAGEVIEW_SOURCE_DIR}
    ${IMAGEVIEW_SOURCE_DIR}/QmlBridge
    ${IMAGEVIEW_DEPS_DIR}
    ${CMAKE_SOURCE_DIR}/Framework/include
    ${CMAKE_SOURCE_DIR}/Framework/interface
)
include_directories(${CMAKE_SOURCE_DIR}/LogUtil)

set(IMAGEVIEW_SOURCES
    ${IMAGEVIEW_SOURCE_DIR}/ImageViewComponent.h
    ${IMAGEVIEW_SOURCE_DIR}/ImageViewComponent.cpp
    ${IMAGEVIEW_SOURCE_DIR}/ImageViewComponentGlobal.h
    ${IMAGEVIEW_SOURCE_DIR}/QmlBridge/ImageViewHostItem.h
    ${IMAGEVIEW_SOURCE_DIR}/QmlBridge/ImageViewHostItem.cpp
    ${IMAGEVIEW_RESOURCE_DIR}/ImageView.qrc
)

add_library(ImageViewComponent SHARED ${IMAGEVIEW_SOURCES})
if(WIN32)
    target_compile_definitions(ImageViewComponent PRIVATE IMAGEVIEWCOMPONENT_EXPORTS)
endif()

target_link_libraries(ImageViewComponent PUBLIC
    Framework
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Qml
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::Widgets
)

# 独立进程播放器宿主（stb_image 解码 + PicPlayer 静态双图）
add_executable(ImageViewPlayerHost
    ${IMAGEVIEW_SOURCE_DIR}/ImageViewPlayerHost.cpp
    ${IMAGEVIEW_SOURCE_DIR}/ImageViewPlayerHostPicLoader.cpp
    ${IMAGEVIEW_SOURCE_DIR}/ImageViewStbImageImpl.cpp
)
target_link_libraries(ImageViewPlayerHost PRIVATE
    Framework
    PicPlayer
    Qt${QT_VERSION_MAJOR}::Core
)
add_dependencies(ImageViewComponent ImageViewPlayerHost)

# Windows：宿主 exe 在 Component/ImageView/bin，PicPlayer.dll 默认在全局输出目录；若不同目录会误加载 PATH/工作目录下的旧 DLL，仍表现为滚动模式
if(WIN32)
    add_custom_command(TARGET ImageViewPlayerHost POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:PicPlayer>"
            "$<TARGET_FILE_DIR:ImageViewPlayerHost>"
        COMMENT "Copy PicPlayer.dll next to ImageViewPlayerHost.exe"
    )
endif()

set(COMPONENT_IMAGEVIEW_BIN "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Component/ImageView/bin")
set_target_properties(ImageViewComponent PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    RUNTIME_OUTPUT_DIRECTORY "${COMPONENT_IMAGEVIEW_BIN}"
    LIBRARY_OUTPUT_DIRECTORY "${COMPONENT_IMAGEVIEW_BIN}"
    ARCHIVE_OUTPUT_DIRECTORY "${COMPONENT_IMAGEVIEW_BIN}"
)
set_target_properties(ImageViewPlayerHost PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${COMPONENT_IMAGEVIEW_BIN}"
    LIBRARY_OUTPUT_DIRECTORY "${COMPONENT_IMAGEVIEW_BIN}"
    ARCHIVE_OUTPUT_DIRECTORY "${COMPONENT_IMAGEVIEW_BIN}"
)

if(CMAKE_CONFIGURATION_TYPES)
    foreach(cfg ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER "${cfg}" cfg_upper)
        set(component_cfg_bin "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${cfg}/Component/ImageView/bin")
        set_target_properties(ImageViewComponent PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY_${cfg_upper} "${component_cfg_bin}"
            LIBRARY_OUTPUT_DIRECTORY_${cfg_upper} "${component_cfg_bin}"
            ARCHIVE_OUTPUT_DIRECTORY_${cfg_upper} "${component_cfg_bin}"
        )
        set_target_properties(ImageViewPlayerHost PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY_${cfg_upper} "${component_cfg_bin}"
            LIBRARY_OUTPUT_DIRECTORY_${cfg_upper} "${component_cfg_bin}"
            ARCHIVE_OUTPUT_DIRECTORY_${cfg_upper} "${component_cfg_bin}"
        )
    endforeach()
endif()

# 头文件同步（复用 PicPlayer 公共头）
add_custom_command(TARGET ImageViewComponent PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${IMAGEVIEW_COMPONENT_DIR}/deps/PicPlayer/include"
)
add_custom_command(TARGET ImageViewComponent PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/Component/Common/PicPlayer/src/PicPlayerApi.h" "${IMAGEVIEW_COMPONENT_DIR}/deps/PicPlayer/include/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/Component/Common/PicPlayer/src/PicPlayerDataDef.h" "${IMAGEVIEW_COMPONENT_DIR}/deps/PicPlayer/include/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/Component/Common/PicPlayer/src/PicPlayerGlobal.h" "${IMAGEVIEW_COMPONENT_DIR}/deps/PicPlayer/include/"
)

add_custom_command(TARGET ImageViewComponent POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:ImageViewComponent>/qml"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${IMAGEVIEW_RESOURCE_DIR}/qml/ImageViewPage.qml" "$<TARGET_FILE_DIR:ImageViewComponent>/qml/"
    COMMENT "Copy ImageView qml page"
)

add_custom_command(TARGET ImageViewComponent POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:ImageViewComponent>/../meta_info/runtime_info"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${IMAGEVIEW_RESOURCE_DIR}/icons/image_view.svg" "$<TARGET_FILE_DIR:ImageViewComponent>/../meta_info/runtime_info/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${IMAGEVIEW_RESOURCE_DIR}/meta_info/manifest.json" "$<TARGET_FILE_DIR:ImageViewComponent>/../meta_info/"
    COMMENT "Copy ImageView meta_info"
)

message(STATUS "ImageView_Component: built -> Component/ImageView/bin, meta_info -> Component/ImageView/meta_info")
