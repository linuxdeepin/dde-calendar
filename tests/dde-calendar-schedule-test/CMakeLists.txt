// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

cmake_minimum_required(VERSION 3.7)
include(${CMAKE_SOURCE_DIR}/cmake/qt-version-macros.cmake)

project(dde-calendar-schedule-test)

ADD_COMPILE_OPTIONS(-fno-access-control)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--as-needed -fPIE")
set(CMAKE_EXE_LINKER_FLAGS "-pie")

set(APP_BIN_NAME "dde-calendar-schedule-test")
set(APP_PLUGIN_DIR "${CMAKE_SOURCE_DIR}/src/schedule-plugin")
set(APP_COMMON_DIR "${CMAKE_SOURCE_DIR}/src/calendar-common")
set(APP_3PARTY_DIR "${CMAKE_SOURCE_DIR}/3rdparty/kcalendarcore")

find_package(PkgConfig REQUIRED)
find_package(Dtk${DTK_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widget)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets DBus Svg Test)
find_package(GTest REQUIRED)

set(LINK_LIBS
    Dtk${DTK_VERSION_MAJOR}::Core
    Dtk${DTK_VERSION_MAJOR}::Gui
    Dtk${DTK_VERSION_MAJOR}::Widget
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Svg
    Qt${QT_VERSION_MAJOR}::Test
    ${GTEST_LIBRARIES}
    ${GTEST_MAIN_LIBRARIES}
    pthread
    commondata
)

include_directories(${APP_PLUGIN_DIR}/src)
include_directories(${APP_COMMON_DIR}/src)
include_directories(${APP_3PARTY_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

macro(SUBDIRLIST result curdir)
    file(GLOB children RELATIVE ${curdir} ${curdir}/*)
    set(dirlist "")
    foreach(child ${children})
        if(IS_DIRECTORY ${curdir}/${child})
            LIST(APPEND dirlist ${child})
        endif()
    endforeach()
    set(${result} ${dirlist})
endmacro()

SUBDIRLIST(PLUGIN_SRC_DIRS ${APP_PLUGIN_DIR}/src)
foreach(subdir ${PLUGIN_SRC_DIRS})
    include_directories(${APP_PLUGIN_DIR}/src/${subdir})
endforeach()

QT_INCLUDE_GUI_PRIVATE_DIRS()

add_definitions("-DSCHEDULEPLUGIN_LIBRARY")
add_definitions("-DQT_DEPRECATED_WARNINGS")

# Get all schedule-plugin source files
file(GLOB_RECURSE SCHEDULE_PLUGIN_SRC ${APP_PLUGIN_DIR}/src/*.cpp)
# Remove plugin entry point
# Ensure moc runs on service.h (IService has Q_OBJECT)
list(APPEND SCHEDULE_PLUGIN_SRC ${APP_PLUGIN_DIR}/src/interface/service.h)
list(REMOVE_ITEM SCHEDULE_PLUGIN_SRC ${APP_PLUGIN_DIR}/src/scheduleplugin.cpp)

set(SCHEDULE_TEST_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_confirwfeedbackstate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_getchangedatastate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_queryschedulestate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_repeatfeedbackstate.cpp
    # 来自 V-6636（schedule-plugin data 模块）的测试，并入本目标
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_canceljsondata.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_changejsondata.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_clocaldata.cpp
)

add_executable(${APP_BIN_NAME}
    ${SCHEDULE_TEST_SRC}
    ${SCHEDULE_PLUGIN_SRC}
    ${APP_PLUGIN_DIR}/assets/resources.qrc
)

target_link_libraries(${APP_BIN_NAME} ${LINK_LIBS})
