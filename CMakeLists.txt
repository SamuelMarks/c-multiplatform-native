cmake_minimum_required(VERSION 3.14)

if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()
if(POLICY CMP0077)
    cmake_policy(SET CMP0077 NEW)
endif()
if(POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW)
endif()
if(POLICY CMP0069)
    cmake_policy(SET CMP0069 NEW)
endif()

project(cmn C CXX)

set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED ON)
if(EMSCRIPTEN)
    set(CMAKE_C_EXTENSIONS ON)
else()
    set(CMAKE_C_EXTENSIONS OFF)
endif()

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

if(WIN32 AND BUILD_SHARED_LIBS)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

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

if(CMAKE_INTERPROCEDURAL_OPTIMIZATION)
    set(C_ORM_LTO ON CACHE BOOL "Enable LTO for c-orm" FORCE)
    if(NOT MSVC)
        if(CMAKE_C_COMPILER_AR)
            set(CMAKE_AR "${CMAKE_C_COMPILER_AR}")
        endif()
        if(CMAKE_C_COMPILER_RANLIB)
            set(CMAKE_RANLIB "${CMAKE_C_COMPILER_RANLIB}")
        endif()
    endif()
endif()

if(MSVC)
    if(NOT DEFINED CMAKE_MSVC_RUNTIME_LIBRARY OR CMAKE_MSVC_RUNTIME_LIBRARY STREQUAL "")
        if(${PROJECT_NAME}_CRT_LINKAGE STREQUAL "Static" OR ${PROJECT_NAME}_CRT_LINKAGE STREQUAL "static")
            set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
        else()
            set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
        endif()
    endif()
    set(_CMN_SAVED_MSVC_RUNTIME_LIBRARY "${CMAKE_MSVC_RUNTIME_LIBRARY}")
    if(CMAKE_MSVC_RUNTIME_LIBRARY AND NOT CMAKE_MSVC_RUNTIME_LIBRARY MATCHES "DLL")
        set(C_ORM_CRT_LINKAGE "static" CACHE STRING "CRT Linkage for c-orm" FORCE)
        set(C_CDD_STATIC_CRT ON CACHE BOOL "Static CRT for cdd" FORCE)
    else()
        set(C_ORM_CRT_LINKAGE "dynamic" CACHE STRING "CRT Linkage for c-orm" FORCE)
        set(C_CDD_STATIC_CRT OFF CACHE BOOL "Static CRT for cdd" FORCE)
    endif()

    if(DEFINED CDD_MSVC_RTC AND NOT CDD_MSVC_RTC STREQUAL "" AND NOT CDD_MSVC_RTC STREQUAL "OFF")
        if(CDD_MSVC_RTC MATCHES "^/")
            set(${PROJECT_NAME}_MSVC_RTC "${CDD_MSVC_RTC}")
        else()
            set(${PROJECT_NAME}_MSVC_RTC "/${CDD_MSVC_RTC}")
        endif()
    endif()

    if(NOT ${PROJECT_NAME}_MSVC_RTC STREQUAL "" AND NOT ${PROJECT_NAME}_MSVC_RTC STREQUAL "OFF")
        string(REGEX REPLACE "/RTC(1|c|s|u)+" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
        string(REGEX REPLACE "/RTC(1|c|s|u)+" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
        string(REGEX REPLACE "/RTC(1|c|s|u)+" "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
        string(REGEX REPLACE "/RTC(1|c|s|u)+" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${${PROJECT_NAME}_MSVC_RTC}")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${${PROJECT_NAME}_MSVC_RTC}")
    endif()

    if(DEFINED CDD_CHARSET AND NOT CDD_CHARSET STREQUAL "")
        string(TOUPPER "${CDD_CHARSET}" _CMN_CHARSET_UPPER)
    else()
        string(TOUPPER "${${PROJECT_NAME}_CHARSET}" _CMN_CHARSET_UPPER)
    endif()
    if(_CMN_CHARSET_UPPER STREQUAL "UNICODE")
        add_compile_definitions(UNICODE _UNICODE)
    endif()

    if(${PROJECT_NAME}_THREADING STREQUAL "Multi-threaded")
        # Multi-threaded is default in MSVC when using CRT
    endif()
    
    # Highest strictness for MSVC
    string(REPLACE "/W3" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    string(REPLACE "/W3" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
else()
    add_compile_options(-Wno-deprecated-declarations -Wno-unused-result)
    if(EMSCRIPTEN)
        set(CMAKE_C_EXTENSIONS ON)
        add_compile_options(-pthread -Wno-gnu-statement-expression -Wno-variadic-macros -Wno-dollar-in-identifier-extension)
        add_link_options(-pthread "SHELL:-s USE_PTHREADS=1" "SHELL:-s STACK_SIZE=5MB" "SHELL:-s EMULATE_FUNCTION_POINTER_CASTS=1" "SHELL:-s EXIT_RUNTIME=1" "SHELL:-s PTHREAD_POOL_SIZE=4")
    endif()
    if(CMN_ENABLE_ASAN)
        add_compile_options(-fsanitize=address -fno-omit-frame-pointer -g)
        add_link_options(-fsanitize=address)
    endif()
    if(CMN_ENABLE_UBSAN)
        add_compile_options(-fsanitize=undefined -fno-omit-frame-pointer -g)
        add_link_options(-fsanitize=undefined)
    endif()
endif()

set(FETCHCONTENT_UPDATES_DISCONNECTED ON CACHE BOOL "Do not update FetchContent dependencies on every run" FORCE)
include(FetchContent)

if(DEFINED FETCHCONTENT_SOURCE_DIR_C_FS AND NOT DEFINED FETCHCONTENT_SOURCE_DIR_CFS)
    set(FETCHCONTENT_SOURCE_DIR_CFS "${FETCHCONTENT_SOURCE_DIR_C_FS}" CACHE PATH "Local cfs" FORCE)
endif()
if(DEFINED FETCHCONTENT_SOURCE_DIR_C_ABSTRACT_HTTP AND NOT DEFINED FETCHCONTENT_SOURCE_DIR_C-ABSTRACT-HTTP)
    set(FETCHCONTENT_SOURCE_DIR_C-ABSTRACT-HTTP "${FETCHCONTENT_SOURCE_DIR_C_ABSTRACT_HTTP}" CACHE PATH "Local c-abstract-http" FORCE)
endif()
if(DEFINED FETCHCONTENT_SOURCE_DIR_C_ORM AND NOT DEFINED FETCHCONTENT_SOURCE_DIR_C-ORM)
    set(FETCHCONTENT_SOURCE_DIR_C-ORM "${FETCHCONTENT_SOURCE_DIR_C_ORM}" CACHE PATH "Local c-orm" FORCE)
endif()

macro(use_local_repo_if_exists NAME DIR)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../${DIR}/CMakeLists.txt")
        file(READ "${CMAKE_CURRENT_SOURCE_DIR}/../${DIR}/CMakeLists.txt" _LOCAL_CMAKE_CONTENT)
        if(NOT "${NAME}" STREQUAL "cdd_c" OR _LOCAL_CMAKE_CONTENT MATCHES "c_cdd")
            string(TOUPPER "${NAME}" UPPER_NAME)
            set(FETCHCONTENT_SOURCE_DIR_${UPPER_NAME} "${CMAKE_CURRENT_SOURCE_DIR}/../${DIR}" CACHE PATH "Local ${NAME}" FORCE)
        else()
            string(TOUPPER "${NAME}" UPPER_NAME)
            unset(FETCHCONTENT_SOURCE_DIR_${UPPER_NAME} CACHE)
        endif()
    elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../${DIR}/.git")
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

set(_CMN_SAVED_IPO "${CMAKE_INTERPROCEDURAL_OPTIMIZATION}")
FetchContent_MakeAvailable(c-ci)
FetchContent_MakeAvailable(cfs)
FetchContent_MakeAvailable(c-abstract-http)
FetchContent_MakeAvailable(cdd_c)
if(TARGET c_cdd)
    target_compile_definitions(c_cdd PRIVATE CDD_BUILD_TESTS=1)
    if(MSVC)
        target_compile_options(c_cdd PRIVATE /wd4005)
    endif()
endif()
FetchContent_MakeAvailable(c-orm)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION "${_CMN_SAVED_IPO}")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

foreach(_dep_tgt cfs c_str_span c89stringutils parson sqlite3 libcurl c-abstract-http c_cdd c-orm c-orm-async c_orm_standalone_fixtures c_orm_legacy_cdd_db)
    if(TARGET ${_dep_tgt})
        set_target_properties(${_dep_tgt} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
            LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
            ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        )
        if(MSVC AND DEFINED _CMN_SAVED_MSVC_RUNTIME_LIBRARY AND NOT _CMN_SAVED_MSVC_RUNTIME_LIBRARY STREQUAL "")
            set_target_properties(${_dep_tgt} PROPERTIES
                MSVC_RUNTIME_LIBRARY "${_CMN_SAVED_MSVC_RUNTIME_LIBRARY}"
            )
        endif()
    endif()
endforeach()
if(MSVC AND DEFINED _CMN_SAVED_MSVC_RUNTIME_LIBRARY AND NOT _CMN_SAVED_MSVC_RUNTIME_LIBRARY STREQUAL "")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "${_CMN_SAVED_MSVC_RUNTIME_LIBRARY}")
endif()

if(MSVC)
    add_compile_options(/W4 /WX /wd4456 /wd4996)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror -Wno-deprecated-declarations -Wno-unused-result)
    if(EMSCRIPTEN)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-gnu-statement-expression -Wno-variadic-macros -Wno-gnu-zero-variadic-macro-arguments -Wno-dollar-in-identifier-extension -Wno-c99-extensions -Wno-c11-extensions -Wno-long-long")
    endif()
endif()

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
                if(EMSCRIPTEN)
                    list(REMOVE_ITEM iface_opts "-pedantic" "-Wpedantic")
                endif()
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
