/*
  Created on 07.07.18.
*/

#pragma once

#include <glez/font.hpp>
#include <glez/color.hpp>

namespace zerokernel
{

class TextComponent
{
public:
    TextComponent(glez::font &font);

    void render(const std::string &string, glez::rgba color,
                glez::rgba outline);
    // Uses default colors
    void render(const std::string &string);

    void move(int x, int y);

    int x{};
    int y{};
    int padding_x{};
    int padding_y{};
    int size_x{};
    int size_y{};

    glez::font &font;
    const glez::rgba &default_text;
    const glez::rgba &default_outline;
};
} // namespace zerokernel