cmake_minimum_required(VERSION 3.7)
project(dde-calendar)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX /usr)
endif()

include(GNUInstallDirs)

# Find Qt version
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
message("   >>> Found Qt version: ${QT_VERSION_MAJOR}")
# 检查并设置环境变量
if(NOT DEFINED ENV{QT_SELECT})
  set(ENV{QT_SELECT} "qt${QT_VERSION_MAJOR}")
endif()
message("   >>> Build with Qt version: $ENV{QT_SELECT}")

if (QT_VERSION_MAJOR MATCHES 6)
  set(SUPPORT_QT6 TRUE)
  set(DTK_VERSION_MAJOR 6)
else()
  set(DTK_VERSION_MAJOR "")
endif()
message("   >>> Build with DTK: ${DTK_VERSION_MAJOR}")

# compile flags
if(CMAKE_BUILD_TYPE MATCHES Debug)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Wextra")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wextra")

  # Enable Qt builtin debug mode
  add_definitions("-DQT_MESSAGELOGCONTEXT")
else()
  # -Wl, -O2 Enable linker optimizations
  # -Wl, --gc-sections Remove unused code resulting from -fdsta-sections and
  # -ffunction-sections
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O2 -Wl,--gc-sections")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O2 -Wl,--gc-sections")
endif()

# 代码覆盖率开关
if(CMAKE_COVERAGE_ARG STREQUAL "CMAKE_COVERAGE_ARG_ON")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -fprofile-arcs -ftest-coverage")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -fprofile-arcs -ftest-coverage")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)

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

ADD_SUBDIRECTORY(3rdparty/kcalendarcore)
ADD_SUBDIRECTORY(calendar-common)
ADD_SUBDIRECTORY(calendar-client)
ADD_SUBDIRECTORY(calendar-service)
ADD_SUBDIRECTORY(schedule-plugin)

# FIXME: Unit tests have not been maintained for a long time
# ADD_SUBDIRECTORY(tests)

# ##### Translation #####
include(translation-generate)
TRANSLATION_GENERATE(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR}/translations)
add_custom_target(${PROJECT_NAME}_qm_files DEPENDS ${QM_FILES})
add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_qm_files)
install(FILES ${QM_FILES} DESTINATION ${CMAKE_INSTALL_DATADIR}/dde-calendar/translations)

# ##### Translation end #####

# Install log json config file.
install(FILES configs/logconfig/dde-calendar.json DESTINATION ${CMAKE_INSTALL_PREFIX}/share/deepin-log-viewer/deepin-log.conf.d/)

# Install debugmode json config file.
install(FILES configs/debugconfig/org.deepin.dde.calendar.json DESTINATION ${CMAKE_INSTALL_PREFIX}/share/deepin-debug-config/deepin-debug-config.d/)

# 安装 DConfig 配置文件，1040及以下的默认DTK环境不支持DConfig
set(DCONFIG_APPID org.deepin.dde.calendar)
file(GLOB DCONFIG_FILES "${CMAKE_CURRENT_SOURCE_DIR}/configs/dconfig/*.json")
if(DEFINED DSG_DATA_DIR)
    if (DTK_VERSION_MAJOR EQUAL "6")
        dtk_add_config_meta_files(APPID ${DCONFIG_APPID} FILES ${DCONFIG_FILES})
    else()
        dconfig_meta_files(APPID ${DCONFIG_APPID} FILES ${DCONFIG_FILES})
    endif()
    message("-- DConfig is supported by DTK")
else()
    install(FILES ${DCONFIG_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/dsg/configs/${DCONFIG_APPID})
    message("-- DConfig is not supported by DTK")
endif()