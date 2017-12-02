/*
 * gameinfo.hpp
 *
 *  Created on: May 11, 2017
 *      Author: nullifiedcat
 */

#ifndef GAMEINFO_HPP_
#define GAMEINFO_HPP_

#include "common.hpp"

#ifdef BUILD_GAME

constexpr bool IsTF2() { return !c_strcmp(TO_STRING(BUILD_GAME), "tf2"); }
constexpr bool IsTF2C() { return !c_strcmp(TO_STRING(BUILD_GAME), "tf2c"); }
constexpr bool IsHL2DM() { return !c_strcmp(TO_STRING(BUILD_GAME), "hl2dm"); }
constexpr bool IsCSS() { return !c_strcmp(TO_STRING(BUILD_GAME), "css"); }
constexpr bool IsDynamic() { return !c_strcmp(TO_STRING(BUILD_GAME), "dynamic"); }

constexpr bool IsTF() { return IsTF2() || IsTF2C(); }

// This one is supposed to be `if constexpr` but I have to upgrade to gcc7
#define IF_GAME(x) if (x)

#else
inline bool IsTF2() { return g_AppID == 440; }
inline bool IsTF2C() { return g_AppID == 243750; }
inline bool IsHL2DM() { return g_AppID == 320; }
inline bool IsCSS() { return g_AppID == 240; }
constexpr bool IsDynamic() { return false; }

inline bool IsTF() { return IsTF2() || IsTF2C(); }

#define IF_GAME(x) if (x)

#endif

#endif /* GAMEINFO_HPP_ */
