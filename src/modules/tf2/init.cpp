
/*
 *	Init file for tf2 module
 *	Please keep everything in the module in its own namespace!
 *	
 */

#include "../../logging.h"	
#include "hacks/hooks.h"
#include "hacks/sharedobj.h"
#include "hacks/interfaces.h"
#include "offsets.hpp"

#include "../valve_surface_drawing/init.hpp" // Put compiler option here for graphics modules

#include "../xlib_input/init.hpp"			// Another compiler option for input module

#include "hooked.hpp"


namespace modules { namespace tf2 {
	
// The startup function for the module
void Init(){
	
	logging::Info("Begin Team Fortress 2 module init...");
	// Please remove unneeded hooks, objects, and interfaces
	sharedobj::LoadAllSharedObjects();
	CreateInterfaces();
	
	// Pasted strait from the old hack.cpp
	// Clean out unneed shit please
	logging::Info("Begin tf2 Hooking...");
	// Panel
	hooks::panel.Set(g_IPanel);
	hooks::panel.HookMethod((void*)PaintTraverse_hook, offsets::PaintTraverse()); // Draw Tick
	hooks::panel.Apply();
	// Client mode
	uintptr_t* clientMode = 0;// Bad way to get clientmode. FIXME [MP]?
	while(!(clientMode = **(uintptr_t***)((uintptr_t)((*(void***)g_IBaseClient)[10]) + 1))) { sleep(1); }
	hooks::clientmode.Set((void*)clientMode);
	hooks::clientmode.HookMethod((void*)CreateMove_hook, offsets::CreateMove()); // World Tick
	hooks::clientmode.Apply();
	logging::Info("Finish tf2 Hooking!");
	
	// Put possible graphics modules here. 
	// Put compiler options here to allow user choice
	logging::Info("Loading graphics module...");
	modules::valvesurface::Init(g_ISurface);
	
	// TODO, add more compiler options
	logging::Info("Loading input module...");
	modules::xlibinput::Init();
	
	
	logging::Info("Finish Team Fortress 2 module init!");
	
}
	
}}