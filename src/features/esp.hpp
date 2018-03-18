
/*
 *
 *	The esp features header file!
 *
 */

#pragma once

#include "../util/functions.hpp"
#include "../util/colors.hpp" 	// For colors and colored strings
#include "../util/catvars.hpp"	// I would like to use catvars

#define MAX_ESP_STRINGS 32	  // Strings allowed for esp

namespace features::esp {

extern CatEnum esp_menu;

extern std::vector<CMFunction<std::pair<const char*, CatVector4>(CatEntity*)>> GetEntityStrings;

void Init(); // Call to init esp

}
