 
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

#include <math.h>

#include "colors.hpp"

namespace colors {

// Currently broken without a way to get time. Replace "50" with a float of time in seconds.
rgba_t RainbowCurrent() {
	return colors::FromHSL(fabs(sin(50 / 2.0f)) * 360.0f, 0.85f, 0.9f);
}
	
}