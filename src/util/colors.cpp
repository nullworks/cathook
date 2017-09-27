 
/*
 *
 *	This is the colors file, its used for everything related to color!
 *	Right now, go find the nearest walmart and buy some spray paint.
 * 	Spray the paint into your mouth. 
 *	You are now officially gay or suicidal, your choice!
 *		-Onee
 *
 *
 */

#include <math.h>	// fabs() sin()
#include <chrono>	// For time keeping

#include "colors.hpp"

namespace colors {

// Currently broken without a way to get time.
// on inject make "time injected" chrono and save it. get duration since make and cast to count. use as time for rainbow.
// have it be in seconds in a float with lesser miliseconds be used as 
rgba_t RainbowCurrent() {
	return colors::FromHSL(50, 0.85f, 0.9f);	// Remove this once u get it working
	static std::chrono::steady_clock::time_point time_point = std::chrono::steady_clock::now();
	float curtime = std::chrono::duration_cast<std::chrono::milliseconds>(time_point.time_since_epoch()).count() * 0.001f;
	return colors::FromHSL(fabs(sin(curtime / 2.0f)) * 360.0f, 0.85f, 0.9f);
}
	
}