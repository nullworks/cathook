/*
 * playerlist.cpp
 *
 *  Created on: Apr 11, 2017
 *      Author: nullifiedcat
 */

#include "playerlist.hpp"
#include "common.hpp"

#include <stdint.h>
#include <dirent.h>
#include <sys/stat.h>
#include <boost/algorithm/string.hpp>

namespace playerlist
{

std::unordered_map<unsigned, userdata> data{};

const std::string k_Names[]                                     = { "DEFAULT", "FRIEND", "RAGE", "IPC", "TEXTMODE", "CAT", "PARTY" };
const char *const k_pszNames[]                                  = { "DEFAULT", "FRIEND", "RAGE", "IPC", "TEXTMODE", "CAT", "PARTY" };
const std::array<std::pair<k_EState, size_t>, 5> k_arrGUIStates = { std::pair(k_EState::DEFAULT, 0), { k_EState::FRIEND, 1 }, { k_EState::RAGE, 2 } };
const userdata null_data{};
#if ENABLE_VISUALS
std::array<rgba_t, 7> k_Colors = { colors::empty, colors::FromRGBA8(99, 226, 161, 255), colors::FromRGBA8(226, 204, 99, 255), colors::FromRGBA8(232, 134, 6, 255), colors::FromRGBA8(232, 134, 6, 255), colors::empty, colors::FromRGBA8(99, 226, 161, 255) };
#endif
bool ShouldSave(const userdata &data)
{
#if ENABLE_VISUALS
    return data.color || data.state == k_EState::FRIEND || data.state == k_EState::RAGE;
#endif
    return data.state == k_EState::FRIEND || data.state == k_EState::RAGE;
}

void Save()
{
    DIR *cathook_directory = opendir(paths::getDataPath().c_str());
    if (!cathook_directory)
    {
        logging::Info("[ERROR] cathook data directory doesn't exist! How did "
                      "the cheat even get injected?");
        return;
    }
    else
        closedir(cathook_directory);
    try
    {
        std::ofstream file(paths::getDataPath("/plist"), std::ios::out | std::ios::binary);
        file.write(reinterpret_cast<const char *>(&SERIALIZE_VERSION), sizeof(SERIALIZE_VERSION));
        int size = 0;
        for (const auto &item : data)
        {
            if (ShouldSave(item.second))
                size++;
        }
        file.write(reinterpret_cast<const char *>(&size), sizeof(size));
        for (const auto &item : data)
        {
            if (!ShouldSave(item.second))
                continue;
            file.write(reinterpret_cast<const char *>(&item.first), sizeof(item.first));
            file.write(reinterpret_cast<const char *>(&item.second), sizeof(item.second));
        }
        file.close();
        logging::Info("Writing successful");
    }
    catch (std::exception &e)
    {
        logging::Info("Writing unsuccessful: %s", e.what());
    }
}

void Load()
{
    data.clear();
    DIR *cathook_directory = opendir(paths::getDataPath().c_str());
    if (!cathook_directory)
    {
        logging::Info("[ERROR] cathook data directory doesn't exist! How did "
                      "the cheat even get injected?");
        return;
    }
    else
        closedir(cathook_directory);
    try
    {
        std::ifstream file(paths::getDataPath("/plist"), std::ios::in | std::ios::binary);
        int file_serialize = 0;
        file.read(reinterpret_cast<char *>(&file_serialize), sizeof(file_serialize));
        if (file_serialize != SERIALIZE_VERSION)
        {
            logging::Info("Outdated/corrupted playerlist file! Cannot load this.");
            file.close();
            return;
        }
        int count = 0;
        file.read(reinterpret_cast<char *>(&count), sizeof(count));
        logging::Info("Reading %i entries...", count);
        for (int i = 0; i < count; i++)
        {
            int steamid;
            userdata udata;
            file.read(reinterpret_cast<char *>(&steamid), sizeof(steamid));
            file.read(reinterpret_cast<char *>(&udata), sizeof(udata));
            data.emplace(steamid, udata);
        }
        file.close();
        logging::Info("Reading successful!");
    }
    catch (std::exception &e)
    {
        logging::Info("Reading unsuccessful: %s", e.what());
    }
}
#if ENABLE_VISUALS
rgba_t Color(unsigned steamid)
{
    const auto &pl = AccessData(steamid);
    if (pl.state == k_EState::CAT)
        return colors::RainbowCurrent();
    else if (pl.color.a)
        return pl.color;

    int state = static_cast<int>(pl.state);
    if (state < k_Colors.size())
        return k_Colors[state];

    return colors::empty;
}

rgba_t Color(CachedEntity *player)
{
    if (CE_GOOD(player))
        return Color(player->player_info.friendsID);
    return colors::empty;
}
#endif
userdata &AccessData(unsigned steamid)
{
    return data[steamid];
}

// Assume player is non-null
userdata &AccessData(CachedEntity *player)
{
    if (player && player->player_info.friendsID)
        return AccessData(player->player_info.friendsID);
    return AccessData(0U);
}

bool IsDefault(unsigned steamid)
{
    const userdata &data = AccessData(steamid);
#if ENABLE_VISUALS
    return data.state == k_EState::DEFAULT && !data.color.a;
#endif
    return data.state == k_EState ::DEFAULT;
}

bool IsDefault(CachedEntity *entity)
{
    if (entity && entity->player_info.friendsID)
        return IsDefault(entity->player_info.friendsID);
    return true;
}

bool IsFriend(unsigned steamid)
{
    const userdata &data = AccessData(steamid);
    return data.state == k_EState::PARTY || data.state == k_EState::FRIEND;
}

bool IsFriend(CachedEntity *entity)
{
    if (entity && entity->player_info.friendsID)
        return IsFriend(entity->player_info.friendsID);
    return false;
}

bool ChangeState(unsigned int steamid, k_EState state, bool force)
{
    auto &data = AccessData(steamid);
    if (force)
    {
        data.state = state;
        return true;
    }
    switch (data.state)
    {
    case k_EState::FRIEND:
        return false;
    case k_EState::TEXTMODE:
        if (state == k_EState::IPC || state == k_EState::FRIEND)
        {
            ChangeState(steamid, state, true);
            return true;
        }
        else
            return false;
    case k_EState::PARTY:
        if (state == k_EState::FRIEND)
        {
            ChangeState(steamid, state, true);
            return true;
        }
        else
            return false;
    case k_EState::IPC:
        if (state == k_EState::FRIEND || state == k_EState::TEXTMODE || state == k_EState::PARTY)
        {
            ChangeState(steamid, state, true);
            return true;
        }
        else
            return false;
    case k_EState::CAT:
        if (state == k_EState::FRIEND || state == k_EState::IPC || state == k_EState::TEXTMODE || state == k_EState::PARTY)
        {
            ChangeState(steamid, state, true);
            return true;
        }
        else
            return false;
    case k_EState::DEFAULT:
        if (state != k_EState::DEFAULT)
        {
            ChangeState(steamid, state, true);
            return true;
        }
        else
            return false;
    case k_EState::RAGE:
        return false;
    }
    return true;
}

bool ChangeState(CachedEntity *entity, k_EState state, bool force)
{
    if (entity && entity->player_info.friendsID)
        return ChangeState(entity->player_info.friendsID, state, force);
    return false;
}

CatCommand pl_save("pl_save", "Save playerlist", Save);
CatCommand pl_load("pl_load", "Load playerlist", Load);

CatCommand pl_print("pl_print", "Print current player list", [](const CCommand &args) {
    userdata empty{};
    bool include_all = args.ArgC() >= 2 && *args.Arg(1) == '1';

    logging::Info("Known entries: %lu", data.size());
    for (auto it = data.begin(); it != data.end(); ++it)
    {
        if (!include_all && !std::memcmp(&it->second, &empty, sizeof(empty)))
            continue;

        const auto &ent = it->second;
#if ENABLE_VISUALS
        logging::Info("%u -> %d (%f,%f,%f,%f) %f %u %u", it->first, ent.state, ent.color.r, ent.color.g, ent.color.b, ent.color.a, ent.inventory_value, ent.deaths_to, ent.kills);
#else
        logging::Info("%u -> %d %f %u %u", it->first, ent.state,
            ent.inventory_value, ent.deaths_to, ent.kills);
#endif
    }
});

CatCommand pl_add_id("pl_add_id", "Sets state for steamid", [](const CCommand &args) {
    if (args.ArgC() <= 2)
        return;

    uint32_t id       = std::strtoul(args.Arg(1), nullptr, 10);
    const char *state = args.Arg(2);
    for (int i = 0; i <= int(k_EState::STATE_LAST); ++i)
        if (k_Names[i] == state)
        {
            AccessData(id).state = k_EState(i);
            return;
        }

    logging::Info("Unknown State");
});

static void pl_cleanup()
{
    userdata empty{};
    size_t counter = 0;
    for (auto it = data.begin(); it != data.end(); ++it)
    {
        if (std::memcmp(&it->second, &empty, sizeof(empty)))
            continue;

        ++counter;
        data.erase(it);
        /* Start all over again. Iterator is invalidated once erased. */
        it = data.begin();
    }
    logging::Info("%lu elements were removed", counter);
}

CatCommand pl_clean("pl_clean", "Removes empty entries to reduce RAM usage", pl_cleanup);

CatCommand pl_set_state("pl_set_state", "cat_pl_set_state [playername] [state] (Tab to autocomplete)", [](const CCommand &args) {
    if (args.ArgC() != 3)
    {
        logging::Info("Invalid call");
        return;
    }
    auto name = args.Arg(1);
    int id    = -1;
    for (int i = 0; i <= g_IEngine->GetMaxClients(); i++)
    {
        player_info_s info;
        if (!g_IEngine->GetPlayerInfo(i, &info))
            continue;
        std::string currname(info.name);
        std::replace(currname.begin(), currname.end(), ' ', '-');
        std::replace_if(
            currname.begin(), currname.end(), [](char x) { return !isprint(x); }, '*');
        if (currname.find(name) != 0)
            continue;
        id = i;
        break;
    }
    if (id == -1)
    {
        logging::Info("Unknown Player Name. (Use tab for autocomplete)");
        return;
    }
    std::string state = args.Arg(2);
    boost::to_upper(state);
    player_info_s info;
    g_IEngine->GetPlayerInfo(id, &info);

    for (int i = 0; i <= int(k_EState::STATE_LAST); ++i)
        if (k_Names[i] == state)
        {
            AccessData(info.friendsID).state = k_EState(i);
            return;
        }

    logging::Info("Unknown State %s. (Use tab for autocomplete)", state.c_str());
});

static int cat_pl_set_state_completionCallback(const char *c_partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH])
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

    std::vector<std::string> names;

    for (int i = 0; i <= g_IEngine->GetMaxClients(); i++)
    {
        player_info_s info;
        if (!g_IEngine->GetPlayerInfo(i, &info))
            continue;
        std::string name(info.name);
        std::replace(name.begin(), name.end(), ' ', '-');
        std::replace_if(
            name.begin(), name.end(), [](char x) { return !isprint(x); }, '*');
        names.push_back(name);
    }
    std::sort(names.begin(), names.end());

    if (parts[0].empty() || (parts[1].empty() && (!parts[0].empty() && partial.back() != ' ')))
    {
        boost::to_lower(parts[0]);
        for (const auto &s : names)
        {
            // if (s.find(parts[0]) == 0)
            if (boost::to_lower_copy(s).find(parts[0]) == 0)
            {
                snprintf(commands[count++], COMMAND_COMPLETION_ITEM_LENGTH - 1, "cat_pl_set_state %s", s.c_str());
            }
        }
        return count;
    }
    boost::to_lower(parts[1]);
    for (const auto &s : k_Names)
    {
        if (boost::to_lower_copy(s).find(parts[1]) == 0)
        {
            snprintf(commands[count++], COMMAND_COMPLETION_ITEM_LENGTH - 1, "cat_pl_set_state %s %s", parts[0].c_str(), s.c_str());
            if (count == COMMAND_COMPLETION_MAXITEMS)
                break;
        }
    }
    return count;
}

