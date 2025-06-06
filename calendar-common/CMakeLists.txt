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

add_library(${PROJECT_NAME} STATIC ${BASESTRUCT_SRCS}
    ${BASESTRUCT_SRCS_HUANGLI}
    ${BASESTRUCT_SRCS_NONGLI}
    ${BASESTRUCT_SRCS_PINYIN}
)
target_include_directories(${PROJECT_NAME} PUBLIC ../3rdparty/kcalendarcore/src)

target_link_libraries(${PROJECT_NAME}
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Sql
    kcalendarcore
    Dtk${DTK_VERSION_MAJOR}::Core
)
