/*
 * common.hpp
 *
 *  Created on: Dec 5, 2016
 *      Author: nullifiedcat
 */

#pragma once

#include "config.h"

#include <emmintrin.h>
#include <vector>
#include <bitset>
#include <string>
#include <array>
#include <cassert>
#include <functional>
#include <mutex>
#include <atomic>
#include <cmath>
#include <memory>
#include <iomanip>
#include <list>
#include <fstream>
#include <set>
#include <unordered_map>
#include <algorithm>
#include <csignal>

#include <sys/prctl.h>
#include <unistd.h>
#include <link.h>
#include <sys/sysinfo.h>
#include <dlfcn.h>
#include <elf.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <cstring>
#include <cstdio>
#include <cassert>

#include "timer.hpp"
#include "averager.hpp"

#include "core/macros.hpp"
#include <visual/colors.hpp>

#if ENABLE_VISUALS
#include <visual/drawing.hpp>
#include "visual/fidgetspinner.hpp"
#include <visual/EffectGlow.hpp>
#include <visual/atlas.hpp>
#include <visual/EffectChams.hpp>
#include <visual/drawmgr.hpp>
#include "visual/menu/compatlayer.hpp"
#endif

#include "core/profiler.hpp"
#include "core/offsets.hpp"
#include <entitycache.hpp>
#include "hoovy.hpp"
#include <enums.hpp>
#include "projlogging.hpp"
#include "velocity.hpp"
#include "angles.hpp"
#include "entityhitboxcache.hpp"
#include "globals.h"
#include <helpers.hpp>
#include "playerlist.hpp"
#include <core/interfaces.hpp>
#include <localplayer.hpp>
#include <conditions.hpp>
#include <core/logging.hpp>
#include <targethelper.hpp>
#include "playerresource.h"
#include "sdk/usercmd.hpp"
#include "trace.hpp"
#include <core/cvwrapper.hpp>
#include "core/netvars.hpp"
#include "core/vfunc.hpp"
#include "hooks.hpp"
#include <prediction.hpp>
#include <conditions.hpp>
#include <itemtypes.hpp>
#include <chatstack.hpp>
#include "textfile.hpp"
#include "ipc.hpp"
#include "tfmm.hpp"
#include "hooks/HookedMethods.hpp"
#include "classinfo/classinfo.hpp"
#include "votelogger.hpp"
#include "crits.hpp"
#include "textmode.hpp"
#include "backpacktf.hpp"
#include "core/sharedobj.hpp"
#include "init.hpp"
#include "reclasses/reclasses.hpp"

#include "copypasted/Netvar.h"
#include "copypasted/CSignature.h"

#if ENABLE_GUI
#include "visual/menu/GUI.h"
#endif

#include <core/sdk.hpp>

template <typename T> constexpr T _clamp(T _min, T _max, T _val)
{
    return ((_val > _max) ? _max : ((_val < _min) ? _min : _val));
}

#define _FASTCALL __attribute__((fastcall))
#define STRINGIFY(x) #x

#include "gameinfo.hpp"

#define SQR(x) (x) * (x)

#define CON_NAME "cat"
#define CON_PREFIX CON_NAME "_"

#define SUPER_VERBOSE_DEBUG false
#if SUPER_VERBOSE_DEBUG == true
#define SVDBG(...) logging::Info(__VA_ARGS__)
#else
#define SVDBG(...)
#endif

#ifndef DEG2RAD
#define DEG2RAD(x) (float) (x) * (PI / 180.0f)
#endif

#define STR(c) #c

#define GET_RENDER_CONTEXT                                                     \
    (IsTF2() ? g_IMaterialSystem->GetRenderContext()                           \
             : g_IMaterialSystemHL->GetRenderContext())
