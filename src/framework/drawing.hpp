
/*
 *
 *	Header for draw manager
 *
 *
 */

#pragma once

#include <functional> // std::pair()

#include "../util/functional.hpp" // CMFunction
#include "../util/colors.hpp" // CatColor
#include "../util/catvars.hpp" // CatVars

// Font enums
enum {
	OPENSANS,
	VERASANS,
	UNISPACE,
	FONT_COUNT
};

namespace draw {

// Use these when trying to draw strings to be synced with the framework
// It is optional so you can make small text if needed
extern CatVarEnum default_font;
extern CatVarInt default_font_size;

// Check drawing.cpp for proper args
extern CMFunction<void(int, int, int, int, CatColor)> Line;
extern CMFunction<void(int, int, int, int, CatColor)> Rect;
extern CMFunction<void(int, int, int, int, CatColor)> RectFilled;
extern CMFunction<void(int, int, float, int, CatColor)> Circle;
extern CMFunction<void(int, int, float, int, CatColor)> CircleFilled;

// Draw string and get length, returns length of widthxheight
extern CMFunction<void(const char*, int, int, int, int, CatColor)> String;
extern CMFunction<std::pair<int, int>(const char*, int , int)> GetStringLength;

extern CMFunction<bool(CatVector, std::pair<int, int>&)> WorldToScreen;

// Fonts, they should be the true name of fonts so u can iterate and load them(hopefully)
extern const char* Fonts[];

}
