
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

// Font enums
enum {
	OPENSANS,
	VERASANS,
	UNISPACE,
	TF2BUILD,	// Le meme
	FONT_COUNT
};

namespace draw {

extern CMFunction<void(int, int, int, int, const CatVector4&)> Line;
extern CMFunction<void(int, int, int, int, const CatVector4&)> Rect;
extern CMFunction<void(int, int, int, int, const CatVector4&)> RectFilled;
extern CMFunction<void(int, int, float, int, const CatVector4&)> Circle;

extern CMFunction<void(const char* text, int x, int y, int font, int size, const CatVector4& color)> String;
extern CMFunction<std::pair<int, int>(const char* string, int font, int size)> GetStringLength;

extern CMFunction<bool(const CatVector&, CatVector&)> WorldToScreen;

// Fonts, first is the pretty name and the second is the real name
extern const char* Fonts[];

}
