cmake_minimum_required(VERSION 3.7)
project(commondata)

find_package(PkgConfig REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core DBus Sql)
find_package(Dtk${DTK_VERSION_MAJOR} REQUIRED Core)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

# 安全编译参数
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-strong -z noexecstack -pie -fPIC -z lazy")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
aux_source_directory(src BASESTRUCT_SRCS)
aux_source_directory(src/huangliData BASESTRUCT_SRCS_HUANGLI)
aux_source_directory(src/lunarandfestival BASESTRUCT_SRCS_NONGLI)
aux_source_directory(src/pinyin BASESTRUCT_SRCS_PINYIN)
pkg_check_modules(LIBICAL REQUIRED IMPORTED_TARGET libical>=3.0)

add_library(${PROJECT_NAME} STATIC ${BASESTRUCT_SRCS}
    ${BASESTRUCT_SRCS_HUANGLI}
    ${BASESTRUCT_SRCS_NONGLI}
    ${BASESTRUCT_SRCS_PINYIN}
    src/calendarcore/core/calstore.cpp
    src/calendarcore/item/calevent.cpp
    src/calendarcore/recurrence/rrule.cpp
    src/calendarcore/recurrence/repeatinfo.cpp
    src/calendarcore/ics/icscodec.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/calendarcore/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/calendarcore/item
    ${CMAKE_CURRENT_SOURCE_DIR}/src/calendarcore/recurrence
    ${CMAKE_CURRENT_SOURCE_DIR}/src/calendarcore/ics
)

target_link_libraries(${PROJECT_NAME}
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Sql
    PkgConfig::LIBICAL
    Dtk${DTK_VERSION_MAJOR}::Core
)
