
/*
 *
 *	Header for draw manager
 *
 *
 */

#pragma once

#include <functional> // std::pair()

#include "../util/mathlib.hpp" // CatVector4

class CDraw {
public:

// Line
virtual void Line(const int& x, const int& y, const int& w, const int& h, const CatVector4& color);
// Outline rect
virtual void Rect(const int& x, const int& y, const int& w, const int& h, const CatVector4& color);
// Filled Rect
virtual void RectFilled(const int& x, const int& y, const int& w, const int& h, const CatVector4& color);
// Outline circle
virtual void Circle(const int& x, const int& y, const float& radius, const int& steps, const CatVector4& color);

// String
virtual void String(const char* text, const int& x, const int& y, const int& font, const int& size, const CatVector4& color);
// String length in pixels
virtual std::pair<int, int> GetStringLength(const char* string, const int& font, const int& size);

};

// Font enums
enum {
	OPENSANS,
	VERASANS,
	UNISPACE,
	TF2BUILD,	// Le meme
	FONT_COUNT
};

namespace draw {

// Object that we use to draw with
extern CDraw drawmachine;
// Functions that act like defines to make things clean
inline void Line(const int& x, const int& y, const int& w, const int& h, const CatVector4& color) { drawmachine.Line(x, y, w, h, color); }
inline void Rect(const int& x, const int& y, const int& w, const int& h, const CatVector4& color) { drawmachine.Rect(x, y, w, h, color); }
inline void RectFilled(const int& x, const int& y, const int& w, const int& h, const CatVector4& color) { drawmachine.RectFilled(x, y, w, h, color); }
inline void Circle(const int& x, const int& y, const float& radius, const int& steps, const CatVector4& color) { drawmachine.Circle(x, y, radius, steps, color); }
inline void String(const char* text, const int& x, const int& y, const int& font, const int& size, const CatVector4& color) { drawmachine.String(text, x, y, font, size, color); }
inline std::pair<int, int> GetStringLength(const char* string, const int& font, const int& size) { return drawmachine.GetStringLength(string, font, size); }

// Fonts, first is the pretty name and the second is the real name
extern const char* Fonts[];

// The main world to screen function used by most of the cheats esp features
bool WorldToScreen(const CatVector&, CatVector&);
void InitWorldToScreen(bool(*func)(const CatVector&, CatVector&));
}
