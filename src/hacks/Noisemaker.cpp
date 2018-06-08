/*
 * Noisemaker.cpp
 *
 *  Created on: Feb 2, 2017
 *      Author: nullifiedcat
 */

#include "common.hpp"

namespace hacks
{
namespace tf2
{
namespace noisemaker
{

static CatVar
    enabled(CV_SWITCH, XORSTR("noisemaker"), XORSTR("0"), XORSTR("Noisemaker spam"),
            XORSTR("Spams noisemakers Infinitly\nWorks with limited use noisemakers"));

void CreateMove()
{
    if (enabled)
    {
        if (g_GlobalVars->framecount % 100 == 0)
        {
            KeyValues *kv = new KeyValues(XORSTR("+use_action_slot_item_server"));
            g_IEngine->ServerCmdKeyValues(kv);
            KeyValues *kv2 = new KeyValues(XORSTR("-use_action_slot_item_server"));
            g_IEngine->ServerCmdKeyValues(kv2);
        }
    }
    return;
}
}
}
}
