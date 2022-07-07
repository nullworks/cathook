/*
  Created on 25.06.18.
*/

#pragma once

#include <config.h>
#include <string>
#include <functional>
#include <atomic>

#if ENABLE_VISUALS
#include <drawing.hpp>
#endif

/*

 Value types:
    - bool
    - int
    - float
    - string
    - rgba

 Interface:
    from/to String
    get, set
    on change callback

    increment / decrement

    set max string length

    set default value
    reset to default

 */

namespace settings
{
extern std::atomic<bool> cathook_disabled;
enum class VariableType
{
    BOOL,
    INT,
    FLOAT,
    STRING,
    COLOR,
    KEY
};

class IVariable
{
public:
    virtual ~IVariable() = default;

    // Faster than checking with dynamic_cast
    virtual VariableType getType()                                        = 0;
    virtual void fromString(const std::string &string, bool init = false) = 0;
    // Const reference because every variable will cache the string value
    // instead of generating it every call
    virtual const std::string &toString() = 0;
};

template <typename T> class VariableBase : public IVariable
{
public:
    using type = T;

    ~VariableBase() override = default;

    virtual const T &operator*() = 0;

    void installChangeCallback(std::function<void(VariableBase<T> &var, T after)> callback)
    {
        callbacks.push_back(callback);
    }

protected:
    void fireCallbacks(T next)
    {
        for (auto &c : callbacks)
            c(*this, next);
    }

    std::vector<std::function<void(VariableBase<T> &, T)>> callbacks{};
};

template <typename T> class Variable
{
};

template <typename T> class ArithmeticVariable : public VariableBase<T>
{
public:
    ~ArithmeticVariable() override = default;

    explicit inline operator T() const
    {
        return value;
    }
    const inline std::string &toString() override
    {
        return string;
    }
    const inline T &operator*() override
    {
        return value;
    }

    // T min{ std::numeric_limits<T>::min() };
    // T max{ std::numeric_limits<T>::max() };

    virtual void set(T next)
    {
        // if (next < min) next = min;
        // if (next > max) next = max;
        if (next != value || string.empty())
        {
            this->fireCallbacks(next);
            value = next;
            this->updateString();
        }
    }

protected:
    T value{};
    std::string string{};

    virtual void updateString()
    {
        string = std::to_string(value);
    }
};
} // namespace settings

#include "Bool.hpp"
#include "Float.hpp"
#include "Int.hpp"
#include "String.hpp"
#include "Key.hpp"
#include "Registered.hpp"

#if ENABLE_VISUALS
#include "Rgba.hpp"
#endif
