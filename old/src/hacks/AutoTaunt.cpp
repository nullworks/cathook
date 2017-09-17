/*
 * AutoTaunt.cpp
 *
 *  Created on: Jul 27, 2017
 *      Author: nullifiedcat
 */

#include "../common.h"
#include "../init.hpp"
#include "../hack.h"

namespace hacks { namespace tf { namespace autotaunt {

CatVar enabled(CV_SWITCH, "autotaunt", "0", "AutoTaunt", "Automatically taunt after killing an enemy, use with walkbots I guess");
CatVar chance(CV_FLOAT, "autotaunt_chance", "8", "AutoTaunt chance", "Chance of taunting after kill. 0 to 100.", 0.0f, 100.0f);

class AutoTauntListener : public IGameEventListener2 {
public:
	virtual void FireGameEvent(IGameEvent* event) {
		if (!enabled) {
			return;
		}
		if (g_IEngine->GetPlayerForUserID(event->GetInt("attacker")) == g_IEngine->GetLocalPlayer()) {
			if (RandomFloat(0, 100) <= float(chance)) {
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

}}}
