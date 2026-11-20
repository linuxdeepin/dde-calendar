cmake_minimum_required(VERSION 3.7)
include(${CMAKE_SOURCE_DIR}/cmake/qt-version-macros.cmake)

project(dde-calendar-kernel-test)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--as-needed -fPIE")
set(CMAKE_EXE_LINKER_FLAGS "-pie")

set(APP_BIN_NAME "dde-calendar-kernel-test")
set(APP_COMMON_DIR "${CMAKE_SOURCE_DIR}/src/calendar-common")
set(FIXTURES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/fixtures")
set(GOLDEN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/golden")

find_package(PkgConfig REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Test)
find_package(GTest REQUIRED)

# Comparison mode: link commondata (which includes calendarkernel)
include_directories(${APP_COMMON_DIR}/src)

add_executable(${APP_BIN_NAME}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_ics_roundtrip.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_recurrence_expansion.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_rrule_summary.cpp
)

target_link_libraries(${APP_BIN_NAME}
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Test
    ${GTEST_LIBRARIES}
    ${GTEST_MAIN_LIBRARIES}
    pthread
    commondata
)

target_compile_definitions(${APP_BIN_NAME} PRIVATE
    FIXTURES_DIR="${FIXTURES_DIR}"
    GOLDEN_DIR="${GOLDEN_DIR}"
)
