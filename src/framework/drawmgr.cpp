 
/*
 *
 *	Welcome to the draw manager!
 *	This handles everything that would like to draw in the cheat.
 *	It should be very easy to use!
 *		-Onee
 *
 */

#include <mutex>
#include <vector>

#include "drawmgr.hpp"

namespace drawmgr {
std::mutex drawing_mutex;// Help prevent multithreadding call this multiple times

// Stores functions that request to be called at times during draw
typedef void(*funcptr)(void);
std::vector<funcptr> before_draw_functions;
std::vector<funcptr> draw_functions;
std::vector<funcptr> after_draw_functions;
	
// This should be run on drawtick
void DrawTick() {
	std::lock_guard<std::mutex> draw_lock(drawing_mutex);	// multithreadding fix?
	
	// Reset things. We prepare drawing stuff for other input asked from request
	for (const auto& func : before_draw_functions) {
		func();
	}
	
	// During draw, esp and stuff should be done here!
	for (const auto& func : draw_functions) {
		func();
	}
	
	// After Draw, finalizing and stuff.
	for (const auto& func : after_draw_functions) {
		func();
	}
}
// Input your function to request that draw mgr runs your function during a stage of draw.
void RequestDrawOnBefore(void(*func)(void)) {
	before_draw_functions.push_back(func);
}
void RequestDrawOnDraw(void(*func)(void)) {
	draw_functions.push_back(func);
}
void RequestDrawOnAfter(void(*func)(void)) {
	after_draw_functions.push_back(func);
}
}

	
