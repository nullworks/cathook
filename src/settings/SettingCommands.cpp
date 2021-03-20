#include <core/sdk.hpp>
#include <core/cvwrapper.hpp>
#include <settings/Manager.hpp>
#include <init.hpp>
#include <settings/SettingsIO.hpp>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <thread>
#include <MiscTemporary.hpp>
#if ENABLE_VISUALS
#include "Menu.hpp"
#include "special/SettingsManagerList.hpp"
#include "special/ConfigsManagerList.hpp"
#include "settings/Bool.hpp"
#endif
/*
  Created on 29.07.18.
*/

namespace settings::commands
{
static settings::Boolean autosave{ "settings.autosave", "true" };

#if ENABLE_VISUALS
void refreshConfigList()
{
    zerokernel::Container *cl = dynamic_cast<zerokernel::Container *>(zerokernel::Menu::instance->wm->getElementById("cfg-list"));
    if (cl)
    {
        zerokernel::special::ConfigsManagerList list(*cl);
        list.construct();
        printf("CL found\n");
    }
}
#endif

static void getAndSortAllConfigs();
static std::string getAutoSaveConfigPath()
{
    static std::string path;
    if (!path.empty())
        return path;
    time_t current_time;
    struct tm *time_info = nullptr;
    char timeString[100];
    time(&current_time);
    time_info = localtime(&current_time);
    strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M", time_info);

    DIR *config_directory = opendir(paths::getConfigPath().c_str());
    if (!config_directory)
    {
        logging::Info("Configs directory doesn't exist, creating one!");
        mkdir(paths::getConfigPath().c_str(), S_IRWXU | S_IRWXG);
    }
    else
        closedir(config_directory);

    config_directory = opendir((paths::getConfigPath() + "/autosaves").c_str());
    if (!config_directory)
    {
        logging::Info("Autosave directory doesn't exist, creating one!");
        mkdir((paths::getConfigPath() + "/autosaves").c_str(), S_IRWXU | S_IRWXG);
    }
    else
        closedir(config_directory);

    return path = paths::getConfigPath() + "/autosaves/" + timeString + ".conf";
}

static CatCommand cat("cat", "", [](const CCommand &args) {
    if (args.ArgC() < 3)
    {
        g_ICvar->ConsoleColorPrintf(MENU_COLOR, "Usage: cat <set/get> <variable> [value]\n");
        return;
    }

    auto variable = settings::Manager::instance().lookup(args.Arg(2));
    if (variable == nullptr)
    {
        g_ICvar->ConsoleColorPrintf(MENU_COLOR, "Variable not found: %s\n", args.Arg(2));
        return;
    }

    if (!strcmp(args.Arg(1), "set"))
    {
        if (args.ArgC() < 4)
        {
            g_ICvar->ConsoleColorPrintf(MENU_COLOR, "Usage: cat <set> <variable> <value>\n");
            return;
        }
        variable->fromString(args.Arg(3));
        g_ICvar->ConsoleColorPrintf(MENU_COLOR, "%s = \"%s\"\n", args.Arg(2), variable->toString().c_str());
        if (autosave)
        {
            settings::SettingsWriter writer{ settings::Manager::instance() };
            writer.saveTo(getAutoSaveConfigPath(), true);
        }
        return;
    }
    else if (!strcmp(args.Arg(1), "get"))
    {
        g_ICvar->ConsoleColorPrintf(MENU_COLOR, "%s = \"%s\"\n", args.Arg(2), variable->toString().c_str());
        return;
    }
    else
    {
        g_ICvar->ConsoleColorPrintf(MENU_COLOR, "Usage: cat <set/get> <variable> <value>\n");
        return;
    }
});

static CatCommand toggle("toggle", "", [](const CCommand &args) {
    if (args.ArgC() < 4)
    {
        g_ICvar->ConsoleColorPrintf(MENU_COLOR, "Usage: cat_toggle <variable> <value 1> <value 2>\n");
        return;
    }

    std::string rvar = args.Arg(1);
    auto variable    = settings::Manager::instance().lookup(rvar);

    if (variable == nullptr)
    {
        g_ICvar->ConsoleColorPrintf(MENU_COLOR, "Variable not found: %s\n", rvar.c_str());
        return;
    }

    std::string Value1 = args.Arg(2);
    std::string Value2 = args.Arg(3);

    if (variable->toString() == Value1)
        variable->fromString(Value2);
    else
        variable->fromString(Value1);
    g_ICvar->ConsoleColorPrintf(MENU_COLOR, "%s = \"%s\"\n", rvar.c_str(), variable->toString().c_str());
});

static CatCommand save("save", "", [](const CCommand &args) {
    settings::SettingsWriter writer{ settings::Manager::instance() };

    DIR *config_directory = opendir(paths::getConfigPath().c_str());
    if (!config_directory)
    {
        logging::Info("Configs directory doesn't exist, creating one!");
        mkdir(paths::getConfigPath().c_str(), S_IRWXU | S_IRWXG);
    }

    if (args.ArgC() == 1)
    {
        writer.saveTo((paths::getConfigPath() + "/default.conf").c_str());
    }
    else
    {
        writer.saveTo(paths::getConfigPath() + "/" + args.Arg(1) + ".conf");
    }
    logging::Info("cat_save: Sorting configs...");
    getAndSortAllConfigs();
    logging::Info("cat_save: Closing dir...");
#if ENABLE_VISUALS
    refreshConfigList();
#endif
    closedir(config_directory);
});

static CatCommand load("load", "", [](const CCommand &args) {
    settings::SettingsReader loader{ settings::Manager::instance() };
    if (args.ArgC() == 1)
    {
        loader.loadFrom((paths::getConfigPath() + "/default.conf").c_str());
    }
    else
    {
        loader.loadFrom(paths::getConfigPath() + "/" + args.Arg(1) + ".conf");
    }
});

#if ENABLE_VISUALS
static CatCommand delete_config("delete_config", "", [](const CCommand &args) {
    if (args.ArgC() == 2)
    {
        remove((paths::getConfigPath() + "/" + args.Arg(1) + ".conf").c_str());
        refreshConfigList();
        g_ICvar->ConsoleColorPrintf(MENU_COLOR, "CAT: cat_config_delete: Config Deleted!\n");
    }
    else
    {
        g_ICvar->ConsoleColorPrintf(MENU_COLOR, "CAT: cat_config_delete: No config specified!\n");
    }
});

static CatCommand list_config("list_config", "", [](const CCommand &args) {
    DIR *direc;
    struct dirent *directory;

    direc = opendir(paths::getConfigPath().c_str());
    if (direc)
    {
        while ((directory = readdir(direc)) != NULL)
        {
            if (std::string(directory->d_name) != "." && std::string(directory->d_name) != "..")
                g_ICvar->ConsoleColorPrintf(MENU_COLOR, (std::string(directory->d_name) + std::string("\n")).c_str());
        }

        closedir(direc);
    }
});
#endif

static std::vector<std::string> sortedVariables{};
#if ENABLE_VISUALS
static CatCommand list_missing("list_missing", "List rvars missing in menu", []() {
    auto *sv = zerokernel::Menu::instance->wm->getElementById("special-variables");
    if (!sv)
    {
        logging::Info("Special Variables tab missing!");
        return;
    }
    for (auto &var : sortedVariables)
        if (!zerokernel::special::SettingsManagerList::isVariableMarked(var))
            logging::Info("%s", var.c_str());
});
#endif

static void getAndSortAllVariables()
{
    for (auto &v : settings::Manager::instance().registered)
    {
        sortedVariables.push_back(v.first);
    }

    std::sort(sortedVariables.begin(), sortedVariables.end());

    logging::Info("Sorted %u variables\n", sortedVariables.size());
}

std::vector<std::string> sortedConfigs{};

static void getAndSortAllConfigs()
{
    DIR *config_directory = opendir(paths::getConfigPath().c_str());
    if (!config_directory)
    {
        logging::Info("Config directoy does not exist.");
        closedir(config_directory);
        return;
    }
    sortedConfigs.clear();

    struct dirent *ent;
    while ((ent = readdir(config_directory)))
    {
        std::string s(ent->d_name);
        s = s.substr(0, s.find_last_of("."));
        if (s != "autosaves")
            sortedConfigs.push_back(s);
    }
    std::sort(sortedConfigs.begin(), sortedConfigs.end());
    sortedConfigs.erase(sortedConfigs.begin());
    sortedConfigs.erase(sortedConfigs.begin());

    closedir(config_directory);
    logging::Info("Sorted %u config files\n", sortedConfigs.size());
}

static CatCommand cat_find("find", "Find a command by name", [](const CCommand &args) {
    // We need arguments
    if (args.ArgC() < 2)
        return logging::Info("Usage: cat_find (name)");
    // Store all found rvars
    std::vector<std::string> found_rvars;
    for (const auto &s : sortedVariables)
    {
        // Store std::tolower'd rvar
        std::string lowered_str;
        for (auto &i : s)
            lowered_str += std::tolower(i);
        std::string to_find = args.Arg(1);
        // store rvar to find in lowercase too
        std::string to_find_lower;
        for (auto &s : to_find)
            to_find_lower += std::tolower(s);
        // If it matches then add to vector
        if (lowered_str.find(to_find_lower) != lowered_str.npos)
            found_rvars.push_back(s);
    }
    // Yes
    g_ICvar->ConsoleColorPrintf(MENU_COLOR, "Found rvars:\n");
    // Nothing found :C
    if (found_rvars.empty())
        g_ICvar->ConsoleColorPrintf(MENU_COLOR, "No rvars found.\n");
    // Found rvars
    else
        for (auto &s : found_rvars)
            g_ICvar->ConsoleColorPrintf(MENU_COLOR, "%s\n", s.c_str());
});

static int cat_completionCallback(const char *c_partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH])
{
    std::string partial = c_partial;
    std::array<std::string, 2> parts{};
    auto j    = 0u;
    auto f    = false;
    int count = 0;

    for (auto i = 0u; i < partial.size() && j < 3; ++i)
    {
        auto space = (bool) isspace(partial.at(i));
        if (!space)
        {
            if (j)
                parts.at(j - 1).push_back(partial[i]);
            f = true;
        }

        if (i == partial.size() - 1 || (f && space))
        {
            if (space)
                ++j;
            f = false;
        }
    }

    // "" -> cat [get, set]
    // "g" -> cat get
    // "get " -> cat get <variable>

    // logging::Info("%s|%s", parts.at(0).c_str(), parts.at(1).c_str());

    if (parts.at(0).empty() || (parts.at(1).empty() && (!parts.at(0).empty() && partial.back() != ' ')))
    {
        if (std::string("get").find(parts.at(0)) != std::string::npos)
            snprintf(commands[count++], COMMAND_COMPLETION_ITEM_LENGTH, "cat get ");
        if (std::string("set").find(parts[0]) != std::string::npos)
            snprintf(commands[count++], COMMAND_COMPLETION_ITEM_LENGTH, "cat set ");
        return count;
    }

    for (const auto &s : sortedVariables)
    {
        if (s.find(parts.at(1)) == 0)
        {
            auto variable = settings::Manager::instance().lookup(s);
            if (variable)
            {
                if (s.compare(parts.at(1)))
                    snprintf(commands[count++], COMMAND_COMPLETION_ITEM_LENGTH - 1, "cat %s %s", parts.at(0).c_str(), s.c_str());
                else
                    snprintf(commands[count++], COMMAND_COMPLETION_ITEM_LENGTH - 1, "cat %s %s %s", parts.at(0).c_str(), s.c_str(), variable->toString().c_str());
                if (count == COMMAND_COMPLETION_MAXITEMS)
                    break;
            }
        }
    }
    return count;
}

