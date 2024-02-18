cmake_minimum_required(VERSION 3.16)
project(digitail VERSION 0.1)
if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

set(BREEZEICONS_DIR ${CMAKE_SOURCE_DIR}/3rdparty/breeze-icons/)
set(DISABLE_DBUS true)

set(KF_MIN_VERSION "5.240.0")
set(QT_MIN_VERSION "6.5")

include(FeatureSummary)
find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(ECMSetupVersion)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
if (ANDROID)
    include(ECMAddAndroidApk)
endif ()

# Android has no DBus, so we can't use that. Sniff things and tell the code
if(CMAKE_SYSTEM_NAME STREQUAL Android)
    message("Building for Android - this means no dbus, and other small details. Work with that")
    add_definitions(-DANDROID)
    find_package(Qt6 ${Qt_MIN_VERSION} NO_MODULE REQUIRED Core Gui Quick Multimedia Sensors Test Widgets QuickControls2 Svg Bluetooth RemoteObjects)
    find_package(OpenSSL REQUIRED)
elseif(WIN32)
    message("Building for Windows - this means no dbus, and other small details. Work with that")
    add_definitions(-DWINDOWS)
    find_package(Qt6 ${Qt_MIN_VERSION} NO_MODULE REQUIRED Core Gui Quick Multimedia Sensors Test Widgets QuickControls2 Svg Bluetooth RemoteObjects)
else()
    find_package(Qt6 ${Qt_MIN_VERSION} NO_MODULE REQUIRED Core Gui Quick Multimedia Sensors Test Widgets QuickControls2 Svg Bluetooth RemoteObjects)
endif()

find_package(KF6 ${KF_MIN_VERSION} REQUIRED Kirigami I18n)
set_package_properties(KF6Kirigami PROPERTIES
    DESCRIPTION "KDE's lightweight user interface framework for mobile and convergent applications"
    URL "https://techbase.kde.org/Kirigami"
    PURPOSE "Required for the main UI of the app"
    TYPE RUNTIME)

add_subdirectory(src)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
