/*
  Created by Jenny White on 29.04.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#include "HookedMethods.hpp"

static CatVar ipc_name(CV_STRING, XORSTR("name_ipc"), XORSTR(""), XORSTR("IPC Name"));

static CatEnum namesteal_enum({ XORSTR("OFF"), XORSTR("PASSIVE"), XORSTR("ACTIVE") });
static CatVar
    namesteal(namesteal_enum, XORSTR("name_stealer"), XORSTR("0"), XORSTR("Name Stealer"),
              XORSTR("Attemt to steal your teammates names. Usefull for avoiding ")
              XORSTR("kicks\nPassive only changes when the name stolen is no ")
              XORSTR("longer the best name to use\nActive Attemps to change the ")
              XORSTR("name whenever possible"));

static std::string stolen_name;

// Func to get a new entity to steal name from and returns true if a target has
// been found
bool StolenName()
{

    // Array to store potential namestealer targets with a bookkeeper to tell
    // how full it is
    int potential_targets[32];
    int potential_targets_length = 0;

    // Go through entities looking for potential targets
    for (int i = 1; i < HIGHEST_ENTITY; i++)
    {
        CachedEntity *ent = ENTITY(i);

        // Check if ent is a good target
        if (!ent)
            continue;
        if (ent == LOCAL_E)
            continue;
        if (!ent->m_Type() == ENTITY_PLAYER)
            continue;
        if (ent->m_bEnemy())
            continue;

        // Check if name is current one
        player_info_s info;
        if (g_IEngine->GetPlayerInfo(ent->m_IDX, &info))
        {

            // If our name is the same as current, than change it
            if (std::string(info.name) == stolen_name)
            {
                // Since we found the ent we stole our name from and it is still
                // good, if user settings are passive, then we return true and
                // dont alter our name
                if ((int) namesteal == 1)
                {
                    return true;
                    // Otherwise we continue to change our name to something
                    // else
                }
                else
                    continue;
            }

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
    int target_random_num =
        floor(RandFloatRange(0, potential_targets_length - 0.1F));

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

namespace hooked_methods
{

DEFINE_HOOKED_METHOD(GetFriendPersonaName, const char *, ISteamFriends *this_,
                     CSteamID steam_id)
{
#if ENABLE_IPC
    if (ipc::peer)
    {
        static std::string namestr(ipc_name.GetString());
        namestr.assign(ipc_name.GetString());
        if (namestr.length() > 3)
        {
            ReplaceString(namestr, XORSTR("%%"), std::to_string(ipc::peer->client_id));
            return namestr.c_str();
        }
    }
#endif

    // Check User settings if namesteal is allowed
    if (namesteal && steam_id == g_ISteamUser->GetSteamID())
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
                return format(stolen_name, XORSTR("\x0F")).c_str();
            }
        }
    }

    if ((strlen(force_name.GetString()) > 1) &&
        steam_id == g_ISteamUser->GetSteamID())
    {

        return force_name_newlined;
    }
    return original::GetFriendPersonaName(this_, steam_id);
}
}
