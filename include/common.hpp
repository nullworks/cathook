/*
 * common.hpp
 *
 *  Created on: Dec 5, 2016
 *      Author: nullifiedcat
 */

#pragma once
#ifndef __CATHOOK_COMMON_HPP
#define __CATHOOK_COMMON_HPP

#include "config.h"

#include <vector>
#include <string>
#include <array>
#include <mutex>
#include <cmath>
#include <memory>
#include <fstream>
#include <unordered_map>
#include <algorithm>

#include <unistd.h>
#include <link.h>
#include <sys/sysinfo.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <cstring>

#include "timer.hpp"

#include "core/macros.hpp"
#include "Constants.hpp"
#if ENABLE_VISUALS
#include <visual/colors.hpp>
#include <visual/drawing.hpp>
#include <visual/atlas.hpp>
#endif

#include "core/profiler.hpp"
#include "core/offsets.hpp"
#include <entitycache.hpp>
#include "hoovy.hpp"
#include <enums.hpp>
#include "projlogging.hpp"
#include "velocity.hpp"
#include "globals.h"
#include <helpers.hpp>
#include "playerlist.hpp"
#include <core/interfaces.hpp>
#include <localplayer.hpp>
#include <conditions.hpp>
#include <core/logging.hpp>
#include "playerresource.h"
#include "sdk/usercmd.hpp"
#include "trace.hpp"
#include <core/cvwrapper.hpp>
#include "core/netvars.hpp"
#include "core/vfunc.hpp"
#include "hooks.hpp"
#include <prediction.hpp>
#include <itemtypes.hpp>
#include <chatstack.hpp>
#include "pathio.hpp"
#include "ipc.hpp"
#include "tfmm.hpp"
#include "hooks/HookedMethods.hpp"
#include "classinfo/classinfo.hpp"
#include "crits.hpp"
#include "textmode.hpp"
#include "core/sharedobj.hpp"
#include "init.hpp"
#include "reclasses/reclasses.hpp"
#include "HookTools.hpp"
#include "bytepatch.hpp"

#include "copypasted/Netvar.h"
#include "copypasted/CSignature.h"

#include <core/sdk.hpp>

template <typename T> constexpr T _clamp(T _min, T _max, T _val)
{
    return ((_val > _max) ? _max : ((_val < _min) ? _min : _val));
}

#define _FASTCALL __attribute__((fastcall))
#define STRINGIFY(x) #x

#include "gameinfo.hpp"

#define SQR(x) (x) * (x)

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

#define GET_RENDER_CONTEXT (IsTF2() ? g_IMaterialSystem->GetRenderContext() : g_IMaterialSystemHL->GetRenderContext())
#endif
