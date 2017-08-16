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

GAME=tf2

ENABLE_VISUALS=1
ENABLE_GUI=1
ENABLE_IPC=1
ENABLE_NULL_GRAPHICS=0
TEXTMODE_STDIN=0
TEXTMODE_VAC=0
DATA_PATH="/opt/cathook/data"
NO_LTO=0
ifdef CLANG
override NO_LTO=1
endif

ifdef NO_VISUALS
ENABLE_VISUALS:=0
ENABLE_GUI:=0
endif
ifdef NO_IPC
ENABLE_IPC:=0
endif
ifdef NO_GUI
ENABLE_GUI:=0
endif
ifdef NO_TF2_RENDERING
ENABLE_NULL_GRAPHICS:=1
endif

OUT_NAME = libcathook.so
SSDK_DIR=$(realpath source-sdk-2013/mp/src)
SIPC_DIR=$(realpath simple-ipc/src/include)
LIB_DIR=lib
SRC_DIR=src
RES_DIR=res
OUT_DIR=bin
TARGET = $(OUT_DIR)/$(OUT_NAME)

INCLUDES=-Iucccccp -isystem/usr/include/c++/6.3.1 -isystem$(SSDK_DIR)/public -isystem$(SSDK_DIR)/mathlib -isystem$(SSDK_DIR)/common -isystem$(SSDK_DIR)/public/tier1 -isystem$(SSDK_DIR)/public/tier0 -isystem$(SSDK_DIR)
LDLIBS=-static -l:libc.so.6 -l:libstdc++.so.6 -l:libtier0.so -l:libvstdlib.so
LDFLAGS=-shared -L$(realpath $(LIB_DIR))
SOURCES=$(shell find $(SRC_DIR) -name "*.c*" -print)

ifndef CLANG
CXX=$(shell sh -c "which g++-6 || which g++")
CC=$(shell sh -c "which gcc-6 || which gcc")
LD=$(CXX)
LDFLAGS+=-m32 -fno-gnu-unique
else
CXX=clang++
CC=clang
LD=ld.lld
LDFLAGS+=-melf_i386
endif

DEFINES:=_GLIBCXX_USE_CXX11_ABI=0 _POSIX=1 FREETYPE_GL_USE_VAO=1 RAD_TELEMETRY_DISABLED=1 LINUX=1 USE_SDL=1 _LINUX=1 POSIX=1 GNUC=1 NO_MALLOC_OVERRIDE=1
DEFINES+=ENABLE_VISUALS=$(ENABLE_VISUALS) ENABLE_GUI=$(ENABLE_GUI) ENABLE_IPC=$(ENABLE_IPC) BUILD_GAME=$(GAME) ENABLE_NULL_GRAPHICS=$(ENABLE_NULL_GRAPHICS) TEXTMODE_STDIN=$(TEXTMODE_STDIN) TEXTMODE_VAC=$(TEXTMODE_VAC) DATA_PATH="\"$(DATA_PATH)\""

WARNING_FLAGS=-pedantic -Wall -Wextra -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-declarations -Wmissing-include-dirs -Wnoexcept -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=5 -Wswitch-default -Wundef
COMMON_FLAGS=-fpermissive -O3 -shared -Wno-unknown-pragmas -fmessage-length=0 -m32 -fvisibility=hidden -fPIC -march=native -mtune=native

ifdef CLANG
COMMON_FLAGS+=-Wno-c++11-narrowing
endif

ifdef BUILD_DEBUG
COMMON_FLAGS+=-g3 -ggdb
else
ifneq ($(NO_LTO),1)
COMMON_FLAGS+=-flto
endif
endif

CFLAGS=$(COMMON_FLAGS)
CXXFLAGS=-std=gnu++1z $(COMMON_FLAGS)

ifdef NO_WARNINGS
CFLAGS+=-w
CXXFLAGS+=-w
else
CFLAGS+=$(WARNING_FLAGS)
CXXFLAGS+=$(WARNING_FLAGS)
endif

ifeq ($(ENABLE_VISUALS),1)
INCLUDES+=-isystemsrc/freetype-gl -isystemsrc/imgui -isystem/usr/local/include/freetype2 -isystem/usr/include/freetype2
LDLIBS+=-lssl -l:libSDL2-2.0.so.0 -l:libGLEW.so -l:libfreetype.so
CXXFLAGS+=$(shell sdl2-config --cflags)
CFLAGS+=$(shell sdl2-config --cflags)
else
EXCL_SOURCES:=hacks/ESP.cpp hacks/SkinChanger.cpp hacks/SpyAlert.cpp hacks/Radar.cpp fidgetspinner.cpp ftrender.cpp hooks/sdl.cpp drawing.cpp drawmgr.cpp drawgl.cpp hooks/PaintTraverse.cpp EffectChams.cpp EffectGlow.cpp atlas.cpp
EXCL_SOURCES:=$(addprefix $(SRC_DIR)/,$(EXCL_SOURCES))

