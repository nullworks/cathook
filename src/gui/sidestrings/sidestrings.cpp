
/*
 *
 *	This is what draws the strings on the top left of the screen!
 *	
 *
 */

#include "sidestrings.hpp"
#include "../../framework/drawing.hpp"
#include "../gui.hpp"			// guicolor
#include "../../util/logging.h"
#include "../../framework/game.hpp"

// TODO, Remake this entire mess

namespace gui { namespace sidestrings {
	
HudStrings side_strings;// Stores side strings
void HudStrings::AddString(const std::string& input_string, const rgba_t& input_color) {
	if (string_count >= MAX_SIDESTRINGS) return; // To prevent overflow 
	strings[string_count] = colors::ColoredString(input_string, input_color);
	string_count++;
}

// Draw strings in the object
void DrawSideStrings() {
	int y = 8;	// TODO, make this change depending on screen size.
	static int tmp, tmp2;
	for (int i = 0; i < side_strings.string_count; i++) {
		draw::String(side_strings.strings[i].string.c_str(), 8, y, OPENSANS, 20, side_strings.strings[i].color); // TODO, make gui use user defined font as well as size
		draw::GetStringLength(side_strings.strings[i].string.c_str(), OPENSANS, 20, tmp2, tmp);
		y += tmp + 1;
	}
	side_strings.string_count = 0; // Clear out the object
}
	
std::string top_string = "Cathook";
void DefaultSideStrings() {
	side_strings.AddString(top_string, colors::RainbowCurrent());
#if defined(GIT_COMMIT_HASH) && defined(GIT_COMMIT_DATE)
	side_strings.AddString("Version: #" GIT_COMMIT_HASH " " GIT_COMMIT_DATE, gui::GUIColor());
#endif
}

// Rebuilds the top string with our authors included
void BuildTopString() {
	// Default to cathook
	top_string = "Cathook";
	
	// Check if we have authors
	if (!g_GameInfo.authors.empty()) {
		top_string += " by ";
		int count = 0;
		for (std::string& tmp : g_GameInfo.authors) {
			// Determine whether we should add a comma
			if (count > 0) {
				if (count - 2 >= g_GameInfo.authors.size())
					top_string += " and ";
				else top_string += ", ";
			}
			top_string += tmp; // Add our author
			count++;
		}
	}
}

}}