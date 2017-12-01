
/*
 *	This is the worldtick area. This is for when the world processes a tick or something related.
 *		Here we will do things such as aimbot, triggerbot and other related cheats as well as cacheing for the playermgr.
 *			-Onee
 *
 */

#include <vector>

#include "entitys.hpp"		// Stuff to reset

#include "worldtickmgr.hpp"

namespace worldtick {

// Stored pointers of functions to run on world tick
typedef void(*funcptr)(void);
std::vector<funcptr> before_wtick_functions;
std::vector<funcptr> wtick_functions;
std::vector<funcptr> after_wtick_functions;

// Please call this while the game is in a level.
void WorldTickRun() {

	// Used for stuff that refill the entity mgr and related
	for (const auto& func : before_wtick_functions) {
		func();
	}

	// Cheat features
	for (const auto& func : wtick_functions) {
		func();
	}

	// Frameworks features
	for (const auto& func : after_wtick_functions) {
		func();
	}
}

// Input a void function to request the worldtickmgr to run it at times
void RequestBeforeWTick(void(*func)(void)) {
	before_wtick_functions.push_back(func);
}

void RequestWTick(void(*func)(void)) {
	wtick_functions.push_back(func);
}

void RequestAfterWTick(void(*func)(void)) {
	after_wtick_functions.push_back(func);
}

}
