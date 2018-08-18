/*
  Created on 07.07.18.
*/

#pragma once

#include <string>
#include <glez/font.hpp>

namespace zerokernel
{

class StaticTextComponent
{
public:
    StaticTextComponent(const std::string &string, glez::font &font);

    int x{};
    int y{};
    int padding_x{};
    int padding_y{};
    int size_x{};
    int size_y{};

    const std::string string;
    glez::font &font;
};
} // namespace zerokernel