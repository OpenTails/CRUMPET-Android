cmake_minimum_required(VERSION 3.2)
project(digitail VERSION 0.1)
if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif()

find_package(ECM 5.52.0 REQUIRED CONFIG)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/ ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

set(BREEZEICONS_DIR ${CMAKE_SOURCE_DIR}/3rdparty/breeze-icons/)
set(DISABLE_DBUS true)

# Android has no DBus, so we can't use that. Sniff things and tell the code
if(CMAKE_SYSTEM_NAME STREQUAL Android)
    message("Building for Android - this means no dbus, and other small details. Work with that")
    add_definitions(-DANDROID)
    SET(QT_QMAKE_EXECUTABLE "$ENV{Qt5_android}/bin/qmake")
    find_package(Qt5 5.11 REQUIRED Core Gui Quick Multimedia Test Widgets QuickControls2 Svg Bluetooth AndroidExtras)
elseif(WIN32)
    message("Building for Windows - this means no dbus, and other small details. Work with that")
    add_definitions(-DWINDOWS)
    find_package(Qt5 5.11 REQUIRED Core Gui Quick Multimedia Test Widgets QuickControls2 Svg Bluetooth)
else()
    find_package(Qt5 5.11 REQUIRED Core Gui Quick Multimedia Test Widgets QuickControls2 Svg Bluetooth)
endif()


include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(3rdparty)
add_subdirectory(src)
