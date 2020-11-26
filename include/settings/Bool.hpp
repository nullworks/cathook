/*
  Created on 01.07.18.
*/

#pragma once

#include "Settings.hpp"
#include "Manager.hpp"

namespace settings
{

template <> class Variable<bool> : public VariableBase<bool>
{
public:
    ~Variable() override = default;

    Variable() = default;

    explicit Variable(bool v)
    {
        setInternal(v);
    }

    VariableType getType() override
    {
        return VariableType::BOOL;
    }

    void fromString(const std::string &string, bool = false) override
    {
        if (string == "0" || string == "false")
            setInternal(false);
        else if (string == "1" || string == "true")
            setInternal(true);
    }

    inline void operator=(const std::string &string)
    {
        fromString(string);
    }

    inline const std::string &toString() override
    {
        return string;
    }

    inline explicit operator bool() const
    {
        return value;
    }

    inline void operator=(bool next)
    {
        setInternal(next);
    }

    inline const bool &operator*() override
    {
        return value;
    }

    inline void flip()
    {
        setInternal(!value);
    }

protected:
    void setInternal(bool next)
    {
        if (next == value)
            return;
        fireCallbacks(next);
        value  = next;
        string = value ? "true" : "false";
    }

    bool value{ false };
    std::string string{ "false" };
};
} // namespace settings
