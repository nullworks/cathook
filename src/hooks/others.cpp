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
#include "netmessage.hpp"
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

CatCommand spectate(XORSTR("spectate"), XORSTR("Spectate"), [](const CCommand &args) {
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
    XORSTR("+cat_use_action_slot_item_server"), XORSTR("use_action_slot_item_server"), []() {
        KeyValues *kv = new KeyValues(XORSTR("+use_action_slot_item_server"));
        g_pLocalPlayer->using_action_slot_item = true;
        g_IEngine->ServerCmdKeyValues(kv);
    });

static CatCommand minus_use_action_slot_item_server(
    XORSTR("-cat_use_action_slot_item_server"), XORSTR("use_action_slot_item_server"), []() {
        KeyValues *kv = new KeyValues(XORSTR("-use_action_slot_item_server"));
        g_pLocalPlayer->using_action_slot_item = false;
        g_IEngine->ServerCmdKeyValues(kv);
    });

// TODO replace \\n with \n
// TODO name \\n = \n
// static CatVar queue_messages(CV_SWITCH, XORSTR("chat_queue"), XORSTR("0"), XORSTR("Queue messages"),
// XORSTR("Use this if you want to use spam/killsay and still be able to chat normally
// (without having your msgs eaten by valve cooldown)XORSTR(");
static CatVar airstuck(CV_KEY, XORSTR("airstuck"), XORSTR("0"), XORSTR("Airstuck"), XORSTR(""));
