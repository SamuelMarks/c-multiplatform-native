if(APPLE AND IOS)
    add_library(cmn_backend_ios OBJECT
        ${CMAKE_CURRENT_SOURCE_DIR}/cmn_ios_app.m
        ${CMAKE_CURRENT_SOURCE_DIR}/cmn_ios_widget.m
        ${CMAKE_CURRENT_SOURCE_DIR}/cmn_ios_a11y.m
        ${CMAKE_CURRENT_SOURCE_DIR}/cmn_ios_automation.m
    )

    set_target_properties(cmn_backend_ios PROPERTIES
        C_STANDARD 99
    )

    set_source_files_properties(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmn_ios_app.m
        ${CMAKE_CURRENT_SOURCE_DIR}/cmn_ios_widget.m
        ${CMAKE_CURRENT_SOURCE_DIR}/cmn_ios_a11y.m
        ${CMAKE_CURRENT_SOURCE_DIR}/cmn_ios_automation.m
        PROPERTIES
        COMPILE_FLAGS "-fobjc-arc -Wno-pedantic"
    )

    target_include_directories(cmn_backend_ios PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../..>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    )

    target_sources(cmn PRIVATE $<TARGET_OBJECTS:cmn_backend_ios>)

    target_link_libraries(cmn
        PRIVATE
            "-framework UIKit"
    )
else()
    add_library(cmn_backend_ios INTERFACE)
endif()
