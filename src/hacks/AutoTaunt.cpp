/*
 * AutoTaunt.cpp
 *
 *  Created on: Jul 27, 2017
 *      Author: nullifiedcat
 */

#include <settings/Float.hpp>
#include "common.hpp"
#include "hack.hpp"

static settings::Bool enable{ "autotaunt.enable", "false" };
static settings::Float chance{ "autotaunt.chance", "8" };

namespace hacks::tf::autotaunt
{

class AutoTauntListener : public IGameEventListener2
{
public:
    virtual void FireGameEvent(IGameEvent *event)
    {
        if (!enable)
        {
            return;
        }
        if (g_IEngine->GetPlayerForUserID(event->GetInt("attacker")) ==
            g_IEngine->GetLocalPlayer())
        {
            if (RandomFloat(0, 100) <= float(chance))
            {
                hack::ExecuteCommand("taunt");
            }
        }
    }
};

AutoTauntListener listener;

// TODO remove event listener when uninjecting?
InitRoutine init([]() {
    g_IEventManager2->AddListener(&listener, "player_death", false);
});
} // namespace hacks::tf::autotaunt
