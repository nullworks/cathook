 
/*
 *
 *	Welcome to the draw manager!
 *	This handles everything that would like to draw in the cheat.
 *	It should be very easy to use!
 *		-Onee
 *
 */
#include <mutex>

#include "drawmgr.hpp"

namespace drawmgr {
std::mutex drawing_mutex;// Help prevent multithreadding call this multiple times
HudStrings side_strings;// Stores side strings
	
void DrawSideStrings() {
	int y = 8;	// TODO, make this change depending on screen size.
	int tmp, tmp2;
	for (int i = 0; i < side_strings.count; i++) {
		strings::String(side_strings.string[i].c_str(), 8, y, OPENSANS, 30, side_strings.color[i]); // TODO, make gui use user defined font as well as size
		strings::GetStringLength(side_strings.string[i].c_str(), OPENSANS, 30, tmp2, tmp);
		y += tmp + 1;
	}
}
	
// This should be run on drawtick
void DrawTick(){
	std::lock_guard<std::mutex> draw_lock(drawing_mutex);	// multithreadding fix
	
	// Reset things
	other::Reset();
	side_strings.Reset();
	
	side_strings.AddString("Cathook", rgba_t(100, 100, 100, 255));
	
	//RectFilled(40, 40, 40, 40, rgba_t(100, 100, 100, 255));
	DrawSideStrings();// Do this last
}
	
void HudStrings::AddString(const std::string& input_string, const rgba_t& input_color) {
	string[count] = input_string;
	color[count] = input_color;
	++count;
}
void HudStrings::Reset() {
	count = 0;
}

}

	
