
/*
 *
 *	Welcome to the draw manager!
 *	This handles everything that would like to draw in the cheat.
 *	It should be very easy to use!
 *		-Onee
 *
 */

#include "gameticks.hpp"

// Stores functions that request to be called at times during draw
CMEventGroup drawmgr;

// Stored pointers of functions to run on world tick
CMEventGroup wtickmgr;