static int load_CompletionCallback(const char *c_partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH])
{
    std::string partial = c_partial;
    std::array<std::string, 2> parts{};
    auto j    = 0u;
    auto f    = false;
    int count = 0;

    for (auto i = 0u; i < partial.size() && j < 3; ++i)
    {
        auto space = (bool) isspace(partial.at(i));
        if (!space)
        {
            if (j)
                parts.at(j - 1).push_back(partial[i]);
            f = true;
        }

        if (i == partial.size() - 1 || (f && space))
        {
            if (space)
                ++j;
            f = false;
        }
    }

    for (const auto &s : sortedConfigs)
    {
        if (s.find(parts.at(0)) == 0)
        {
            snprintf(commands[count++], COMMAND_COMPLETION_ITEM_LENGTH - 1, "cat_load %s", s.c_str());
            if (count == COMMAND_COMPLETION_MAXITEMS)
                break;
        }
    }
    return count;
}

static int save_CompletionCallback(const char *c_partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH])
{
    std::string partial = c_partial;
    std::array<std::string, 2> parts{};
    auto j    = 0u;
    auto f    = false;
    int count = 0;

    for (auto i = 0u; i < partial.size() && j < 3; ++i)
    {
        auto space = (bool) isspace(partial.at(i));
        if (!space)
        {
            if (j)
                parts.at(j - 1).push_back(partial[i]);
            f = true;
        }

        if (i == partial.size() - 1 || (f && space))
        {
            if (space)
                ++j;
            f = false;
        }
    }

    for (const auto &s : sortedConfigs)
    {
        if (s.find(parts.at(0)) == 0)
        {
            snprintf(commands[count++], COMMAND_COMPLETION_ITEM_LENGTH - 1, "cat_save %s", s.c_str());
            if (count == COMMAND_COMPLETION_MAXITEMS)
                break;
        }
    }
    return count;
}