#if ENABLE_VISUALS
CatCommand pl_set_color("pl_set_color", "pl_set_color uniqueid r g b", [](const CCommand &args) {
    if (args.ArgC() < 5)
    {
        logging::Info("Invalid call");
        return;
    }
    unsigned steamid          = strtoul(args.Arg(1), nullptr, 10);
    int r                     = strtol(args.Arg(2), nullptr, 10);
    int g                     = strtol(args.Arg(3), nullptr, 10);
    int b                     = strtol(args.Arg(4), nullptr, 10);
    rgba_t color              = colors::FromRGBA8(r, g, b, 255);
    AccessData(steamid).color = color;
    logging::Info("Changed %d's color", steamid);
});
#endif
CatCommand pl_info("pl_info", "pl_info uniqueid", [](const CCommand &args) {
    if (args.ArgC() < 2)
    {
        logging::Info("Invalid call");
        return;
    }
    unsigned steamid;
    try
    {
        steamid = strtoul(args.Arg(1), nullptr, 10);
    }
    catch (const std::invalid_argument &)
    {
        return;
    }
    auto &pl = AccessData(steamid);
    const char *str_state;
    if (pl.state < k_EState::STATE_FIRST || pl.state > k_EState::STATE_LAST)
        str_state = "UNKNOWN";
    else
        str_state = k_pszNames[int(pl.state)];

    logging::Info("Data for %i: ", steamid);
    logging::Info("State: %i %s", pl.state, str_state);
    /*int clr = AccessData(steamid).color;
    if (clr) {
        ConColorMsg(*reinterpret_cast<::Color*>(&clr), "[CUSTOM COLOR]\n");
    }*/
});

static InitRoutine init([]() {
    pl_set_state.cmd->m_bHasCompletionCallback = true;
    pl_set_state.cmd->m_fnCompletionCallback   = cat_pl_set_state_completionCallback;
});
} // namespace playerlist
