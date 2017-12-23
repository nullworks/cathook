
#
# Cathooks fancy new cmake file
#	CMAKE OPTIONS (cmake -DOPTION=true ...args)
#
# CMAKE_BUILD_TYPE - Change between RELEASE and DEBUG depending on your use
#	GAME - compile for specific game (tf2, hl2dm, dab, tf2c, css, dynamic), tf2 by default, other ones probably won't compile/crash on inject
#	NO_GUI - disable GUI
#	NO_IPC - disable IPC module completely (also disables followbot lol)
#	ENABLE_NULL_GRAPHICS - disable in-game rendering (does not work yet)
#	TEXTMODE_STDIN - allows using console with textmode tf2
#	TEXTMODE_VAC - allows joining VAC-secured servers in textmode
#

cmake_minimum_required (VERSION 2.6)
project (libcathook)

## Defaults
# User options can be set with cmake -D"NAME OF OPTION HERE"="VARIABLE", e.g. "cmake -DCGAME=csgo"
set(GAME tf2 CACHE STRING "Game to build with")
set(ENABLE_VISUALS true CACHE BOOL "Whether to compile visuals at all")
set(ENABLE_GUI true CACHE BOOL "Whether to enable cathooks gui")
set(ENABLE_IPC true CACHE BOOL "Whether to compile IPC into cathook")
set(ENABLE_NULL_GRAPHICS false CACHE BOOL "Whether to disable all ingame graphics for tf2")
set(TEXTMODE_STDIN false CACHE BOOL "Allow using console in textmode")
set(TEXTMODE_VAC false CACHE BOOL "Whether to compile vac bypass for textmode into cathook")
set(DATA_PATH /opt/cathook/data CACHE STRING "Where to store data files")
set(USE_WARNINGS false CACHE bool "Whether the compiler should spit out warnings")
if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, default to Release")
    set(CMAKE_BUILD_TYPE RELEASE)
endif(NOT CMAKE_BUILD_TYPE)


## Here we get all of our default stuff for later


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

# Add cathooks source files except specific dirs, we add those selectivly
file(GLOB_RECURSE SOURCE_FILES "${CMAKE_SOURCE_DIR}/src/*.c*")

# Optimization flags
set(CCOMMON_FLAGS "-mtune=native -march=native -O3")
# Ease of life flags
set(CCOMMON_FLAGS "${CCOMMON_FLAGS} -fpermissive -Wno-unknown-pragmas -fmessage-length=0 -fvisibility=hidden")
# Required flags
set(CCOMMON_FLAGS "${CCOMMON_FLAGS} -m32")
if (USE_WARNINGS)
  set(CCOMMON_FLAGS "${CCOMMON_FLAGS} -pedantic -Wall -Wextra -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-declarations -Wmissing-include-dirs -Wnoexcept -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=5 -Wswitch-default -Wundef")
else()
  set(CCOMMON_FLAGS "${CCOMMON_FLAGS} -w")
endif()
# Source SDK DEFINES
add_definitions(-DCATHOOK_SOURCE_2013=1 -D_GLIBCXX_USE_CXX11_ABI=0 -D_POSIX=1 -DRAD_TELEMETRY_DISABLED=1 -DLINUX=1 -DUSE_SDL=1 -D_LINUX=1 -DPOSIX=1 -DGNUC=1 -DNO_MALLOC_OVERRIDE=1)

# Preprosessor options for cathook
add_definitions(-DENABLE_VISUALS=${ENABLE_VISUALS} -DENABLE_GUI=${ENABLE_GUI} -DENABLE_IPC=${ENABLE_IPC} -DBUILD_GAME=${GAME} -DENABLE_NULL_GRAPHICS=${ENABLE_NULL_GRAPHICS} -DTEXTMODE_STDIN=${TEXTMODE_STDIN} -DTEXTMODE_VAC=${TEXTMODE_VAC} "-DDATA_PATH=${DATA_PATH}")

# Set git hash and date if possible
find_package(Git REQUIRED) # we later use git
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

