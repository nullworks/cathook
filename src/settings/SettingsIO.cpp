/*
  Created on 27.07.18.
*/

#include <settings/SettingsIO.hpp>
#include <fstream>
#include <sstream>

settings::SettingsWriter::SettingsWriter(settings::Manager &manager)
    : manager(manager)
{
}

bool settings::SettingsWriter::saveTo(std::string path, bool only_changed)
{
    this->only_changed = only_changed;

    stream.open(path, std::ios::out);

    if (stream.bad())
        return false;

    using pair_type = std::pair<std::string, settings::IVariable *>;
    std::vector<pair_type> all_registered{};
    for (auto &v : settings::Manager::instance().registered)
    {
        if (!only_changed || v.second.isChanged())
            all_registered.emplace_back(
                std::make_pair(v.first, &v.second.variable));
    }

    std::sort(all_registered.begin(), all_registered.end(),
              [](const pair_type &a, const pair_type &b) -> bool {
                  return a.first.compare(b.first) < 0;
              });

    for (auto &v : all_registered)
    {
        write(v.first, v.second);
    }

    return true;
}

void settings::SettingsWriter::write(std::string name, IVariable *variable)
{
    writeEscaped(name);
    stream << "=";
    writeEscaped(variable->toString());
    stream << '\n';
}

void settings::SettingsWriter::writeEscaped(std::string str)
{
    for (auto c : str)
    {
        switch (c)
        {
        case '#':
        case '\n':
        case '=':
            stream << '\\';
            break;
        default:
            break;
        }
        stream << c;
    }
}

settings::SettingsReader::SettingsReader(settings::Manager &manager)
    : manager(manager)
{
}

bool settings::SettingsReader::loadFrom(std::string path)
{
    stream.open(path, std::ios::in | std::ios::binary);

    if (stream.bad())
        return false;

    while (stream)
    {
        char c;
        stream.read(&c, 1);
        if (stream.eof())
            break;
        pushChar(c);
    }
    finishString(true);

    return true;
}

void settings::SettingsReader::pushChar(char c)
{
    if (comment)
    {
        if (c == '\n')
            comment = false;
        return;
    }

    if (isspace(c))
    {
        if (c != '\n' && !was_non_space)
            return;
    }
    else
    {

        was_non_space = true;
    }

    if (!escape)
    {
        if (c == '\\')
            escape = true;
        else if (c == '#' && !quote)
        {
            finishString(true);
            comment = true;
        }
        else if (c == '"')
            quote = !quote;
        else if (c == '=' && !quote && reading_key)
            finishString(false);
        else if (c == '\n')
            finishString(true);
        else if (isspace(c) && !quote)
            temporary_spaces.push_back(c);
        else
        {
            for (auto x : temporary_spaces)
                oss.push_back(x);
            temporary_spaces.clear();
            oss.push_back(c);
        }
    }
    else
    {
        // Escaped character can be anything but null
        if (c != 0)
            oss.push_back(c);
        escape = false;
    }
}

void settings::SettingsReader::finishString(bool complete)
{
    if (complete && reading_key)
    {
        oss.clear();
        return;
    }

    std::string str = oss;
    oss.clear();
    if (reading_key)
    {
        stored_key = std::move(str);
    }
    else
    {
        onReadKeyValue(stored_key, str);
    }
    reading_key   = !reading_key;
    was_non_space = false;
    temporary_spaces.clear();
}

void settings::SettingsReader::onReadKeyValue(std::string key,
                                              std::string value)
{
    printf("Read: '%s' = '%s'\n", key.c_str(), value.c_str());
    auto v = manager.lookup(key);
    if (v == nullptr)
    {
        printf("Could not find variable %s\n", key.c_str());
        return;
    }
    v->fromString(value);
    printf("Set:  '%s' = '%s'\n", key.c_str(), v->toString().c_str());
}
