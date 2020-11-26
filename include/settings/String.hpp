/*
  Created on 01.07.18.
*/

#pragma once

#include "Settings.hpp"

namespace settings
{

template <> class Variable<std::string> : public VariableBase<std::string>
{
public:
    ~Variable() override = default;

    VariableType getType() override
    {
        return VariableType::STRING;
    }

    void fromString(const std::string &string, bool = false) override
    {
        fireCallbacks(string);
        value = string;
    }

    inline const std::string &toString() override
    {
        return value;
    }

    inline void operator=(const std::string &string)
    {
        fireCallbacks(std::string(string));
        value = string;
    }

    inline const std::string &operator*() override
    {
        return value;
    }

    inline explicit operator bool() const
    {
        return !value.empty();
    }

protected:
    std::string value{};
};
} // namespace settings
