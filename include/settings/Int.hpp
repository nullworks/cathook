/*
  Created on 01.07.18.
*/

#pragma once

#include "Settings.hpp"

namespace settings
{

template <> class Variable<int> : public ArithmeticVariable<int>
{
public:
    ~Variable() override = default;

    VariableType getType() override
    {
        return VariableType::INT;
    }

    void fromString(const std::string &string, bool = false) override
    {
        errno       = 0;
        auto result = std::strtol(string.c_str(), nullptr, 10);
        if (result == 0 && errno)
            return;
        set(result);
    }

    inline void operator=(const std::string &string)
    {
        fromString(string);
    }

    inline void operator=(const int &next)
    {
        set(next);
    }

    inline explicit operator bool() const
    {
        return value != 0;
    }
};
} // namespace settings
