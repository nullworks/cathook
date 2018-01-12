
/*
 *
 *	The esp features header file!
 *
 */

#pragma once
#include "../util/colors.hpp" 	// For colors and colored strings
#include "../util/catvars.hpp"	// I would like to use catvars

#define MAX_ESP_STRINGS 32	  // Strings allowed for esp

namespace features { namespace esp {

extern CatEnum esp_menu;
extern CatVarBool esp_enabled;

void Init(); // Call to init esp

}}
