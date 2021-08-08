set(BUILD_SHARED_LIBS OFF)
option(DISABLE_DBUS "Build without D-Bus support" ON)
option(BUILD_TESTING "Build tests for ki18n" OFF)

add_subdirectory(kirigami)
add_subdirectory(ki18n)
