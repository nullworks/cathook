/*
  Created on 20.06.18.
*/

#include <menu/KeyValue.hpp>
#include <sstream>

namespace zerokernel
{

KeyValue::Value &KeyValue::Value::operator=(int value)
{
    value_int   = value;
    value_float = value;
    return *this;
}

KeyValue::Value &KeyValue::Value::operator=(float value)
{
    value_int   = int(value);
    value_float = value;
    return *this;
}

KeyValue::Value &KeyValue::Value::operator=(const std::string &value)
{
    value_string = value;
    return *this;
}

KeyValue::Value::operator int() const
{
    return value_int;
}

KeyValue::Value::operator float() const
{
    return value_float;
}

KeyValue::Value::operator std::string() const
{
    if (value_string.has_value())
        return *value_string;
    return "";
}

KeyValue::Value &KeyValue::Value::operator[](const std::string &key)
{
    if (!value_keys)
        value_keys = std::make_unique<KeyValue>();
    return (*value_keys)[key];
}

void KeyValue::Value::set(const std::string &value)
{
    *this   = value;
    float i = 0.0f;
    std::istringstream ss(value);
    ss >> i;
    *this = i;
}

KeyValue::Value &KeyValue::operator[](const std::string &key)
{
    return values[key];
}

KeyValue::KeyValue(std::string name) : name{ std::move(name) }
{
}
} // namespace zerokernel