static int toggle_CompletionCallback(const char *c_partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH])
{
    std::string partial = c_partial;
    std::array<std::string, 3> parts{};
    auto j    = 0u;
    auto f    = false;
    int count = 0;

    for (auto i = 0u; i < partial.size() && j < 4; ++i)
    {
        auto space = (bool) isspace(partial.at(i));
        if (!space)
        {
            if (j)
                parts.at(j - 1).push_back(partial[i]);
            f = true;
        }

        if (i == partial.size() - 1 || (f && space))
        {
            if (space)
                ++j;
            f = false;
        }
    }

    for (const auto &s : sortedVariables)
    {
        if (s.find(parts.at(0)) == 0)
        {
            if (parts.at(1) == "")
                snprintf(commands[count++], COMMAND_COMPLETION_ITEM_LENGTH - 1, "cat_toggle %s", s.c_str());
            else if (parts.at(2) == "")
                snprintf(commands[count++], COMMAND_COMPLETION_ITEM_LENGTH - 1, "cat_toggle %s %s", s.c_str(), parts.at(1).c_str());
            else
                snprintf(commands[count++], COMMAND_COMPLETION_ITEM_LENGTH - 1, "cat_toggle %s %s %s", s.c_str(), parts.at(1).c_str(), parts.at(2).c_str());

            if (count == COMMAND_COMPLETION_MAXITEMS)
                break;
        }
    }

    return count;
}

static InitRoutine init([]() {
    getAndSortAllVariables();
    getAndSortAllConfigs();
    cat.cmd->m_bHasCompletionCallback    = true;
    cat.cmd->m_fnCompletionCallback      = cat_completionCallback;
    load.cmd->m_bHasCompletionCallback   = true;
    load.cmd->m_fnCompletionCallback     = load_CompletionCallback;
    save.cmd->m_bHasCompletionCallback   = true;
    save.cmd->m_fnCompletionCallback     = save_CompletionCallback;
    toggle.cmd->m_bHasCompletionCallback = true;
    toggle.cmd->m_fnCompletionCallback   = toggle_CompletionCallback;
});
} // namespace settings::commands
