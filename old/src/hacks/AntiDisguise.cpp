/*
 * AntiDisguise.cpp
 *
 *  Created on: Nov 16, 2016
 *      Author: nullifiedcat
 */

#include "AntiDisguise.h"

#include "../common.h"
#include "../sdk.h"

namespace hacks { namespace tf2 { namespace antidisguise {

CatVar enabled(CV_SWITCH, "antidisguise", "0", "Remove spy disguise", "Removes the disguise from spys\nUsefull for aimbot");

void Draw() {
	CachedEntity *ent;
	if (!enabled) return;
	for (int i = 0; i < 32 && i < HIGHEST_ENTITY; i++) {
		ent = ENTITY(i);
		if (CE_BAD(ent)) continue;
		if (ent->m_Type == ENTITY_PLAYER) {
			if (CE_INT(ent, netvar.iClass) == tf_class::tf_spy) {
				RemoveCondition<TFCond_Disguised>(ent);
			}
		}
	}
}

}}}
