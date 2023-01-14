cmake_minimum_required(VERSION 3.20)

set(NativePrecompiledHeaders 1)
# Ensure that only 32 bit libraries are used.
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/Toolchain-cross-m32.cmake")

#
# Config stuff
#

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type")
endif()
set(CMAKE_BUILD_TYPE_VALUES "Debug;Release" CACHE INTERNAL "List of supported build")
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS ${CMAKE_BUILD_TYPE_VALUES})
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()
set(Force_CXX_23B 0 CACHE BOOL "Force 23B as the CXX Standard, useful for disrepancies in compiler and IDE")

project(cathook VERSION 0.0.1)
if (NOT Force_CXX_23B)
    set(CMAKE_CXX_STANDARD 23)
endif()
add_library(cathook SHARED "")

set(GameSpecific 1 CACHE BOOL "Build for specific target game (As opposed to universal, but slower, lib)")
set(Game "tf2" CACHE STRING "Target game")
set(GameValues "tf2;hl2dm;dab;tf2c;css;dynamic" CACHE INTERNAL "List of supported game types")
set_property(CACHE Game PROPERTY STRINGS ${GameValues})

set(Visuals_DrawType "IMGUI" CACHE STRING "Target drawing api")
set(DrawTypes "Textmode;No Visuals;Glez;Engine;IMGUI;IMGUI Streamproof" CACHE INTERNAL "Select the drawing api that cathook should use")
set_property(CACHE Visuals_DrawType PROPERTY STRINGS ${DrawTypes})
set(Visuals_EnforceStreamSafety 1 CACHE BOOL "If a streamsafe drawing method is used, this will remove features that are not streamsafe.")

set(EnableVisuals 1)
set(EnableGlezDrawing 0)
set(EnableEngineDrawing 0)
set(EnableImGuiDrawing 0)
set(ExternalDrawing 0)
set(Textmode 0)

if(Visuals_DrawType STREQUAL "Textmode")
    set(EnableVisuals 0)
    set(Textmode 1)
elseif(Visuals_DrawType STREQUAL "No Visuals")
    set(EnableVisuals 0)
elseif(Visuals_DrawType STREQUAL "Glez")
    set(EnableGlezDrawing 1)
elseif(Visuals_DrawType STREQUAL "Engine")
    set(EnableEngineDrawing 1)
elseif(Visuals_DrawType STREQUAL "IMGUI")
    set(EnableImGuiDrawing 1)
elseif(Visuals_DrawType STREQUAL "IMGUI Streamproof")
    set(EnableImGuiDrawing 1)
    set(ExternalDrawing 1)
endif()
if (NOT ExternalDrawing)
    set(Visuals_EnforceStreamSafety 0)
endif()

set(EnableProfiler 1 CACHE BOOL "Enable Profiler")
set(EnableGUI 1 CACHE BOOL "Enable GUI")
set(EnableIPC 1 CACHE BOOL "Enable IPC")
set(DataPath "/opt/cathook/data" CACHE FILEPATH "Data location")
set(VACBypass 0 CACHE BOOL "Textmode VAC bypass")
set(EnableTextmodeStdin 0 CACHE BOOL "Textmode Stdin -> Console bridge (EXPERIMENTAL)")
set(EnableWarnings 0 CACHE BOOL "Enable compile warnings")
set(EnableNullNexus 1 CACHE BOOL "Enable NullNexus chat and other features")
set(EnableLogging 1 CACHE BOOL "Enable logging to /tmp/")
set(EnableClip 1 CACHE BOOL "Enable anti electron/chromium freezing")
set(Unity_ZeroKernel 1 CACHE BOOL "Use a unity build for zerokernel files")
if(NOT EnableVisuals)
    set(EnableGUI 0)
endif()

# Internal variables
# INTERNAL implies "FORCE" by default, so don't set if already set
if (NOT DEFINED Internal_Binarymode)
    set(Internal_Binarymode 0 CACHE INTERNAL "Build cathook to be used in 'binary mode'")
endif()
if (NOT DEFINED Internal_Symbolized)
    set(Internal_Symbolized 0 CACHE INTERNAL "Build cathook with symbols so crashes can be sent")
endif()

#
# End of config stuff
#

if (EnableVisuals)
    set(OpenGL_GL_PREFERENCE "LEGACY")
    if (EnableImGuiDrawing)
        find_package(Freetype REQUIRED)
        target_include_directories(cathook PRIVATE "${FREETYPE_INCLUDE_DIRS}")
        target_link_libraries(cathook ${FREETYPE_LIBRARIES})
    endif()
endif()

find_package(Git REQUIRED)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/external")
find_package(SDL2 REQUIRED)
find_package(Boost 1.76.5 EXACT)
if (NOT ${Boost_FOUND})
    set(BOOST_LIBS_OPTIONAL "" CACHE STRING "Boost libs to be compiled")
    add_subdirectory(external/boost-cmake)
    SET (BOOST_ROOT ${CMAKE_BINARY_DIR}/_deps/boost-src)
    find_package(Boost 1.76.5 EXACT)
    include_directories(${Boost_INCLUDE_DIRS})
