cmake_minimum_required(VERSION 3.14)
project(simple_password_flow C)

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

add_library(simple_password_flow_lib STATIC
    src/app_state.c
    src/login_view.c
    src/welcome_view.c
)
target_include_directories(simple_password_flow_lib PUBLIC src)
target_link_libraries(simple_password_flow_lib PUBLIC cmn)

add_executable(simple_password_flow
    src/main.c
)
target_link_libraries(simple_password_flow PRIVATE simple_password_flow_lib)

cmn_package_macos_app(simple_password_flow "")
cmn_package_windows_exe(simple_password_flow)
cmn_package_linux_app(simple_password_flow)
cmn_package_android_apk(simple_password_flow)
cmn_package_ios_ipa(simple_password_flow)
cmn_package_web_bundle(simple_password_flow)

enable_testing()
add_executable(simple_password_flow_tests
    tests/test_main.c
    src/main.c
)
target_compile_definitions(simple_password_flow_tests PRIVATE -DCMN_TESTING=1)
target_include_directories(simple_password_flow_tests PRIVATE ${greatest_SOURCE_DIR} src)
target_link_libraries(simple_password_flow_tests PRIVATE simple_password_flow_lib)
add_test(NAME simple_password_flow_tests COMMAND simple_password_flow_tests)
