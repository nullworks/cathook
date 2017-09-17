/*
 * colors.hpp
 *
 *  Created on: May 22, 2017
 *      Author: nullifiedcat
 */

#ifndef COLORS_HPP_
#define COLORS_HPP_

class CachedEntity;

namespace colors {

namespace chat {

constexpr unsigned red = 0xB8383B;
constexpr unsigned blu = 0x5885A2;

constexpr unsigned white = 0xE6E6E6;
constexpr unsigned purple = 0x7D4071;
constexpr unsigned yellowish = 0xF0E68C;
constexpr unsigned orange = 0xCF7336;

constexpr unsigned team(int team) {
	if (team == 2) return red;
	if (team == 3) return blu;
	return white;
}

}

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

constexpr rgba_t white = FromRGBA8(255, 255, 255, 255);
constexpr rgba_t black(0, 0, 0, 1);

constexpr rgba_t pink = FromRGBA8(255, 105, 180, 255);

constexpr rgba_t red = FromRGBA8(237, 42, 42, 255), blu = FromRGBA8(28, 108, 237, 255);
constexpr rgba_t red_b = FromRGBA8(64, 32, 32, 178),  blu_b = FromRGBA8(32, 32, 64, 178);  // Background
constexpr rgba_t red_v = FromRGBA8(196, 102, 108, 255),  blu_v = FromRGBA8(102, 182, 196, 255);  // Vaccinator
constexpr rgba_t red_u = FromRGBA8(216, 34, 186, 255),  blu_u = FromRGBA8(167, 75, 252, 255);  // Ubercharged
constexpr rgba_t yellow = FromRGBA8(255, 255, 0, 255);
constexpr rgba_t orange = FromRGBA8(255, 120, 0, 255);
constexpr rgba_t green = FromRGBA8(0, 255, 0, 255);
constexpr rgba_t empty = FromRGBA8(0, 0, 0, 0);

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
constexpr rgba_t Health(int health, int max) {
	float hf = float(health) / float(max);
	if (hf > 1) {
		return colors::FromRGBA8(64, 128, 255, 255);
	}
	return rgba_t { (hf <= 0.5f ? 1.0f : 1.0f - 2.0f * (hf - 0.5f)), (hf <= 0.5f ? (2.0f * hf) : 1.0f), 0.0f, 1.0f };
}
rgba_t RainbowCurrent();
rgba_t EntityF(CachedEntity* ent);

}

using rgba_t = colors::rgba_t;

#endif /* COLORS_HPP_ */
