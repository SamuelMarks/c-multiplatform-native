if(APPLE AND NOT IOS)
    add_library(cmn_backend_macos OBJECT
        ${CMAKE_CURRENT_SOURCE_DIR}/cmn_macos_app.m
        ${CMAKE_CURRENT_SOURCE_DIR}/cmn_macos_widget.m
        ${CMAKE_CURRENT_SOURCE_DIR}/cmn_macos_a11y.m
    )

    set_target_properties(cmn_backend_macos PROPERTIES
        C_STANDARD 99
    )

    set_source_files_properties(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmn_macos_app.m
        ${CMAKE_CURRENT_SOURCE_DIR}/cmn_macos_widget.m
        ${CMAKE_CURRENT_SOURCE_DIR}/cmn_macos_a11y.m
        PROPERTIES
        COMPILE_FLAGS "-fobjc-arc -Wno-pedantic"
    )

    target_include_directories(cmn_backend_macos PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../..>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${cfs_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${c-abstract-http_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${c-orm_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${c_cdd_SOURCE_DIR}/src>
    )

    target_compile_definitions(cmn_backend_macos PRIVATE CMN_TESTING)

    target_sources(cmn PRIVATE $<TARGET_OBJECTS:cmn_backend_macos>)

    target_link_libraries(cmn
        PRIVATE
            "-framework Cocoa"
    )
endif()
