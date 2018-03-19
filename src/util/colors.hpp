/*
 * colors.hpp
 *
 *  Created on: May 22, 2017
 *      Author: nullifiedcat
 */

#pragma once

#include "functions.hpp" // CMFunction
#include "../framework/entitys.hpp" // We use entitys as an argument for our entity to color

namespace colors {

// Color utils
inline CatVector4 FromRGBA8(CatVector4 color) { return color / 255; }
inline CatVector4   ToRGBA8(CatVector4 color) { return color * 255; }
inline CatVector4   ToRGBA8(float r, float g, float b, float a) { return CatVector4(r * 255, g * 255, b * 255, a * 255); }
inline CatVector4 Transparent(CatVector4 color, float multiplier = 0.5f) { return CatVector4(color.x, color.y, color.z, color.a * multiplier); }
inline CatVector4 FromHSL(float h, float s, float v) {
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
const CatVector4 white 	= CatVector4(255, 255, 255, 255);
const CatVector4 black 	= CatVector4(0, 0, 0, 255);
const CatVector4 pink 	= CatVector4(255, 105, 180, 255);
const CatVector4 red 		= CatVector4(237, 42, 42, 255);
const CatVector4 blue 	= CatVector4(28, 108, 237, 255);
const CatVector4 yellow = CatVector4(255, 255, 0, 255);
const CatVector4 orange = CatVector4(255, 120, 0, 255);
const CatVector4 green 	= CatVector4(0, 255, 0, 255);
const CatVector4 gray   = CatVector4(100, 100, 100, 255);
const CatVector4 empty 	= CatVector4(0, 0, 0, 0);

// Color functions
extern CMFunction<CatVector4(CatEntity*)> EntityColor;

// Returns a color based on entity health
inline CatVector4 Health(CatEntity* entity) {
	if (GetHealth(entity) > GetMaxHealth(entity))	// If health is too much, they must be over their normal health so we make them blue
		return CatVector4(64, 128, 255, 255);

	// Percentage of health our of max
	float hf = (float)GetHealth(entity) / (float)GetMaxHealth(entity);
	// Sick logic
	return CatVector4((hf <= 0.5 ? 1.0 : 1.0 - 2 * (hf - 0.5)) * 255, (hf <= 0.5 ? (2 * hf) : 1) * 255, 0, 255);
}

CatVector4 RainbowCurrent();

}
