
/*
 *
 *	Welcome to the draw module manager!
 *	Here is the modular drawing system you can tap into if needed!
 *	It is designed to only require draw line but you may add other specialized functions if needed.
 *		-Onee
 *
 */

// TODO, add texture func with texture load with drawline

#include <math.h>
#include <cstring> // strlen

#include "../util/mathlib.hpp" // Contains pi defines

#include "drawing.hpp"

namespace draw {

// Line
CMFunction<void(int, int, int, int, CatColor)> Line;

// Outline rect
CMFunction<void(int, int, int, int, CatColor)> Rect {
	[] (int x, int y, int w, int h, CatColor color) {
	// Make outline rect with draw line
	Line(x, y, w, 0, color); // Top
	Line(x, y + 1, 0, h, color); // Left
	Line(x + 1, y + h, w - 1, 0, color); // Botton
	Line(x + w, y + 1, 0, h - 1, color); // Right
}};

// Filled Rect
CMFunction<void(int, int, int, int, CatColor)> RectFilled {
	[] (int x, int y, int w, int h, CatColor color) {
	// Make filled rect with lines
	for (int i = 0; i < w; i++) {
		Line(x + i, y, 0, h, color);
	}
}};

// Outline circle
CMFunction<void(int, int, float, int, CatColor)> Circle {
	[](int x, int y, float radius, int steps, CatColor color) {
	if (radius < 0 || steps <= 3) return; // cant draw a cirle without specific parameters
	float px = 0;
	float py = 0;
	for (int i = 0; i < steps; i++) {
		float ang = 2 * PI * (float(i) / steps);
		float dx = x + radius * cos(ang);
		float dy = y + radius * sin(ang);
		if (!i) {
			ang = 2 * PI * (float(steps - 1) / steps);
			px = x + radius * cos(ang);
			py = y + radius * sin(ang);
		}
		Line(px, py, dx - px, dy - py, color);
		px = dx;
		py = dy;
	}
}};

// Filled circle
CMFunction<void(int, int, float, int, CatColor)> CircleFilled {
	[](int x, int y, float radius, int steps, CatColor color) {
	if (radius < 0) return; // cant draw a cirle without specific parameters
	// Interate Left and right
	for (int i = 0; i < radius; i++) {
		// Up and down
		for (int ii = radius; ii >= 0; ii--) {
			// Check if our distance is within the radius
			if (sqrt(pow(i, 2) + pow(ii, 2)) > radius) continue;
			Line(x - i, y - ii, 0, ii * 2, color); // Left line
			Line(x + i, y - ii, 0, ii * 2, color); // Right line
			break;
		}
	}
}};

// String
CMFunction<void(const char*, int, int, int, int, CatColor)> String {
	[](const char* text, int x, int y, int font, int size, CatColor color) {}};

// String length in pixels
CMFunction<std::pair<int, int>(const char*, int, int)> GetStringLength {
	[](const char* string, int font, int size) { return std::make_pair((int)strlen(string), size); }}; // a psudo string len, so spacings dont get ruined

// The main world to screen function used by most of the cheats esp features
// New breakthrough, we can potentially make our own wts function to utilize view matrixes, I didnt know this is used across many games.
// TODO, impliment that ^
CMFunction<bool(CatVector, std::pair<int, int>&)> WorldToScreen {
	[](CatVector, std::pair<int, int>&) { return false; }};

const char* Fonts[] = {
	"Open Sans",
	"Bitstream Vera Sans Mono",
	"Unispace"
};
static CatEnum gui_menu({"GUI"});
static CatEnum default_font_enum({Fonts[0], Fonts[1], Fonts[2]});
CatVarEnum default_font(gui_menu, default_font_enum, "gui_font", 0, "Gui font", "The main font setting");
CatVarInt default_font_size(gui_menu, "gui_font_size", 13, "Gui font size", "The main font size");

}
