/*
 * targethelper.cpp
 *
 *  Created on: Oct 16, 2016
 *      Author: nullifiedcat
 */

#include "common.h"

/*
 * Targeting priorities:
 * passive bullet vacc medic
 * zoomed snipers ALWAYS
 * medics
 * snipers
 * spies
 */

/* Assuming given entity is a valid target range 0 to 100 */
int GetScoreForEntity(CachedEntity* entity) {
	if (!entity) return 0;
	// TODO
	if (entity->m_Type == ENTITY_BUILDING) {
		if (entity->m_iClassID == g_pClassID->CObjectSentrygun) {
			return 1;
		}
		return 0;
	}
	int clazz = CE_INT(entity, netvar.iClass);
	int health = CE_INT(entity, netvar.iHealth);
	float distance = (g_pLocalPlayer->v_Origin - entity->m_vecOrigin).Length();
	bool zoomed = HasCondition(entity, TFCond_Zoomed);
	bool pbullet = HasCondition(entity, TFCond_SmallBulletResist);
	bool special = false;
	bool kritz = IsPlayerCritBoosted(entity);
	int total = 0;
	switch(clazz) {
	case tf_sniper:
		total += 25;
		if (zoomed) {
			total += 50;
		}
		special = true;
	break;
	case tf_medic:
		total += 50;
		if (pbullet) return 100;
	break;
	case tf_spy:
		total += 20;
		if (distance < 400) total += 60;
		special = true;
	break;
	case tf_soldier:
		if (HasCondition(entity, TFCond_BlastJumping)) total += 30;
	break;
	}
	if (!special) {
		if (pbullet) {
			total += 50;
		}
		if (kritz) {
			total += 99;
		}
		if (distance != 0) {
			int distance_factor = (4096 / distance) * 4;
			total += distance_factor;
			if (health != 0) {
				int health_factor = (450 / health) * 4;
				if (health_factor > 30) health_factor = 30;
				total += health_factor;
			}
		}
	}
	if (total > 99) total = 99;
	if (playerlist::AccessData(entity).state == playerlist::k_EState::RAGE) total = 999;
	if (IsSentryBuster(entity)) total = 0;
	if (clazz == tf_medic) total = 999; // TODO only for mvm
	return total;
}
