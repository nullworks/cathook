/*
  Created on 02.07.18.
*/

#pragma once

#include "Settings.hpp"

namespace settings
{

void registerVariable(IVariable &variable, std::string name);

template <typename T> class RegisteredVariableProxy : public Variable<T>
{
public:
    using Variable<T>::operator=;

    explicit RegisteredVariableProxy(std::string name)
    {
        registerVariable(*this, name);
    }
    RegisteredVariableProxy(std::string name, std::string value)
    {
        this->fromString(value, true);
        registerVariable(*this, name);
    }
};

template <typename T> using RVariable = RegisteredVariableProxy<T>;

using Boolean = RegisteredVariableProxy<bool>;
using Int     = RegisteredVariableProxy<int>;
using Float   = RegisteredVariableProxy<float>;
using String  = RegisteredVariableProxy<std::string>;
using Button  = RegisteredVariableProxy<settings::Key>;

#if ENABLE_VISUALS
using Rgba = RegisteredVariableProxy<rgba_t>;
#endif
} // namespace settings
