#
#	The following are old options for this modified makefile.
#	
#	The current makefile needs a rewrite for the modularity
#
#
#
#	MAKEFILE OPTIONS (make OPTION=1 ...args)
#
#	GAME - compile for specific game (tf2, hl2dm, dab, tf2c, css, dynamic), tf2 by default, other ones probably won't compile/crash on inject
#	CLANG - compile with clang instead of g++
#	BUILD_DEBUG - include debug info in the build
#	NO_VISUALS - disable all visuals completely
#	NO_IPC - disable IPC module completely (also disables followbot lol)
#	NO_GUI - disable GUI
#	NO_LTO - disable Link-Time Optimization
#	NO_WARNINGS - disable warnings during compilation
#	NO_TF2_RENDERING - disable in-game rendering (does not work yet)
#	TEXTMODE_STDIN - allows using console with textmode tf2
#	TEXTMODE_VAC - allows joining VAC-secured servers in textmode
#

# Directory/file names & info
OUT_NAME = libcathook.so
LIB_DIR=lib
SRC_DIR=src
RES_DIR=res
OUT_DIR=bin
TARGET = $(OUT_DIR)/$(OUT_NAME)


### Common Compiler Options

# Common flags used for compiler
#-o3: Optimizations, -shared: Probs due to this being a shared library that it needs it, -fmessage-length: Compiler message formatting, 
#-m32: compile for x86_32 programs, -fvisibility: Hides symbols for potential undetection, -fPIC: to allow the librarys machine code to be dynamic in memory, 
#-march and stuff is to optimise the program for the cpu compiling this
COMMON_FLAGS=-O3 -shared -fmessage-length=0 -m32 -fvisibility=hidden -fPIC -march=native -mtune=native
ifdef BUILD_DEBUG
COMMON_FLAGS+=-g3 -ggdb
endif

# Warning flags for the compiler, use these if you want to use 
WARNING_FLAGS=-pedantic -Wall -Wextra -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-declarations -Wmissing-include-dirs -Wnoexcept -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=5 -Wswitch-default -Wundef

# We make a define var we can add to later
DEFINES:=CATHOOK=1

# General compiler information 
INCLUDES=-isystem/usr/include/c++/6.3.1 # Global includes
LDLIBS=-l:libc.so.6 -l:libstdc++.so.6 -l:libtier0.so -l:libvstdlib.so # Do these need to be changed depending on game?
LDFLAGS=-shared -L$(realpath $(LIB_DIR)) # Tell the linker we want a shared library + give it our libs location
SOURCES=$(shell find $(SRC_DIR) -name "*.c*" -print) # Any c/cpp files


### Compiler Switcher, uses clang by default
ifndef USE_GCC
CXX=clang++
CC=clang
LD=ld.lld
LDFLAGS+=-melf_i386
DEFINES+=CLANG=1
else
CXX=$(shell sh -c "which g++-6 || which g++")
CC=$(shell sh -c "which gcc-6 || which gcc")
LD=$(CXX)
LDFLAGS+=-m32 -fno-gnu-unique -fPIC
endif


### Selectivly add to our base options depending on the modules used

# Clear out our modules folder from the sources so we can selectivly add them
SOURCES:=$(filter-out $(shell find $(SRC_DIR)/modules -name "*.c*" -print),$(SOURCES))

## Game modules
# Default game if none listed
ifndef GAME
GAME=fof
endif

# Define our game
DEFINES+=GAME=$(GAME)

# Team Fortress game module
ifeq ($(GAME), tf2)
USE_SOURCE_2013_SDK=1
SOURCES+=$(shell find $(SRC_DIR)/modules/tf2 -name "*.c*" -print) # add our tf2 files
DEFINES+=CATHOOK_TF2=1
# Default sub modules for tf2
ifndef GRAPHICS_MOD
GRAPHICS_MOD=valve_surface
endif
ifndef INPUT_MOD
INPUT_MOD=xlib
endif
endif

# Fistful of Frags game module
ifeq ($(GAME), fof)
USE_SOURCE_2013_SDK=1
SOURCES+=$(shell find $(SRC_DIR)/modules/fistfulfrags -name "*.c*" -print) # add our game files
DEFINES+=CATHOOK_FOF=1
# Default sub modules for fof
ifndef GRAPHICS_MOD
GRAPHICS_MOD=valve_surface
endif
ifndef INPUT_MOD
INPUT_MOD=xlib
endif
endif

# Left 4 dead 2 game module
ifeq ($(GAME), l4d2)
USE_SOURCE_2013_SDK=1
SOURCES+=$(shell find $(SRC_DIR)/modules/l4d2 -name "*.c*" -print) # add our game files
DEFINES+=CATHOOK_L4D2=1
# Default sub modules for l4d2
ifndef GRAPHICS_MOD
GRAPHICS_MOD=valve_surface
endif
ifndef INPUT_MOD
INPUT_MOD=xlib
endif
endif

