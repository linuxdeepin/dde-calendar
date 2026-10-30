cmake_minimum_required(VERSION 3.7)
include(${CMAKE_SOURCE_DIR}/cmake/qt-version-macros.cmake)

project(dde-schedule-dbus-test)

ADD_COMPILE_OPTIONS(-fno-access-control)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(APP_BIN_NAME "dde-schedule-dbus-test")
set(APP_SCHEDULE_DIR "${CMAKE_SOURCE_DIR}/src/schedule-plugin")
set(APP_COMMON_DIR "${CMAKE_SOURCE_DIR}/src/calendar-common")

find_package(PkgConfig REQUIRED)
find_package(Dtk${DTK_VERSION_MAJOR} REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core DBus Test)
find_package(GTest REQUIRED)

include_directories(${APP_SCHEDULE_DIR}/src/dbus)
include_directories(${APP_COMMON_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

set(SCHEDULE_DBUS_SRC
    ${APP_SCHEDULE_DIR}/src/dbus/cdbuspendingcallwatcher.cpp
    ${APP_SCHEDULE_DIR}/src/dbus/dbusrequestbase.cpp
    ${APP_SCHEDULE_DIR}/src/dbus/dbushuanglirequest.cpp
)

set(SCHEDULE_DBUS_TEST_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_dbusrequestbase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_dbushuanglirequest.cpp
)

add_executable(${APP_BIN_NAME}
    ${SCHEDULE_DBUS_SRC}
    ${SCHEDULE_DBUS_TEST_SRC}
)

target_link_libraries(${APP_BIN_NAME}
    Dtk${DTK_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Test
    ${GTEST_LIBRARIES}
    ${GTEST_MAIN_LIBRARIES}
    pthread
)
