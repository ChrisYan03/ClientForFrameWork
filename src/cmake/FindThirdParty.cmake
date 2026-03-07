# FindThirdParty.cmake - Unified third-party library finding module
# This module provides consistent finding for GLEW, GLFW, spdlog, and OpenCV

include(FindPackageHandleStandardArgs)

# ==============================================================================
# Find GLEW
# ==============================================================================
find_package(GLEW QUIET)

if(NOT GLEW_FOUND)
    # Manual search if not found via standard module
    if(WIN32)
        find_path(GLEW_INCLUDE_DIR
            NAMES GL/glew.h
            PATHS
            "${EXT_DIR}/include"
            "$ENV{GLEW_ROOT}/include"
            DOC "GLEW include directory"
        )

        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            find_library(GLEW_LIBRARY
                NAMES glew32 glew64
                PATHS
                "${EXT_DIR}/lib"
                "$ENV{GLEW_ROOT}/lib/Release/x64"
                DOC "GLEW library"
            )
        else()
            find_library(GLEW_LIBRARY
                NAMES glew32
                PATHS
                "${EXT_DIR}/lib"
                "$ENV{GLEW_ROOT}/lib/Release/Win32"
                DOC "GLEW library"
            )
        endif()
    elseif(APPLE)
        find_path(GLEW_INCLUDE_DIR
            NAMES GL/glew.h
            PATHS
            "${EXT_DIR}/include"
            "/usr/local/include"
            "/opt/homebrew/include"
            DOC "GLEW include directory"
        )

        find_library(GLEW_LIBRARY
            NAMES GLEW glew
            PATHS
            "${EXT_DIR}/lib"
            "/usr/local/lib"
            "/opt/homebrew/lib"
            DOC "GLEW library"
        )
    endif()

    find_package_handle_standard_args(GLEW
        REQUIRED_VARS GLEW_INCLUDE_DIR GLEW_LIBRARY
    )

    if(GLEW_FOUND)
        add_library(GLEW::GLEW UNKNOWN IMPORTED)
        set_target_properties(GLEW::GLEW PROPERTIES
            IMPORTED_LOCATION "${GLEW_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${GLEW_INCLUDE_DIR}"
        )
    endif()
else()
    message(STATUS "Found GLEW via standard find module")
endif()

# ==============================================================================
# Find GLFW
# ==============================================================================
find_package(GLFW3 QUIET)

if(NOT GLFW3_FOUND)
    # Manual search if not found via standard module
    if(WIN32)
        find_path(GLFW_INCLUDE_DIR
            NAMES GLFW/glfw3.h
            PATHS
            "${EXT_DIR}/include"
            "$ENV{GLFW_ROOT}/include"
            DOC "GLFW include directory"
        )

        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            find_library(GLFW_LIBRARY
                NAMES glfw3 glfw
                PATHS
                "${EXT_DIR}/lib"
                "$ENV{GLFW_ROOT}/lib-vc2019"
                DOC "GLFW library"
            )
        else()
            find_library(GLFW_LIBRARY
                NAMES glfw3 glfw
                PATHS
                "${EXT_DIR}/lib"
                "$ENV{GLFW_ROOT}/lib-vc2019"
                DOC "GLFW library"
            )
        endif()
    elseif(APPLE)
        find_path(GLFW_INCLUDE_DIR
            NAMES GLFW/glfw3.h
            PATHS
            "${EXT_DIR}/include"
            "/usr/local/include"
            "/opt/homebrew/include"
            DOC "GLFW include directory"
        )

        find_library(GLFW_LIBRARY
            NAMES glfw glfw3
            PATHS
            "${EXT_DIR}/lib"
            "/usr/local/lib"
            "/opt/homebrew/lib"
            DOC "GLFW library"
        )

        find_library(GLFW_COCOA_LIBRARY
            NAMES Cocoa
            PATHS /usr/lib /System/Library/Frameworks
        )

        find_library(GLFW_IOKIT_LIBRARY
            NAMES IOKit
            PATHS /usr/lib /System/Library/Frameworks
        )

        find_library(GLFW_COREVIDEO_LIBRARY
            NAMES CoreVideo
            PATHS /usr/lib /System/Library/Frameworks
        )
    endif()

    if(APPLE)
        find_package_handle_standard_args(GLFW
            REQUIRED_VARS GLFW_INCLUDE_DIR GLFW_LIBRARY GLFW_COCOA_LIBRARY
        )
    else()
        find_package_handle_standard_args(GLFW
            REQUIRED_VARS GLFW_INCLUDE_DIR GLFW_LIBRARY
        )
    endif()

    if(GLFW_FOUND)
        add_library(GLFW::GLFW UNKNOWN IMPORTED)
        set_target_properties(GLFW::GLFW PROPERTIES
            IMPORTED_LOCATION "${GLFW_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${GLFW_INCLUDE_DIR}"
        )

        if(APPLE)
            set_target_properties(GLFW::GLFW PROPERTIES
                INTERFACE_LINK_LIBRARIES "${GLFW_COCOA_LIBRARY};${GLFW_IOKIT_LIBRARY};${GLFW_COREVIDEO_LIBRARY}"
            )
        endif()
    endif()
