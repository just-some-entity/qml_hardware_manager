qml_module(qml_hardware_controls
        URI
        HardwareControls
        SOURCES
        brightness.cpp

        LIBRARIES
        Qt6::Core
        Qt6::Quick
)

qt_add_executable(ExampleEngine
        main.cpp
        brightness.cpp
)

target_link_libraries(ExampleEngine PRIVATE Qt6::Core Qt6::Quick Qt6::Gui Qt6::Qml)