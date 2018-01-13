
/*
 *	The cheats entry point is in entry.cpp, which creates a thread for cathook to use.
 *	From there, the init below here is run
 *
 */

#include <chrono> // For time so we can sleep
#include <thread> // Sleep in thread^
#include <ctime> // time()
#include <stdlib.h> // srand()

#include "util/logging.hpp" // To log progress

// TODO, ifdef this to shutup errors
// Game Modules
#include "modules/Source_2013/init.hpp"
#include "modules/csgo/init.hpp"
// Features
#include "features/features.hpp"
// GUI
#include "gui/init.hpp"

#include "hack.h"

void hack::Initialize() {

	// We wait to allow the constructors to finish before we initialize
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	g_CatLogging.log("Initializing...");
	srand(time(0)); // Setting random seed?

	g_CatLogging.log("Initializing main cheat features...");
	features::Init();

	g_CatLogging.log("Initializing gui...");
	gui::Init(); // init the gui

	// Please put only one GAME module here. Any other type of module can screw with things.
	// Please also put compiler options to not compile the modules that are not needed.
	// Modules need to be initialized first to let the managers know what they can do.
	g_CatLogging.log("Loading Modules...");
#if defined(CATHOOK_SOURCE_2013) 	// Any source 2013 game
	modules::source::Init();
#endif
#if defined(CATHOOK_CSGO) 	// CSGO
	modules::csgo::Init();
#endif


	g_CatLogging.log("Initializing Done!");
}
