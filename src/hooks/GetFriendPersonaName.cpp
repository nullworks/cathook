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
static settings::Boolean namesteal_reconnect("name.namesteal.reconnect", "true");
static settings::Boolean glitchy_newlines("name.namesteal.use-newlines", "false");

static std::string stolen_name;
static unsigned stolen_target;

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
    for (auto const &ent : entity_cache::player_cache)
    {
        // Check if ent is a good target
        int i = ent->m_IDX;
        if (i == g_pLocalPlayer->entity_idx)
            continue;
        if (g_pPlayerResource->GetTeam(i) != g_pLocalPlayer->team)
            continue;

        // Check if name is current one
        player_info_s info;
        if (GetPlayerInfo(i, &info))
        {
            // Should be ignored
            if (!player_tools::shouldTargetSteamId(info.friendsID))
                continue;
            // Invisible character won't fit into name with max. length
            if (std::strlen(info.name) >= 29)
                continue;
            // Ignore Friendly
            if (!player_tools::shouldTargetSteamId(info.friendsID))
                continue;
            // If our name is the same as current, then change it
            if (stolen_name == info.name && *namesteal == 1)
                return true;
        }
        // a ent without a name is no ent we need, contine for a different
        // one
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
    if (GetPlayerInfo(new_target, &info))
    {
        // If our name is the same as current, than change it and return true
        stolen_name   = std::string(info.name);
        stolen_target = info.friendsID;
        return true;
    }

    // Didnt get playerinfo
    return false;
}

// Used to reconnect after changing name
static bool has_changed = false;

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

            std::string previous_name = stolen_name;
            // Check if we have a username to steal, func automaticly steals a
            // name in it.
            StolenName();

            has_changed = stolen_name != previous_name;

            if (stolen_name != "")
                // Return the name that has changed from the func above
                return format(stolen_name, glitchy_newlines ? "\n\n\n" : "\u2063");
        }
        else if (stolen_name != "")
            return format(stolen_name, glitchy_newlines ? "\n\n\n" : "\u2063");
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
static InitRoutine init(
    []()
    {
        namesteal.installChangeCallback(
            [](settings::VariableBase<int> &, int new_val)
            {
                if (new_val != 0)
                {
                    std::string new_name = GetNamestealName(g_ISteamUser->GetSteamID());
                    if (CE_BAD(LOCAL_E) || new_name.empty() || !strcmp(LOCAL_E->player_info->name, new_name.c_str()))
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
static Timer reconnect{};
static void cm()
{
    if (!namesteal)
        return;
    // Reconnect after name change
    if (has_changed && reconnect.test_and_set(120000) && namesteal_reconnect)
    {
        has_changed = false;
        // Only passive should reconnect
        if (*namesteal == 1)
        {
            static std::string previous_server = "";
            static int retry_count             = 0;
            if (previous_server != ((INetChannel *) g_IEngine->GetNetChannelInfo())->GetAddress())
            {
                previous_server = ((INetChannel *) g_IEngine->GetNetChannelInfo())->GetAddress();
                retry_count     = 0;
            }
            retry_count++;
            // Retry only up to 3 times, else you get an ad-hoc error
            if (retry_count <= 3)
                g_IEngine->ClientCmd_Unrestricted("retry");
        }
    }
    if (!set_name.test_and_set(30000))
        return;
    std::string new_name = GetNamestealName(g_ISteamUser->GetSteamID());
    if (CE_BAD(LOCAL_E) || new_name.empty())
        return;
    // Didn't change name - update timer a bit
    if (!strcmp(LOCAL_E->player_info->name, new_name.c_str()))
    {
        set_name.last -= std::chrono::seconds(170);
        return;
    }
    has_changed = true;
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
