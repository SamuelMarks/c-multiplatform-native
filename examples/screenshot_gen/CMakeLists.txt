cmake_minimum_required(VERSION 3.14)
project(screenshot_gen C)

set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror -Wno-deprecated-declarations)
endif()

if(NOT TARGET cmn)
    include(FetchContent)
    set(BUILD_TESTING OFF)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../include/cmn/app.h")
        add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../.." "${CMAKE_CURRENT_BINARY_DIR}/cmn" EXCLUDE_FROM_ALL)
    else()
        FetchContent_Declare(
            cmn
            GIT_REPOSITORY https://github.com/SamuelMarks/c-multiplatform-native.git
            GIT_TAG        master
        )
        FetchContent_MakeAvailable(cmn)
    endif()
    set(BUILD_TESTING ON)
endif()

add_library(screenshot_gen_lib STATIC
    src/screenshot_gen.c
)
target_include_directories(screenshot_gen_lib PUBLIC src)
target_link_libraries(screenshot_gen_lib PUBLIC cmn cfs)

add_executable(screenshot_gen
    src/main.c
)
target_link_libraries(screenshot_gen PRIVATE screenshot_gen_lib)

enable_testing()
add_executable(screenshot_gen_tests
    tests/test_main.c
)
target_compile_definitions(screenshot_gen_tests PRIVATE -DCMN_TESTING=1)
target_include_directories(screenshot_gen_tests PRIVATE ${greatest_SOURCE_DIR} src)
target_link_libraries(screenshot_gen_tests PRIVATE screenshot_gen_lib)
add_test(NAME screenshot_gen_tests COMMAND screenshot_gen_tests)