SOURCES:=$(filter-out $(shell find $(SRC_DIR)/gui -name "*.cpp" -print),$(SOURCES))
SOURCES:=$(filter-out $(shell find $(SRC_DIR)/freetype-gl -name "*.c*" -print),$(SOURCES))
SOURCES:=$(filter-out $(shell find $(SRC_DIR)/imgui -name "*.c*" -print),$(SOURCES))
SOURCES:=$(filter-out $(EXCL_SOURCES),$(SOURCES))
endif

ifneq ($(ENABLE_GUI),1)
SOURCES := $(filter-out $(shell find $(SRC_DIR)/gui -name "*.cpp" -print),$(SOURCES))
endif


GIT_COMMIT_HASH=$(shell git log -1 --pretty="%h")
GIT_COMMIT_DATE=$(shell git log -1 --pretty="%ai")

DEFINES+=GIT_COMMIT_HASH="\"$(GIT_COMMIT_HASH)\"" GIT_COMMIT_DATE="\"$(GIT_COMMIT_DATE)\""

ifeq ($(ENABLE_IPC),1)
SOURCES+=$(shell find $(SIPC_DIR) -name "*.cpp" -print)
INCLUDES+=-I$(SIPC_DIR)
endif

CXXFLAGS+=$(addprefix -D,$(DEFINES))
CFLAGS+=$(addprefix -D,$(DEFINES))

CXXFLAGS+=$(INCLUDES)
CFLAGS+=$(INCLUDES)

OBJECTS = $(patsubst %.c,%.o, $(patsubst %.cpp,%.o, $(SOURCES)))
OBJECTS += $(shell find $(RES_DIR) -name "*.o" -print)
DEPENDS = $(patsubst %.c,%.d, $(patsubst %.cpp,%.d, $(SOURCES)))

.PHONY: clean directories echo data

all:
	mkdir -p $(OUT_DIR)
	$(MAKE) data
	$(MAKE) $(TARGET)
	
echo:
	echo $(OBJECTS)
	
data:
	./check-data "$(DATA_PATH)"

# 3rd party source files, we don't need warnings there

# c++
src/imgui/imgui_demo.o : CXXFLAGS+=-w
src/imgui/imgui_draw.o : CXXFLAGS+=-w
src/imgui/imgui_impl_sdl.o : CXXFLAGS+=-w
src/imgui/imgui.o : CXXFLAGS+=-w
src/sdk/checksum_md5.o : CXXFLAGS+=-w
src/sdk/convar.o : CXXFLAGS+=-w
src/sdk/KeyValues.o : CXXFLAGS+=-w
src/sdk/MaterialSystemUtil.o : CXXFLAGS+=-w
src/sdk/tier1.o : CXXFLAGS+=-w
src/sdk/utlbuffer.o : CXXFLAGS+=-w

# c
src/freetype-gl/distance-field.o : CFLAGS+=-w
src/freetype-gl/edtaa3func.o : CFLAGS+=-w
src/freetype-gl/font-manager.o : CFLAGS+=-w
src/freetype-gl/mat4.o : CFLAGS+=-w
src/freetype-gl/platform.o : CFLAGS+=-w
src/freetype-gl/shader.o : CFLAGS+=-w
src/freetype-gl/text-buffer.o : CFLAGS+=-w
src/freetype-gl/texture-atlas.o : CFLAGS+=-w
src/freetype-gl/utf8-utils.o : CFLAGS+=-w
src/freetype-gl/texture-font.o : CFLAGS+=-w
src/freetype-gl/vector.o : CFLAGS+=-w
src/freetype-gl/vertex-attribute.o : CFLAGS+=-w
src/freetype-gl/vertex-buffer.o : CFLAGS+=-w

# end of 3rd party sources

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
	$(LD) -o $@ $(LDFLAGS) $(OBJECTS) $(LDLIBS)
ifndef BUILD_DEBUG
	strip --strip-all $@
endif

clean:
	find src -type f -name '*.o' -delete
	find src -type f -name '*.d' -delete
	find simple-ipc -type f -name '*.o' -delete
	find simple-ipc -type f -name '*.d' -delete
	rm -rf ./bin

ifneq ($(MAKECMDGOALS), clean)
-include $(DEPENDS)
endif
