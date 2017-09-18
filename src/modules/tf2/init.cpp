
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

#include "hooked.hpp"


namespace modules { namespace tf2 {
	
// The startup function for the module
void Init(){
	
	// Please remove unneeded hooks, objects, and interfaces
	modules::tf2::hacks::sharedobj::LoadAllSharedObjects();
	modules::tf2::hacks::CreateInterfaces();
	
	// Pasted strait from the old hack.cpp
	// Clean out unneed shit please
	logging::Info("Begin Hooking for tf2");
	modules::tf2::hacks::panel.Set(modules::tf2::hacks::g_IPanel);
	modules::tf2::hacks::panel.HookMethod((void*)PaintTraverse_hook, modules::tf2::hacks::offsets::PaintTraverse()); // Draw Tick
	modules::tf2::hacks::panel.Apply();

	uintptr_t* clientMode = 0;// Bad way to get clientmode. FIXME [MP]?
	while(!(clientMode = **(uintptr_t***)((uintptr_t)((*(void***)modules::tf2::hacks::g_IBaseClient)[10]) + 1))) { sleep(1); }
	modules::tf2::hacks::clientmode.Set((void*)clientMode);
	modules::tf2::hacks::clientmode.HookMethod((void*)CreateMove_hook, modules::tf2::hacks::offsets::CreateMove()); // World Tick
	modules::tf2::hacks::clientmode.Apply();

	/*modules::tf2::hacks::client.Set(modules::tf2::hacks::g_IBaseClient);
	modules::tf2::hacks::client.HookMethod((void*)IN_KeyEvent_hook, modules::tf2::hacks::offsets::IN_KeyEvent()); // Keypress detection for gui
	modules::tf2::hacks::client.Apply();*/
	logging::Info("Finish Tf2 Hooking!");
	
}
	
}}