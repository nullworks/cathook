/*
 * others.cpp
 *
 *  Created on: Jan 8, 2017
 *      Author: nullifiedcat
 *
 */

#include "common.hpp"
#include "ucccccp.hpp"
#include "hack.hpp"
#include "hitrate.hpp"
#include "chatlog.hpp"
#include "sdk/netmessage.hpp"
#include <boost/algorithm/string.hpp>
#include <MiscTemporary.hpp>
#include "HookedMethods.hpp"
#if ENABLE_VISUALS

// This hook isn't used yet!
/*int C_TFPlayer__DrawModel_hook(IClientEntity *_this, int flags)
{
    float old_invis = *(float *) ((uintptr_t) _this + 79u);
    if (no_invisibility)
    {
        if (old_invis < 1.0f)
        {
            *(float *) ((uintptr_t) _this + 79u) = 0.5f;
        }
    }

    *(float *) ((uintptr_t) _this + 79u) = old_invis;
}*/

float last_say = 0.0f;

CatCommand spectate("spectate", "Spectate", [](const CCommand &args) {
    if (args.ArgC() < 1)
    {
        spectator_target = 0;
        return;
    }
    int id = atoi(args.Arg(1));
    if (!id)
        spectator_target = 0;
    else
    {
        spectator_target = g_IEngine->GetPlayerForUserID(id);
    }
});

#endif

static CatCommand plus_use_action_slot_item_server(
    "+cat_use_action_slot_item_server", "use_action_slot_item_server", []() {
        KeyValues *kv = new KeyValues("+use_action_slot_item_server");
        g_pLocalPlayer->using_action_slot_item = true;
        g_IEngine->ServerCmdKeyValues(kv);
    });

static CatCommand minus_use_action_slot_item_server(
    "-cat_use_action_slot_item_server", "use_action_slot_item_server", []() {
        KeyValues *kv = new KeyValues("-use_action_slot_item_server");
        g_pLocalPlayer->using_action_slot_item = false;
        g_IEngine->ServerCmdKeyValues(kv);
    });