endif()

execute_process(COMMAND
        "${GIT_EXECUTABLE}" log -1 --pretty=\"%h\"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        OUTPUT_VARIABLE GIT_COMMIT_HASH
        ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

execute_process(COMMAND
        "${GIT_EXECUTABLE}" log -1 --pretty=\"%ai\"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        OUTPUT_VARIABLE GIT_COMMITTER_DATE
        ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

set( CMAKE_SKIP_RPATH true )
find_library(ValveLibraryVStdLib NAMES vstdlib PATHS "${PROJECT_SOURCE_DIR}/lib" NO_DEFAULT_PATH)

if(EnableIPC)
    add_subdirectory(external/simple-ipc)
    target_link_libraries(cathook SimpleIPC)
endif()

if(EnableNullNexus)
    add_subdirectory(external/libnullnexus)
    target_link_libraries(cathook libnullnexus)
endif()

if(EnableVisuals)
    if (EnableGlezDrawing)
        add_subdirectory(external/libglez)
    endif()
    target_include_directories(cathook PRIVATE include/visual)
    find_package(SDL2 REQUIRED)
    if(ExternalDrawing)
        add_subdirectory(external/libxoverlay)
        target_link_libraries(cathook xoverlay)
        target_include_directories(cathook PRIVATE external/libxoverlay/include)
    endif()
    target_include_directories(cathook PRIVATE "${SDL2_INCLUDE_DIR}")
    if (EnableGlezDrawing)
        target_link_libraries(cathook glez)
    endif()
    if (EnableGlezDrawing)
        find_package(GLEW REQUIRED)
        target_link_libraries(cathook ${GLEW_LIBRARIES})
    endif()
endif()

configure_file(include/config.h.in ${CMAKE_SOURCE_DIR}/include/config.h @ONLY)
configure_file(include/version.h.in ${CMAKE_SOURCE_DIR}/include/version.h @ONLY)

set(CMAKE_CXX_FLAGS "-m32 -march=native -fexceptions -DNDEBUG")
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-gnu-unique")
endif()
set(CMAKE_CXX_FLAGS_DEBUG "-march=native -rdynamic -ggdb -Og")
if (Internal_Symbolized)
    set(CMAKE_CXX_FLAGS_RELEASE "-Ofast -ggdb -fvisibility=hidden -fvisibility-inlines-hidden")
else()
    set(CMAKE_CXX_FLAGS_RELEASE "-Ofast -s -fmerge-all-constants -fdevirtualize-at-ltrans -fvisibility=hidden -fvisibility-inlines-hidden")
endif()
if (Force_CXX_23B)
    set(CMAKE_CXX_FLAGS "-std=gnu++2b ${CMAKE_CXX_FLAGS}")
endif()


target_compile_definitions(cathook PRIVATE
        _GLIBCXX_USE_CXX11_ABI=0
        _POSIX=1
        FREETYPE_GL_USE_VAO=1
        RAD_TELEMETRY_DISABLED=1
        LINUX=1
        USE_SDL=1
        _LINUX=1
        POSIX=1
        GNUC=1
        NO_MALLOC_OVERRIDE=1)

target_include_directories(cathook SYSTEM PRIVATE
        "external/source-sdk-2013-headers/mp/src/mathlib"
        "external/source-sdk-2013-headers/mp/src/public"
        "external/source-sdk-2013-headers/mp/src/common"
        "external/source-sdk-2013-headers/mp/src/public/tier0"
        "external/source-sdk-2013-headers/mp/src/public/tier1"
        "external/source-sdk-2013-headers/mp/src/public/steam"
        "external/source-sdk-2013-headers/mp/src/tier1"
        "external/source-sdk-2013-headers/mp/src")

if(EnableWarnings)
    target_compile_options(cathook PRIVATE -Wall -Wextra -Wno-unused-parameter -Wno-class-memaccess -Wno-strict-aliasing -Wno-sign-compare)
else()
    target_compile_options(cathook PRIVATE -w)
endif()
if(Internal_Binarymode)
    target_compile_options(cathook PRIVATE -march=x86-64)
endif()


target_include_directories(cathook PRIVATE include include/hooks)

add_custom_target(data
        bash ./install-data "${DataPath}"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        VERBATIM)

# SDL included because of https://cathook.club/issues/689
target_link_libraries(cathook ${ValveLibraryVStdLib} ${SDL2_LIBRARY})
set_target_properties(cathook PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(src)
add_subdirectory(include)
add_subdirectory(external)
add_subdirectory(modules)


target_precompile_headers(cathook PRIVATE "${CMAKE_SOURCE_DIR}/include/common.hpp")

if (NOT Internal_Symbolized)
    add_custom_command(TARGET cathook POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:cathook> "${CMAKE_SOURCE_DIR}/bin/$<TARGET_FILE_NAME:cathook>")
else()
    message("Symbolized build")
    set(Internal_Symbolized 0 CACHE INTERNAL "Build cathook with symbols so crashes can be sent" FORCE)
endif()
