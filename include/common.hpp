/*
 * common.h
 *
 *  Created on: Dec 5, 2016
 *      Author: nullifiedcat
 */

#ifndef COMMON_H_
#define COMMON_H_

#if defined(LINUX) and not defined(NO_IPC)
#define ENABLE_IPC 1
#else
#undef ENABLE_IPC
#endif

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
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <pwd.h>
#include <assert.h>

#include "timer.hpp"
#include "averager.hpp"

#include "macros.hpp"
#include <visual/colors.hpp>

#if ENABLE_VISUALS == 1

#include <visual/drawex.hpp>
#include <visual/drawing.hpp>
#include "fidgetspinner.hpp"
#include <visual/EffectGlow.hpp>
#include <visual/atlas.hpp>
#include <visual/EffectChams.hpp>
#include <visual/drawmgr.hpp>

#endif

#include "profiler.hpp"
#include "offsets.hpp"
#include <entitycache.hpp>
#include "hoovy.hpp"
#include <enums.hpp>
#include "projlogging.hpp"
#include "ucccccp_cmds.hpp"
#include "velocity.hpp"
#include "angles.hpp"
#include "entityhitboxcache.hpp"
#include "globals.h"
#include <helpers.hpp>
#include "playerlist.hpp"
#include <interfaces.hpp>
#include <localplayer.hpp>
#include <conditions.hpp>
#include <logging.hpp>
#include <targethelper.hpp>
#include "playerresource.h"
#include "usercmd.hpp"
#include "trace.hpp"
#include <cvwrapper.hpp>
#include "netvars.hpp"
#include "vfunc.hpp"
#include "hooks.hpp"
#include <prediction.hpp>
#include <conditions.hpp>
#include <itemtypes.hpp>
#include <chatstack.hpp>
#include "textfile.hpp"
#include "ipc.hpp"
#include "tfmm.hpp"
#include "hooks/hookedmethods.hpp"
#include "classinfo/classinfo.hpp"
#include "votelogger.hpp"
#include "crits.hpp"
#include "textmode.hpp"
#include "backpacktf.hpp"
#include "sharedobj.hpp"
#include "init.hpp"
#include "reclasses/reclasses.hpp"

#include "copypasted/Netvar.h"
#include "copypasted/CSignature.h"

#if ENABLE_GUI
#include "gui/GUI.hpp"
#endif

#include <hacks/hacklist.hpp>

#include <sdk.hpp>

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

#endif /* COMMON_H_ */
