cmake_minimum_required(VERSION 3.2)
project(digitail VERSION 0.1)
if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif()

find_package(ECM 5.52.0 REQUIRED CONFIG)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/ ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

set(BREEZEICONS_DIR ${CMAKE_SOURCE_DIR}/3rdparty/breeze-icons/)
set(DISABLE_DBUS true)

find_package(Qt5 5.11 REQUIRED Core Quick Multimedia Test Widgets QuickControls2 Bluetooth)

include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(3rdparty)
add_subdirectory(src)
