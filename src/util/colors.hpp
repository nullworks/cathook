/*
 * colors.hpp
 *
 *  Created on: May 22, 2017
 *      Author: nullifiedcat
 */

#ifndef COLORS_HPP_
#define COLORS_HPP_

namespace colors {

struct rgba_t {
	union {
		struct {
			float r, g, b, a;
		};
		float rgba[4];
	};

	constexpr rgba_t() : r(0.0f), g(0.0f), b(0.0f), a(0.0f) {};
	constexpr rgba_t(float _r, float _g, float _b, float _a = 1.0f) : r(_r), g(_g), b(_b), a(_a) {};

	constexpr operator bool() const {
		return r || g || b || a;
	}

	constexpr operator int() const = delete;

	operator float*() {
		return rgba;
	}

	constexpr operator const float*() const {
		return rgba;
	}

	constexpr rgba_t operator*(float value) const {
		return rgba_t(r * value, g * value, b * value, a * value);
	}

};
constexpr rgba_t FromRGBA8(float r, float g, float b, float a) {
	return rgba_t { r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f };
}
constexpr rgba_t Transparent(const rgba_t& in, float multiplier = 0.5f) {
	return rgba_t { in.r, in.g, in.b, in.a * multiplier };
}
constexpr rgba_t FromHSL(float h, float s, float v) {
	if(s <= 0.0) {       // < is bogus, just shuts up warnings
		return rgba_t { v, v, v, 1.0f };
	}
	float hh = h;
	if(hh >= 360.0) hh = 0.0;
	hh /= 60.0;
	long i = long(hh);
	float ff = hh - i;
	float p = v * (1.0 - s);
	float q = v * (1.0 - (s * ff));
	float t = v * (1.0 - (s * (1.0 - ff)));

	switch(i) {
	case 0:
		return rgba_t { v, t, p, 1.0f };
	case 1:
		return rgba_t { q, v, p, 1.0f };
	case 2:
		return rgba_t { p, v, t, 1.0f };
	case 3:
		return rgba_t { p, q, v, 1.0f };
		break;
	case 4:
		return rgba_t { t, p, v, 1.0f };
	case 5:
	default:
		return rgba_t { v, p, q, 1.0f };
	}
}
rgba_t RainbowCurrent();
}
using rgba_t = colors::rgba_t;

#endif /* COLORS_HPP_ */
