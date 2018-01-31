
/*
 *
 *	Stuff for hud strings
 *
 */

#include "../../framework/drawing.hpp"

#include "hudstring.hpp"

namespace gui {

HudString::HudString(int x, int y, bool _center) : draw_point(std::make_pair(x, y)), center(_center) {}

void HudString::AddString(const std::string& input_string, const CatVector4& input_color) {
	strings.push_back(std::make_pair(input_string, input_color));
}

void HudString::Draw() {
	int y = draw_point.second;
	for (const auto& str : strings) {

		auto str_size = draw::GetStringLength(str.first.c_str(), draw::default_font, draw::default_font_size);

		if (!center)
			draw::String(str.first.c_str(), draw_point.first, y, draw::default_font, draw::default_font_size, str.second); // TODO, make gui use user defined font as well as size
		else
			draw::String(str.first.c_str(), draw_point.first - (str_size.first / 2), y, draw::default_font, draw::default_font_size, str.second);

		y += str_size.second + 1;
	}
	strings.clear(); // Clear out the object
}

}
