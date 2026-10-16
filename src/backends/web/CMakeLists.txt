if(EMSCRIPTEN)
    target_sources(cmn PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/cmn_web_app.c
        ${CMAKE_CURRENT_SOURCE_DIR}/cmn_web_widget.c
    )
endif()
