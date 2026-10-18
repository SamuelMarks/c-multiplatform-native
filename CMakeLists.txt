cmake_minimum_required(VERSION 3.14)
project(cmn C CXX)

set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Build Variants & Toggles (CMake Options)
option(${PROJECT_NAME}_CRT_LINKAGE "CRT Linkage: Static (/MT, /MTd) vs Shared (/MD, /MDd)" "Shared")
option(${PROJECT_NAME}_CHARSET "Charset: UNICODE vs ANSI" "UNICODE")
option(${PROJECT_NAME}_THREADING "Threading: Multi-threaded vs Single-threaded" "Multi-threaded")
option(${PROJECT_NAME}_LTO "Enable Link-Time Optimization" OFF)
option(BUILD_SHARED_LIBS "Target type (Static vs Shared libraries)" OFF)
option(CMN_ENABLE_COVERAGE "Enable coverage reporting (GCC/Clang)" OFF)
option(CMN_ENABLE_ASAN "Enable AddressSanitizer (GCC/Clang)" OFF)
option(CMN_ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer (GCC/Clang)" OFF)
set(${PROJECT_NAME}_MSVC_RTC "/RTC1" CACHE STRING "MSVC Runtime Checks (/RTC1, /RTCs, /RTCu)")

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

if(${PROJECT_NAME}_LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT ipo_supported OUTPUT error)
    if(ipo_supported)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
endif()

if(MSVC)
    if(${PROJECT_NAME}_CRT_LINKAGE STREQUAL "Static")
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    else()
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    endif()

    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${${PROJECT_NAME}_MSVC_RTC}")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${${PROJECT_NAME}_MSVC_RTC}")

    if(${PROJECT_NAME}_CHARSET STREQUAL "UNICODE")
        add_compile_definitions(UNICODE _UNICODE)
    endif()

    if(${PROJECT_NAME}_THREADING STREQUAL "Multi-threaded")
        # Multi-threaded is default in MSVC when using CRT
    endif()
    
    # Highest strictness for MSVC
    string(REPLACE "/W3" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    string(REPLACE "/W3" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    add_compile_options(/W4 /WX /wd4456 /wd4996)
else()
    # Highest strictness for GCC/Clang
    add_compile_options(-Wall -Wextra -Wpedantic -Werror -Wno-deprecated-declarations -Wno-unused-result)
    if(CMN_ENABLE_ASAN)
        add_compile_options(-fsanitize=address -fno-omit-frame-pointer -g)
        add_link_options(-fsanitize=address)
    endif()
    if(CMN_ENABLE_UBSAN)
        add_compile_options(-fsanitize=undefined -fno-omit-frame-pointer -g)
        add_link_options(-fsanitize=undefined)
    endif()
endif()

include(FetchContent)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON CACHE BOOL "Do not update FetchContent dependencies on every run")

macro(use_local_repo_if_exists NAME DIR)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../${DIR}")
        string(TOUPPER "${NAME}" UPPER_NAME)
        set(FETCHCONTENT_SOURCE_DIR_${UPPER_NAME} "${CMAKE_CURRENT_SOURCE_DIR}/../${DIR}" CACHE PATH "Local ${NAME}" FORCE)
    endif()
endmacro()

use_local_repo_if_exists(c-ci c-ci)
use_local_repo_if_exists(cfs c-fs)
use_local_repo_if_exists(c_str_span c-str-span)
use_local_repo_if_exists(c-abstract-http c-abstract-http)
use_local_repo_if_exists(c-orm c-orm)
use_local_repo_if_exists(cdd_c cdd-c)
use_local_repo_if_exists(c89stringutils c89stringutils)
use_local_repo_if_exists(parson parson)
use_local_repo_if_exists(greatest greatest)

# SamuelMarks Dependencies
FetchContent_Declare(
    c-ci
    GIT_REPOSITORY https://github.com/SamuelMarks/c-ci.git
    GIT_TAG        master
)

FetchContent_Declare(
    cfs
    GIT_REPOSITORY https://github.com/SamuelMarks/c-fs.git
    GIT_TAG        master
)

FetchContent_Declare(
    c-abstract-http
    GIT_REPOSITORY https://github.com/SamuelMarks/c-abstract-http.git
    GIT_TAG        master
)

FetchContent_Declare(
    c-orm
    GIT_REPOSITORY https://github.com/SamuelMarks/c-orm.git
    GIT_TAG        master
)

FetchContent_Declare(
    cdd_c
    GIT_REPOSITORY https://github.com/SamuelMarks/cdd-c.git
    GIT_TAG        master
)

set(TEMP_BUILD_TESTING ${BUILD_TESTING})
set(BUILD_TESTING OFF CACHE BOOL "Disable testing for dependencies" FORCE)
set(C_ORM_BUILD_EXAMPLES OFF CACHE BOOL "Disable examples for c-orm" FORCE)

FetchContent_MakeAvailable(c-ci)
FetchContent_MakeAvailable(cfs)
FetchContent_MakeAvailable(c-abstract-http)
FetchContent_MakeAvailable(cdd_c)
FetchContent_MakeAvailable(c-orm)

if(NOT MSVC)
    if(TARGET sqlite3)
        target_compile_options(sqlite3 PRIVATE -w)
    endif()
    if(CMN_ENABLE_COVERAGE)
        add_compile_options(--coverage -O0 -g)
        add_link_options(--coverage)
    endif()
    foreach(flag_tgt c_cdd_compiler_flags cdd_c_compiler_flags c_str_span_compiler_flags c89stringutils_compiler_flags)
        if(TARGET ${flag_tgt})
            get_target_property(iface_opts ${flag_tgt} INTERFACE_COMPILE_OPTIONS)
            if(iface_opts)
                list(REMOVE_ITEM iface_opts "-Wcast-qual")
                set_target_properties(${flag_tgt} PROPERTIES INTERFACE_COMPILE_OPTIONS "${iface_opts}")
            endif()
        endif()
    endforeach()
endif()

set(BUILD_TESTING ${TEMP_BUILD_TESTING} CACHE BOOL "Restore testing for main project" FORCE)

include(cmake/cmn_packaging.cmake)

add_subdirectory(src)

enable_testing()
add_subdirectory(tests)

# Only add examples if we are the top-level project
if(CMAKE_PROJECT_NAME STREQUAL "cmn")
    add_subdirectory(examples/simple_password_flow)
    add_subdirectory(examples/room_example)
    add_subdirectory(examples/hello_world)
    add_subdirectory(examples/screenshot_gen)
endif()

# Bundle helper macro
macro(cmn_create_bundle TARGET_NAME)
    if(APPLE AND NOT IOS)
        set_target_properties(${TARGET_NAME} PROPERTIES
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in"
        )
        if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in")
            file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in" "<?xml version=\"1.0\" encoding=\"UTF-8\"?>
<!DOCTYPE plist PUBLIC \"-//Apple Computer//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">
<plist version=\"1.0\">
<dict>
    <key>CFBundleDevelopmentRegion</key>
    <string>English</string>
    <key>CFBundleExecutable</key>
    <string>\${MACOSX_BUNDLE_EXECUTABLE_NAME}</string>
    <key>CFBundleIconFile</key>
    <string>\${MACOSX_BUNDLE_ICON_FILE}</string>
    <key>CFBundleIdentifier</key>
    <string>com.example.\${MACOSX_BUNDLE_GUI_IDENTIFIER}</string>
    <key>CFBundleInfoDictionaryVersion</key>
    <string>6.0</string>
    <key>CFBundleName</key>
    <string>\${MACOSX_BUNDLE_BUNDLE_NAME}</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
    <key>CSResourcesFileMapped</key>
    <true/>
    <key>NSHighResolutionCapable</key>
    <true/>
</dict>
</plist>")
        endif()
    elseif(WIN32)
        # Windows GUI subsystem
        set_target_properties(${TARGET_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
    endif()
endmacro()
