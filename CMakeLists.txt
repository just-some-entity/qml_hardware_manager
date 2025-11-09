cmake_minimum_required(VERSION 4.0)
project(qml_hardware_controls)

set(VERSION 1.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(QT_QML_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/qml")
set(INSTALL_QMLDIR "/usr/lib/qt6/qml")

find_package(Qt6 REQUIRED COMPONENTS Core Quick Qml Gui)

include(${CMAKE_SOURCE_DIR}/cmake/utils.cmake)

qt_standard_project_setup(REQUIRES 6.6)

add_subdirectory("src")