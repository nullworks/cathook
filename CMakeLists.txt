
#
#
# Welcome to the cmake lists file, this controls how nekohook will build for your system
#
# TODO, make wasy way to do "touch ../src/CMakeLists.txt" to make things easy
#
#

cmake_minimum_required (VERSION 2.6)
project (libnekohook)

## Defaults
# User options can be set with cmake -D"NAME OF OPTION HERE"="VARIABLE", e.g. "cmake -DCGAME=csgo"
set(CGAME tf2 CACHE STRING "Game to build with")
set(CGFX valve_surface CACHE STRING "Graphics module to use")
set(CINP xlib CACHE STRING "Input module to use")
set(CUSERNAME UNREGISTERED CACHE STRING "Register a username")
set(CUSERID 0 CACHE STRING "Register a id")
set(CForceClang false CACHE STRING "Should cmake force compilation with cmake?")
if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, default to Release")
    set(CMAKE_BUILD_TYPE RELEASE)
endif(NOT CMAKE_BUILD_TYPE)

# Function to remove source files from a directory, ripped from aimtux fuzion <3
FUNCTION(EXCLUDE_FILES_FROM_DIR_IN_LIST _InFileList _excludeDirName)
  foreach(ITR ${_InFileList})
    if("${ITR}" MATCHES "(.*)${_excludeDirName}(.*)") # Check if the item matches the directory name in _excludeDirName
      if(CMAKE_BUILD_TYPE MATCHES DEBUG)
        message(STATUS "Remove Item from List:${ITR}")
      endif()
      list(REMOVE_ITEM _InFileList ${ITR}) # Remove the item from the list
    endif()
  endforeach()
  set(SOURCE_FILES ${_InFileList} PARENT_SCOPE) # Return the SOURCE_FILES variable to the calling parent
ENDFUNCTION()

# Add cathooks source files except the modules, we add those selectivly
file(GLOB_RECURSE SOURCE_FILES "${CMAKE_SOURCE_DIR}/src/*.c*")
EXCLUDE_FILES_FROM_DIR_IN_LIST("${SOURCE_FILES}" "${CMAKE_SOURCE_DIR}/src/modules/")

# Flags we use for every build type, cpp 17 standard is nice
set(CCOMMON_FLAGS "-mtune=generic -O3")

## Game modules
message(STATUS "Game module: ${CGAME}")
# Team Fortress game module
if(CGAME STREQUAL "tf2")
	add_definitions("-DCATHOOK_TF2=1")
	set(CBASE "SOURCE_2013_SDK")
# Fistful of Frags game module
elseif(CGAME STREQUAL "fof")
	add_definitions("-DCATHOOK_FOF=1")
	set(CBASE "SOURCE_2013_SDK")
elseif(CGAME STREQUAL "csgo")
	add_definitions("-DCATHOOK_CSGO=1")
  file(GLOB_RECURSE TMP_SOURCES "${CMAKE_SOURCE_DIR}/src/modules/csgo/*.c*")
	list(APPEND SOURCE_FILES ${TMP_SOURCES})
endif()

## Graphics modules
message(STATUS "Graphics module: ${CGFX}")
# Valve surface
if(CGFX STREQUAL "valve_surface")
	add_definitions("-DCATHOOK_GFX_SURFACE=1")
endif()

## Input modules
message(STATUS "Input module: ${CINP}")
# Valve surface
if(CINP STREQUAL "xlib")
	add_definitions("-DCATHOOK_INP_XLIB=1")
  list(APPEND SOURCE_FILES "${CMAKE_SOURCE_DIR}/src/modules/xlib/input.cpp")
  set(USE_XLIB true)
endif()

## SDK/Bases if needed
# The source sdk that everyone loves
if(CBASE STREQUAL "SOURCE_2013_SDK")
	set(CCOMMON_FLAGS "${CCOMMON_FLAGS} -m32")
	add_definitions(-DCATHOOK_SOURCE_2013=1 -D_GLIBCXX_USE_CXX11_ABI=0 -D_POSIX=1 -DRAD_TELEMETRY_DISABLED=1 -DLINUX=1 -DUSE_SDL=1 -D_LINUX=1 -DPOSIX=1 -DGNUC=1 -DNO_MALLOC_OVERRIDE=1)
	file(GLOB_RECURSE TMP_SOURCES "${CMAKE_SOURCE_DIR}/src/modules/Source_2013/*.c*")
	list(APPEND SOURCE_FILES ${TMP_SOURCES}) # add our source sdk files
endif()

# If anything uses xlib, we need the main file
if(USE_XLIB)
  list(APPEND SOURCE_FILES "${CMAKE_SOURCE_DIR}/src/modules/xlib/xlib.cpp")
endif()

# Debug info
if (CMAKE_BUILD_TYPE MATCHES DEBUG)
  foreach (ITR ${SOURCE_FILES})
    message(STATUS "Building with:${ITR}")
  endforeach()
endif()

# Set system version
add_definitions("-DCMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME}")

# Set git hash and date if possible
find_package(Git)
if(GIT_FOUND)
	execute_process(
	  COMMAND git log -1 --pretty="%h"
	  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	  OUTPUT_VARIABLE GIT_COMMIT_HASH
	  OUTPUT_STRIP_TRAILING_WHITESPACE
	)
	add_definitions("-DGIT_COMMIT_HASH=${GIT_COMMIT_HASH}")
	execute_process(
	  COMMAND git log -1 --pretty="%ai"
	  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	  OUTPUT_VARIABLE GIT_COMMIT_DATE
	  OUTPUT_STRIP_TRAILING_WHITESPACE
	)
	add_definitions("-DGIT_COMMIT_DATE=${GIT_COMMIT_DATE}")
endif()


# Default compiler flags
set(CMAKE_CXX_FLAGS_DEBUG "${CCOMMON_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG} -g3 -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "${CCOMMON_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}")
set(CMAKE_C_FLAGS_DEBUG "${CCOMMON_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG} -g3 -ggdb")
set(CMAKE_C_FLAGS_RELEASE "${CCOMMON_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${CCOMMON_LINKER_FLAGS} -O3")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Flags: ${CMAKE_CXX_FLAGS_RELEASE}")
message(STATUS "Linker Flags: ${CMAKE_SHARED_LINKER_FLAGS}")

#include_directories(${INCLUDE_DIRS})
add_library(nekohook SHARED ${SOURCE_FILES})

target_link_libraries(nekohook c stdc++)
