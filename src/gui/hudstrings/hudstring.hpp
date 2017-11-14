
/*
 *
 *
 *	This contains a class for drawing strings at points on the screen with ease
 *
 *
 */

#pragma once

namespace gui {
	
class HudString {
public:
	// Constructor
	HudString(int x, int y, bool _center = false);
	void AddString(const std::string& input_string, const CatVector4& input_color = colors::white);
	void Draw();
	const bool center; 											// Whether to center the strings
	std::pair<int, int> draw_point;								// Where to draw them
	std::vector<std::pair<std::string, CatVector4>> strings;	// The strings themselves
};

}