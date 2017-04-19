/*
 * AutoHeal.cpp
 *
 *  Created on: Dec 3, 2016
 *      Author: nullifiedcat
 */

#include "AutoHeal.h"

#include "../common.h"
#include "../sdk.h"

namespace hacks { namespace tf { namespace autoheal {

CatVar enabled(CV_SWITCH, "autoheal_enabled", "0", "AutoHeal", "Automatically heals nearby teammates");
CatVar silent(CV_SWITCH, "autoheal_silent", "1", "Silent AutoHeal", "Silent AutoHeal. Disable this to make ghetto followbot");
//extern CatVar target_only;

int m_iCurrentHealingTarget { -1 };
int m_iNewTarget { 0 };

static CatVar pop_uber_auto(CV_SWITCH, "autoheal_uber", "1", "AutoUber");
static CatVar pop_uber_percent(CV_FLOAT, "autoheal_uber_health", "30", "Pop uber if health% <");
static CatVar share_uber(CV_SWITCH, "autoheal_share_uber", "1", "Share ubercharge");

int force_healing_target { 0 };

static CatCommand heal_steamid("autoheal_heal_steamid", "Heals a player with SteamID (ONCE. Use for easy airstuck med setup)", [](const CCommand& args) {
	if (args.ArgC() < 2) {
		logging::Info("Invalid call!");
		return;
	}
	for (int i = 1; i <= 32 && i < HIGHEST_ENTITY; i++) {
		CachedEntity* ent = ENTITY(i);
		if (CE_BAD(ent)) continue;
		if (ent->m_Type != ENTITY_PLAYER) continue;
		if (!ent->m_pPlayerInfo) continue;
		if (ent->m_pPlayerInfo->friendsID == strtol(args.Arg(1), nullptr, 10)) {
			force_healing_target = i;
			return;
		}
	}
});

bool IsPopped() {
	CachedEntity* weapon = g_pLocalPlayer->weapon();
	if (CE_BAD(weapon) || weapon->m_iClassID != g_pClassID->CWeaponMedigun) return false;
	return CE_BYTE(weapon, netvar.bChargeRelease);
}

bool ShouldChargePlayer(int idx) {
	CachedEntity* target = ENTITY(idx);
	const float damage_accum_duration = g_GlobalVars->curtime - data[idx].accum_damage_start;
	const int health = target->m_iHealth;
	if (!data[idx].accum_damage_start) return false;
	if (health > 30 && data[idx].accum_damage < 45) return false;
	const float dd = ((float)data[idx].accum_damage / damage_accum_duration);
	if (dd > 40) {
		return true;
	}
	if (health < 30 && data[idx].accum_damage > 10) return true;
	return false;
}

bool ShouldPop() {
	if (IsPopped()) return false;
	if (m_iCurrentHealingTarget != -1) {
		CachedEntity* target = ENTITY(m_iCurrentHealingTarget);
		if (CE_GOOD(target)) {
			if (ShouldChargePlayer(m_iCurrentHealingTarget)) return true;
		}
	}
	return ShouldChargePlayer(LOCAL_E->m_IDX);
}

void CreateMove() {
	if (!enabled && !force_healing_target) return;
	if (GetWeaponMode(g_pLocalPlayer->entity) != weapon_medigun) return;
	if (force_healing_target) {
		CachedEntity* target = ENTITY(force_healing_target);
		if (CE_GOOD(target)) {
			Vector out;
			GetHitbox(target, 7, out);
			AimAt(g_pLocalPlayer->v_Eye, out, g_pUserCmd);
			g_pUserCmd->buttons |= IN_ATTACK;
			force_healing_target = 0;
		}
	}
	if (!enabled) return;
	UpdateData();
	int old_target = m_iCurrentHealingTarget;
	m_iCurrentHealingTarget = BestTarget();
	if (m_iNewTarget > 0 && m_iNewTarget < 10) m_iNewTarget++;
	else m_iNewTarget = 0;
	bool new_target = (old_target != m_iCurrentHealingTarget);
	if (new_target) {
		m_iNewTarget = 1;
	}
	if (m_iCurrentHealingTarget == -1) return;
	CachedEntity* target = ENTITY(m_iCurrentHealingTarget);
	Vector out;
	GetHitbox(target, 7, out);
	AimAt(g_pLocalPlayer->v_Eye, out, g_pUserCmd);
	if (silent) g_pLocalPlayer->bUseSilentAngles = true;
	if (!m_iNewTarget && (g_GlobalVars->tickcount % 300)) g_pUserCmd->buttons |= IN_ATTACK;
	if (pop_uber_auto && ShouldPop()) g_pUserCmd->buttons |= IN_ATTACK2;
	return;
}

std::vector<patient_data_s> data(32);
void UpdateData() {
	for (int i = 1; i < 32; i++) {
		CachedEntity* ent = ENTITY(i);
		if (CE_GOOD(ent)) {
			int health = ent->m_iHealth;
			if (data[i].last_damage > g_GlobalVars->curtime) {
				data[i].last_damage = 0.0f;
			}
			if (g_GlobalVars->curtime - data[i].last_damage > 5.0f) {
				data[i].accum_damage = 0;
				data[i].accum_damage_start = 0.0f;
			}
			const int last_health = data[i].last_health;
			if (health != last_health) {
				data[i].last_health = health;
				if (health < last_health) {
					data[i].accum_damage += (last_health - health);
					if (!data[i].accum_damage_start) data[i].accum_damage_start = g_GlobalVars->curtime;
					data[i].last_damage = g_GlobalVars->curtime;
				}
			}
		}
	}
}

int BestTarget() {
	int best = -1;
	int best_score = -65536;
	for (int i = 0; i < 32 && i < HIGHEST_ENTITY; i++) {
		int score = HealingPriority(i);
		if (score > best_score && score != -1) {
			best = i;
			best_score = score;
		}
	}
	return best;
}

int HealingPriority(int idx) {
	if (!CanHeal(idx)) return -1;
	CachedEntity* ent = ENTITY(idx);
	if (share_uber && IsPopped()) {
		return !HasCondition(ent, TFCond_Ubercharged);
	}

	int priority = 0;
	int health = CE_INT(ent, netvar.iHealth);
	int maxhealth = g_pPlayerResource->GetMaxHealth(ent);
	int maxbuffedhealth = maxhealth * 1.5;
	int maxoverheal = maxbuffedhealth - maxhealth;
	int overheal = maxoverheal - (maxbuffedhealth - health);
	float overhealp = ((float)overheal / (float)maxoverheal);
	float healthp = ((float)health / (float)maxhealth);
	switch (playerlist::AccessData(ent).state) {
	case playerlist::k_EState::FRIEND:
		priority += 70 * (1 - healthp);
		priority += 15 * (1 - overhealp);
		break;
	case playerlist::k_EState::IPC:
		priority += 100 * (1 - healthp);
		priority += 20 * (1 - overhealp);
		break;
	default:
		priority += 50 * (1 - healthp);
		priority += 10 * (1 - overhealp);
	}
	if (ipc::peer) {
		if (hacks::shared::followbot::bot && hacks::shared::followbot::following_idx == idx) {
			priority *= 3.0f;
		}
	}
	return priority;
}

bool CanHeal(int idx) {
	CachedEntity* ent = ENTITY(idx);
	if (!ent) return false;
	if (CE_BAD(ent)) return false;
	if (ent->m_Type != ENTITY_PLAYER) return false;
	if (g_IEngine->GetLocalPlayer() == idx) return false;
	if (!ent->m_bAlivePlayer) return false;
	if (ent->m_bEnemy) return false;
	if (ent->m_flDistance > 420) return false;
	// TODO visible any hitbox
	if (!IsEntityVisible(ent, 7)) return false;
	if (IsPlayerInvisible(ent)) return false;
	return true;
}

}}}
