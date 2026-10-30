cmake_minimum_required(VERSION 3.7)
project(dde-calendar-schedule-plugin-test)

ADD_COMPILE_OPTIONS(-fno-access-control)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(APP_BIN_NAME "dde-calendar-schedule-plugin-test")

set(SCHEDULE_PLUGIN_DIR "${CMAKE_SOURCE_DIR}/src/schedule-plugin")
set(CALENDAR_COMMON_DIR "${CMAKE_SOURCE_DIR}/src/calendar-common")
set(KCALENDARCORE_DIR "${CMAKE_SOURCE_DIR}/3rdparty/kcalendarcore")

find_package(PkgConfig REQUIRED)
find_package(Dtk${DTK_VERSION_MAJOR} REQUIRED COMPONENTS Widget)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core DBus Svg Widgets Test)
find_package(GTest REQUIRED)

set(LINK_LIBS
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Svg
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Test
    Dtk${DTK_VERSION_MAJOR}::Widget
)

# Include paths
include_directories(${SCHEDULE_PLUGIN_DIR}/src)
include_directories(${CALENDAR_COMMON_DIR}/src)
include_directories(${KCALENDARCORE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/tests/third-party_stub)

# SUBDIRLIST macro (in case not inherited from parent)
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

# Include all schedule-plugin subdirectories
SUBDIRLIST(SCHEDULE_PLUGIN_SUBDIRS ${SCHEDULE_PLUGIN_DIR}/src)
foreach(subdir ${SCHEDULE_PLUGIN_SUBDIRS})
    include_directories(${SCHEDULE_PLUGIN_DIR}/src/${subdir})
endforeach()

# Glob all schedule-plugin source files
file(GLOB_RECURSE SCHEDULE_PLUGIN_SRC ${SCHEDULE_PLUGIN_DIR}/src/*.cpp)

# Test source files
set(TEST_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_schedulestate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_selectandquerystate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_selectinquirystate.cpp
    # 本轮（V-6645/6646/6647）新增的插件测试；Scheduleplugin 入口类仅在本目标编译
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_scheduleplugin.cpp
)

add_executable(${APP_BIN_NAME}
    ${TEST_SRC}
    ${SCHEDULE_PLUGIN_SRC}
    ${SCHEDULE_PLUGIN_DIR}/src/interface/service.h
)

target_link_libraries(${APP_BIN_NAME}
    ${LINK_LIBS}
    ${GTEST_LIBRARIES}
    ${GTEST_MAIN_LIBRARIES}
    pthread
    commondata
)
