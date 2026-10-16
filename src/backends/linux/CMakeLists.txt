if(UNIX AND NOT APPLE AND NOT EMSCRIPTEN)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
    
    target_sources(cmn PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/cmn_linux_app.c
        ${CMAKE_CURRENT_SOURCE_DIR}/cmn_linux_widget.c
    )
    
    target_include_directories(cmn PRIVATE ${GTK3_INCLUDE_DIRS})
    target_link_libraries(cmn PRIVATE ${GTK3_LIBRARIES})
    target_link_directories(cmn PRIVATE ${GTK3_LIBRARY_DIRS})
    target_compile_options(cmn PRIVATE ${GTK3_CFLAGS_OTHER})
endif()