else()
    message(STATUS "Found GLFW via standard find module")
endif()

# ==============================================================================
# Find spdlog
# ==============================================================================
find_package(spdlog QUIET)

if(NOT spdlog_FOUND)
    # Manual search if not found via standard module
    find_path(SPDLOG_INCLUDE_DIR
        NAMES spdlog/spdlog.h
        PATHS
        "${EXT_DIR}/include"
        "${EXT_DIR}/include/spdlog"
        "$ENV{SPDLOG_ROOT}/include"
        "/usr/local/include"
        "/opt/homebrew/include"
        DOC "spdlog include directory"
    )

    find_package_handle_standard_args(spdlog
        REQUIRED_VARS SPDLOG_INCLUDE_DIR
    )

    if(spdlog_FOUND)
        # spdlog is header-only, so we create an interface library
        if(NOT TARGET spdlog::spdlog)
            add_library(spdlog::spdlog INTERFACE IMPORTED)
            set_target_properties(spdlog::spdlog PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES "${SPDLOG_INCLUDE_DIR}"
            )
        endif()
    endif()
else()
    message(STATUS "Found spdlog via standard find module")
endif()

# ==============================================================================
# Find OpenCV
# ==============================================================================
find_package(OpenCV QUIET COMPONENTS core imgproc imgcodecs objdetect)

