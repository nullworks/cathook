
/*
 *
 *	This is what draws the strings on the top left of the screen!
 *
 *
 */

#include "../../framework/gameticks.hpp"	// To init our sidestrings
#include "../../framework/game.hpp"		// Module authors
#include "../gui.hpp"					// guicolor

#include "sidestrings.hpp"

namespace gui { namespace sidestrings {

HudString SideStrings(8, 8);// Stores side strings

// The main string to use at the top
std::string top_string = "Nekohook";

// Adds default strings
static void AddDefault() {
	// Init strings
	static bool init = false;
	if (!init) {

		// Default to cathook
		top_string = "Nekohook";

		// Check if we have authors
		if (!g_GameInfo.authors.empty()) {
			top_string += " by ";
			for (int i = 0; i < g_GameInfo.authors.size(); i++) {
				const std::string& tmp = g_GameInfo.authors.at(i);
				// Determine whether we should add a comma
				if (i > 0) {
					if (i - 2 >= g_GameInfo.authors.size())
						top_string += " and ";
					else
						top_string += ", ";
				}
				top_string += tmp; // Add our author
			}
		}
		// Suqqe
		init = true;
	}

	// Our default strings
	SideStrings.AddString(top_string, colors::RainbowCurrent());
#if defined(GIT_COMMIT_HASH) && defined(GIT_COMMIT_DATE)
	SideStrings.AddString("Version: #" GIT_COMMIT_HASH " " GIT_COMMIT_DATE, colors::pink);
#endif
}

void Init() {
	// To Get our default strings up there
	drawmgr_before(AddDefault);
	// To draw our strings
	drawmgr_after([](){SideStrings.Draw();});
}

}}
