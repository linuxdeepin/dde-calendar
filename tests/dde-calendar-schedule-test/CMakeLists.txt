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
include_directories(${CMAKE_SOURCE_DIR}/tests/third-party_stub)
include_directories(${CMAKE_SOURCE_DIR}/tests/dde-calendar-schedule-plugin-test/src)  # test_helpers.h

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
    # 本轮（V-6635/6637/6638/6642-6647）新增的插件测试
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_accountitem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_accountmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_buttonwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_cancelscheduletask.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_changescheduletask.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_createjsondata.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_createscheduletask.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_createschedulewidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_dscheduledatamanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_icondframe.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_itemwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_jsondata.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_modifyscheduleitem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_opencalendarwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_queryjsondata.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_queryscheduleproxy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_queryscheduletask.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_repeatschedulewidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_schedulebasetask.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_scheduledatamanage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_scheduleitemwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_schedulelistwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_schedulemanagetask.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_scheduleservice.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_semanticanalysistask.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_sp_cdbuspendingcallwatcher.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_sp_dbusaccountmanagerrequest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_sp_dbusaccountrequest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_viewschedulewidget.cpp
)

add_executable(${APP_BIN_NAME}
    ${SCHEDULE_TEST_SRC}
    ${SCHEDULE_PLUGIN_SRC}
    ${APP_PLUGIN_DIR}/assets/resources.qrc
)

target_link_libraries(${APP_BIN_NAME} ${LINK_LIBS})
