
/*
 *
 * Header for the draw manager.
 *
 *
 */

#pragma once

#include "../util/functions.hpp" // CMFunction

extern CMFunctionGroup drawmgr[3];
inline void drawmgr_before(void_func in) { drawmgr[0] + in; }
inline void drawmgr_on(void_func in) { drawmgr[1] + in; }
inline void drawmgr_after(void_func in) { drawmgr[2] + in; }
void drawmgr_run(); // So that a hooked function from a module can call this

extern CMFunctionGroup wtickmgr[3];
inline void wtickmgr_before(void_func in) { wtickmgr[0] + in; }
inline void wtickmgr_on(void_func in) { wtickmgr[1] + in; }
inline void wtickmgr_after(void_func in) { wtickmgr[2] + in; }
void wtickmgr_run();
