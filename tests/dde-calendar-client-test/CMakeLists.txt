cmake_minimum_required(VERSION 3.7)

# common resource names
project(dde-calendar-client-test)

ADD_COMPILE_OPTIONS(-fno-access-control)

set(APP_CLIENT_DIR "${CMAKE_SOURCE_DIR}/calendar-client")
set(APP_COMMON_DIR "${CMAKE_SOURCE_DIR}/calendar-common")
set(APP_3PARTY_DIR "${CMAKE_SOURCE_DIR}/3rdparty/kcalendarcore")

# set(APP_RES_DIR "${APP_CLIENT_DIR}/assets")
# set(APP_QRC "${APP_RES_DIR}/resources.qrc")
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
SUBDIRLIST(all_src ${APP_CLIENT_DIR}/src)

include_directories(${APP_COMMON_DIR}/src)
SUBDIRLIST(all_src ${APP_COMMON_DIR}/src)

include_directories(${APP_3PARTY_DIR}/src)
SUBDIRLIST(all_src ${APP_3PARTY_DIR}/src)

# Include all app own subdirectories
foreach(subdir ${all_src})
    include_directories(${APP_CLIENT_DIR}/src/${subdir})
    include_directories(${APP_COMMON_DIR}/src/${subdir})
    include_directories(${APP_COMMON_DIR}/src/${subdir})
endforeach()

if(SUPPORT_QT6)
    include_directories(${Qt6Gui_PRIVATE_INCLUDE_DIRS})
else()
    include_directories(${Qt5Gui_PRIVATE_INCLUDE_DIRS})
endif()

file(GLOB_RECURSE Calendar_CLIENTSRC ${APP_CLIENT_DIR}/src/*.cpp)
file(GLOB_RECURSE Calendar_COMMONSRC ${APP_COMMON_DIR}/src/*.cpp)
file(GLOB_RECURSE Calendar_3PARTYSRC ${APP_3PARTY_DIR}/src/*.cpp)
list(REMOVE_ITEM Calendar_CLIENTSRC ${APP_CLIENT_DIR}/src/main.cpp)
list(REMOVE_ITEM Calendar_COMMONSRC ${APP_COMMON_DIR}/src/main.cpp)
list(REMOVE_ITEM Calendar_3PARTYSRC ${APP_3PARTY_DIR}/src/main.cpp)

# test SRC
SUBDIRLIST(TEST_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src)

foreach(subdir ${TEST_SRC})
    include_directories(${subdir})
endforeach()

file(GLOB_RECURSE Calendar_TEST_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
include_directories(../../calendar-basicstruct/)

list(APPEND CMAKE_MODULE_PATH "${ASAN_CMAKE_DIR}")
include(asan)

# Tell CMake to create the executable
add_executable(${APP_BIN_NAME} ${Calendar_TEST_SRC} ${Calendar_CLIENTSRC} ${Calendar_COMMONSRC} ${Calendar_3PARTYSRC} ${APP_QRC})

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
    basestruct
)
