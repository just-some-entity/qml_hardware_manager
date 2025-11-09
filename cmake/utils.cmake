# https://github.com/caelestia-dots/shell/blob/main/plugin/src/Caelestia/CMakeLists.txt
# Thanks ~~andrew~~ (Soramane)
function(qml_module arg_TARGET)
    cmake_parse_arguments(PARSE_ARGV 1 arg "" "URI" "SOURCES;LIBRARIES")

    qt_add_qml_module(${arg_TARGET}
            URI ${arg_URI}
            VERSION ${VERSION}
            SOURCES ${arg_SOURCES}
    )

    qt_query_qml_module(${arg_TARGET}
            URI module_uri
            VERSION module_version
            PLUGIN_TARGET module_plugin_target
            TARGET_PATH module_target_path
            QMLDIR module_qmldir
            TYPEINFO module_typeinfo
    )

    message(STATUS "Created QML module ${module_uri}, version ${module_version}")

    set(module_dir "${INSTALL_QMLDIR}/${module_target_path}")

    message(STATUS "Target ${arg_TARGET} will be installed in dir: ${module_dir}")

    install(TARGETS ${arg_TARGET} LIBRARY DESTINATION "${module_dir}" RUNTIME DESTINATION "${module_dir}")
    install(TARGETS "${module_plugin_target}" LIBRARY DESTINATION "${module_dir}" RUNTIME DESTINATION "${module_dir}")
    install(FILES "${module_qmldir}"   DESTINATION "${module_dir}")
    install(FILES "${module_typeinfo}" DESTINATION "${module_dir}")

    target_link_libraries(${arg_TARGET} PRIVATE Qt6::Core Qt6::Qml ${arg_LIBRARIES})
endfunction()