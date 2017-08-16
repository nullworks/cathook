ifndef CLANG
CXX=$(shell sh -c "which g++-6 || which g++")
CC=$(shell sh -c "which gcc-6 || which gcc")
LD=$(CXX)
else
CXX=clang++
CC=clang
LD=ld.lld
endif

DEFINES=_GLIBCXX_USE_CXX11_ABI=0 _POSIX=1 FREETYPE_GL_USE_VAO RAD_TELEMETRY_DISABLED LINUX=1 USE_SDL _LINUX=1 POSIX=1 GNUC=1 NO_MALLOC_OVERRIDE

WARNING_FLAGS=-pedantic -Wall -Wextra -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-declarations -Wmissing-include-dirs -Wnoexcept -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=5 -Wswitch-default -Wundef
COMMON_FLAGS=-fpermissive -O3 -shared -Wno-unknown-pragmas -fmessage-length=0 -m32 -fvisibility=hidden -fPIC -march=native -mtune=native


ifdef CLANG
COMMON_FLAGS+=-Wno-c++11-narrowing
endif

ifdef BUILD_DEBUG
COMMON_FLAGS+=-g3 -ggdb
else
ifndef CLANG
COMMON_FLAGS+=-flto
endif
endif

CFLAGS=$(COMMON_FLAGS)
CXXFLAGS=-std=gnu++14 $(COMMON_FLAGS)

ifndef NO_WARNINGS
CFLAGS+=$(WARNING_FLAGS)
CXXFLAGS+=$(WARNING_FLAGS)
else
CFLAGS+=-w
CXXFLAGS+=-w
endif

SDKFOLDER=$(realpath source-sdk-2013/mp/src)
SIMPLE_IPC_DIR = $(realpath simple-ipc/src/include)
INCLUDES=-Iucccccp -isystemsrc/freetype-gl -isystemsrc/imgui -isystem/usr/local/include/freetype2 -isystem/usr/include/freetype2 -I$(SIMPLE_IPC_DIR) -isystem$(SDKFOLDER)/public -isystem$(SDKFOLDER)/mathlib -isystem$(SDKFOLDER)/common -isystem$(SDKFOLDER)/public/tier1 -isystem$(SDKFOLDER)/public/tier0 -isystem$(SDKFOLDER)
LIB_DIR=lib
LDFLAGS=-shared -L$(realpath $(LIB_DIR))
ifdef CLANG
LDFLAGS+=-melf_i386
else
LDFLAGS+=-m32 -fno-gnu-unique
endif

ifndef BUILD_DEBUG
ifndef CLANG
LDFLAGS+=-flto
endif
endif
LDLIBS=-lssl -l:libSDL2-2.0.so.0 -static -l:libc.so.6 -static -l:libstdc++.so.6 -l:libtier0.so -l:libvstdlib.so -static -l:libGLEW.so -l:libfreetype.so

OUT_NAME = libcathook.so

ifdef TEXTMODE
$(info Compiling for text mode only!)
N_LDLIBS = -lssl -l:libSDL2-2.0.so.0 -l:libGLEW.so -l:libfreetype.so
LDLIBS := $(filter-out $(N_LDLIBS),$(LDLIBS))
N_INCLUDES = -isystemsrc/freetype-gl -isystemsrc/imgui -isystem/usr/local/include/freetype2 -isystem/usr/include/freetype2
INCLUDES := $(filter-out $(N_INCLUDES),$(INCLUDES))
DEFINES += TEXTMODE=1
#OUT_NAME := libcathook-textmode.so
endif

ifdef TEXTMODE_STDIN
DEFINES+=-DTEXTMODE_STDIN
endif

