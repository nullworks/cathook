/*
 * others.cpp
 *
 *  Created on: Jan 8, 2017
 *      Author: nullifiedcat
 *
 */

#include "common.hpp"
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
    if (args.ArgC() < 2)
    {
        spectator_target = 0;
        return;
    }
    int id;
    try
    {
        id = std::stoi(args.Arg(1));
    }
    catch (const std::exception &e)
    {
        logging::Info("Error while setting spectate target. Error: %s", e.what());
        id = 0;
    }
    if (!id)
        spectator_target = 0;
    else
    {
        spectator_target = g_IEngine->GetPlayerForUserID(id);
    }
});

#endif
