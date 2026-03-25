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
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS LinguistTools)

if(USE_QML)
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Quick QuickControls2)
endif()

message("ClientForFrame qt_directory: Qt${QT_VERSION_MAJOR} found at ${Qt${QT_VERSION_MAJOR}_DIR}")
# 设置源码目录与资源目录
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(RESOURCE_DIR ${SOURCE_DIR}/resource)
include_directories(${SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/Framework/include)  # 添加 Framework 头文件目录

# 纯 QML 框架：不包含业务组件头文件，组件由运行时动态加载
include_directories(${CMAKE_SOURCE_DIR}/LogUtil)

# 定义源码文件
# 使用 file(GLOB ...) 自动收集源文件
file(GLOB_RECURSE PROJECT_SOURCES_RECURSIVE
    "${SOURCE_DIR}/*.cpp"
    "${SOURCE_DIR}/*.h"
    "${RESOURCE_DIR}/*.qrc"
)
list(APPEND PROJECT_SOURCES ${PROJECT_SOURCES_RECURSIVE})

# 添加公共目录的TranslationManager
list(APPEND PROJECT_SOURCES
    "${CMAKE_SOURCE_DIR}/Common/TranslationManager.cpp"
    "${CMAKE_SOURCE_DIR}/Common/TranslationManager.h"
)

if(USE_QML)
    list(REMOVE_ITEM PROJECT_SOURCES "${SOURCE_DIR}/main.cpp"
        "${SOURCE_DIR}/QmlBridge/PlayerHostItem.cpp"
        "${SOURCE_DIR}/QmlBridge/PlayerHostItem.h"
        "${SOURCE_DIR}/Common/BaseWidget.cpp"
        "${SOURCE_DIR}/Common/BaseWidget.h")
else()
    list(REMOVE_ITEM PROJECT_SOURCES "${SOURCE_DIR}/main_qml.cpp"
        "${SOURCE_DIR}/QmlBridge/AppController.cpp"
        "${SOURCE_DIR}/QmlBridge/AppController.h"
        "${SOURCE_DIR}/QmlBridge/PlayerHostItem.cpp"
        "${SOURCE_DIR}/QmlBridge/PlayerHostItem.h")
endif()
list(REMOVE_DUPLICATES PROJECT_SOURCES)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    # USE_QML 时用 add_executable 避免 qt_finalize_executable 触发的 QML 导入扫描（在部分环境会失败）
    if(USE_QML)
        add_executable(ClientForFrame ${PROJECT_SOURCES})
        target_link_libraries(ClientForFrame PRIVATE
            Framework
            Qt${QT_VERSION_MAJOR}::Core
            Qt${QT_VERSION_MAJOR}::Gui
            Qt${QT_VERSION_MAJOR}::Widgets
            Qt${QT_VERSION_MAJOR}::Quick
            Qt${QT_VERSION_MAJOR}::QuickControls2
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
            target_link_libraries(ClientForFrame PRIVATE)
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
            target_link_libraries(ClientForFrame PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
        endif()
        qt_finalize_executable(ClientForFrame)
    endif()
else()
    if(ANDROID)
        add_library(ClientForFrame SHARED ${PROJECT_SOURCES})
        target_link_libraries(ClientForFrame PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
    else()
        add_executable(ClientForFrame ${PROJECT_SOURCES})
        target_link_libraries(ClientForFrame PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
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
    target_link_libraries(ClientForFrame PRIVATE dwmapi)
    # Windows 应用图标
    target_sources(ClientForFrame PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/resource/app.rc")
endif()
if(APPLE)
    # macOS .app 使用 .icns；由 app_icon.ico 在构建时生成并放入 Contents/Resources
    set(APP_ICON_ICO "${CMAKE_CURRENT_SOURCE_DIR}/src/resource/app_icon.ico")
    set(APP_ICON_ICNS "${CMAKE_CURRENT_BINARY_DIR}/app_icon.icns")
    set(APP_ICON_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/cmake/make_app_icon_icns.sh")
    execute_process(COMMAND chmod +x "${APP_ICON_SCRIPT}")
    add_custom_command(
        OUTPUT "${APP_ICON_ICNS}"
        COMMAND /bin/bash "${APP_ICON_SCRIPT}" "${APP_ICON_ICO}" "${APP_ICON_ICNS}"
        DEPENDS "${APP_ICON_ICO}" "${APP_ICON_SCRIPT}"
        COMMENT "Generate app_icon.icns from app_icon.ico"
        VERBATIM
    )
    target_sources(ClientForFrame PRIVATE "${APP_ICON_ICNS}")
    set_source_files_properties("${APP_ICON_ICNS}" PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources"
        GENERATED TRUE
    )
    # CFBundleIconFile 必须为不含 .icns 扩展名的基名；空 CFBundleIdentifier 不利于 Launch Services / 图标缓存
    set_target_properties(ClientForFrame PROPERTIES
        MACOSX_BUNDLE_ICON_FILE "app_icon"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.clientforframe.ClientForFrame"
    )
    # Info.plist 放入 Contents/Info.plist（空 MACOSX_PACKAGE_LOCATION 即 Contents 根目录）
    set(APP_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/src/resource/Info.plist")
    target_sources(ClientForFrame PRIVATE "${APP_INFO_PLIST}")
    set_source_files_properties("${APP_INFO_PLIST}" PROPERTIES
        MACOSX_PACKAGE_LOCATION ""
        GENERATED TRUE
    )
endif()

# Crashpad：minidump 崩溃捕获，需在 target 创建后配置（产物在 ext/lib/crashpad/Mac 或 ext/lib/crashpad/Win64）
# ClientForFrame 在 src/ClientForFrame，项目根为 src，ext 在 src/../ext
set(CRASHPAD_ROOT "${CMAKE_SOURCE_DIR}/../ext")
set(CRASHPAD_INCLUDE "${CRASHPAD_ROOT}/include/crashpad")
# mini_chromium 头文件优先从 ext/include 读取；若未同步则回退到本地 crashpad 源码目录
set(CRASHPAD_MINI_CHROMIUM "${CRASHPAD_INCLUDE}/third_party/mini_chromium/mini_chromium")
if(NOT EXISTS "${CRASHPAD_MINI_CHROMIUM}/base/files/file_path.h")
    set(CRASHPAD_MINI_CHROMIUM "${CRASHPAD_ROOT}/OrginPackage/crashpad_main_/crashpad/third_party/mini_chromium/mini_chromium")
endif()
set(CRASHPAD_MAC "${CRASHPAD_ROOT}/lib/crashpad/Mac")
set(CRASHPAD_WIN "${CRASHPAD_ROOT}/lib/crashpad/Win64")
if(APPLE)
set(CRASHPAD_OBJ "${CRASHPAD_MAC}/obj")
set(CRASHPAD_HANDLER "${CRASHPAD_MAC}/crashpad_handler")
endif()
if(WIN32)
set(CRASHPAD_OBJ "${CRASHPAD_WIN}/obj")
set(CRASHPAD_HANDLER "${CRASHPAD_WIN}/crashpad_handler.exe")
endif()
if(APPLE AND EXISTS "${CRASHPAD_OBJ}/client/libclient.a")
    target_include_directories(ClientForFrame PRIVATE "${CRASHPAD_INCLUDE}" "${CRASHPAD_MINI_CHROMIUM}")
    target_link_libraries(ClientForFrame PRIVATE
        "${CRASHPAD_OBJ}/client/libclient.a"
        "${CRASHPAD_OBJ}/client/libcommon.a"
        "${CRASHPAD_OBJ}/util/libutil.a"
        "${CRASHPAD_OBJ}/util/libmig_output.a"
        "${CRASHPAD_OBJ}/minidump/libminidump.a"
        "${CRASHPAD_OBJ}/minidump/libformat.a"
        "${CRASHPAD_OBJ}/snapshot/libsnapshot.a"
        "${CRASHPAD_OBJ}/snapshot/libcontext.a"
        "${CRASHPAD_OBJ}/third_party/mini_chromium/mini_chromium/base/libbase.a"
        bsm
        "-framework Security"
    )
    # 将 crashpad_handler 拷贝到 target/Mac/bin，并拷贝进 app bundle，供 app 直接调用
    add_custom_command(TARGET ClientForFrame POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CRASHPAD_HANDLER}" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/crashpad_handler"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ClientForFrame.app/Contents/MacOS"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CRASHPAD_HANDLER}" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ClientForFrame.app/Contents/MacOS/crashpad_handler"
        COMMENT "Copy crashpad_handler to target/Mac/bin and into app bundle"
    )
    message(STATUS "Crashpad: enabled (handler in ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} and app bundle)")
endif()
if(WIN32 AND EXISTS "${CRASHPAD_OBJ}/client/client.lib")
    target_include_directories(ClientForFrame PRIVATE "${CRASHPAD_INCLUDE}" "${CRASHPAD_MINI_CHROMIUM}")
    target_link_libraries(ClientForFrame PRIVATE
        "${CRASHPAD_OBJ}/client/client.lib"
        "${CRASHPAD_OBJ}/client/common.lib"
        "${CRASHPAD_OBJ}/util/util.lib"
        "${CRASHPAD_OBJ}/minidump/minidump.lib"
        "${CRASHPAD_OBJ}/minidump/format.lib"
        "${CRASHPAD_OBJ}/snapshot/snapshot.lib"
        "${CRASHPAD_OBJ}/snapshot/context.lib"
        "${CRASHPAD_OBJ}/third_party/mini_chromium/mini_chromium/base/base.lib"
        rpcrt4
    )
    add_custom_command(TARGET ClientForFrame POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CRASHPAD_HANDLER}" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/crashpad_handler.exe"
        COMMENT "Copy crashpad_handler.exe to runtime output directory"
    )
    message(STATUS "Crashpad: enabled (Windows, handler in ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})")
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

# 翻译文件配置
set(TRANSLATIONS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/translations")
set(TS_FILES
    "${TRANSLATIONS_DIR}/clientframe_zh_CN.ts"
    "${TRANSLATIONS_DIR}/clientframe_en_US.ts"
)

# 将.ts文件编译为.qm文件
set(QM_FILES)
foreach(TS_FILE ${TS_FILES})
    get_filename_component(TS_NAME ${TS_FILE} NAME_WE)
    set(QM_FILE "${CMAKE_CURRENT_BINARY_DIR}/${TS_NAME}.qm")
    list(APPEND QM_FILES ${QM_FILE})

    # 使用Qt6的lrelease工具编译翻译文件
    add_custom_command(
        OUTPUT ${QM_FILE}
        COMMAND Qt6::lrelease ${TS_FILE} -qm ${QM_FILE}
        DEPENDS ${TS_FILE}
        COMMENT "Generating translation file: ${TS_NAME}.qm"
        VERBATIM
    )
endforeach()

# 创建自定义目标来生成翻译文件
add_custom_target(translate_files ALL DEPENDS ${QM_FILES})

# 创建translations目录并复制.qm文件
# 多配置生成器（VS）下，运行目录是 .../bin/<Config>，翻译文件也必须跟随到该目录
set(CLIENTFRAME_TRANSLATIONS_OUTPUT_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/translations")
if(WIN32 AND CMAKE_CONFIGURATION_TYPES)
    set(CLIENTFRAME_TRANSLATIONS_OUTPUT_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/translations")
endif()

add_custom_command(TARGET translate_files POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CLIENTFRAME_TRANSLATIONS_OUTPUT_DIR}"
    COMMENT "Creating translations directory"
)

foreach(QM_FILE ${QM_FILES})
    get_filename_component(QM_NAME ${QM_FILE} NAME)
    add_custom_command(TARGET translate_files POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${QM_FILE}" "${CLIENTFRAME_TRANSLATIONS_OUTPUT_DIR}/${QM_NAME}"
        COMMENT "Copying translation file: ${QM_NAME}"
    )
endforeach()

# macOS特殊处理：复制翻译文件到app bundle内部
if(APPLE)
    add_custom_command(TARGET translate_files POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ClientForFrame.app/Contents/MacOS/translations"
        COMMENT "Creating translations directory in app bundle"
    )

    foreach(QM_FILE ${QM_FILES})
        get_filename_component(QM_NAME ${QM_FILE} NAME)
        add_custom_command(TARGET translate_files POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${QM_FILE}" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ClientForFrame.app/Contents/MacOS/translations/${QM_NAME}"
            COMMENT "Copying translation file to app bundle: ${QM_NAME}"
        )
    endforeach()
endif()

# 让主目标依赖于翻译文件生成
add_dependencies(ClientForFrame translate_files)

# Windows 下确保主程序构建时组件 DLL 同步更新，避免因旧 PicMatchComponent.dll 与新 Framework/IComponent ABI 不一致
# 导致运行时虚函数调用落到无效地址（典型症状：onThemeChanged 调用崩溃）。
if(WIN32 AND TARGET PicMatchComponent)
    add_dependencies(ClientForFrame PicMatchComponent)
endif()

# 设置目标属性以包含翻译文件的依赖
set_target_properties(ClientForFrame PROPERTIES
    QT_TRANSLATION_FILES "${TS_FILES}"
)

message("Translation files: ${TS_FILES}")
message("Compiled translation files: ${QM_FILES}")

