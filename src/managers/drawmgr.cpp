 
/*
 *
 *	Welcome to the draw manager!
 *	Here is the modular drawing system you can tap into if needed!
 *	It is designed to only require draw line but you may add other specialized functions if needed.
 *	Currently the init functions are unable to save the functions input into them!!!
 *	Drawing will NOT work for the time being.
 *		-Onee
 *
 */

#include <math.h>
#include <stdarg.h>
#include "drawmgr.hpp"

#define PI 3.14159265


namespace drawmgr {

// Dont use more than needed, credits to cat for giving this to me
typedef void(*fn_name_int)(int, ...); 
typedef void(*fn_name_const_char)(const char*, ...);
typedef bool(*fn_name_catvector)(const CatVector&, ...);	
	
// Line
static bool line_defined = false;
void(*StoredLine)(int, int, int, int, rgba_t);
void Line(int x, int y, int w, int h, rgba_t color) {
	if (!line_defined) return; // Check if module has claimed this yet	
	StoredLine(x, y, w, h, color);
}
void InitLine (void *func(int, int, int, int, rgba_t)) {
	line_defined = true;
	StoredLine = func;
}

	
// Outline rect
static bool rect_defined = false;
void(*StoredRect)(int, int, int, int, rgba_t);
void Rect(int x, int y, int w, int h, rgba_t color) {
	if (rect_defined) { // Check if module has claimed this yet	
		StoredRect(x, y, w, h, color);
		return;	// We dont want it being drawn again
	}
	if (!line_defined) return; // We cant use a work-around it if line isnt even defined...
	
	// Make outline rect with draw line
	Line(x, y, w, 0, color); // Top
	Line(x, y + 1, 0, h, color); // Left
	Line(x + 1, y + h, w - 1, 0, color); // Botton
	Line(x + w, y + 1, 0, h - 1, color); // Right
	
}
void InitRect (void *func(int, int, int, int, rgba_t)) {
	rect_defined = true;
	StoredRect = func; 
}
	
	
// Filled Rect
static bool rect_filled_defined = false;
void(*StoredRectFilled)(int, int, int, int, rgba_t);
void RectFilled(int x, int y, int w, int h, rgba_t color) {
	if (rect_filled_defined) {
		StoredRectFilled(x, y, w, h, color);
		return;
	}
	if (!line_defined) return;
	
	// Make filled rect with lines
	for (int i = 0; i < w; i++) {
		Line(x + i, y, w, h, color);
	}
}
void InitRectFilled (void *func(int, int, int, int, rgba_t)) {
	rect_filled_defined = true;
	StoredRectFilled = func;
}

	
// Outline circle
static bool circle_defined = false;
void(*StoredCircle)(int, int, float, int, rgba_t);
void Circle(int x, int y, float radius, int steps, rgba_t &color) {
	if (circle_defined) {
		StoredCircle(x, y, radius, steps, color);
		return;
	}
	if (!line_defined) return;
	
	// Draw a circle with lines
	if (radius <= 0 || steps <= 3) return; // cant draw a cirle without specific parameters
	
	float px = 0;
	float py = 0;
	for (int i = 0; i < steps; i++) {
		float ang = 2 * PI * (i / steps);
		float dx = x + radius * cos(ang);
		float dy = y + radius * sin(ang);
		if (!i) {
			ang = 2 * PI * ((steps - 1) / steps);
			px = x + radius * cos(ang);
			py = y / 2 + radius * sin(ang);
		}
		Line(px, py, dx - px, dy - py, color);
		px = dx;
		py = dy;
	}	
}
void InitCircle (void *func(int, int, float, int, rgba_t)) {
	circle_defined = true;
	StoredCircle = func;
}
	
	
// Keep string stuff tidy
namespace strings {

// String
static bool string_defined = false;
void(*StoredString)(const char*, int, int, EFont, rgba_t);
void String(const char* text, int x, int y, EFont font, rgba_t color) {
	if (string_defined) {
		StoredString(text, x, y, font, color);
	}
	//if (!line_defined) return; // TODO, make shitty font system with drawline
}
// Crying, but in spanish
void String(std::string text, int x, int y, EFont font, rgba_t color) {
	String(text.c_str(), x, y, font, color);
}
void InitString (void *func(const char*, int, int, EFont, rgba_t)) {
	string_defined = true;
	StoredString = func;
}

// String length in pixels
static bool string_length_defined = false;
void(*StoredStringLength)(const char*, EFont, int&, int&);
void GetStringLength(const char* string, EFont font, int& length, int& height) {
	if (string_defined && string_length_defined) {
		StoredStringLength(string, font, length, height);
		return;
	}
	//if (!string_defined && !line_defined) return; // Use the crappy font rendering workaround
}
void InitStringLength (void *func(const char*, int, int, EFont, rgba_t)) {
	string_length_defined = true;
	StoredStringLength = func;
}
}
	
	
// Other stuff that doesnt belong to either namespace but is needed
namespace other {
	
// World to screen, this is performed as a module option as the game can do this for us, but In the future, I would like to be able to do this fully in the cheat
static bool world_to_screen_defined = false;
bool(*StoredWorldToScreen)(const CatVector&, CatVector&);
bool WorldToScreen(const CatVector& world, CatVector& screen) {
	if (world_to_screen_defined) {
		return StoredWorldToScreen(world, screen);
	}
	return false; // We cant do this ourself quite yet sadly...
}
void InitWorldToScreen (void *func(const CatVector&, CatVector&)) {
	world_to_screen_defined = true;
	StoredWorldToScreen = func;
}
	
}}



/*
	// Check if newlined
	bool newlined = false;
	size_t len = strlen(text);
	for (int i = 0; i < len; i++) {
		if (text[i] == '\n') {
			newlined = true; break;
		}
	}
	// Correct for newlines?
	if (newlined) {
		char ch[512];
		memset(ch, 0, sizeof(char) * 512);
		int w, h;
		GetStringLength("W", font, w, h);
		strncpy(ch, text, 511);
		int s = 0;
		int n = 0;
		for (int i = 0; i < len; i++) {
			if (ch[i] == '\n') {
				ch[i] = 0;
				draw::String(font, x, y + n * (h), color, shadow, &ch[0] + s);
				n++;
				s = i + 1;
			}
		}
		draw::String(font, x, y + n * (h), color, shadow, &ch[0] + s);
	} else {
		wchar_t string[512];
		memset(string, 0, sizeof(wchar_t) * 512);
		mbstowcs(string, text, 511);
		draw::WString(font, x, y, color, shadow, string);
	}*/


/*wchar_t buf[512];
	memset(buf, 0, sizeof(wchar_t) * 512);
	mbstowcs(buf, string, strlen(string));
	g_ISurface->GetTextSize(font, buf, length, height);*/

