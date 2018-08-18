/*
  Created on 20.06.18.
*/

#pragma once

#include <string>
#include <memory>
#include <unordered_map>

namespace zerokernel
{

class KeyValue
{
public:
    class Value
    {
    public:
        Value()              = default;
        Value(const Value &) = delete;

        Value &operator=(int value);
        Value &operator=(float value);
        Value &operator=(const std::string &value);

        void set(const std::string &value);

        explicit operator int() const;
        explicit operator float() const;
        explicit operator std::string() const;
        Value &operator[](const std::string &key);

    protected:
        int value_int{};
        float value_float{};
        std::optional<std::string> value_string{};
        std::unique_ptr<class KeyValue> value_keys{};
    };

public:
    KeyValue()                 = default;
    KeyValue(const KeyValue &) = delete;
    KeyValue(std::string name);
    Value &operator[](const std::string &key);

    const std::string name{};

protected:
    std::unordered_map<std::string, Value> values{};
};
} // namespace zerokernel