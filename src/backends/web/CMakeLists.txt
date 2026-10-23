if(EMSCRIPTEN)
    target_sources(cmn PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/cmn_web_app.c
        ${CMAKE_CURRENT_SOURCE_DIR}/cmn_web_widget.c
    )
    target_compile_options(cmn PRIVATE
        -fdollars-in-identifiers
        -Wno-gnu-statement-expression
        -Wno-variadic-macros
        -Wno-gnu-zero-variadic-macro-arguments
        -Wno-variadic-macro-arguments-omitted
        -Wno-dollar-in-identifier-extension
        -Wno-c99-extensions
        -Wno-c11-extensions
        -Wno-long-long
    )
endif()
