/*
  Created on 01.07.18.
*/

#pragma once

#include "Settings.hpp"

namespace settings
{

template <> class Variable<float> : public ArithmeticVariable<float>
{
public:
    ~Variable() override = default;

    VariableType getType() override
    {
        return VariableType::FLOAT;
    }

    void fromString(const std::string &string, bool = false) override
    {
        errno     = 0;
        auto next = std::strtof(string.c_str(), nullptr);
        if (next == 0.0f && errno)
            return;
        set(next);
    }

    inline void operator=(const std::string &string)
    {
        fromString(string);
    }

    inline void operator=(const float &next)
    {
        set(next);
    }

    inline explicit operator bool() const
    {
        return value != 0.0f;
    }

protected:
    void updateString() override
    {
        char str[32];
        snprintf(str, 31, "%.2f", value);
        string = str;
    }
};
} // namespace settings
