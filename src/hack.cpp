/*
 * hack.cpp
 *
 *  Created on: Oct 3, 2016
 *      Author: nullifiedcat
 */

#include "hack.h"

#include <vector>
//#include <map>
#include <cstring>
//#include <fstream>
//#include <iostream>
//#include <sys/prctl.h> // Linux specific headers for dumping and stuff
#include <unistd.h> // Unix like headers for read write and stuff like that
//#include <link.h> // unknown, uncomment if needed
//#include <unordered_map>
#include <memory> // Pointers and ownership???
//#include "segvcatch/segvcatch.h"
#include <csignal>
#include <sys/sysinfo.h>

//#include <steam/isteamuser.h>
//#include <dbg.h>
// All Hacks
//#include "hacks/hacklist.h"

//#include "common.h"
//#include "sharedobj.h"
//#include "hooks.h"
//#include "netmessage.h"
//#include "profiler.h"
//#include "cvwrapper.h"

#define STRINGIFY(x) #x
#define TO_STRING(x) STRINGIFY(x)

//#if ENABLE_VISUALS == 1
//#include "ftrender.hpp"
//#endif

//#include "hooks/hookedmethods.h"
//#include "init.hpp"

//#include "sdk.h"
//#include "vfunc.h"
//#include "copypasted/CSignature.h"
//#include "copypasted/Netvar.h"
//#include "CDumper.h"
//#include <KeyValues.h>

#include "logging.h"
#include "modules/tf2/init.hpp"


/*
 *	The cheats entry point is in entry.cpp, which creates a thread for cathook to use.
 *	From there, the init below here is run
 *
 *
 *
 *
 *
 */

/*
 *  Credits to josh33901 aka F1ssi0N for butifel F1Public and Darkstorm 2015 Linux
 */

bool hack::shutdown = false;

/*const std::string& hack::GetVersion() {
	static std::string version("Unknown Version");
	static bool version_set = false;
	if (version_set) return version;
#if defined(GIT_COMMIT_HASH) && defined(GIT_COMMIT_DATE)
		version = "Version: #" GIT_COMMIT_HASH " " GIT_COMMIT_DATE;
#endif
	version_set = true;
	return version;
}*/

/*const std::string& hack::GetType() {
	static std::string version("Unknown Type");
	static bool version_set = false;
	if (version_set) return version;
	version = "";
#if not defined(ENABLE_IPC)
	version += " NOIPC";
#endif
#if not ENABLE_GUI
		version += " NOGUI";
#else
	version += " IMGUI";
#endif

#ifndef DYNAMIC_CLASSES

#ifdef BUILD_GAME
		version += " GAME " TO_STRING(BUILD_GAME);
#else
		version += " UNIVERSAL";
#endif

#else
		version += " DYNAMIC";
#endif
		
#if not ENABLE_VISUALS
		version += " NOVISUALS";
#endif

	version = version.substr(1);
	version_set = true;
	return version;
}*/

/*std::mutex hack::command_stack_mutex;
std::stack<std::string>& hack::command_stack() {
	static std::stack<std::string> stack;
	return stack;
}*/

/*void hack::ExecuteCommand(const std::string command) {
	std::lock_guard<std::mutex> guard(hack::command_stack_mutex);
	hack::command_stack().push(command);
}*/


//ConCommand* hack::c_Cat = 0;

/*void hack::CC_Cat(const CCommand& args) {
	g_ICvar->ConsoleColorPrintf(Color(255, 255, 255, 255), "cathook");
	g_ICvar->ConsoleColorPrintf(Color(  0,   0, 255, 255), " by ");
	g_ICvar->ConsoleColorPrintf(Color(255,   0,   0, 255), "nullifiedcat\n");
}*/

void hack::Initialize() {
	signal(SIGPIPE, SIG_IGN);
	//time_injected = time(nullptr);

	logging::Info("Initializing...");
	srand(time(0)); // Setting random seed?
	//prctl(PR_SET_DUMPABLE,0,42,42,42); // Lets not change this to prevent detection, its usefull for debugging I think. http://man7.org/linux/man-pages/man2/prctl.2.html
	
	
	// Please put modules here with correct compiler options to load only the ones needed.
	// Please also put compiler options to not compile the modules if they are not needed.
	// Modules need to be initialized first to let the managers know what they can do.
	logging::Info("Loading Modules...");
	
	modules::tf2::Init();
	
	
	
	//logging::Info("Clearing initializer stack");
	/*while (!init_stack().empty()) {
		init_stack().top()();
		init_stack().pop();
	}*/
	//logging::Info("Initializer stack done");

	
	logging::Info("Initializing Done!");
}

// Umm, what??? Does this need to be here?
void hack::Think() {
	usleep(250000);
}

// Do we need this???
void hack::Shutdown() {
	/*if (hack::shutdown) return;
	hack::shutdown = true;
	playerlist::Save();
	DoSDLUnhooking();
	logging::Info("Unregistering convars..");
	ConVar_Unregister();
	logging::Info("Shutting down killsay...");
	hacks::shared::killsay::Shutdown();
	logging::Info("Success..");*/
}
