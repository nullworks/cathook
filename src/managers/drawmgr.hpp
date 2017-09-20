 
#include "../util/colors.hpp"

namespace drawmgr {
	
// Drawing Functions
void Line(int x, int y, int w, int h, rgba_t color);
void Rect(int x, int y, int w, int h, rgba_t color);
void RectFilled(int x, int y, int w, int h, rgba_t color);
void Circle(int x, int y, float radius, int steps, rgba_t color);
	
// Init Functions, these are fucked rn!
void InitLine (void *func(int, int, int, int, rgba_t));
void InitRect (void *func(int, int, int, int, rgba_t));
void InitRectFilled (void(*func)(int, int, int, int, rgba_t));
void InitCircle (void(*func)(int, int, float, int, rgba_t));

	
}