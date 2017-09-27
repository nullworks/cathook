
/*
 *
 *	This is what draws the strings on the top left of the screen!
 *	
 *
 */

#include "sidestrings.hpp"
#include "../../managers/drawmodulemgr.hpp"
#include "../gui.hpp"			// guicolor
#include "../../logging.h"

namespace gui { namespace sidestrings {
HudStrings side_strings;// Stores side strings
void HudStrings::AddString(const std::string& input_string, const rgba_t& input_color) {
	if (count >= 32) return; // To prevent overflow 
	string[count] = input_string;
	color[count] = input_color;
	++count;
}

// Draw strings in the object
void DrawSideStrings() {
	int y = 8;	// TODO, make this change depending on screen size.
	int tmp, tmp2;
	for (int i = 0; i < side_strings.count; i++) {
		drawmgr::strings::String(side_strings.string[i].c_str(), 8, y, OPENSANS, 30, side_strings.color[i]); // TODO, make gui use user defined font as well as size
		drawmgr::strings::GetStringLength(side_strings.string[i].c_str(), OPENSANS, 30, tmp2, tmp);
		y += tmp + 1;
	}
	side_strings.count = 0; // Clear out the object
}

void DefaultSideStrings() {
	logging::Info("Color: %f, %f, %f, %f", colors::RainbowCurrent().r, colors::RainbowCurrent().g, colors::RainbowCurrent().b, colors::RainbowCurrent().a);
	side_strings.AddString("Cathook", colors::RainbowCurrent());
}
}}