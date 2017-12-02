/*
 * hoovy.cpp
 *
 *  Created on: Apr 14, 2017
 *      Author: nullifiedcat
 */

#include "common.hpp"

static bool hoovy_list[32] = { 0 };

bool HasSandvichOut(CachedEntity* entity) {
	IF_GAME (!IsTF2()) return false;

	int weapon_idx;
	CachedEntity *weapon;

	weapon_idx = CE_INT(entity, netvar.hActiveWeapon) & 0xFFF;
	if (!(weapon_idx > 0 && weapon_idx < HIGHEST_ENTITY)) return false;
	weapon = ENTITY(weapon_idx);
	if (CE_GOOD(weapon)) {
		if (weapon->m_iClassID == CL_CLASS(CTFLunchBox) && CE_INT(entity, netvar.iClass) == tf_heavy) {
			return true;
		}
	}
	return false;
}

bool IsHoovyHelper(CachedEntity* entity) {
	if (HasSandvichOut(entity) && (CE_INT(entity, netvar.iFlags) & FL_DUCKING)) return true;
	return false;
}

void UpdateHoovyList() {
	static CachedEntity *ent;

	for (int i = 1; i < 32 && i < g_IEntityList->GetHighestEntityIndex(); i++) {
		ent = ENTITY(i);
		if (CE_GOOD(ent) && CE_BYTE(ent, netvar.iLifeState) == LIFE_ALIVE) {
			if (!hoovy_list[i - 1]) {
				if (IsHoovyHelper(ent)) hoovy_list[i - 1] = true;
			} else {
				if (!HasSandvichOut(ent)) hoovy_list[i - 1] = false;
			}
		}
	}
}

bool IsHoovy(CachedEntity* entity) {
	if (!entity->m_IDX || entity->m_IDX > 32) return false;
	return hoovy_list[entity->m_IDX - 1];
}
