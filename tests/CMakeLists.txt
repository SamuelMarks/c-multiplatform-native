add_executable(cmn_tests
    test_main.c
    test_nulls.c
    test_room.c
    test_error.c
    test_memory.c
    test_string_safe.c
    test_a11y_i18n.c
    test_automation.c
    test_app.c
    test_signals.c
    test_form.c
    test_router.c
    test_handle_map.c
    test_layout.c
)

target_link_libraries(cmn_tests PRIVATE cmn cfs c-abstract-http c-orm c_cdd)
target_include_directories(cmn_tests PRIVATE ${greatest_SOURCE_DIR})

add_test(NAME cmn_tests COMMAND cmn_tests)
