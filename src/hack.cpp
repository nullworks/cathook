
/*
 *	The cheats entry point is in entry.cpp, which creates a thread for cathook to use.
 *	From there, the init below here is run
 *
 */


#include <csignal> // Unknown

#include "util/logging.h"
#include "shutdown.hpp"

// Game Modules
#if defined(CATHOOK_SOURCE_2013)
#include "modules/Source_2013/init.hpp"
#endif
// Features
#include "features/features.hpp"
// GUI
#include "gui/init.hpp"

#include "hack.h"

bool hack::shutdown = false;

void hack::Initialize() {
	//signal(SIGPIPE, SIG_IGN);	// Is this needed?

	CatLogging("Initializing...");
	srand(time(0)); // Setting random seed?
	
	// Please put only one GAME module here. Any other type of module can screw with things.
	// Please also put compiler options to not compile the modules that are not needed.
	// Modules need to be initialized first to let the managers know what they can do.
	CatLogging("Loading Modules...");
#if defined(CATHOOK_SOURCE_2013) 	// Any source 2013 game
	modules::source::Init();
#endif
	
	CatLogging("Initializing main cheat features...");
	features::Init();
	
	CatLogging("Initializing gui...");
	gui::Init(); // init the gui
	
	CatLogging("Initializing Done!");
}

// Do we need this???
void hack::Shutdown() {
	if (hack::shutdown) return;
	hack::shutdown = true;
	
	RunShutdown();

	CatLogging("Shutdown Success..");
}
