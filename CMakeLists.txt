cmake_minimum_required(VERSION 3.7)
project(dde-calendar)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX /usr)
endif()

include(GNUInstallDirs)

# check Qt version, support Qt5 default
find_package(Qt6 QUIET)

if(Qt6_FOUND)
  set(SUPPORT_QT6 TRUE)
  set(QT_VERSION_MAJOR 6)
  set(DTK_VERSION_MAJOR 6)
else()
  set(QT_VERSION_MAJOR 5)
endif()

message(STATUS "--- Enable Qt6: ${ENABLE_Qt6} Current use Qt Version: ${QT_VERSION_MAJOR}")

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
# There are translations on both the front and back ends, so take the translations to a higher level
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS LinguistTools)

# In fact, it only makes sense to manually translate .ts files without automatic translation,
# and you can create TS files with different language names, the same below
find_program(LUPDATE_EXECUTABLE lupdate)
find_program(LRELEASE_EXECUTABLE lrelease)
set(TS_FILES "${CMAKE_CURRENT_SOURCE_DIR}/translations/${PROJECT_NAME}_en_US.ts")
set(TS_SERVICEFILES "${CMAKE_CURRENT_SOURCE_DIR}/translations/${PROJECT_NAME}-service_en_US.ts")

foreach(_ts_file ${TS_FILES})
  execute_process(
    COMMAND ${LUPDATE_EXECUTABLE} -recursive ${CMAKE_CURRENT_SOURCE_DIR}/calendar-client -ts ${_ts_file})
endforeach()

foreach(_ts_file ${TS_SERVICEFILES})
  execute_process(
    COMMAND ${LUPDATE_EXECUTABLE} -recursive ${CMAKE_CURRENT_SOURCE_DIR}/calendar-service -ts ${_ts_file})
endforeach()

file(GLOB DTNG_TS_FILES translations/*.ts)

foreach(_ts_file ${DTNG_TS_FILES})
  execute_process(COMMAND ${LRELEASE_EXECUTABLE} ${_ts_file})
endforeach()

# lrelease end
if(SUPPORT_QT6)
  qt_create_translation(DTNG_QM_FILES
    ${DTNG_TS_FILES}
    ${DTNG_QM_FILES}
  )
else()
  qt5_create_translation(DTNG_QM_FILES
    ${DTNG_TS_FILES}
    ${DTNG_QM_FILES}
  )
endif()

file(GLOB APP_QM_FILES "translations/*.qm")
install(FILES ${APP_QM_FILES} DESTINATION ${CMAKE_INSTALL_DATADIR}/dde-calendar/translations)

# ##### Translation end #####

# 安装日志搜集工具的配置
install(FILES "org.deepin.calendar.json" DESTINATION ${CMAKE_INSTALL_DATADIR}/deepin-log-viewer/deepin-log.conf.d/)
