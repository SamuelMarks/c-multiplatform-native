cmake_minimum_required(VERSION 3.14)
project(hello_world C)

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

add_library(hello_world_lib STATIC
    src/app.c
)
target_include_directories(hello_world_lib PUBLIC src)
target_link_libraries(hello_world_lib PUBLIC cmn)

add_executable(hello_world
    src/main.c
)
target_link_libraries(hello_world PRIVATE hello_world_lib)

cmn_package_macos_app(hello_world "")
cmn_package_windows_exe(hello_world)
cmn_package_linux_app(hello_world)
cmn_package_android_apk(hello_world)
cmn_package_ios_ipa(hello_world)
cmn_package_web_bundle(hello_world)

enable_testing()
add_executable(hello_world_tests
    tests/test_main.c
)
target_compile_definitions(hello_world_tests PRIVATE -DCMN_TESTING=1)
target_include_directories(hello_world_tests PRIVATE ${greatest_SOURCE_DIR} src)
target_link_libraries(hello_world_tests PRIVATE hello_world_lib)
add_test(NAME hello_world_tests COMMAND hello_world_tests)
