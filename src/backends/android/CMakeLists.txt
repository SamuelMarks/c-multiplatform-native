if(ANDROID)
    add_library(cmn_backend_android OBJECT
        ${CMAKE_CURRENT_SOURCE_DIR}/cmn_android_app.c
        ${CMAKE_CURRENT_SOURCE_DIR}/cmn_android_widget.c
        ${CMAKE_CURRENT_SOURCE_DIR}/cmn_android_a11y.c
        ${CMAKE_CURRENT_SOURCE_DIR}/cmn_android_automation.c
    )

    target_include_directories(cmn_backend_android PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../..>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    )

    target_sources(cmn PRIVATE $<TARGET_OBJECTS:cmn_backend_android>)

    target_link_libraries(cmn
        PRIVATE
            jnigraphics
            android
            log
    )
else()
    add_library(cmn_backend_android INTERFACE)
endif()
