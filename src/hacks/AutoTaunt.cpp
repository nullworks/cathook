/*
 * AutoTaunt.cpp
 *
 *  Created on: Jul 27, 2017
 *      Author: nullifiedcat
 */

#include "common.hpp"
#include "hack.hpp"

namespace hacks
{
namespace tf
{
namespace autotaunt
{

static CatVar enabled(
    CV_SWITCH, XORSTR("autotaunt"), XORSTR("0"), XORSTR("AutoTaunt"),
    XORSTR("Automatically taunt after killing an enemy, use with walkbots I guess"));
static CatVar chance(CV_FLOAT, XORSTR("autotaunt_chance"), XORSTR("8"), XORSTR("AutoTaunt chance"),
                     XORSTR("Chance of taunting after kill. 0 to 100."), 0.0f, 100.0f);

class AutoTauntListener : public IGameEventListener2
{
public:
    virtual void FireGameEvent(IGameEvent *event)
    {
        if (!enabled)
        {
            return;
        }
        if (g_IEngine->GetPlayerForUserID(event->GetInt(XORSTR("attacker"))) ==
            g_IEngine->GetLocalPlayer())
        {
            if (RandomFloat(0, 100) <= float(chance))
            {
                hack::ExecuteCommand(XORSTR("taunt"));
            }
        }
    }
};

AutoTauntListener listener;

// TODO remove event listener when uninjecting?
InitRoutine init([]() {
    g_IEventManager2->AddListener(&listener, XORSTR("player_death"), false);
});
}
}
}