if(NOT OpenCV_FOUND)
    # Manual search if not found via standard module
    find_path(OpenCV_INCLUDE_DIR
        NAMES opencv2/opencv.hpp
        PATHS
        "${EXT_DIR}/include"
        "$ENV{OpenCV_DIR}/include"
        "/usr/local/include"
        "/opt/homebrew/include"
        DOC "OpenCV include directory"
    )

    if(WIN32)
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            find_library(OpenCV_CORE_LIBRARY
                NAMES opencv_core${OpenCV_VERSION_MAJOR}${OpenCV_VERSION_MINOR}${OpenCV_VERSION_PATCH} opencv_core
                PATHS
                "${EXT_DIR}/lib"
                "$ENV{OpenCV_DIR}/x64/vc16/lib"
                DOC "OpenCV core library"
            )

            find_library(OpenCV_IMGPROC_LIBRARY
                NAMES opencv_imgproc${OpenCV_VERSION_MAJOR}${OpenCV_VERSION_MINOR}${OpenCV_VERSION_PATCH} opencv_imgproc
                PATHS
                "${EXT_DIR}/lib"
                "$ENV{OpenCV_DIR}/x64/vc16/lib"
                DOC "OpenCV imgproc library"
            )

            find_library(OpenCV_IMGCODECS_LIBRARY
                NAMES opencv_imgcodecs${OpenCV_VERSION_MAJOR}${OpenCV_VERSION_MINOR}${OpenCV_VERSION_PATCH} opencv_imgcodecs
                PATHS
                "${EXT_DIR}/lib"
                "$ENV{OpenCV_DIR}/x64/vc16/lib"
                DOC "OpenCV imgcodecs library"
            )

            find_library(OpenCV_OBJDETECT_LIBRARY
                NAMES opencv_objdetect${OpenCV_VERSION_MAJOR}${OpenCV_VERSION_MINOR}${OpenCV_VERSION_PATCH} opencv_objdetect
                PATHS
                "${EXT_DIR}/lib"
                "$ENV{OpenCV_DIR}/x64/vc16/lib"
                DOC "OpenCV objdetect library"
            )
        endif()
    elseif(APPLE)
        find_library(OpenCV_CORE_LIBRARY
            NAMES opencv_core opencv4
            PATHS
            "${EXT_DIR}/lib"
            "/usr/local/lib"
            "/opt/homebrew/lib"
            DOC "OpenCV core library"
        )

        find_library(OpenCV_IMGPROC_LIBRARY
            NAMES opencv_imgproc
            PATHS
            "${EXT_DIR}/lib"
            "/usr/local/lib"
            "/opt/homebrew/lib"
            DOC "OpenCV imgproc library"
        )

        find_library(OpenCV_IMGCODECS_LIBRARY
            NAMES opencv_imgcodecs
            PATHS
            "${EXT_DIR}/lib"
            "/usr/local/lib"
            "/opt/homebrew/lib"
            DOC "OpenCV imgcodecs library"
        )

        find_library(OpenCV_OBJDETECT_LIBRARY
            NAMES opencv_objdetect
            PATHS
            "${EXT_DIR}/lib"
            "/usr/local/lib"
            "/opt/homebrew/lib"
            DOC "OpenCV objdetect library"
        )
    endif()

    set(OpenCV_LIBRARIES
        ${OpenCV_CORE_LIBRARY}
        ${OpenCV_IMGPROC_LIBRARY}
        ${OpenCV_IMGCODECS_LIBRARY}
        ${OpenCV_OBJDETECT_LIBRARY}
    )

    find_package_handle_standard_args(OpenCV
        REQUIRED_VARS OpenCV_INCLUDE_DIR OpenCV_CORE_LIBRARY OpenCV_IMGPROC_LIBRARY
    )

    if(OpenCV_FOUND)
        add_library(OpenCV::core UNKNOWN IMPORTED)
        set_target_properties(OpenCV::core PROPERTIES
            IMPORTED_LOCATION "${OpenCV_CORE_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${OpenCV_INCLUDE_DIR}"
        )

        add_library(OpenCV::imgproc UNKNOWN IMPORTED)
        set_target_properties(OpenCV::imgproc PROPERTIES
            IMPORTED_LOCATION "${OpenCV_IMGPROC_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${OpenCV_INCLUDE_DIR}"
        )

        add_library(OpenCV::imgcodecs UNKNOWN IMPORTED)
        set_target_properties(OpenCV::imgcodecs PROPERTIES
            IMPORTED_LOCATION "${OpenCV_IMGCODECS_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${OpenCV_INCLUDE_DIR}"
        )

        add_library(OpenCV::objdetect UNKNOWN IMPORTED)
        set_target_properties(OpenCV::objdetect PROPERTIES
            IMPORTED_LOCATION "${OpenCV_OBJDETECT_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${OpenCV_INCLUDE_DIR}"
        )
    endif()
else()
    message(STATUS "Found OpenCV via standard find module")
endif()

# ==============================================================================
# Summary of Found Libraries
# ==============================================================================
message(STATUS "")
message(STATUS "Third-party libraries summary:")
message(STATUS "  GLEW: ${GLEW_FOUND}")
if(GLEW_FOUND)
    message(STATUS "    Include: ${GLEW_INCLUDE_DIR}")
    message(STATUS "    Library: ${GLEW_LIBRARY}")
endif()

message(STATUS "  GLFW: ${GLFW_FOUND}")
if(GLFW_FOUND)
    message(STATUS "    Include: ${GLFW_INCLUDE_DIR}")
    message(STATUS "    Library: ${GLFW_LIBRARY}")
endif()

message(STATUS "  spdlog: ${spdlog_FOUND}")
if(spdlog_FOUND)
    message(STATUS "    Include: ${SPDLOG_INCLUDE_DIR}")
endif()

message(STATUS "  OpenCV: ${OpenCV_FOUND}")
if(OpenCV_FOUND)
    message(STATUS "    Include: ${OpenCV_INCLUDE_DIR}")
    message(STATUS "    Libraries: ${OpenCV_LIBRARIES}")
endif()
message(STATUS "")