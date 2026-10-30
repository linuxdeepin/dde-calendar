cmake_minimum_required(VERSION 3.7)
project(dde-calendar-test)

set(ASAN_CMAKE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

option(BUILD_LEGACY_CALENDAR_TESTS
    "Build legacy calendar test suites that target removed scheduling APIs" OFF)

ADD_SUBDIRECTORY(dde-calendar-client-test)
ADD_SUBDIRECTORY(dde-calendar-service-test)
# schedule-plugin 单元测试目标（来自 V-6640 / V-6641，含 V-6636 的 data 测试）
ADD_SUBDIRECTORY(dde-calendar-schedule-test)
ADD_SUBDIRECTORY(dde-calendar-schedule-plugin-test)
# schedule-plugin dbus 单元测试目标（来自 V-6639）
ADD_SUBDIRECTORY(dde-schedule-dbus-test)

include_directories(third-party_stub)

add_custom_target(test)
add_custom_command(TARGET test
    COMMAND echo " =================== CREAT LCOV REPROT BEGIN ==================== "
    COMMAND ${CMAKE_BINARY_DIR}/tests/dde-calendar-client-test/dde-calendar-test
    COMMAND ${CMAKE_BINARY_DIR}/tests/dde-calendar-service-test/dde-calendar-service-test
    COMMAND ${CMAKE_BINARY_DIR}/tests/dde-calendar-schedule-test/dde-calendar-schedule-test
    COMMAND ${CMAKE_BINARY_DIR}/tests/dde-calendar-schedule-plugin-test/dde-calendar-schedule-plugin-test
    COMMAND ${CMAKE_BINARY_DIR}/tests/dde-schedule-dbus-test/dde-schedule-dbus-test
    COMMAND echo " =================== TEST END ==================== "
    )
