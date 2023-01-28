cmake_minimum_required(VERSION 3.16)
project(digitail VERSION 0.1)
if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

set(BREEZEICONS_DIR ${CMAKE_SOURCE_DIR}/3rdparty/breeze-icons/)
set(DISABLE_DBUS true)

include(FeatureSummary)
find_package(ECM 5.52.0 REQUIRED CONFIG)
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)

# Android has no DBus, so we can't use that. Sniff things and tell the code
if(CMAKE_SYSTEM_NAME STREQUAL Android)
    message("Building for Android - this means no dbus, and other small details. Work with that")
    add_definitions(-DANDROID)
    SET(QT_QMAKE_EXECUTABLE "$ENV{Qt5_android}/bin/qmake")
    find_package(Qt5 5.15 REQUIRED Core Gui Quick Multimedia Sensors Test Widgets QuickControls2 Svg Bluetooth RemoteObjects AndroidExtras)
    find_package(OpenSSL REQUIRED)
elseif(WIN32)
    message("Building for Windows - this means no dbus, and other small details. Work with that")
    add_definitions(-DWINDOWS)
    find_package(Qt5 5.15 REQUIRED Core Gui Quick Multimedia Sensors Test Widgets QuickControls2 Svg Bluetooth RemoteObjects)
else()
    find_package(Qt5 5.15 REQUIRED Core Gui Quick Multimedia Sensors Test Widgets QuickControls2 Svg Bluetooth RemoteObjects)
endif()

find_package(KF5 5.52 REQUIRED I18n)
find_package(KF5Kirigami2 2.7.0 REQUIRED)
set_package_properties(KF5Kirigami2 PROPERTIES
    DESCRIPTION "KDE's lightweight user interface framework for mobile and convergent applications"
    URL "https://techbase.kde.org/Kirigami"
    PURPOSE "Required for the main UI of the app"
    TYPE RUNTIME)

add_subdirectory(src)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
