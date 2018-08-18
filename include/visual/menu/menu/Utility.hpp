/*
  Created by Jenny White on 01.05.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#pragma once

#include <string>
#include <glez/font.hpp>
#include <glez/glez.hpp>
#include <glez/color.hpp>

namespace utility
{

std::string wrapString(const std::string &input, glez::font &font, int width,
                       int *resultWidth, int *resultLines);
std::string dotCompactString(const std::string &input, glez::font &font,
                             int width, bool reverse);
void drawCenteredString(int x, int y, const std::string &string,
                        glez::font &font, glez::rgba color);

template <typename T> T mod(T first, T second);

template <> inline float mod(float first, float second)
{
    return std::fmod(first, second);
}
template <> inline int mod(int first, int second)
{
    return first % second;
}

extern const std::string empty_string;
} // namespace utility