# Includes
include_directories("ucccccp" "include" "." "simple-ipc/src/include" "source-sdk-2013/mp/src/public" "source-sdk-2013/mp/src" "source-sdk-2013/mp/src/mathlib" "source-sdk-2013/mp/src/common" "source-sdk-2013/mp/src/tier1" "source-sdk-2013/mp/src/tier0")

# default libs
set(CLINKERLIBS "c stdc++ pthread tier0 vstdlib")


## Here we make important  changes depending on the build settings


## Game specific
message(STATUS "Game module: ${GAME}")
# Team Fortress
if(GAME STREQUAL "tf2")
	add_definitions(-DCATHOOK_TF2=1)
endif()

# if visuals are enabled
if(ENABLE_VISUALS)
  # GLEW
  find_package(GLEW REQUIRED)
  include_directories(GLEW_INCLUDE_DIRS)
  set(CLINKERLIBS "${CLINKERLIBS} ${GLEW_LIBRARIES}")
  message(STATUS, "Uhh: ${GLEW_INCLUDE_DIRS}")
  # SDL
  find_package(SDL REQUIRED)
  set(CLINKERLIBS "${CLINKERLIBS} :libSDL2-2.0")
  execute_process(
    COMMAND sdl2-config --cflags
    OUTPUT_VARIABLE SDL_CFG_FLAGS
  	OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  set(CCOMMON_FLAGS "${CCOMMON_FLAGS} ${SDL_CFG_FLAGS}")
# Visuals are disabled, we must remove visual sources
else()
  list(REMOVE_ITEM SOURCE_FILES ${CMAKE_SOURCE_DIR}/src/visual/atlas.cpp ${CMAKE_SOURCE_DIR}/src/visual/colors.cpp ${CMAKE_SOURCE_DIR}/src/visual/drawex.cpp ${CMAKE_SOURCE_DIR}/src/visual/drawing.cpp ${CMAKE_SOURCE_DIR}/src/visual/drawmgr.cpp ${CMAKE_SOURCE_DIR}/src/visual/EffectChams.cpp ${CMAKE_SOURCE_DIR}/src/visual/EffectGlow.cpp ${CMAKE_SOURCE_DIR}/src/hacks/ESP.cpp ${CMAKE_SOURCE_DIR}/src/hacks/SkinChanger.cpp ${CMAKE_SOURCE_DIR}/src/hacks/SpyAlert.cpp ${CMAKE_SOURCE_DIR}/src/hacks/Radar.cpp ${CMAKE_SOURCE_DIR}/src/fidgetspinner.cpp  ${CMAKE_SOURCE_DIR}/src/hooks/sdl.cpp ${CMAKE_SOURCE_DIR}/src/drawing.cpp ${CMAKE_SOURCE_DIR}/src/hooks/PaintTraverse.cpp)
  EXCLUDE_FILES_FROM_DIR_IN_LIST("${SOURCE_FILES}" "${CMAKE_SOURCE_DIR}/src/gui/")
endif()

if(ENABLE_GUI)
  EXCLUDE_FILES_FROM_DIR_IN_LIST("${SOURCE_FILES}" "${CMAKE_SOURCE_DIR}/src/gui/")
endif()

if(ENABLE_IPC)
  file(GLOB_RECURSE TMP_SOURCES "${CMAKE_SOURCE_DIR}/simple-ipc/src/include/*.cpp")
  list(APPEND SOURCE_FILES ${TMP_SOURCES}) # add our source sdk files
endif()


## We are finished with changes, finish and compile


# Default compiler flags
set(CMAKE_CXX_FLAGS_DEBUG "${CCOMMON_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG} -g -ggdb")
set(CMAKE_C_FLAGS_DEBUG "${CCOMMON_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG} -g -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "${CCOMMON_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}")
set(CMAKE_C_FLAGS_RELEASE "${CCOMMON_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -m32")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# make our main target
add_library(cathook SHARED ${SOURCE_FILES})

# Add our library files for linking
link_directories(lib)

# link with libraries
target_link_libraries(cathook ${CLINKERLIBS})
