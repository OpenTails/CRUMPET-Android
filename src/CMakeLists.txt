include_directories(${CMAKE_SOURCE_DIR}/3rdparty/kirigami/src)
include_directories(${Qt5RemoteObjects_INCLUDEDIR})
include_directories(${CMAKE_BINARY_DIR}/src)
include(${CMAKE_SOURCE_DIR}/3rdparty/kirigami/KF5Kirigami2Macros.cmake)

# Make sure we can register the types from the static library
add_definitions(-DKIRIGAMI_BUILD_TYPE_STATIC)

set(digitail_SRCS
    main.cpp
    btconnectionmanager.cpp
    btdevicemodel.cpp
    commandqueue.cpp
    filterproxymodel.cpp
    settings.cpp
    tailcommandmodel.cpp
    )

qt5_add_resources(RESOURCES kirigami-icons.qrc resources.qrc)

if(Qt5RemoteObjects_FOUND)
    qt5_generate_repc(digitail_SRCS btconnectionmanagerproxy.rep SOURCE)
    qt5_generate_repc(digitail_SRCS btconnectionmanagerproxy.rep REPLICA)
    qt5_generate_repc(digitail_SRCS commandqueueproxy.rep SOURCE)
    qt5_generate_repc(digitail_SRCS commandqueueproxy.rep REPLICA)
    qt5_generate_repc(digitail_SRCS settingsproxy.rep SOURCE)
    qt5_generate_repc(digitail_SRCS settingsproxy.rep REPLICA)
endif()

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

if(Qt5RemoteObjects_FOUND)
    target_link_libraries(digitail Qt5::RemoteObjects)
    target_compile_definitions(digitail PUBLIC -DHAS_QT5REMOTEOBJECTS)
endif()

install(TARGETS digitail ${INSTALL_TARGETS_DEFAULT_ARGS})

# kirigami_package_breeze_icons(ICONS application-menu document-decrypt folder-sync go-next go-previous go-up handle-left handle-right view-list-icons applications-graphics media-record-symbolic)
