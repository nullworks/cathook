
/*
 *
 *	Header for draw manager
 *
 *
 */

#pragma once

#include <functional> // std::pair()

#include "../util/functions.hpp" // CMFunction
#include "../util/mathlib.hpp" // CatVector4
#include "../util/catvars.hpp" // CatVars

// Font enums
enum {
	OPENSANS,
	VERASANS,
	UNISPACE,
	FONT_COUNT
};

namespace draw {

extern CatVarEnum default_font;
extern CatVarInt default_font_size;

extern CMFunction<void(int, int, int, int, CatVector4)> Line;
extern CMFunction<void(int, int, int, int, CatVector4)> Rect;
extern CMFunction<void(int, int, int, int, CatVector4)> RectFilled;
extern CMFunction<void(int, int, float, int, CatVector4)> Circle;
extern CMFunction<void(int, int, float, int, CatVector4)> CircleFilled;

extern CMFunction<void(const char*, int, int, int, int, CatVector4)> String;
extern CMFunction<std::pair<int, int>(const char*, int , int)> GetStringLength;

extern CMFunction<bool(CatVector, CatVector&)> WorldToScreen;

// Fonts, first is the pretty name and the second is the real name
extern const char* Fonts[];

}
