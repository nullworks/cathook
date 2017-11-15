
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
set(CMAKE_BUILD_TYPE RELEASE CACHE STRING "Cmake build type")

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
set(CCOMMON_FLAGS "-std=gnu++17 -O3 -fpic -shared -mtune=generic ")

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
endif()

## Graphics modules
message(STATUS "Graphics module: ${CGFX}")
# Valve surface
if(CGFX STREQUAL "tf2")
	add_definitions("-DCATHOOK_GFX_SURFACE=1")
endif()

## Input modules
message(STATUS "Input module: ${CINP}")
# Valve surface
if(CINP STREQUAL "xlib")
	add_definitions("-DCATHOOK_GFX_SURFACE=1")
  list(APPEND SOURCE_FILES "${CMAKE_SOURCE_DIR}/src/modules/xlib/input.cpp")
endif()

## SDK/Bases if needed
# The source sdk that everyone loves
if(CBASE STREQUAL "SOURCE_2013_SDK")
	set(CCOMMON_FLAGS "${CCOMMON_FLAGS} -m32")
	add_definitions("-DCATHOOK_SOURCE_2013=1 -D_GLIBCXX_USE_CXX11_ABI=0 -D_POSIX=1 RAD_TELEMETRY_DISABLED=1 -DLINUX=1 -DUSE_SDL=1 -D_LINUX=1 -DPOSIX=1 -DGNUC=1 -DNO_MALLOC_OVERRIDE=1")
	file(GLOB_RECURSE TMP_SOURCES "${CMAKE_SOURCE_DIR}/src/modules/Source_2013/*.c*")
	list(APPEND SOURCE_FILES ${TMP_SOURCES}) # add our source sdk files
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
# use -std=gnu++14 for source 2013
set(CMAKE_CXX_FLAGS_DEBUG "${CCOMMON_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG} -g -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "${CCOMMON_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}")

include_directories(${INCLUDE_DIRS})

add_library(libnekohook SHARED ${SOURCE_FILES})
target_link_libraries(libnekohook dl)
