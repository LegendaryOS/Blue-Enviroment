cmake_minimum_required(VERSION 3.16)
project(blue-panel LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Core WebEngineWidgets REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP jsoncpp REQUIRED)
pkg_check_modules(DBUS dbus-1 REQUIRED)
pkg_check_modules(WAYLAND wayland-client REQUIRED)

add_executable(blue-panel
    backend/main.cpp
    backend/system_status.cpp
    backend/app_parser.cpp
    backend/config_manager.cpp
    backend/webview_bridge.cpp
    backend/logger.cpp
    backend/utils.cpp
    backend/dbus_manager.cpp
    backend/wayland_layer_shell.cpp
)

target_include_directories(blue-panel PRIVATE
    ${JSONCPP_INCLUDE_DIRS}
    ${DBUS_INCLUDE_DIRS}
    ${WAYLAND_INCLUDE_DIRS}
)

target_link_libraries(blue-panel PRIVATE
    Qt5::Core
    Qt5::WebEngineWidgets
    ${JSONCPP_LIBRARIES}
    ${DBUS_LIBRARIES}
    ${WAYLAND_LIBRARIES}
)

install(TARGETS blue-panel DESTINATION bin)
