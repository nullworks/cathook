/*
 * Noisemaker.cpp
 *
 *  Created on: Feb 2, 2017
 *      Author: nullifiedcat
 */

#include "Noisemaker.h"

#include "../copypasted/CSignature.h"
#include "../common.h"
#include "../sdk.h"

namespace hacks { namespace tf2 { namespace noisemaker {

CatVar enabled(CV_SWITCH, "noisemaker", "0", "Noisemaker spam", "Spams noisemakers Infinitly\nWorks with limited use noisemakers");

void CreateMove() {
	if (enabled) {
		if (g_GlobalVars->framecount % 100 == 0) {
			KeyValues* kv = new KeyValues("+use_action_slot_item_server");
			g_IEngine->ServerCmdKeyValues(kv);
			KeyValues* kv2 = new KeyValues("-use_action_slot_item_server");
			g_IEngine->ServerCmdKeyValues(kv2);
		}
	}
	return;
}

}}}
