
/*
 *
 *	Header for draw manager
 *
 *
 */

#pragma once

#include <functional> // std::pair(), std::function

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

extern std::function<void(const int&, const int&, const int&, const int&, const CatVector4&)> Line;
extern std::function<void(const int&, const int&, const int&, const int&, const CatVector4&)> Rect;
extern std::function<void(const int&, const int&, const int&, const int&, const CatVector4&)> RectFilled;
extern std::function<void(const int&, const int&, const float&, const int&, const CatVector4&)> Circle;

extern std::function<void(const char* text, const int& x, const int& y, const int& font, const int& size, const CatVector4& color)> String;
extern std::function<std::pair<int, int>(const char* string, const int& font, const int& size)> GetStringLength;

extern std::function<bool(const CatVector&, CatVector&)> WorldToScreen;

// Fonts, first is the pretty name and the second is the real name
extern const char* Fonts[];
}
