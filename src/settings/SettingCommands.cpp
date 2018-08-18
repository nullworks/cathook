#include <core/sdk.hpp>
#include <core/cvwrapper.hpp>
#include <settings/Manager.hpp>
#include <init.hpp>
#include <settings/SettingsIO.hpp>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>

/*
  Created on 29.07.18.
*/

static CatCommand cat("cat", "", [](const CCommand &args) {
    if (args.ArgC() < 3)
    {
        g_ICvar->ConsolePrintf("Usage: cat <set/get> <variable> [value]\n");
        return;
    }

    auto variable = settings::Manager::instance().lookup(args.Arg(2));
    if (variable == nullptr)
    {
        g_ICvar->ConsolePrintf("Variable not found: %s\n", args.Arg(2));
        return;
    }

    if (!strcmp(args.Arg(1), "set"))
    {
        if (args.ArgC() < 4)
        {
            g_ICvar->ConsolePrintf("Usage: cat <set> <variable> <value>\n");
            return;
        }
        variable->fromString(args.Arg(3));
        g_ICvar->ConsolePrintf("%s = \"%s\"\n", args.Arg(2),
                               variable->toString().c_str());
        return;
    }
    else if (!strcmp(args.Arg(1), "get"))
    {
        g_ICvar->ConsolePrintf("%s = \"%s\"\n", args.Arg(2),
                               variable->toString().c_str());
        return;
    }
    else
    {
        g_ICvar->ConsolePrintf("Usage: cat <set/get> <variable> <value>\n");
        return;
    }
});

static CatCommand save("save", "", [](const CCommand &args) {
    settings::SettingsWriter writer{ settings::Manager::instance() };

    DIR *config_directory = opendir(DATA_PATH "/configs");
    if (!config_directory)
    {
        logging::Info("Configs directory doesn't exist, creating one!");
        mkdir(DATA_PATH "/configs", S_IRWXU | S_IRWXG);
    }

    if (args.ArgC() == 1)
    {
        writer.saveTo(DATA_PATH "/configs/default.conf", false);
    }
    else
    {
        writer.saveTo(
            std::string(DATA_PATH "/configs/") + args.Arg(1) + ".conf", false);
    }
});

static CatCommand load("load", "", [](const CCommand &args) {
    settings::SettingsReader loader{ settings::Manager::instance() };
    if (args.ArgC() == 1)
    {
        loader.loadFrom(DATA_PATH "/configs/default.conf");
    }
    else
    {
        loader.loadFrom(std::string(DATA_PATH "/configs/") + args.Arg(1) +
                        ".conf");
    }
});

static std::vector<std::string> sorted{};

static void getAndSortAllVariables()
{
    for (auto &v : settings::Manager::instance().registered)
    {
        sorted.push_back(v.first);
    }

    std::sort(sorted.begin(), sorted.end());

    logging::Info("Sorted %u variables\n", sorted.size());
}

static int completionCallback(
    const char *c_partial,
    char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH])
{
    std::string partial = c_partial;
    std::string parts[2]{};
    auto j    = 0u;
    auto f    = false;
    int count = 0;

    for (auto i = 0u; i < partial.size() && j < 3; ++i)
    {
        auto space = (bool) isspace(partial[i]);
        if (!space)
        {
            if (j)
                parts[j - 1].push_back(partial[i]);
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

    logging::Info("%s|%s", parts[0].c_str(), parts[1].c_str());

    if (parts[0].empty() ||
        parts[1].empty() && (!parts[0].empty() && partial.back() != ' '))
    {
        if (std::string("get").find(parts[0]) != std::string::npos)
            snprintf(commands[count++], COMMAND_COMPLETION_ITEM_LENGTH,
                     "cat get ");
        if (std::string("set").find(parts[0]) != std::string::npos)
            snprintf(commands[count++], COMMAND_COMPLETION_ITEM_LENGTH,
                     "cat set ");
        return count;
    }

    for (const auto &s : sorted)
    {
        if (s.find(parts[1]) == 0)
        {
            auto variable = settings::Manager::instance().lookup(s);
            if (variable)
            {
                snprintf(commands[count++], COMMAND_COMPLETION_ITEM_LENGTH - 1,
                         "cat %s %s %s", parts[0].c_str(), s.c_str(),
                         variable->toString().c_str());
                if (count == COMMAND_COMPLETION_MAXITEMS)
                    break;
            }
        }
    }
    return count;
}

static InitRoutine init([]() {
    getAndSortAllVariables();
    cat.cmd->m_bHasCompletionCallback = true;
    cat.cmd->m_fnCompletionCallback   = completionCallback;
});