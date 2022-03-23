cmake_minimum_required(VERSION 3.14)
project( RawKbdQt
    VERSION 0.0.1
    LANGUAGES CXX
    DESCRIPTION "Raw Keyboard Events Test for Qt Applications"
    HOMEPAGE_URL "https://github.com/pedrolcl/rawkeybdqt"
)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(USE_QT "Choose which Qt major version (5 or 6) to prefer. By default uses whatever is found")

if (USE_QT)
    if (NOT (USE_QT EQUAL 5 OR USE_QT EQUAL 6))
        message(FATAL_ERROR "Wrong Qt major version. Only 5 and 6 are valid options")
    endif()
endif()

if (USE_QT EQUAL 5)
    find_package(QT NAMES Qt5)
elseif (USE_QT EQUAL 6)
    find_package(QT NAMES Qt6)
else()
    find_package(QT NAMES Qt5 Qt6)
endif()

if (QT_VERSION VERSION_LESS 6.0)
    find_package(Qt5 5.9 COMPONENTS Widgets REQUIRED)
else()
    if (QT_VERSION VERSION_LESS 6.2.0)
       message (FATAL_ERROR "Qt6 >= 6.2 is required, or use Qt5 >= 5.9")
    endif()
    find_package(Qt6 6.2 COMPONENTS Gui Widgets REQUIRED)
endif()

if (UNIX AND NOT APPLE)
    if (QT_VERSION VERSION_LESS 6.0)
        find_package(Qt5X11Extras REQUIRED)
    endif()
    find_package (PkgConfig REQUIRED)
    pkg_check_modules (XCB xcb)
endif()

add_executable( RawKbdQt
    main.cpp
    mainwindow.cpp
    nativefilter.cpp
    $<$<PLATFORM_ID:Darwin>:maceventhelper.mm>
)

target_link_libraries( RawKbdQt PRIVATE
    Qt${QT_MAJOR_VERSION}::Widgets
    $<$<BOOL:${Qt5X11Extras_FOUND}>:Qt5::X11Extras>
    $<$<BOOL:${XCB_FOUND}>:${XCB_LIBRARIES}>
)

if (QT_VERSION VERSION_GREATER_EQUAL 6.0.0)
    target_link_libraries( RawKbdQt PRIVATE Qt6::Gui )
endif()

if (APPLE)
    target_link_libraries( RawKbdQt PRIVATE "-framework Cocoa" )
endif()
