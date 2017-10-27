/*
 * colors.hpp
 *
 *  Created on: May 22, 2017
 *      Author: nullifiedcat
 */

#pragma once

#include "../framework/entitys.hpp" // We use entitys as an argument for our entity to color

namespace colors {

// Color utils
constexpr CatVector4 FromRGBA8(const CatVector4& color) { return color / 255; }
constexpr CatVector4   ToRGBA8(const CatVector4& color) { return color * 255; }
constexpr CatVector4 Transparent(const CatVector4& color, const float& multiplier = 0.5f) { return CatVector4(color.r, color.g, color.b, color.a * multiplier); }
constexpr CatVector4 FromHSL(const float& h, const float& s, const float& v) {
	if (s <= 0.0) return ToRGBA8(v, v, v, 1.0f);
	float hh = h;
	if (hh >= 360.0) hh = 0.0;
	hh /= 60.0;
	long i = long(hh);
	float ff = hh - i;
	float p = v * (1.0 - s);
	float q = v * (1.0 - (s * ff));
	float t = v * (1.0 - (s * (1.0 - ff)));

	switch(i) {
	case 0:
		return ToRGBA8(v, t, p, 1.0f);
	case 1:
		return ToRGBA8(q, v, p, 1.0f);
	case 2:
		return ToRGBA8(p, v, t, 1.0f);
	case 3:
		return ToRGBA8(p, q, v, 1.0f);
	case 4:
		return ToRGBA8(t, p, v, 1.0f);
	}
	return ToRGBA8(v, p, q, 1.0f);
}

// Colors
constexpr CatVector4 white 	= CatVector4(255, 255, 255, 255);
constexpr CatVector4 black 	= CatVector4(0, 0, 0, 255);
constexpr CatVector4 pink 	= CatVector4(255, 105, 180, 255);
constexpr CatVector4 red 	= CatVector4(237, 42, 42, 255)
constexpr CatVector4 blue 	= CatVector4(28, 108, 237, 255);
constexpr CatVector4 yellow = CatVector4(255, 255, 0, 255);
constexpr CatVector4 orange = CatVector4(255, 120, 0, 255);
constexpr CatVector4 green 	= CatVector4(0, 255, 0, 255);
constexpr CatVector4 empty 	= CatVector4(0, 0, 0, 0);

// Color functions
const CatVector4& Entity(const CatEntity* entity);
void RegisterCustomColorToEntity(const CatVector4&(*func)(const CatEntity*));
CatVector4 Health(const CatEntity* entity);
CatVector4 RainbowCurrent();

// Strings containing a color
class ColoredString {
public:
	ColoredString(const std::string& _string = "", const CatVector4& _color = colors::white) : string(_string), color(_color);
	std::string string;
	rgba_t color;
};
}


