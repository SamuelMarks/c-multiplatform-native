if(WIN32)
    target_sources(cmn PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/cmn_win32_app.c
        ${CMAKE_CURRENT_SOURCE_DIR}/cmn_win32_widget.c
    )
    
    target_link_libraries(cmn
        PRIVATE
            user32
            ole32
    )
endif()
