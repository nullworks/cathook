
/*
 *
 *	Welcome to the draw manager!
 *	This handles everything that would like to draw in the cheat.
 *	It should be very easy to use!
 *		-Onee
 *
 */

#include <mutex>

#include "gameticks.hpp"

// Stores functions that request to be called at times during draw
CMFunctionGroup drawmgr[3];

// Help prevent multithreadding call this multiple times
static std::mutex drawing_mutex;

// This should be run on drawtick
void drawmgr_run() {
	std::lock_guard<std::mutex> draw_lock(drawing_mutex);	// multithreadding fix?

	// Reset things. We prepare drawing stuff for other input asked from request
	drawmgr[0]();

	// During draw, esp and stuff should be done here!
	drawmgr[1]();

	// After Draw, finalizing and stuff.
	drawmgr[2]();
}

// Stored pointers of functions to run on world tick
CMFunctionGroup wtickmgr[3];

// Please call this while the game is in a level. Hopefully...
void wtickmgr_run() {

	// Used for stuff that refill the entity mgr and related
	wtickmgr[0]();

	// Cheat features
	wtickmgr[1]();

	// Frameworks features
	wtickmgr[2]();
}
