
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
inline const std::string top_string(){
	using namespace game;
	std::string ret = "Nekohook";

	// Check if we have authors
	if (authors.empty())
		return ret;

	ret += " by ";
	for (size_t i = 0; i < authors.size(); i++) {
		ret += authors.at(i);
		if (i != authors.size() - 1)
			ret += (i == authors.size() - 2) ? " and " : ", ";
	}
	return ret;
}

// Adds default strings
static void AddDefault() {
	// Our default strings
	SideStrings.AddString(top_string(), colors::RainbowCurrent());
#if defined(GIT_COMMIT_HASH) && defined(GIT_COMMIT_DATE)
	SideStrings.AddString("Version: #" GIT_COMMIT_HASH " " GIT_COMMIT_DATE, colors::pink);
#endif
}

void Init() {
	// To Get our default strings up there
	drawmgr.REventBefore(AddDefault);
	// To draw our strings
	drawmgr.REventDuring([](){SideStrings.Draw();});
}

}}
