/*
 * Noisemaker.cpp
 *
 *  Created on: Feb 2, 2017
 *      Author: nullifiedcat
 */

#include <settings/Bool.hpp>
#include "common.hpp"

static settings::Bool enable{ "noisemaker-spam.enable", "false" };

namespace hacks::tf2::noisemaker
{

void CreateMove()
{
    if (enable)
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
} // namespace hacks::tf2::noisemaker
