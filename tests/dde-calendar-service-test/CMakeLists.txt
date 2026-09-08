cmake_minimum_required(VERSION 3.7)
project(dde-calendar-service-test)

ADD_COMPILE_OPTIONS(-fno-access-control)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(APP_BIN_NAME "dde-calendar-service-test")
set(APP_SERVICE_DIR "${CMAKE_SOURCE_DIR}/src/calendar-service")
set(APP_SERVICE_RES_DIR "${APP_SERVICE_DIR}/assets")
set(APP_QRC "${APP_SERVICE_RES_DIR}/resources.qrc")

# Find the library
find_package(PkgConfig REQUIRED)
find_package(Dtk${DTK_VERSION_MAJOR} REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core DBus Network Sql Test)
find_package(GTest REQUIRED)

set(LINK_LIBS
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Sql
    Dtk${DTK_VERSION_MAJOR}::Core
)

include_directories(${APP_SERVICE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/src/calendar-common/src)
include_directories(${CMAKE_SOURCE_DIR}/3rdparty/kcalendarcore/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

SUBDIRLIST(SERVICE_SRC_DIRS ${APP_SERVICE_DIR}/src)
foreach(subdir ${SERVICE_SRC_DIRS})
    include_directories(${APP_SERVICE_DIR}/src/${subdir})
endforeach()

set(HL_DATABASE_DIR ${APP_SERVICE_RES_DIR}/data/huangli.db)
set(SD_DATABASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/scheduler.db)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/config.h.in config.h @ONLY)

list(APPEND CMAKE_MODULE_PATH "${ASAN_CMAKE_DIR}")
include(asan)

set(CALDAV_SERVICE_SRC
    ${APP_SERVICE_DIR}/src/dbmanager/ddatabase.cpp
    ${APP_SERVICE_DIR}/src/dbmanager/daccountdatabase.cpp
    ${APP_SERVICE_DIR}/src/dbmanager/daccountmanagerdatabase.cpp
    ${APP_SERVICE_DIR}/src/alarmManager/dreminddata.cpp
    ${APP_SERVICE_DIR}/src/calendarDataManager/duploadtaskdata.cpp
)
file(GLOB CALDAV_PROTOCOL_SRC ${APP_SERVICE_DIR}/src/caldav/*.cpp)

set(CALDAV_TEST_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_dcalendareventlog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_dbmanager/test_caldavschema.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mock_caldav_server.cpp
    ${CMAKE_SOURCE_DIR}/src/calendar-common/src/dcalendareventlog.cpp
)

if(BUILD_LEGACY_CALENDAR_TESTS)
    file(GLOB_RECURSE LEGACY_CALENDAR_SERVICE_TEST_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
    list(REMOVE_ITEM LEGACY_CALENDAR_SERVICE_TEST_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/test_main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/test_dcalendareventlog.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/test_dbmanager/test_caldavschema.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/mock_caldav_server.cpp)
    list(APPEND CALDAV_TEST_SRC ${LEGACY_CALENDAR_SERVICE_TEST_SRC})

    file(GLOB_RECURSE LEGACY_CALENDAR_SERVICE_SRC
        ${APP_SERVICE_DIR}/src/*.cpp)
    list(REMOVE_ITEM LEGACY_CALENDAR_SERVICE_SRC ${APP_SERVICE_DIR}/src/main.cpp)
    list(APPEND CALDAV_TEST_SRC ${LEGACY_CALENDAR_SERVICE_SRC})
endif()

add_executable(${APP_BIN_NAME}
    ${CALDAV_TEST_SRC}
    ${CALDAV_SERVICE_SRC}
    ${CALDAV_PROTOCOL_SRC}
)

target_link_libraries(${APP_BIN_NAME}
    ${LINK_LIBS}
    ${GTEST_LIBRARIES}
    ${GTEST_MAIN_LIBRARIES}
    pthread
    commondata
)
