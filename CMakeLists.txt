
#
#
# Welcome to the cmake lists file, this controls how cathook will build for your system
#
# TODO, make wasy way to do "touch ../src/CMakeLists.txt" to make things easy
#
#

cmake_minimum_required (VERSION 2.6)
project (Cathook)

# Our project options
option(CGAME "Game to build for, Avaible games below:\ntf2\nfof\nEND!" tf2)
option(CGFX "Graphics module to build with, Only works with specific games\nAvaible GFX modules below:\nvalve_surface\nEND!")
option(CUSERNAME "Input a name to register build to." DEVELOPMENT_BUILD)
option(CUserId "Input an id to register build to." 0)
option(CForceClang "Forces cmake to use clang." true)

# Flags we use for every build type
set(CCOMMON_FLAGS "-O3 -fpic -shared ")

# Function to remove source files from a directory, ripped from aimtux fuzion <3
FUNCTION (EXCLUDE_FILES_FROM_DIR_IN_LIST _InFileList _excludeDirName)
    foreach (ITR ${_InFileList})
        if ("${ITR}" MATCHES "(.*)${_excludeDirName}(.*)") # Check if the item matches the directory name in _excludeDirName
            message(STATUS "Remove Item from List:${ITR}")
            list (REMOVE_ITEM _InFileList ${ITR}) # Remove the item from the list
        endif ("${ITR}" MATCHES "(.*)${_excludeDirName}(.*)")
    endforeach(ITR)
    set(SOURCE_FILES ${_InFileList} PARENT_SCOPE) # Return the SOURCE_FILES variable to the calling parent
ENDFUNCTION (EXCLUDE_FILES_FROM_DIR_IN_LIST)

# Add cathooks source files except the modules, we add those selectivly
file(GLOB_RECURSE SOURCE_FILES "${CMAKE_SOURCE_DIR}/src/*.c*")
EXCLUDE_FILES_FROM_DIR_IN_LIST("${SOURCE_FILES}" "${CMAKE_SOURCE_DIR}/src/modules/")

## Here we add our games
# Team Fortress game module
if(CGAME STREQUAL "tf2")
	message(STATUS "Making for tf2")
	add_definitions("-DCATHOOK_TF2=1")
	set(CBASE "SOURCE_2013_SDK")
	if(not CGFX) 
		message(STATUS "No GFX module selected, defaulting to valve_surface!")
		set(CGFX "valve_surface")
	endif()
	if(not CINP)
		message(STATUS "No INP module selected, defaulting to xlib-input!")
		set(CINP "xlib")
	endif()
# Fistful of Frags game module
elseif(CGAME STREQUAL "fof")
	message(STATUS "Making for fof!")
	add_definitions("-DCATHOOK_FOF=1")
	set(CBASE "SOURCE_2013_SDK")
endif()

## SDK/Bases if needed
# The source sdk that everyone loves
if(CBASE STREQUAL "SOURCE_2013_SDK")
	set(CCOMMON_FLAGS "${CCOMMON_FLAGS} -m32")
	add_definitions("-D,CATHOOK_SOURCE_2013=1 _GLIBCXX_USE_CXX11_ABI=0 _POSIX=1 RAD_TELEMETRY_DISABLED=1 LINUX=1 USE_SDL=1 _LINUX=1 POSIX=1 GNUC=1 NO_MALLOC_OVERRIDE=1")
	file(GLOB_RECURSE TMP_SOURCES "${CMAKE_SOURCE_DIR}/src/modules/Source_2013/*.c*")
	list(APPEND SOURCE_FILES TMP_SOURCES) # add our source sdk files
endif()

## Graphics modules


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
endif(GIT_FOUND)







# Default compiler flags
# use -std=gnu++14 for source 2013
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}  -mtune=native -g -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -fpic -shared -mtune=generic")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}")
# Default release type
if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, default to Release")
    set(CMAKE_BUILD_TYPE RELEASE)
endif(NOT CMAKE_BUILD_TYPE)

include_directories(${INCLUDE_DIRS})

add_library(Cathook SHARED ${SOURCE_FILES})
target_link_libraries(Cathook SDL2 GL dl xdo)