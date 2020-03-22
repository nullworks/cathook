/*
 * Noisemaker.cpp
 *
 *  Created on: Feb 2, 2017
 *      Author: nullifiedcat
 */

#include "common.hpp"
#include <settings/Bool.hpp>

namespace hacks::tf2::noisemaker
{

// Merry Christmas
#if ENABLE_TEXTMODE
static settings::Boolean enable{ "noisemaker-spam.enable", "true" };
#else
static settings::Boolean enable{ "noisemaker-spam.enable", "false" };
#endif

static void CreateMove()
{
    if (enable && CE_GOOD(LOCAL_E))
    {
        if (g_GlobalVars->framecount % 100 == 0)
        {
            KeyValues *kv = new KeyValues("+use_action_slot_item_server");
            g_IEngine->ServerCmdKeyValues(kv);
            KeyValues *kv2 = new KeyValues("-use_action_slot_item_server");
            g_IEngine->ServerCmdKeyValues(kv2);
        }
    }
}

FnCommandCallbackVoid_t plus_use_action_slot_item_original;
FnCommandCallbackVoid_t minus_use_action_slot_item_original;

void plus_use_action_slot_item_hook()
{
    KeyValues *kv = new KeyValues("+use_action_slot_item_server");
    g_IEngine->ServerCmdKeyValues(kv);
}

void minus_use_action_slot_item_hook()
{
    KeyValues *kv = new KeyValues("-use_action_slot_item_server");
    g_IEngine->ServerCmdKeyValues(kv);
}

static void init()
{
    auto plus  = g_ICvar->FindCommand("+use_action_slot_item");
    auto minus = g_ICvar->FindCommand("-use_action_slot_item");
    if (!plus || !minus)
        return ConColorMsg({ 255, 0, 0, 255 }, "Could not find +use_action_slot_item! INFINITE NOISE MAKER WILL NOT WORK!!!\n");
    plus_use_action_slot_item_original  = plus->m_fnCommandCallbackV1;
    minus_use_action_slot_item_original = minus->m_fnCommandCallbackV1;
    plus->m_fnCommandCallbackV1         = plus_use_action_slot_item_hook;
    minus->m_fnCommandCallbackV1        = minus_use_action_slot_item_hook;
}
static void shutdown()
{
    auto plus  = g_ICvar->FindCommand("+use_action_slot_item");
    auto minus = g_ICvar->FindCommand("-use_action_slot_item");
    if (!plus || !minus)
        return ConColorMsg({ 255, 0, 0, 255 }, "Could not find +use_action_slot_item! INFINITE NOISE MAKER WILL NOT WORK!!!\n");
    plus->m_fnCommandCallbackV1  = plus_use_action_slot_item_original;
    minus->m_fnCommandCallbackV1 = minus_use_action_slot_item_original;
}
static InitRoutine EC([]() {
    init();
    EC::Register(EC::CreateMove, CreateMove, "Noisemaker", EC::average);
    EC::Register(EC::Shutdown, shutdown, "Noisemaker", EC::average);
});
} // namespace hacks::tf2::noisemaker
