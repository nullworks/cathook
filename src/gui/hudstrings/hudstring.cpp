 
/*
 *
 *	Stuff for hud strings
 *
 */

#include "../../framework/drawing.hpp"

#include "hudstring.hpp"

namespace gui {
	
HudString(int x, int y, bool _center) draw_point(std::make_pair(x, y)), center(_center) {}

void HudString::AddString(const std::string& input_string, const CatVector4& input_color) {
	strings.push_back(std::make_pair(input_string, input_color));
}	

void HudString::Draw() {
	int y = draw_point.second;
	for (const auto& str : strings) {
		
		int length, height;
		draw::GetStringLength(side_strings.strings[i].string.c_str(), OPENSANS, 20, tmp2, tmp);
		
		if (!center)
			draw::String(str.first.c_str(), draw_point.first, y, OPENSANS, 20, str.second); // TODO, make gui use user defined font as well as size
		else
			draw::String(str.first.c_str(), draw_point.first - (length / 2), y, OPENSANS, 20, str.second);
		
		y += tmp + 1;
	}
	side_strings.clear(); // Clear out the object
}
	
}