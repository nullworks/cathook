/*
  Created on 27.07.18.
*/

#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include "Manager.hpp"

namespace settings
{

class SettingsReader
{
public:
    explicit SettingsReader(Manager &manager);

    bool loadFrom(std::string path);

protected:
    void pushChar(char c);
    void finishString(bool complete);
    void onReadKeyValue(std::string key, std::string value);
    bool reading_key{ true };

    bool escape{ false };
    bool quote{ false };
    bool comment{ false };
    bool was_non_space{ false };

    std::string temporary_spaces{};
    std::string oss{};
    std::string stored_key{};
    std::ifstream stream{};
    Manager &manager;
};

class SettingsWriter
{
public:
    explicit SettingsWriter(Manager &manager);

    bool saveTo(std::string path, bool only_changed);

protected:
    void write(std::string name, IVariable *variable);
    void writeEscaped(std::string str);

    bool only_changed{ false };
    std::ofstream stream{};
    Manager &manager;
};
} // namespace settings