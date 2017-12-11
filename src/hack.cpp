
/*
 *	The cheats entry point is in entry.cpp, which creates a thread for cathook to use.
 *	From there, the init below here is run
 *
 */

#include <ctime> // time()
#include <stdlib.h> // srand()

#include "util/logging.hpp" // To log progress

// Game Modules
#include "modules/Source_2013/init.hpp"
// Features
#include "features/features.hpp"
// GUI
#include "gui/init.hpp"

#include "hack.h"

void hack::Initialize() {
	//signal(SIGPIPE, SIG_IGN);	// Is this needed?

	g_CatLogging.log("Initializing...");
	srand(time(0)); // Setting random seed?

	// Please put only one GAME module here. Any other type of module can screw with things.
	// Please also put compiler options to not compile the modules that are not needed.
	// Modules need to be initialized first to let the managers know what they can do.
	g_CatLogging.log("Loading Modules...");
#if defined(CATHOOK_SOURCE_2013) 	// Any source 2013 game
	modules::source::Init();
#endif

	g_CatLogging.log("Initializing main cheat features...");
	features::Init();

	g_CatLogging.log("Initializing gui...");
	gui::Init(); // init the gui

	g_CatLogging.log("Initializing Done!");
}
