/*
  Created on 01.07.18.
*/

#pragma once

#include "Settings.hpp"

constexpr uint8_t hexToInt(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    else if (c >= 'a' && c <= 'f')
        return 10 + c - 'a';
    else if (c >= 'A' && c <= 'F')
        return 10 + c - 'A';
    return 0;
}

namespace settings
{

template <> class Variable<rgba_t> : public VariableBase<rgba_t>
{
public:
    ~Variable() override = default;

    VariableType getType() override
    {
        return VariableType::COLOR;
    }

    void fromString(const std::string &string, bool = false) override
    {
        uint8_t rgba[4] = { 0, 0, 0, 255 };

        for (int i = 0; i < string.length() && i < 8; ++i)
        {
            if (!(i % 2))
                rgba[i / 2] = 0;
            rgba[i / 2] |= (hexToInt(string[i]) << ((i % 2) ? 0 : 4));
        }

        rgba_t next{};
        next.r = float(rgba[0]) / 255.f;
        next.g = float(rgba[1]) / 255.f;
        next.b = float(rgba[2]) / 255.f;
        next.a = float(rgba[3]) / 255.f;
        setInternal(next);
    }

    inline void operator=(const rgba_t &rgba)
    {
        setInternal(rgba);
    }

    inline void operator=(const std::string &string)
    {
        fromString(string);
    }

    inline const std::string &toString() override
    {
        return string;
    }
    inline const rgba_t &operator*() override
    {
        return value;
    }

protected:
    void setInternal(rgba_t next)
    {
        fireCallbacks(next);
        value               = next;
        uint8_t rgba_int[4] = { uint8_t(next.r * 255.f), uint8_t(next.g * 255.f), uint8_t(next.b * 255.f), uint8_t(next.a * 255.f) };
        for (int i = 0; i < 4; ++i)
        {
            string[i * 2]     = "0123456789abcdef"[(rgba_int[i] >> 4u) & 0xF];
            string[i * 2 + 1] = "0123456789abcdef"[rgba_int[i] & 0xF];
        }
    }

    rgba_t value{};
    std::string string{ "00000000" };
};
} // namespace settings
