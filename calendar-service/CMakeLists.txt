cmake_minimum_required(VERSION 3.7)
project(dde-calendar-service)


set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

#安全编译参数
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-strong -z noexecstack -pie -fPIC -z lazy")

set(APP_RES_DIR "assets")
set(APP_SERVICE "${APP_RES_DIR}/data/com.deepin.dataserver.Calendar.service")
set(APP_SYSTEMD_SERVICE "${APP_RES_DIR}/data/com.dde.calendarserver.calendar.service")
set(APP_SYSTEMD_TIMER "${APP_RES_DIR}/data/com.dde.calendarserver.calendar.timer")
set(AUTOSTART_DESKTOP "${APP_RES_DIR}/dde-calendar-service.desktop")
set(HUANGLIDB "${APP_RES_DIR}/data/huangli.db")
set(APP_QRC "${APP_RES_DIR}/resources.qrc")


# Find the library
find_package(PkgConfig REQUIRED)
find_package(DtkCore REQUIRED)
find_package(Qt5 COMPONENTS
    Core
    DBus
    Sql
REQUIRED)

set(LINK_LIBS
    Qt5::Core
    Qt5::DBus
    Qt5::Sql
    ${DtkCore_LIBRARIES}
    commondata
    -lpthread
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

SUBDIRLIST(all_src ${CMAKE_CURRENT_SOURCE_DIR}/src)

#Include all app own subdirectories
foreach(subdir ${all_src})
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/${subdir})
endforeach()
file(GLOB_RECURSE CALENDARSERVICE_SRCS ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)
#添加资源文件
QT5_ADD_RESOURCES(CALENDARSERVICE_SRCS ${APP_QRC})


#后端程序自动退出宏控制
if (NOT (CMAKE_BUILD_TYPE MATCHES Debug))
add_definitions(-DCALENDAR_SERVICE_AUTO_EXIT)
endif()

add_executable(${PROJECT_NAME} ${CALENDARSERVICE_SRCS})

target_include_directories(${PROJECT_NAME} PUBLIC ${DtkWidget_INCLUDE_DIRS} ${OBJECT_BINARY_DIR} ../calendar-common/src)

target_link_libraries(${PROJECT_NAME}
    ${LINK_LIBS}
    )

#根据环境修改dbus服务文件
if(DEFINED ENV{PREFIX})
   set(APP_DBUS_SERVICE
"[D-BUS Service]
Name=com.deepin.dataserver.Calendar
Exec=/usr/bin/ll-cli run  org.dde.calendar --exec  /opt/apps/org.dde.calendar/files/lib/deepin-daemon/dde-calendar-service
")
   message("${APP_DBUS_SERVICE}")
   file(WRITE ${APP_SERVICE} "${APP_DBUS_SERVICE}")

    set(CMAKE_INSTALL_PREFIX $ENV{PREFIX})
else()
    set(CMAKE_INSTALL_PREFIX /usr)
endif()

add_definitions(-DLINGLONG_PREFIX=\"${CMAKE_INSTALL_PREFIX}/\")
 

# Install files
install(TARGETS ${PROJECT_NAME} DESTINATION lib/deepin-daemon/)
install(FILES ${APP_SERVICE} DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/services/)
install(FILES ${AUTOSTART_DESKTOP} DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/xdg/autostart/)
install(FILES ${HUANGLIDB} DESTINATION ${CMAKE_INSTALL_DATADIR}/dde-calendar/data/)
install(FILES ${APP_SYSTEMD_SERVICE} DESTINATION lib/systemd/user/)
install(FILES ${APP_SYSTEMD_TIMER} DESTINATION lib/systemd/user/)
