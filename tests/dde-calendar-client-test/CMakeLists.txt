cmake_minimum_required(VERSION 3.7)
include(${CMAKE_SOURCE_DIR}/cmake/qt-version-macros.cmake)

project(dde-calendar-client-test)

ADD_COMPILE_OPTIONS(-fno-access-control)

set(APP_CLIENT_DIR "${CMAKE_SOURCE_DIR}/src/calendar-client")
set(APP_COMMON_DIR "${CMAKE_SOURCE_DIR}/src/calendar-common")
set(APP_3PARTY_DIR "${CMAKE_SOURCE_DIR}/3rdparty/kcalendarcore")
set(APP_BIN_NAME "dde-calendar-test")
set(PROJECT_NAME_TEST ${APP_BIN_NAME})

find_package(PkgConfig REQUIRED)
find_package(Dtk${DTK_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widget)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED Gui Widgets DBus Svg Test)
find_package(GTest REQUIRED)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--as-needed -fPIE")
set(CMAKE_EXE_LINKER_FLAGS "-pie")

include_directories(${APP_CLIENT_DIR}/src)
include_directories(${APP_CLIENT_DIR}/src/dbus)
include_directories(${APP_COMMON_DIR}/src)
include_directories(${APP_3PARTY_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

QT_INCLUDE_GUI_PRIVATE_DIRS()

set(CALENDAR_CLIENT_TEST_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/client_main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_dbus/test_caldavdbussignals.cpp
    ${APP_CLIENT_DIR}/src/dbus/cdbuspendingcallwatcher.cpp
    ${APP_CLIENT_DIR}/src/dbus/dbusrequestbase.cpp
    ${APP_CLIENT_DIR}/src/dbus/dbusaccountmanagerrequest.cpp
    ${APP_CLIENT_DIR}/src/dbus/dbusaccountrequest.cpp
)

if(BUILD_LEGACY_CALENDAR_TESTS)
    SUBDIRLIST(CLIENT_TEST_SRC_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src)
    foreach(subdir ${CLIENT_TEST_SRC_DIRS})
        include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/${subdir})
    endforeach()

    file(GLOB_RECURSE LEGACY_CALENDAR_CLIENT_TEST_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
    list(REMOVE_ITEM LEGACY_CALENDAR_CLIENT_TEST_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/client_main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/test_dbus/test_caldavdbussignals.cpp)
    list(APPEND CALENDAR_CLIENT_TEST_SRC ${LEGACY_CALENDAR_CLIENT_TEST_SRC})

    file(GLOB_RECURSE CALENDAR_CLIENT_SRC ${APP_CLIENT_DIR}/src/*.cpp)
    file(GLOB_RECURSE CALENDAR_COMMON_SRC ${APP_COMMON_DIR}/src/*.cpp)
    file(GLOB_RECURSE CALENDAR_3PARTY_SRC ${APP_3PARTY_DIR}/src/*.cpp)
    list(REMOVE_ITEM CALENDAR_CLIENT_SRC ${APP_CLIENT_DIR}/src/main.cpp)
    list(REMOVE_ITEM CALENDAR_COMMON_SRC ${APP_COMMON_DIR}/src/main.cpp)
    list(REMOVE_ITEM CALENDAR_3PARTY_SRC ${APP_3PARTY_DIR}/src/main.cpp)
    list(APPEND CALENDAR_CLIENT_TEST_SRC
        ${CALENDAR_CLIENT_SRC}
        ${CALENDAR_COMMON_SRC}
        ${CALENDAR_3PARTY_SRC})
endif()

add_executable(${APP_BIN_NAME} ${CALENDAR_CLIENT_TEST_SRC})

target_include_directories(${APP_BIN_NAME} PUBLIC ${OBJECT_BINARY_DIR})

target_link_libraries(${APP_BIN_NAME}
    Dtk${DTK_VERSION_MAJOR}::Core
    Dtk${DTK_VERSION_MAJOR}::Gui
    Dtk${DTK_VERSION_MAJOR}::Widget
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Svg
    Qt${QT_VERSION_MAJOR}::Test
    ${GTEST_LIBRARIES}
    ${GTEST_MAIN_LIBRARIES}
    pthread
    commondata
)
