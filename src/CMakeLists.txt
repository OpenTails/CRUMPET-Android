set(CMAKE_AUTORCC ON)
include_directories(${CMAKE_SOURCE_DIR}/3rdparty/kirigami/src)
include_directories(${Qt5RemoteObjects_INCLUDEDIR})
include_directories(${CMAKE_BINARY_DIR}/src)
include(${CMAKE_SOURCE_DIR}/3rdparty/kirigami/KF5Kirigami2Macros.cmake)

# Make sure we can register the types from the static library
add_definitions(-DKIRIGAMI_BUILD_TYPE_STATIC)

qt5_generate_repc(replication_sources BTConnectionManagerProxy.rep SOURCE)
qt5_generate_repc(replication_sources BTConnectionManagerProxy.rep REPLICA)
qt5_generate_repc(replication_sources CommandQueueProxy.rep SOURCE)
qt5_generate_repc(replication_sources CommandQueueProxy.rep REPLICA)
qt5_generate_repc(replication_sources SettingsProxy.rep SOURCE)
qt5_generate_repc(replication_sources SettingsProxy.rep REPLICA)
qt5_generate_repc(replication_sources GestureControllerProxy.rep SOURCE)
qt5_generate_repc(replication_sources GestureControllerProxy.rep REPLICA)

add_executable(digitail ${replication_sources})

target_sources(digitail
    PRIVATE
    main.cpp
    BTConnectionManager.cpp
    GearBase.cpp
    GearEars.cpp
    GearFake.cpp
    GearMitail.cpp
    GearDigitail.cpp
    BTDeviceCommandModel.cpp
    BTDeviceModel.cpp
    CommandInfo.cpp
    CommandPersistence.cpp
    CommandQueue.cpp
    FilterProxyModel.cpp
    GestureController.cpp
    GestureDetectorModel.cpp
    IdleMode.cpp
    AppSettings.cpp
    TailCommandModel.cpp
    Utilities.cpp
    Alarm.cpp
    AlarmList.cpp
    PermissionsManager.cpp
    WalkingSensorGestureReconizer.cpp

    kirigami-icons.qrc
    resources.qrc
    )

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

#kirigamiplugin is the static library built by us
target_link_libraries(digitail kirigamiplugin Qt5::Core Qt5::RemoteObjects Qt5::Widgets Qt5::Qml Qt5::Quick Qt5::QuickControls2 Qt5::Bluetooth Qt5::Sensors KF5::I18n ${digitail_EXTRA_LIBS})

# Translations
#add_custom_target(convert_translations_for_embedding
    #${CMAKE_CURRENT_SOURCE_DIR}/convert_translations.sh ${CMAKE_BINARY_DIR}
    #WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
#)
#add_dependencies(digitail convert_translations_for_embedding)

install(TARGETS digitail ${INSTALL_TARGETS_DEFAULT_ARGS})

include(${KI18n_BINARY_DIR}/cmake/KF5I18nMacros.cmake)
ki18n_install(po)

# kirigami_package_breeze_icons(ICONS application-menu document-decrypt folder-sync go-next go-previous go-up handle-left handle-right view-list-icons applications-graphics media-record-symbolic)
