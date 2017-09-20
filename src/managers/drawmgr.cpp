 
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

#include "drawmgr.hpp"

#define PI 3.14159265


namespace drawmgr {

// Line
static bool line_defined = false;
void *StoredLine(int, int, int, int, rgba_t);
void Line(int x, int y, int w, int h, rgba_t color) {
	if (!line_defined) return; // Check if module has claimed this yet	
	StoredLine(x, y, w, h, color);
}
void InitLine (void *func(int, int, int, int, rgba_t)) {
	line_defined = true;
	//*StoredLine = *func;
}

	
// Outline rect
static bool rect_defined = false;
void *StoredRect(int, int, int, int, rgba_t);
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
	//StoredRect = func; 
}
	
	
// Filled Rect
static bool rect_filled_defined = false;
void *StoredRectFilled(int, int, int, int, rgba_t);
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
void InitRectFilled (void(*func)(int, int, int, int, rgba_t)) {
	rect_filled_defined = true;
	//*StoredRectFilled = *func;
}

	
// Outline circle
static bool circle_defined = false;
void *StoredCircle(int, int, float, int, rgba_t);
void Circle(int x, int y, float radius, int steps, rgba_t color) {
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
void InitCircle (void(*func)(int, int, float, int, rgba_t)) {
	circle_defined = true;
	//StoredCircle = func;
}
	
// Keep string stuff tidy
namespace strings {
	

	
}}