SRC_DIR = src
RES_DIR = res
TARGET_DIR = bin
TARGET = $(TARGET_DIR)/$(OUT_NAME)
SOURCES = $(shell find $(SRC_DIR) -name "*.c*" -print)
ifdef NOGUI
$(info GUI disabled)
SOURCES := $(filter-out $(shell find $(SRC_DIR)/gui -name "*.cpp" -print),$(SOURCES))
DEFINES+=NOGUI=1
else
$(info GUI enabled)
endif
ifdef GAME
$(info Building for: $(GAME))
DEFINES+=BUILD_GAME=$(GAME)
else
$(info GUI enabled)
endif
SOURCES += $(shell find $(SIMPLE_IPC_DIR) -name "*.cpp" -print)
OBJECTS = $(patsubst %.c,%.o, $(patsubst %.cpp,%.o, $(SOURCES)))
OBJECTS += $(shell find $(RES_DIR) -name "*.o" -print)
DEPENDS = $(patsubst %.c,%.d, $(patsubst %.cpp,%.d, $(SOURCES)))
SRC_SUBDIRS=$(shell find $(SRC_DIR) -type d -print)

GIT_COMMIT_HASH=$(shell git log -1 --pretty="%h")
GIT_COMMIT_DATE=$(shell git log -1 --pretty="%ai")

DEFINES+=GIT_COMMIT_HASH="\"$(GIT_COMMIT_HASH)\"" GIT_COMMIT_DATE="\"$(GIT_COMMIT_DATE)\""

ifdef GAME
DEFINES+=GAME=$(GAME)
endif

ifdef NOIPC
$(info IPC disabled)
DEFINES += NO_IPC
endif

CXXFLAGS+=$(addprefix -D,$(DEFINES))
CFLAGS+=$(addprefix -D,$(DEFINES))

CXXFLAGS+=$(INCLUDES)
CFLAGS+=$(INCLUDES)

ifdef TEXTMODE

N_SOURCES := hacks/ESP.cpp hacks/SkinChanger.cpp hacks/SpyAlert.cpp hacks/Radar.cpp fidgetspinner.cpp ftrender.cpp hooks/sdl.cpp drawmgr.cpp drawgl.cpp hooks/PaintTraverse.cpp EffectChams.cpp EffectGlow.cpp
N_SOURCES := $(addprefix $(SRC_DIR)/,$(N_SOURCES))

SOURCES := $(filter-out $(shell find $(SRC_DIR)/gui -name "*.cpp" -print),$(SOURCES))
SOURCES := $(filter-out $(shell find $(SRC_DIR)/freetype-gl -name "*.c*" -print),$(SOURCES))
SOURCES := $(filter-out $(shell find $(SRC_DIR)/imgui -name "*.c*" -print),$(SOURCES))
SOURCES := $(filter-out $(N_SOURCES),$(SOURCES))


else

CXXFLAGS+=$(shell sdl2-config --cflags)
CFLAGS+=$(shell sdl2-config --cflags)

endif

.PHONY: clean directories echo

all:
	mkdir -p $(TARGET_DIR)
	$(MAKE) $(TARGET)
	
echo:
	echo $(OBJECTS)

src/imgui/imgui_demo.o : CXXFLAGS+=-w
src/imgui/imgui_draw.o : CXXFLAGS+=-w
src/imgui/imgui_impl_sdl.o : CXXFLAGS+=-w
src/imgui/imgui.o : CXXFLAGS+=-w

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
src/sdk/checksum_md5.o : CFLAGS+=-w
src/sdk/convar.o : CFLAGS+=-w
src/sdk/KeyValues.o : CFLAGS+=-w
src/sdk/MaterialSystemUtil.o : CFLAGS+=-w
src/sdk/tier1.o : CFLAGS+=-w
src/sdk/utlbuffer.o : CFLAGS+=-w

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
	$(LD) -o $(TARGET) $(LDFLAGS) $(OBJECTS) $(LDLIBS)

clean:
	find src -type f -name '*.o' -delete
	find src -type f -name '*.d' -delete
	find simple-ipc -type f -name '*.o' -delete
	find simple-ipc -type f -name '*.d' -delete
	rm -rf ./bin

ifneq ($(MAKECMDGOALS), clean)
-include $(DEPENDS)
endif
