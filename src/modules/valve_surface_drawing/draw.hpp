 

#include "../../managers/drawmodulemgr.hpp"
#include "../../util/colors.hpp"
#include "../Source_2013_SDK/sdk.h"

// allows us to convert from rgba into something valve surface understands
constexpr int convert(rgba_t color) {
	unsigned char _r = (static_cast<int>(color.r)) & 0xFF;// I understand ur pain of me casting int from float, deal with it.
	unsigned char _g = (static_cast<int>(color.g)) & 0xFF;
	unsigned char _b = (static_cast<int>(color.b)) & 0xFF;
	unsigned char _a = (static_cast<int>(color.a)) & 0xFF;
	return (int)(_r) | (int)(_g << 8) | (int)(_b << 16) | (int)(_a << 24);
}
constexpr int convertint(int r, int g, int b, int a) {
	unsigned char _r = (r) & 0xFF;// I understand ur pain of me casting int from float, deal with it.
	unsigned char _g = (g) & 0xFF;
	unsigned char _b = (b) & 0xFF;
	unsigned char _a = (a) & 0xFF;
	return (int)(_r) | (int)(_g << 8) | (int)(_b << 16) | (int)(_a << 24);
}

namespace modules { namespace valvesurface { 

void Line(int x, int y, int w, int h, rgba_t color);
void Rect(int x, int y, int w, int h, rgba_t color);
void RectFilled(int x, int y, int w, int h, rgba_t color);
void String1(const char* text, int x, int y, EFont font, int size, rgba_t color);
void StringLength(const char* string, EFont font, int size, int& length, int& height);

extern vgui::ISurface* g_ISurface;
}}