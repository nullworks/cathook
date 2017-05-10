/*
 * HTrigger.cpp
 *
 *  Created on: Oct 5, 2016
 *      Author: nullifiedcat
 */

#include "../beforecheaders.h"
#include <memory> // std::unique_ptr
#include "../aftercheaders.h"

#include "Trigger.h"

#include "../common.h"
#include "../sdk.h"

namespace hacks { namespace shared { namespace triggerbot {

CatVar enabled(CV_SWITCH, "trigger_enabled", "0", "Enable", "Master Triggerbot switch");
CatVar respect_cloak(CV_SWITCH, "trigger_respect_cloak", "1", "Respect cloak", "Don't shoot at cloaked spies");
CatVar zoomed_only(CV_SWITCH, "trigger_zoomed", "1", "Zoomed only", "Don't shoot if you aren't zoomed in");
CatEnum hitbox_enum({
	"ANY", "HEAD", "PELVIS", "SPINE 0", "SPINE 1", "SPINE 2", "SPINE 3", "UPPER ARM L", "LOWER ARM L",
	"HAND L", "UPPER ARM R", "LOWER ARM R", "HAND R", "HIP L", "KNEE L", "FOOT L", "HIP R",
	"KNEE R", "FOOT R"
}, -1);
CatVar hitbox(hitbox_enum, "trigger_hitbox", "-1", "Hitbox", "Triggerbot hitbox. Only useful settings are ANY and HEAD. Use ANY for scatter or any other shotgun-based weapon, HEAD for ambassador/sniper rifle");
CatVar bodyshot(CV_SWITCH, "trigger_bodyshot", "1", "Bodyshot", "Triggerbot will bodyshot enemies if you have enough charge to 1tap them");
CatVar finishing_hit(CV_SWITCH, "trigger_finish", "1", "Noscope weak enemies", "If enemy has <50 HP, noscope them");
CatVar max_range(CV_INT, "trigger_range", "0", "Max range", "Triggerbot won't shoot if enemy is too far away", true, 4096.0f);
CatVar buildings(CV_SWITCH, "trigger_buildings", "1", "Trigger at buildings", "Shoot buildings");
CatVar ignore_vaccinator(CV_SWITCH, "trigger_respect_vaccinator", "1", "Respect vaccinator", "Don't shoot at bullet-vaccinated enemies");
CatVar ambassador(CV_SWITCH, "trigger_ambassador", "1", "Smart Ambassador", "Don't shoot if your ambassador can't headshot yet (Keep that enabled!)");
CatVar accuracy(CV_SWITCH, "trigger_accuracy", "0", "Improve accuracy (NOT WORKING)", "Might cause more lag (NOT WORKING YET!)");

std::unique_ptr<trace_t> trace(new trace_t);

void CreateMove() {
	if (!enabled) return;
	if (GetWeaponMode(g_pLocalPlayer->entity) != weapon_hitscan) return;
	if (ambassador) {
		if (IsAmbassador(g_pLocalPlayer->weapon())) {
			if ((g_GlobalVars->curtime - CE_FLOAT(g_pLocalPlayer->weapon(), netvar.flLastFireTime)) <= 1.0) {
				return;
			}
		}
	}
	Ray_t ray;
	trace::g_pFilterDefault->SetSelf(RAW_ENT(g_pLocalPlayer->entity));
	Vector forward;
	float sp, sy, cp, cy;
	sy = sinf(DEG2RAD(g_pUserCmd->viewangles[1])); // yaw
	cy = cosf(DEG2RAD(g_pUserCmd->viewangles[1]));

	sp = sinf(DEG2RAD(g_pUserCmd->viewangles[0])); // pitch
	cp = cosf(DEG2RAD(g_pUserCmd->viewangles[0]));

	forward.x = cp * cy;
	forward.y = cp * sy;
	forward.z = -sp;
	forward = forward * 8192.0f + g_pLocalPlayer->v_Eye;
	ray.Init(g_pLocalPlayer->v_Eye, forward);
	g_ITrace->TraceRay(ray, 0x4200400B, trace::g_pFilterDefault, trace.get());

	IClientEntity* raw_entity = (IClientEntity*)(trace->m_pEnt);
	if (!raw_entity) return;
	CachedEntity* entity = ENTITY(raw_entity->entindex());
	if (!entity->m_bEnemy) return;

	bool isPlayer = false;
	switch (entity->m_Type) {
	case EntityType::ENTITY_PLAYER:
		isPlayer = true; break;
	case EntityType::ENTITY_BUILDING:
		if (!buildings) return;
		break;
	default:
		return;
	};

	Vector enemy_pos = entity->m_vecOrigin;
	Vector my_pos = g_pLocalPlayer->entity->m_vecOrigin;
	if (max_range) {
		if (entity->m_flDistance > (float)max_range) return;
	}
	if (!isPlayer) {
		g_pUserCmd->buttons |= IN_ATTACK;
		return;
	}
	if (HasCondition(entity, TFCond_UberBulletResist) && ignore_vaccinator) return;
	if (playerlist::IsFriendly(playerlist::AccessData(entity).state)) return;
	if (IsPlayerInvulnerable(entity)) return;
	if (respect_cloak && (IsPlayerInvisible(entity))) return;
	int health = CE_INT(entity, netvar.iHealth);
	bool do_bodyshot = false;
	if (g_pLocalPlayer->clazz == tf_class::tf_sniper) {
		// If sniper..
		if (health <= 50 && finishing_hit) {
			do_bodyshot = true;
		}
		// If we need charge...
		if (!bodyshot && bodyshot) {
			float bdmg = CE_FLOAT(g_pLocalPlayer->weapon(), netvar.flChargedDamage);
			if (CanHeadshot() && (bdmg) >= health) {
				do_bodyshot = true;
			}
		}

	}
	if (!do_bodyshot && (g_pLocalPlayer->clazz == tf_class::tf_sniper) && zoomed_only &&
		!((g_pLocalPlayer->bZoomed) && CanHeadshot())) {
		return;
	}
	//debug->AddBoxOverlay(enemy_trace->endpos, Vector(-1.0f, -1.0f, -1.0f), Vector(1.0f, 1.0f, 1.0f), QAngle(0, 0, 0), 255, 0, 0, 255, 2.0f);
	//IClientEntity* weapon;
	CachedHitbox* hb = entity->hitboxes.GetHitbox(trace->hitbox);
	//logging::Info("hitbox: %i 0x%08x", enemy_trace->hitbox, hb);

	/*if (v_bImproveAccuracy->GetBool()) {
		if (hb) {
			Vector siz = hb->max - hb->min;
			Vector mns = hb->min + siz * 0.2f;
			Vector mxs = hb->max - siz * 0.2f;
			g_IVDebugOverlay->AddLineOverlay(enemy_trace->startpos, forward, 0, 0, 255, true, -1.0f);
			if (LineIntersectsBox(mns, mxs, enemy_trace->startpos, forward)) {
				g_IVDebugOverlay->AddBoxOverlay(mns, Vector(0, 0, 0), mxs - mns, QAngle(0, 0, 0), 0, 255, 0, 255, 1.0f);
				g_IVDebugOverlay->AddLineOverlay(enemy_trace->startpos, forward, 255, 0, 0, true, 1.0f);
				//logging::Info("%.2f %.2f %.2f", hb->center.DistToSqr(enemy_trace->endpos), SQR(hb->min.DistToSqr(hb->min)), SQR(hb->min.DistToSqr(hb->min) * 0.9f));

			} else {
				g_IVDebugOverlay->AddBoxOverlay(hb->min, Vector(0, 0, 0), hb->max - hb->min, QAngle(0, 0, 0), 0, 255, 255, 255, -1.0f);
				g_IVDebugOverlay->AddBoxOverlay(mns, Vector(0, 0, 0), mxs - mns, QAngle(0, 0, 0), 255, 255, 0, 255, 0.5f);
				return;
			}
		} else return;
	}*/
	if ((int)hitbox >= 0 && !bodyshot) {
		if (trace->hitbox != (int)hitbox) return;
	}
	g_pUserCmd->buttons |= IN_ATTACK;
}

void Draw() {

}

}}}
