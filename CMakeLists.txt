cmake_minimum_required(VERSION 3.16)
project(digitail VERSION 0.1)
if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)

set(BREEZEICONS_DIR ${CMAKE_SOURCE_DIR}/3rdparty/breeze-icons/)
set(DISABLE_DBUS true)

add_subdirectory(3rdparty)

find_package(ECM 5.52.0 REQUIRED CONFIG)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/ ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${CMAKE_BINARY_DIR}/3rdparty/ki18n/cmake/)

# Android has no DBus, so we can't use that. Sniff things and tell the code
if(CMAKE_SYSTEM_NAME STREQUAL Android)
    message("Building for Android - this means no dbus, and other small details. Work with that")
    add_definitions(-DANDROID)
    SET(QT_QMAKE_EXECUTABLE "$ENV{Qt5_android}/bin/qmake")
    find_package(Qt5 5.12 REQUIRED Core Gui Quick Multimedia Sensors Test Widgets QuickControls2 Svg Bluetooth RemoteObjects AndroidExtras)
elseif(WIN32)
    message("Building for Windows - this means no dbus, and other small details. Work with that")
    add_definitions(-DWINDOWS)
    find_package(Qt5 5.12 REQUIRED Core Gui Quick Multimedia Sensors Test Widgets QuickControls2 Svg Bluetooth RemoteObjects)
else()
    find_package(Qt5 5.12 REQUIRED Core Gui Quick Multimedia Sensors Test Widgets QuickControls2 Svg Bluetooth RemoteObjects)
endif()

find_package(KF5 ${KF5_MIN_VERSION} REQUIRED I18n)

include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)

add_subdirectory(src)
