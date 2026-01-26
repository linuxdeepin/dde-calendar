# Macro for adding DBus interface with Qt version detection
macro(QT_ADD_DBUS_INTERFACE)
    if(SUPPORT_QT6)
        qt_add_dbus_interface(${ARGN})
    else()
        qt5_add_dbus_interface(${ARGN})
    endif()
endmacro()

# Macro for adding resources with Qt version detection
macro(QT_ADD_RESOURCES_MACRO)
    if(SUPPORT_QT6)
        qt_add_resources(${ARGN})
    else()
        qt5_add_resources(${ARGN})
    endif()
endmacro()

# Macro for including Qt Gui private headers
macro(QT_INCLUDE_GUI_PRIVATE_DIRS)
    if(SUPPORT_QT6)
        include_directories(${Qt6Gui_PRIVATE_INCLUDE_DIRS})
    else()
        include_directories(${Qt5Gui_PRIVATE_INCLUDE_DIRS})
    endif()
endmacro()
