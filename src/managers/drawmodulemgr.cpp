 
/*
 *
 *	Welcome to the draw module manager!
 *	Here is the modular drawing system you can tap into if needed!
 *	It is designed to only require draw line but you may add other specialized functions if needed.
 *	Currently the init functions are unable to save the functions input into them!!!
 *	Drawing will NOT work for the time being.
 *		-Onee
 *
 */

// TODO, add texture func with texture load with drawline
// TODO, make storage pointers = nullptr, remove the static defined and check if the var isnt null ptr

#include <math.h>
#include <stdarg.h>
#include "drawmodulemgr.hpp"

#define PI 3.14159265


namespace drawmgr {
	
// Line
void(*StoredLine)(int, int, int, int, rgba_t) = nullptr;
void Line(int x, int y, int w, int h, rgba_t color) {
	if (StoredLine == nullptr) return; // Check if module has claimed this yet	
	StoredLine(x, y, w, h, color);
}
void InitLine(void *func(int, int, int, int, rgba_t)) {
	StoredLine = func;
}

	
// Outline rect
void(*StoredRect)(int, int, int, int, rgba_t) = nullptr;
void Rect(int x, int y, int w, int h, rgba_t color) {
	if (StoredRect != nullptr) { // Check if module has claimed this yet	
		StoredRect(x, y, w, h, color);
		return;	// We dont want it being drawn again
	}
	if (StoredLine == nullptr) return; // We cant use a work-around it if line isnt even defined...
	
	// Make outline rect with draw line
	Line(x, y, w, 0, color); // Top
	Line(x, y + 1, 0, h, color); // Left
	Line(x + 1, y + h, w - 1, 0, color); // Botton
	Line(x + w, y + 1, 0, h - 1, color); // Right
}
void InitRect(void *func(int, int, int, int, rgba_t)) {
	StoredRect = func; 
}
	
	
// Filled Rect
void(*StoredRectFilled)(int, int, int, int, rgba_t) = nullptr;
void RectFilled(int x, int y, int w, int h, rgba_t color) {
	if (StoredRectFilled != nullptr) {
		StoredRectFilled(x, y, w, h, color);
		return;
	}
	if (StoredLine == nullptr) return;
	
	// Make filled rect with lines
	for (int i = 0; i < w; i++) {
		Line(x + i, y, 0, h, color);
	}
}
void InitRectFilled(void *func(int, int, int, int, rgba_t)) {
	StoredRectFilled = func;
}

	
// Outline circle
void(*StoredCircle)(int, int, float, int, rgba_t) = nullptr;
void Circle(int x, int y, float radius, int steps, rgba_t &color) {
	if (StoredCircle != nullptr) {
		StoredCircle(x, y, radius, steps, color);
		return;
	}
	if (StoredLine == nullptr) return;
	
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
void InitCircle(void *func(int, int, float, int, rgba_t)) {
	StoredCircle = func;
}
	
	
// Keep string stuff tidy
namespace strings {

// String
void(*StoredString)(const char*, int, int, EFont, int, rgba_t) = nullptr;
void String(const char* text, int x, int y, EFont font, int size, rgba_t color) {
	if (StoredString != nullptr) {
		StoredString(text, x, y, font, size, color);
	}
	//if (!line_defined) return; // TODO, make shitty font system with drawline
}
// Crying, but in spanish
void String(std::string text, int x, int y, EFont font, int size, rgba_t color) {
	String(text.c_str(), x, y, font, size, color);
}
void InitString(void *func(const char*, int, int, EFont, int, rgba_t)) {
	StoredString = func;
}

// String length in pixels
void(*StoredStringLength)(const char*, EFont, int, int&, int&) = nullptr;
void GetStringLength(const char* string, EFont font, int size, int& length, int& height) {
	if (StoredString != nullptr && StoredStringLength != nullptr) {
		StoredStringLength(string, font, size, length, height);
		return;
	}
	//if (!string_defined && !line_defined) return; // Use the crappy font rendering workaround
}
void InitStringLength(void *func(const char*, EFont, int, int&, int&)) {
	StoredStringLength = func;
}
}
	
	
// Other stuff that doesnt belong to either namespace but is needed
namespace other {
	
// World to screen, this is performed as a module option as the game can do this for us, but In the future, I would like to be able to do this fully in the cheat
bool(*StoredWorldToScreen)(const CatVector&, CatVector&) = nullptr;
bool WorldToScreen(const CatVector& world, CatVector& screen) {
	if (StoredWorldToScreen != nullptr) {
		return StoredWorldToScreen(world, screen);
	}
	return false; // We cant do this ourself quite yet sadly...
}
void InitWorldToScreen(void *func(const CatVector&, CatVector&)) {
	StoredWorldToScreen = func;
}
	
// A reset command run every draw tick. Some graphics may require one through its not always needed.
void(*StoredReset)() = nullptr;
void Reset() {
	if (StoredReset != nullptr) {
		return StoredReset();
	}
	return;
}
void InitReset(void *func()) {
	StoredReset = func;
}
	
}}



