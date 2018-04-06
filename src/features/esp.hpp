
/*
 *
 *	The esp features header file!
 *
 */

#pragma once

#include "../util/functional.hpp"
#include "../util/colors.hpp" 	// For colors and colored strings
#include "../util/catvars.hpp"	// I would like to use catvars

#define MAX_ESP_STRINGS 32	  // Strings allowed for esp

namespace features::esp {

extern CatEnum esp_menu;

// Add your own strings to make esp fetch them
extern std::vector<std::pair<const char*, CatColor>(*)(CatEntity*)> GetEntityStrings;

void Init(); // Call to init esp

}
