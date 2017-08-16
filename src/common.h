/*
 * common.h
 *
 *  Created on: Dec 5, 2016
 *      Author: nullifiedcat
 */

#ifndef COMMON_H_
#define COMMON_H_

#if defined(LINUX) and not defined(NO_IPC)
#define IPC_ENABLED 1
#else
#undef IPC_ENABLED
#endif


#include "beforecheaders.h"
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

#include "timer.hpp"
#include "averager.hpp"

#include "aftercheaders.h"

#include "macros.hpp"
#include "colors.hpp"

#if ENABLE_VISUALS == 1

extern "C" {
#include <vec234.h>
}

#include "ftrender.hpp"
#include "drawing.h"
#include "fidgetspinner.hpp"
#include "drawgl.hpp"
#include "EffectGlow.hpp"
#include "atlas.hpp"
#include "EffectChams.hpp"
#include "drawmgr.hpp"

#endif

#include "profiler.h"
#include "offsets.hpp"
#include "entitycache.h"
#include "hoovy.hpp"
#include "enums.h"
#include "projlogging.hpp"
#include "ucccccp_cmds.hpp"
#include "velocity.hpp"
#include "angles.hpp"
#include "entityhitboxcache.hpp"
#include "globals.h"
#include "helpers.h"
#include "playerlist.hpp"
#include "interfaces.h"
#include "localplayer.h"
#include "conditions.h"
#include "logging.h"
#include "targethelper.h"
#include "playerresource.h"
#include "usercmd.h"
#include "trace.h"
#include "cvwrapper.h"
#include "netvars.h"
#include "vfunc.h"
#include "hooks.h"
#include "prediction.h"
#include "conditions.h"
#include "itemtypes.h"
#include "chatstack.h"
#include "textfile.h"
#include "ipc.h"
#include "hooks/hookedmethods.h"
#include "classinfo/classinfo.hpp"
#include "crits.h"
#include "textmode.hpp"
#include "backpacktf.hpp"

#if ENABLE_GUI
#include "gui/GUI.h"
#endif

#include "hacks/hacklist.h"

#include "sdk.h"

template<typename T>
constexpr T _clamp(T _min, T _max, T _val) {
	return ((_val > _max) ? _max : ((_val < _min) ? _min : _val));
}

#define _FASTCALL __attribute__((fastcall))
#define STRINGIFY(x) #x

#include "gameinfo.hpp"

#define SQR(x) (x) * (x)

#ifndef CATHOOK_BUILD_NUMBER
#define CATHOOK_BUILD_NUMBER "LATEST"
#endif
#define CATHOOK_BUILD_NAME "Cat's Eye"

#define CON_NAME "cat"
#define CON_PREFIX CON_NAME "_"

#define SUPER_VERBOSE_DEBUG false
#if SUPER_VERBOSE_DEBUG == true
#define SVDBG(...) logging::Info(__VA_ARGS__)
#else
#define SVDBG(...)
#endif

#ifndef DEG2RAD
#define DEG2RAD(x) (float)(x) * (PI / 180.0f)
#endif

#define DEBUG_SEGV false
#define STR(c) #c

#if DEBUG_SEGV == true

#define SEGV_BEGIN \
	try {

#define SEGV_END \
	} catch (...) { \
		logging::Info("SEGV/FPE occured! %s in %s:%d", __func__, __FILE__, __LINE__); \
	}

#define SEGV_END_INFO(x) \
	} catch (...) { \
		logging::Info("SEGV/FPE occured! (%s)", x); \
	}

#define SAFE_CALL(x) \
	SEGV_BEGIN \
	x; \
	SEGV_END_INFO(#x)

#else

#define SEGV_BEGIN
#define SEGV_END
#define SEGV_END_INFO(x)
#define SAFE_CALL(x) x

#endif

#define GET_RENDER_CONTEXT (IsTF2() ? g_IMaterialSystem->GetRenderContext() : g_IMaterialSystemHL->GetRenderContext())

#endif /* COMMON_H_ */
