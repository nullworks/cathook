
/*
 *
 *
 *	This contains a class for drawing strings at points on the screen with ease
 *
 *
 */

#pragma once

#include <vector>

#include "../../util/colors.hpp" // Draw stuff
#include "../../framework/drawing.hpp"

namespace gui {

// TODO, impliment into gui when mouse movement becomes possible
class HudString {
public:
	// Constructor
	inline HudString(int x, int y, bool _center = false) : center(_center), draw_point(std::make_pair(x, y)) {}
	inline void AddString(const char* input_string, const CatVector4& input_color) {
		strings.push_back(std::make_pair(input_string, input_color));
	};
	inline void Draw() {
		int y = draw_point.second;
		for (const auto& str : strings) {
			auto str_size = draw::GetStringLength(str.first, draw::default_font, draw::default_font_size);
			draw::String(str.first, (!center) ? draw_point.first : draw_point.first - str_size.first / 2, y, draw::default_font, draw::default_font_size, str.second);
			y += str_size.second + 1;
		}
		strings.clear();
	}
private:
	const bool center;
	std::pair<int, int> draw_point;
	// we use a char array as it is faster than std::string in our use
	std::vector<std::pair<const char*, CatVector4>> strings;
};

}
