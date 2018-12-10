include_directories(${CMAKE_SOURCE_DIR}/3rdparty/kirigami/src)
include(${CMAKE_SOURCE_DIR}/3rdparty/kirigami/KF5Kirigami2Macros.cmake)

// Make sure we can register the types from the static library
add_definitions(-DKIRIGAMI_BUILD_TYPE_STATIC)

set(digitail_SRCS
    main.cpp
    btconnectionmanager.cpp
    btdevicemodel.cpp
    )

qt5_add_resources(RESOURCES kirigami-icons.qrc resources.qrc)

if (CMAKE_SYSTEM_NAME STREQUAL "Android")
    set(digitail_EXTRA_LIBS Qt5::AndroidExtras
    Qt5::Gui
    #FIXME: we shouldn't have to link to these but otherwise their libs won't be packaged on Android
    Qt5::QuickControls2
    Qt5::Svg)
# else ()
#qstyle-based qqc2 style needs a QApplication
#     set(digitail_EXTRA_LIBS )
endif()


add_executable(digitail ${digitail_SRCS} ${RESOURCES})
#kirigamiplugin is the static library built by us
target_link_libraries(digitail kirigamiplugin Qt5::Core Qt5::Widgets Qt5::Qml Qt5::Quick Qt5::QuickControls2 Qt5::Bluetooth ${digitail_EXTRA_LIBS})

install(TARGETS digitail ${INSTALL_TARGETS_DEFAULT_ARGS})

# kirigami_package_breeze_icons(ICONS application-menu document-decrypt folder-sync go-next go-previous go-up handle-left handle-right view-list-icons applications-graphics media-record-symbolic)
