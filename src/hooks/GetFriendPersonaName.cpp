/*
  Created by Jenny White on 29.04.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#include <settings/String.hpp>
#include "HookedMethods.hpp"
#include "PlayerTools.hpp"

static settings::String ipc_name{ "name.ipc", "" };
settings::String force_name{ "name.custom", "" };
std::string name_forced = "";
static settings::Int namesteal{ "name.namesteal", "0" };

static std::string stolen_name;

int getRng(int min, int max)
{
    static std::random_device rd;
    std::uniform_int_distribution<int> unif(min, max);
    static std::mt19937 rand_engine(rd());

    int x = unif(rand_engine);
    return x;
}

// Func to get a new entity to steal name from and returns true if a target has
// been found
bool StolenName()
{

    // Array to store potential namestealer targets with a bookkeeper to tell
    // how full it is
    int potential_targets[MAX_PLAYERS];
    int potential_targets_length = 0;

    // Go through entities looking for potential targets
    for (int i = 1; i <= g_IEngine->GetMaxClients(); i++)
    {
        CachedEntity *ent = ENTITY(i);

        // Check if ent is a good target
        if (!ent)
            continue;
        if (ent == LOCAL_E)
            continue;
        if (ent->m_Type() != ENTITY_PLAYER)
            continue;
        if (ent->m_bEnemy())
            continue;
        // Check if name is current one
        player_info_s info;
        if (g_IEngine->GetPlayerInfo(ent->m_IDX, &info))
        {
            // Should be ignored
            if (!player_tools::shouldTargetSteamId(info.friendsID))
                continue;
            // Invisible character won't fit into name with max. length
            if (std::strlen(info.name) >= 31)
                continue;
            // Ignore Friendly
            if (!player_tools::shouldTargetSteamId(info.friendsID))
                continue;
            // If our name is the same as current, then change it
            if (stolen_name == info.name && *namesteal == 1)
                return true;

            // a ent without a name is no ent we need, contine for a different
            // one
        }
        else
            continue;

        // Save the ent to our array
        potential_targets[potential_targets_length] = i;
        potential_targets_length++;

        // With our maximum amount of players reached, dont search for anymore
        if (potential_targets_length >= 32)
            break;
    }

    // Checks to prevent crashes
    if (potential_targets_length == 0)
        return false;

    // Get random number that we can use with our array
    int target_random_num = getRng(0, potential_targets_length - 1);

    // Get a idx from our random array position
    int new_target = potential_targets[target_random_num];

    // Grab username of user
    player_info_s info;
    if (g_IEngine->GetPlayerInfo(new_target, &info))
    {

        // If our name is the same as current, than change it and return true
        stolen_name = std::string(info.name);
        return true;
    }

    // Didnt get playerinfo
    return false;
}

std::string GetNamestealName(CSteamID steam_id)
{
    if (steam_id != g_ISteamUser->GetSteamID())
        return std::string();

    // Check User settings if namesteal is allowed
    if (namesteal)
    {

        // We dont want to steal names while not in-game as there are no targets
        // to steal from. We want to be on a team as well to get teammates names
        if (g_IEngine->IsInGame() && g_pLocalPlayer->team)
        {

            // Check if we have a username to steal, func automaticly steals a
            // name in it.
            if (StolenName())
            {

                // Return the name that has changed from the func above
                return format(stolen_name, "\e");
            }
        }
    }

#if ENABLE_IPC
    if (ipc::peer)
    {
        std::string namestr(*ipc_name);
        if (namestr.length() > 3)
        {
            ReplaceString(namestr, "%%", std::to_string(ipc::peer->client_id));
            ReplaceSpecials(namestr);
            return namestr;
        }
    }
#endif

    if ((*force_name).size() > 1)
    {
        auto new_name = force_name.toString();
        ReplaceSpecials(new_name);

        return new_name;
    }
    if (name_forced.size() > 1)
    {
        auto new_name = name_forced;
        ReplaceSpecials(new_name);

        return new_name;
    }
    return std::string();
}

namespace hooked_methods
{
std::string GetFriendPersonaName_name;
DEFINE_HOOKED_METHOD(GetFriendPersonaName, const char *, ISteamFriends *this_, CSteamID steam_id)
{
    if (!isHackActive())
        GetFriendPersonaName_name = "";
    else
        GetFriendPersonaName_name = GetNamestealName(steam_id);
    return (!GetFriendPersonaName_name.empty() ? GetFriendPersonaName_name.c_str() : original::GetFriendPersonaName(this_, steam_id));
}

std::string netvar_name;
static InitRoutine init([]() {
    namesteal.installChangeCallback([](settings::VariableBase<int> &var, int new_val) {
        if (new_val != 0)
        {
            std::string new_name = GetNamestealName(g_ISteamUser->GetSteamID());
            if (CE_BAD(LOCAL_E) || new_name.empty() || !strcmp(LOCAL_E->player_info.name, new_name.c_str()))
                return;
            netvar_name = std::move(new_name);
            NET_SetConVar setname("name", netvar_name.c_str());
            INetChannel *ch = (INetChannel *) g_IEngine->GetNetChannelInfo();
            if (ch)
            {
                setname.SetNetChannel(ch);
                setname.SetReliable(false);
                ch->SendNetMsg(setname, false);
            }
        }
    });
});
static Timer set_name{};
static void cm()
{
    if (!namesteal)
        return;
    if (!set_name.test_and_set(300000))
        return;
    std::string new_name = GetNamestealName(g_ISteamUser->GetSteamID());
    if (CE_BAD(LOCAL_E) || new_name.empty())
        return;
    // Didn't change name - update timer a bit
    if (!strcmp(LOCAL_E->player_info.name, new_name.c_str()))
    {
        set_name.last -= std::chrono::seconds(170);
        return;
    }
    netvar_name = std::move(new_name);
    NET_SetConVar setname("name", netvar_name.c_str());
    INetChannel *ch = (INetChannel *) g_IEngine->GetNetChannelInfo();
    if (ch)
    {
        setname.SetNetChannel(ch);
        setname.SetReliable(false);
        ch->SendNetMsg(setname, false);
    }
}

static InitRoutine runinit([]() { EC::Register(EC::CreateMove, cm, "cm_namesteal", EC::late); });

} // namespace hooked_methods
