
/*
 *
 * Header for the draw manager.
 *
 *
 */

#pragma once

#include "../util/functional.hpp" // CMEventGroup

// Stores functions that request to be called at times during draw
extern CMEventGroup drawmgr;

// Stored pointers of functions to run on world tick
extern CMEventGroup wtickmgr;