## SDK/Bases if needed
ifdef USE_SOURCE_2013_SDK
DEFINES+=_GLIBCXX_USE_CXX11_ABI=0 _POSIX=1 RAD_TELEMETRY_DISABLED=1 LINUX=1 USE_SDL=1 _LINUX=1 POSIX=1 GNUC=1 NO_MALLOC_OVERRIDE=1
SOURCES+=$(shell find $(SRC_DIR)/modules/Source_2013_SDK -name "*.c*" -print) # add our source sdk files
SOURCES:=$(filter-out $(shell find $(SRC_DIR)/modules/Source_2013_SDK/cathook_common/valve_surface_drawing -name "*.c*" -print),$(SOURCES)) # remove the valve surface draw
endif

## Graphics modules
# Valve surface
ifeq ($(GRAPHICS_MOD), valve_surface)
ifdef USE_SOURCE_2013_SDK
DEFINES+=CATHOOK_GPH_SURFACE=1
SOURCES+=$(shell find $(SRC_DIR)/modules/Source_2013_SDK/cathook_common/valve_surface_drawing -name "*.c*" -print) # add our tf2 files
endif
endif

## Input modules
# Xlib input
ifeq ($(INPUT_MOD), xlib)
DEFINES+=CATHOOK_INP_XLIB=1
SOURCES+=$(shell find $(SRC_DIR)/modules/xlib_input -name "*.c*" -print) # add our tf2 files
endif

### Defines that we use in our project that are done by the make file

# Github hash and date
GIT_COMMIT_HASH=$(shell git log -1 --pretty="%h")
GIT_COMMIT_DATE=$(shell git log -1 --pretty="%ai")
DEFINES+=GIT_COMMIT_HASH="\"$(GIT_COMMIT_HASH)\"" GIT_COMMIT_DATE="\"$(GIT_COMMIT_DATE)\""

# Register our user
ifndef REGISTERED_USER
REGISTERED_USER=DEVELOPMENT_BUILD
endif
ifndef REGISTERED_UID
REGISTERED_UID=0
endif
DEFINES+=REGISTERED_USER="\"$(REGISTERED_USER)\"" REGISTERED_UID="\"$(REGISTERED_UID)\""


### Start creating our compilers flags

# Use our common compiler flags as a base
CFLAGS=$(COMMON_FLAGS)
CXXFLAGS=-std=gnu++1z $(COMMON_FLAGS)

# Add warning flags if needed
ifndef GET_WARNINGS
CFLAGS+=-w
CXXFLAGS+=-w
else
CFLAGS+=$(WARNING_FLAGS)
CXXFLAGS+=$(WARNING_FLAGS)
endif

# Add our defines
CXXFLAGS+=$(addprefix -D,$(DEFINES))
CFLAGS+=$(addprefix -D,$(DEFINES))

# Add our global includes
CXXFLAGS+=$(INCLUDES)
CFLAGS+=$(INCLUDES)


### Finally we create our objects
### TODO!!! Look up more on how this works
OBJECTS = $(patsubst %.c,%.o, $(patsubst %.cpp,%.o, $(SOURCES)))
OBJECTS += $(shell find $(RES_DIR) -name "*.o" -print)
DEPENDS = $(patsubst %.c,%.d, $(patsubst %.cpp,%.d, $(SOURCES)))

.PHONY: clean directories echo

all:
	mkdir -p $(OUT_DIR)
	@echo Compiling with $(CC)
	$(MAKE) $(TARGET)
	
echo:
	echo $(OBJECTS)
	
.cpp.o:
	@echo Compiling $<
	@$(CXX) $(CXXFLAGS) -c $< -o $@
	
.c.o:
	@echo Compiling $<
	@$(CC) $(CFLAGS) -c $< -o $@

%.d: %.cpp
	@$(CXX) -M $(CXXFLAGS) $< > $@

$(TARGET): $(OBJECTS)
	@echo Building cathook
	@echo $(LD) -o $@ $(LDFLAGS) $(OBJECTS) $(LDLIBS)
	$(LD) -o $@ $(LDFLAGS) $(OBJECTS) $(LDLIBS)
ifndef BUILD_DEBUG
	strip --strip-all $@
endif

clean:
	find src -type f -name '*.o' -delete
	find src -type f -name '*.d' -delete
	#find simple-ipc -type f -name '*.o' -delete
	#find simple-ipc -type f -name '*.d' -delete
	rm -rf ./bin

ifneq ($(MAKECMDGOALS), clean)
-include $(DEPENDS)
endif