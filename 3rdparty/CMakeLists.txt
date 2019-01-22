set(BUILD_SHARED_LIBS 0)

add_subdirectory(kirigami)

include(ExternalProject)

get_target_property (QT_QMAKE_EXECUTABLE Qt5::qmake IMPORTED_LOCATION)
ExternalProject_Add(qtremoteobjects
                    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/qtremoteobjects/
                    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/3rdparty/qtremoteobjects
                    BUILD_IN_SOURCE 1
                    CONFIGURE_COMMAND ${QT_QMAKE_EXECUTABLE} qtremoteobjects.pro QtRemoteObjects_STATIC=yes PREFIX=${CMAKE_CURRENT_BINARY_DIR}/3rdparty/qtremoteobjects
                    BUILD_COMMAND make
                    INSTALL_COMMAND cmake -E echo "Skipping install step."
                   )
add_library("Qt5RemoteObjects" STATIC IMPORTED)
set_target_properties(Qt5RemoteObjects PROPERTIES
  IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/qtremoteobjects/lib/libQt5RemoteObjects.la")
set(Qt5RemoteObjects_INCLUDEDIR "${CMAKE_CURRENT_SOURCE_DIR}/qtremoteobjects/include/")
