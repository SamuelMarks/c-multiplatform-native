set(CMN_SOURCES
    error.c
    memory.c
    string_safe.c
    signals.c
    form.c
    router.c
    a11y_i18n.c
    automation.c
    widget.c
    handle_map.c
    room.c
)

add_library(cmn ${CMN_SOURCES})

target_include_directories(cmn PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    $<INSTALL_INTERFACE:include>
)
target_include_directories(cmn PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link dependencies
add_subdirectory(backends)

target_link_libraries(cmn
    PUBLIC
        cfs
        c-abstract-http
        c-orm
        c_cdd
)

target_compile_definitions(cmn PRIVATE CMN_TESTING)
