
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

// Framework stuff
#include "framework/config.hpp"
#include "framework/prediction.hpp"
#include "features/features.hpp"
#include "gui/init.hpp"

// TODO, ifdef this to shutup errors
// Game Modules
#if defined(CATHOOK_SOURCE_2013)
#include "modules/source2013/init.hpp"
#elif defined(CATHOOK_CSGO)
#include "modules/csgo/init.hpp"
#endif

#include "hack.h"

void hack::Initialize() {

	// We wait to allow the constructors to finish before we initialize
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	g_CatLogging.log("Initializing...");
	srand(time(0)); // Setting random seed?

	g_CatLogging.log("Initializing main cheat features...");
	features::Init();

	// Please put only one GAME module here. Any other type of module can screw with things.
	// Please also put compiler options to not compile the modules that are not needed.
	// Modules need to be initialized first to let the managers know what they can do.
	g_CatLogging.log("Loading Modules...");
#if defined(CATHOOK_SOURCE_2013) 	// Any source 2013 game
	modules::source::Init();
#elif defined(CATHOOK_CSGO) 	// CSGO
	modules::csgo::Init();
#endif

	//GUI NEEDS	TO INIT AFTER GAMES TO DO FONTS RIGHT
	g_CatLogging.log("Initializing gui...");
	gui::Init(); // init the gui

	// Load configs
	configs::Init();
	pred::Init();

	g_CatLogging.log("Initializing Done!");
}
