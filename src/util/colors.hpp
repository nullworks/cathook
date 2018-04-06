/*
 * colors.hpp
 *
 *  Created on: May 22, 2017
 *      Author: nullifiedcat
 */

#pragma once

#include "functional.hpp" // CMFunction
#include "../framework/entitys.hpp" // We use entitys as an argument for our entity to color

// Color classes
// This is to replace CatVector4, it uses floats, we can make things faster with integers. I will remove this comment eventually.
class CatColor {
public:
	inline CatColor(int _r = 255, int _g = 255, int _b = 255, int _a = 255) : r(_r), g(_g), b(_b), a(_a) {}
	inline bool operator==(CatColor value) const {return r == value.r && g == value.g && b == value.b && a == value.a;}
	inline CatColor Transparent(float multiplier = 0.5f /*= 255 / 2*/) const { return CatColor(r, g, b, a * multiplier); } // need better way to do this
	int r, g, b, a;
};

namespace colors {

// Color utils
inline CatColor ToRGBA8(float r, float g, float b, float a) { return CatColor(r * 255, g * 255, b * 255, a * 255); }
inline CatColor FromHSL(float h, float s, float v) { // Color wheel func, lookup hsl for info, credits goto cathook for this
	if (s <= 0.0) return ToRGBA8(v, v, v, 1.0f);
	if (h >= 360.0) h = 0.0;
	h /= 60.0;
	long i = long(h);
	float ff = h - i;
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
const CatColor white = CatColor();
const CatColor black = CatColor(0, 0, 0);
const CatColor pink = CatColor(255, 105, 180);
const CatColor red = CatColor(237, 42, 42);
const CatColor blue = CatColor(28, 108, 237);
const CatColor yellow = CatColor(255, 255, 0);
const CatColor orange = CatColor(255, 120, 0);
const CatColor green = CatColor(0, 255, 0);
const CatColor gray = CatColor(100, 100, 100);
const CatColor lightgray = CatColor(180, 180, 180);
const CatColor darkgray = CatColor(50, 50, 50);
const CatColor empty = CatColor(0, 0, 0, 0);

// Color functions
extern CMFunction<CatColor(CatEntity*)> EntityColor;

// Returns a color based on entity health
inline CatColor Health(CatEntity* entity) {
	if (GetHealth(entity) > GetMaxHealth(entity))	// If health is too much, they must be over their normal health so we make them blue
		return CatColor(64, 128);

	// Percentage of health our of max
	float hf = (float)GetHealth(entity) / (float)GetMaxHealth(entity);
	// Sick logic
	return CatColor((hf <= 0.5 ? 1.0 : 1.0 - 2 * (hf - 0.5)) * 255, (hf <= 0.5 ? (2 * hf) : 1) * 255, 0);
}

CatColor RainbowCurrent();
CatColor RandomColor();

}
