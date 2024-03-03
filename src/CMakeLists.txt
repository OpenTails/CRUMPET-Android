set(CMAKE_AUTORCC ON)
include_directories(${CMAKE_BINARY_DIR}/src)

# Make sure we can register the types from the static library
add_definitions(-DKIRIGAMI_BUILD_TYPE_STATIC)

add_executable(digitail)

target_sources(digitail
    PRIVATE
    main.cpp
    BTConnectionManager.cpp
    GearCommandModel.cpp
    GearBase.cpp
    CommandInfo.cpp
    CommandModel.cpp
    CommandPersistence.cpp
    CommandQueue.cpp
    DeviceModel.cpp
    FilterProxyModel.cpp
    GestureController.cpp
    GestureDetectorModel.cpp
    GestureSensor.cpp
    IdleMode.cpp
    AppSettings.cpp
    Utilities.cpp
    Alarm.cpp
    AlarmList.cpp
    PermissionsManager.cpp
    WalkingSensorGestureReconizer.cpp

    gearimplementations/GearEars.cpp
    gearimplementations/GearFake.cpp
    gearimplementations/GearFlutterWings.cpp
    gearimplementations/GearMitail.cpp
    gearimplementations/GearMitailMini.cpp
    gearimplementations/GearDigitail.cpp

    kirigami-icons.qrc
    resources.qrc
    )

qt_add_repc_sources(digitail
    AppSettingsProxy.rep
    BTConnectionManagerProxy.rep
    CommandQueueProxy.rep
    GestureControllerProxy.rep
    )
qt_add_repc_replicas(digitail
    AppSettingsProxy.rep
    BTConnectionManagerProxy.rep
    CommandQueueProxy.rep
    GestureControllerProxy.rep
    )

if (CMAKE_SYSTEM_NAME STREQUAL "Android")
    set(digitail_EXTRA_LIBS
        OpenSSL::SSL
        Qt6::Gui
        #FIXME: we shouldn't have to link to these but otherwise their libs won't be packaged on Android
        Qt6::QuickControls2
        Qt6::Svg
        KF6::Kirigami
        PRIVATE
        Qt6::CorePrivate # for QAndroidService
        )
# else ()
#qstyle-based qqc2 style needs a QApplication
#     set(digitail_EXTRA_LIBS )
endif()

target_link_libraries(digitail
    Qt6::Core
    Qt6::RemoteObjects
    Qt6::Widgets
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Bluetooth
    Qt6::Sensors
    KF6::I18n
    ${digitail_EXTRA_LIBS}
    )

# Translations
#add_custom_target(convert_translations_for_embedding
    #${CMAKE_CURRENT_SOURCE_DIR}/convert_translations.sh ${CMAKE_BINARY_DIR}
    #WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
#)
#add_dependencies(digitail convert_translations_for_embedding)

install(TARGETS digitail ${INSTALL_TARGETS_DEFAULT_ARGS})

ki18n_install(po)

# kirigami_package_breeze_icons(ICONS application-menu document-decrypt folder-sync go-next go-previous go-up handle-left handle-right view-list-icons applications-graphics media-record-symbolic)
