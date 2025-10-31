cmake_minimum_required(VERSION 3.31)
project(Question_11)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CURRENT_QT_VERSION "6.9.3")

set(CMAKE_PREFIX_PATH "D:/Qt/${CURRENT_QT_VERSION}/msvc2022_64")

find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        Xml
        SerialPort
        Charts
        REQUIRED)

# 项目源文件
file(GLOB MAIN_SOURCES
        "*.cpp"
        "*.h"
        "*.hpp"
        "*.ui"
)

# QXlsx 源代码文件
file(GLOB QXLSX_SOURCES
        "QXlsx/*.h"
        "QXlsx/*.cpp"
)

# Qt 资源文件
file(GLOB RESOURCES
        "*.qrc"
)

set(SOURCES
        ${MAIN_SOURCES}
        ${QXLSX_SOURCES}
        ${RESOURCES}
        Question_11.rc
)

add_executable(Question_11 ${SOURCES})

# 添加指定的包含目录
target_include_directories(Question_11 PRIVATE
        "${CMAKE_PREFIX_PATH}/include/QtCore/${CURRENT_QT_VERSION}/"
        "${CMAKE_PREFIX_PATH}/include/QtCore/${CURRENT_QT_VERSION}/QtCore/"
        "${CMAKE_PREFIX_PATH}/include/QtCore/${CURRENT_QT_VERSION}/QtCore/private"
)

target_link_libraries(Question_11
        PUBLIC
        Qt::Core
        Qt::Gui
        Qt::Widgets
        Qt::Xml
        Qt::SerialPort
        Qt::Charts
)

if (WIN32 AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(DEBUG_SUFFIX)
    if (MSVC AND CMAKE_BUILD_TYPE MATCHES "Debug")
        set(DEBUG_SUFFIX "d")
    endif ()
    set(QT_INSTALL_PATH "${CMAKE_PREFIX_PATH}")
    if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
        set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
            set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        endif ()
    endif ()
    if (EXISTS "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
    endif ()
    foreach (QT_LIB Core Gui Widgets OpenGL OpenGLWidgets Xml Charts SerialPort)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/bin/Qt6${QT_LIB}${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endforeach (QT_LIB)
endif ()

if(WIN32)
    # Windows：无需额外库，直接使用系统API
    target_compile_definitions(${PROJECT_NAME} PRIVATE _WIN32)
elseif(APPLE)
    # macOS：需要链接Cocoa框架（提供NSBeep()）
    find_library(COCOA_LIBRARY Cocoa)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${COCOA_LIBRARY})
    target_compile_definitions(${PROJECT_NAME} PRIVATE __APPLE__)
elseif(UNIX)
    # Linux：需要链接libcanberra库
    find_library(CANBERRA_LIBRARY canberra)
    if(NOT CANBERRA_LIBRARY)
        message(FATAL_ERROR "libcanberra not found. Install it with: sudo apt install libcanberra-dev (Debian/Ubuntu) or sudo dnf install libcanberra-devel (Fedora)")
    endif()
    target_link_libraries(${PROJECT_NAME} PRIVATE ${CANBERRA_LIBRARY})
    target_compile_definitions(${PROJECT_NAME} PRIVATE __linux__)
endif()