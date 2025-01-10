cmake_minimum_required(VERSION 3.7)
project(kcalendarcore)

# Find the library
find_package(PkgConfig REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui DBus)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
pkg_check_modules(3rd_lib REQUIRED libical)

# 安全编译参数
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-strong -z noexecstack -pie -fPIC -z lazy")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src ${3rd_lib_INCLUDE_DIRS})
aux_source_directory(src CALENDARCORE_SRCS)

add_library(${PROJECT_NAME} STATIC ${CALENDARCORE_SRCS})

target_link_libraries(${PROJECT_NAME}
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Gui
    ${3rd_lib_LIBRARIES}
    ${Libical_LIBRARIES}
    icalvcal
    icalss)
