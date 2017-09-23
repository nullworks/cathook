/*
 * hack.cpp
 *
 *  Created on: Oct 3, 2016
 *      Author: nullifiedcat
 */

#include <unistd.h> // Unix like headers for read write and stuff like that
#include <csignal> // Unknown

#include "logging.h"
#include "shutdown.hpp"

#include "hack.h"

// Game Modules
#include "modules/tf2/init.hpp"

/*
 *	The cheats entry point is in entry.cpp, which creates a thread for cathook to use.
 *	From there, the init below here is run
 *
 */

bool hack::shutdown = false;

void hack::Initialize() {
	signal(SIGPIPE, SIG_IGN);
	//time_injected = time(nullptr);

	logging::Info("Initializing...");
	srand(time(0)); // Setting random seed?
	
	// Please put only one GAME module here. Any other type of module can screw with things.
	// Please also put compiler options to not compile the modules that are not needed.
	// Modules need to be initialized first to let the managers know what they can do.
	logging::Info("Loading Modules...");
	
	modules::tf2::Init();
	
	logging::Info("Modules Done!");
	
	logging::Info("Initializing Done!");
}

// Umm, what??? Does this need to be here?
void hack::Think() {
	usleep(250000);
}

// Do we need this???
void hack::Shutdown() {
	if (hack::shutdown) return;
	hack::shutdown = true;
	
	RunShutdown();

	logging::Info("Shutdown Success..");
}
