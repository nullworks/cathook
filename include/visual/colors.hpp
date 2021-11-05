/*
 * colors.hpp
 *
 *  Created on: May 22, 2017
 *      Author: nullifiedcat
 */

#pragma once
#include "config.h"
#if ENABLE_GLEZ_DRAWING
#include "glez/color.hpp"
#endif

class CachedEntity;
#if ENABLE_VISUALS
namespace colors
{
namespace chat
{

constexpr unsigned red = 0xB8383B;
constexpr unsigned blu = 0x5885A2;

constexpr unsigned white     = 0xE6E6E6;
constexpr unsigned purple    = 0x7D4071;
constexpr unsigned yellowish = 0xF0E68C;
constexpr unsigned orange    = 0xCF7336;

constexpr unsigned team(int team)
{
    switch (team)
    {
    case 2:
    {
        return red;
    }
    case 3:
    {
        return blu;
    }
    default:
    {
        return white;
    }
    }
}
} // namespace chat

struct rgba_t
{
    union
    {
        struct
        {
            float r, g, b, a;
        };
        float rgba[4];
    };

    constexpr rgba_t() : r(0.0f), g(0.0f), b(0.0f), a(0.0f){};
    constexpr rgba_t(float _r, float _g, float _b, float _a = 1.0f) : r(_r), g(_g), b(_b), a(_a){};

    explicit rgba_t(const char hex[6]);
#if ENABLE_GLEZ_DRAWING
    constexpr rgba_t(const glez::rgba &other) : r(other.r), g(other.g), b(other.b), a(other.a){};
#endif

#if ENABLE_GLEZ_DRAWING
#if __clang__
    operator glez::rgba() const
    {
        return *reinterpret_cast<const glez::rgba *>(this);
    }
#else
    constexpr operator glez::rgba() const
    {
        return *reinterpret_cast<const glez::rgba *>(this);
    }
#endif
#endif

    constexpr operator bool() const
    {
        return r || g || b || a;
    }

    constexpr operator int() const = delete;

    operator float *()
    {
        return rgba;
    }

    constexpr operator const float *() const
    {
        return rgba;
    }

    constexpr rgba_t operator*(float value) const
    {
        return rgba_t(r * value, g * value, b * value, a * value);
    }
};

constexpr bool operator==(const rgba_t &lhs, const rgba_t &rhs)
{
    return rhs.r == lhs.r && rhs.g == lhs.g && rhs.b == lhs.b && rhs.a == lhs.a;
}

constexpr bool operator!=(const rgba_t &lhs, const rgba_t &rhs)
{
    return !(lhs == rhs);
}

constexpr rgba_t FromRGBA8(float r, float g, float b, float a)
{
    return rgba_t{ r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f };
}

constexpr rgba_t Transparent(const rgba_t &in, float multiplier = 0.5f)
{
    return rgba_t{ in.r, in.g, in.b, in.a * multiplier };
}

constexpr rgba_t white = FromRGBA8(255, 255, 255, 255);
constexpr rgba_t black(0, 0, 0, 1);

constexpr rgba_t pink = FromRGBA8(255, 105, 180, 255);

extern rgba_t red;                                     // RED team user controlled
extern rgba_t blu;                                     // BLU team user controlled
extern rgba_t red_b;                                   // Background
extern rgba_t blu_b;                                   // Background
extern rgba_t red_v;                                   // Vaccinator
extern rgba_t blu_v;                                   // Vaccinator
extern rgba_t red_u;                                   // Ubercharge
extern rgba_t blu_u;                                   // Ubercharge
extern rgba_t gui;                                     // GUI color
extern rgba_t target;                                  // Aimbot target color
constexpr rgba_t red_s  = FromRGBA8(237, 42, 42, 255); // Not user controlled
constexpr rgba_t yellow = FromRGBA8(255, 255, 0, 255);
constexpr rgba_t orange = FromRGBA8(255, 120, 0, 255);
constexpr rgba_t green  = FromRGBA8(0, 255, 0, 255);
constexpr rgba_t empty  = FromRGBA8(0, 0, 0, 0);

constexpr rgba_t FromHSL(float h, float s, float v)
{
    if (s <= 0.0f)
    { // < is bogus, just shuts up warnings
        return rgba_t{ v, v, v, 1.0f };
    }
    float hh = h;
    if (hh >= 360.0f)
        hh = 0.0f;
    hh /= 60.0f;
    long i   = long(hh);
    float ff = hh - i;
    float p  = v * (1.0f - s);
    float q  = v * (1.0f - (s * ff));
    float t  = v * (1.0f - (s * (1.0f - ff)));

    switch (i)
    {
    case 0:
        return rgba_t{ v, t, p, 1.0f };
    case 1:
        return rgba_t{ q, v, p, 1.0f };
    case 2:
        return rgba_t{ p, v, t, 1.0f };
    case 3:
        return rgba_t{ p, q, v, 1.0f };
    case 4:
        return rgba_t{ t, p, v, 1.0f };
    case 5:
    default:
        return rgba_t{ v, p, q, 1.0f };
    }
}
// Made more readable for future reference
constexpr rgba_t Health(int health, int max)
{
    float hf = float(health) / float(max);
    if (hf > 1)
    {
        return colors::FromRGBA8(64, 128, 255, 255);
    }
    rgba_t to_return{ 0.0f, 0.0f, 0.0f, 1.0f };

    // If More than 50% Health Set Red to 100% (Yes the <= 0.5f is "More than 50%" in this case)
    if (hf <= 0.5f)
        to_return.r = 1.0f;
    // Else if Less than 50% health Make red get reduced based on Percentage
    else
        to_return.r = 1.0f - (2.0f * (hf - 0.5f));
    // If More than 50% Health Make the green get increased based on percentage
    if (hf <= 0.5f)
        to_return.g = 2.0f * hf;
    // Else set Green to 100%
    else
        to_return.g = 1.0f;
    return to_return;
}
constexpr rgba_t Health_dimgreen(int health, int max)
{
    float hf = float(health) / float(max);
    if (hf > 1)
    {
        return colors::FromRGBA8(0, 128, 255, 255);
    }
    rgba_t to_return{ 0.0f, 0.0f, 0.0f, 1.0f };

    // If More than 50% Health Set Red to 100% (Yes the <= 0.5f is "More than 50%" in this case)
    if (hf <= 0.5f)
        to_return.r = 1.0f;
    // Else if Less than 50% health Make red get reduced based on Percentage
    else
        to_return.r = 1.0f - (2.0f * (hf - 0.5f));
    // If More than 50% Health Make the green get increased based on percentage
    if (hf <= 0.5f)
        to_return.g = 2.0f * hf * 0.9f;
    // Else set Green to 100%
    else
        to_return.g = 1.0f;
    return to_return;
}
rgba_t RainbowCurrent();
rgba_t Fade(rgba_t color_a, rgba_t color_b, float time, float time_scale = 1.0f);
rgba_t EntityF(CachedEntity *ent);
} // namespace colors

using rgba_t = colors::rgba_t;
#endif
