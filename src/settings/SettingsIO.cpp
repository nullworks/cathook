/*
  Created on 27.07.18.
*/

#include <settings/SettingsIO.hpp>
#include <fstream>
#include <sstream>
#include "core/logging.hpp"
#include "interfaces.hpp"
#include "icvar.h"
#include "MiscTemporary.hpp"

settings::SettingsWriter::SettingsWriter(settings::Manager &manager) : manager(manager)
{
}

bool settings::SettingsWriter::saveTo(std::string path, bool autosave)
{
    if (autosave)
        logging::File("cat_save: Saving to %s", path.c_str());
    else
    {
        logging::Info("cat_save: Saving to %s", path.c_str());
    }

    this->only_changed = true;

    stream.open(path, std::ios::out);

    if (!stream || stream.bad() || !stream.is_open() || stream.fail())
    {
        logging::File("cat_save: FATAL! FAILED to create stream!");
        if (!autosave)
            g_ICvar->ConsoleColorPrintf(MENU_COLOR, "CAT: cat_save: Can't create config file!\n");
        return false;
    }

    using pair_type = std::pair<std::string, settings::IVariable *>;
    std::vector<pair_type> all_registered{};
    logging::File("cat_save: Getting variable references...");
    for (auto &v : settings::Manager::instance().registered)
    {
        if (!only_changed || v.second.isChanged())
            all_registered.emplace_back(std::make_pair(v.first, &v.second.variable));
    }
    logging::File("cat_save: Sorting...");
    std::sort(all_registered.begin(), all_registered.end(), [](const pair_type &a, const pair_type &b) -> bool { return a.first.compare(b.first) < 0; });
    logging::File("cat_save: Writing...");
    for (auto &v : all_registered)
        if (!v.first.empty())
        {
            write(v.first, v.second);
            stream.flush();
        }
    if (!stream || stream.bad() || stream.fail())
    {
        if (!autosave)
            g_ICvar->ConsoleColorPrintf(MENU_COLOR, "CAT: cat_save: Failed to save config!\n");
        logging::File("cat_save: FATAL! Stream bad!");
    }
    else if (!autosave)
        g_ICvar->ConsoleColorPrintf(MENU_COLOR, "CAT: cat_save: Successfully saved config!\n");
    stream.close();
    if (stream.fail())
        logging::File("cat_save: FATAL! Stream bad (2)!");
    return true;
}

void settings::SettingsWriter::write(std::string name, IVariable *variable)
{
    writeEscaped(name);
    stream << "=";
    if (variable)
        writeEscaped(variable->toString());
    else
    {
        logging::Info("cat_save: FATAL! Variable invalid! %s", name.c_str());
    }
    stream << std::endl;
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
        case '\\':
            stream << '\\';
            break;
        default:
            break;
        }
        stream << c;
    }
}

settings::SettingsReader::SettingsReader(settings::Manager &manager) : manager(manager)
{
}

bool settings::SettingsReader::loadFrom(std::string path)
{
    stream.open(path, std::ios::in | std::ios::binary);

    if (stream.fail())
    {
        logging::Info("cat_load: Can't access file!");
        g_ICvar->ConsoleColorPrintf(MENU_COLOR, "CAT: cat_load: File doesn't exist / can't open file!\n");
        return false;
    }

    for (auto &v : settings::Manager::instance().registered)
    {
        v.second.variable.fromString(v.second.defaults);
    }

    while (!stream.fail())
    {
        char c;
        stream.read(&c, 1);
        if (stream.eof())
            break;
        pushChar(c);
    }
    if (stream.fail() && !stream.eof())
    {
        logging::Info("cat_load: FATAL: Read failed!");
        g_ICvar->ConsoleColorPrintf(MENU_COLOR, "CAT: cat_load: Failed to read config!\n");
        return false;
    }

    logging::Info("cat_load: Read Success!");
    g_ICvar->ConsoleColorPrintf(MENU_COLOR, "CAT: cat_load: Successfully loaded config!\n");
    finishString(true);

    return true;
}

bool settings::SettingsReader::loadFromString(std::string stream)
{
    settings::SettingsReader loader{ settings::Manager::instance() };
    if (stream == "")
    {
        logging::Info("cat_load: Empty String!");
        g_ICvar->ConsoleColorPrintf(MENU_COLOR, "CAT: cat_load: Empty String!\n");
        return false;
    }

    for (auto &v : settings::Manager::instance().registered)
    {
        v.second.variable.fromString(v.second.defaults);
    }

    for (auto &c : stream)
    {
        loader.pushChar(c);
    }

    logging::Info("cat_load: Read Success!");
    g_ICvar->ConsoleColorPrintf(MENU_COLOR, "CAT: cat_load: Successfully loaded config!\n");
    loader.finishString(true);

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

struct migration_struct
{
    const std::string from;
    const std::string to;
};
/* clang-format off */
// Use one per line, from -> to
static std::array<migration_struct, 2> migrations({
    migration_struct{ "misc.semi-auto", "misc.full-auto" },
    migration_struct{ "cat-bot.abandon-if.bots-gte", "cat-bot.abandon-if.ipc-bots-gte" }
});
/* clang-format on */
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
        for (auto &migration : migrations)
            if (stored_key == migration.from)
                for (auto &var : settings::Manager::instance().registered)
                    if (var.first == migration.to)
                    {
                        if (var.second.isChanged())
                            break;
                        stored_key = migration.to;
                    }
    }
    else
    {
        onReadKeyValue(stored_key, str);
    }
    reading_key   = !reading_key;
    was_non_space = false;
    temporary_spaces.clear();
}

void settings::SettingsReader::onReadKeyValue(std::string key, std::string value)
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
