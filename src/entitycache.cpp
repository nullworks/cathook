/*
 * entitycache.cpp
 *
 *  Created on: Nov 7, 2016
 *      Author: nullifiedcat
 */

#include "common.h"
#include "sdk.h"

#include <time.h>
#include "profiler.h"

// This method of const'ing the index is weird.
CachedEntity::CachedEntity() :
	m_IDX(((unsigned)this - (unsigned)&entity_cache::array) / sizeof(CachedEntity)),
	hitboxes(this) {
#if PROXY_ENTITY != true
	m_pEntity = nullptr;
#endif
	m_fLastUpdate = 0.0f;
}

void CachedEntity::Reset() {
	m_Type = ENTITY_GENERIC;
	m_iClassID = 0;
	m_flDistance = 0.0f;
	m_bCritProjectile = false;
	m_bGrenadeProjectile = false;
	m_bAnyHitboxVisible = false;
	m_bVisCheckComplete = false;
	m_vecOrigin.Zero();
	m_ItemType = ITEM_NONE;
	m_iTeam = 0;
	m_bAlivePlayer = false;
	m_bEnemy = false;
	m_iMaxHealth = 0;
	m_iHealth = 0;
	m_lLastSeen = 0;
	m_lSeenTicks = 0;
	memset(&player_info, 0, sizeof(player_info_s));
	memset(&m_Bones, 0, sizeof(matrix3x4_t) * 128);
	m_bBonesSetup = false;
	m_vecAcceleration.Zero();
	m_vecVOrigin.Zero();
	m_vecVelocity.Zero();
	m_fLastUpdate = 0;
	hitboxes.Reset();
}

CachedEntity::~CachedEntity() {}

IClientEntity* CachedEntity::InternalEntity() {
	return g_IEntityList->GetClientEntity(m_IDX);
}

void CachedEntity::Update() {
	SEGV_BEGIN
	if (!RAW_ENT(this)) return;
#if PROXY_ENTITY != true
	m_pEntity = g_IEntityList->GetClientEntity(idx);
	if (!m_pEntity) {
		return;
	}
#endif
	m_iClassID = RAW_ENT(this)->GetClientClass()->m_ClassID;
	//if (TF2 && EstimateAbsVelocity) EstimateAbsVelocity(m_pEntity, m_vecVelocity);
	/*if ((gvars->realtime - m_fLastUpdate) >= 0.05f) {
		//if (gvars->tickcount - m_nLastTick > 1) {
			//logging::Info("Running %i ticks behind!", gvars->tickcount - m_nLastTick);
		//}
		//Vector velnew = (origin - m_vecVOrigin) * (0.05f / (m_fLastUpdate - gvars->realtime)) * 20;
		Vector velnew;
		if (EstimateAbsVelocity)
			EstimateAbsVelocity(m_pEntity, velnew);
		m_vecAcceleration = (velnew - m_vecVelocity);
		m_vecVelocity = (m_vecVelocity + velnew) / 2;
		//logging::Info("Multiplier for %i: %f", m_IDX, (0.1f / (m_fLastUpdate - gvars->realtime)));
		m_vecVOrigin = origin;
		m_fLastUpdate = gvars->realtime;
	}*/
	m_vecOrigin = RAW_ENT(this)->GetAbsOrigin();

	m_ItemType = ITEM_NONE;

	m_lSeenTicks = 0;
	m_lLastSeen = 0;

	m_bGrenadeProjectile = false;
	m_bBonesSetup = false;

	m_bVisCheckComplete = false;

	SAFE_CALL(hitboxes.Update());

	if (m_iClassID == g_pClassID->C_Player) {
		m_Type = EntityType::ENTITY_PLAYER;
	} else if (m_iClassID == g_pClassID->CTFGrenadePipebombProjectile ||
			   m_iClassID == g_pClassID->CTFProjectile_Cleaver ||
			   m_iClassID == g_pClassID->CTFProjectile_Jar ||
			   m_iClassID == g_pClassID->CTFProjectile_JarMilk) {
		m_Type = EntityType::ENTITY_PROJECTILE;
		m_bGrenadeProjectile = true;
	} else if (m_iClassID == g_pClassID->CObjectTeleporter ||
			   m_iClassID == g_pClassID->CObjectSentrygun ||
			   m_iClassID == g_pClassID->CObjectDispenser) {
		m_Type = EntityType::ENTITY_BUILDING;
	} else if (m_iClassID == g_pClassID->CTFProjectile_Arrow ||
			   m_iClassID == g_pClassID->CTFProjectile_EnergyBall ||
			   m_iClassID == g_pClassID->CTFProjectile_EnergyRing ||
			   m_iClassID == g_pClassID->CTFProjectile_GrapplingHook ||
			   m_iClassID == g_pClassID->CTFProjectile_HealingBolt ||
			   m_iClassID == g_pClassID->CTFProjectile_Rocket ||
			   m_iClassID == g_pClassID->CTFProjectile_SentryRocket ||
			   m_iClassID == g_pClassID->CTFProjectile_Flare) {
		m_Type = EntityType::ENTITY_PROJECTILE;
	} else {
		m_ItemType = g_ItemManager.GetItemType(this);
		m_Type = EntityType::ENTITY_GENERIC;
	}

	if (CE_GOOD(g_pLocalPlayer->entity)) {
		m_flDistance = (g_pLocalPlayer->v_Origin.DistTo(m_vecOrigin));
	}
	m_bAlivePlayer = false;
	// TODO temporary!
	/*m_bCritProjectile = false;
	m_bIsVisible = false;
	m_iTeam = 0;
	m_bEnemy = false;
	m_bAlivePlayer = false;
	m_pPlayerInfo = 0;
	m_iHealth = 0;
	m_iMaxHealth = 0;
	m_lLastSeen = 0;
	m_lSeenTicks = 0;*/

	if (CE_BAD(g_pLocalPlayer->entity)) return;

	if (m_Type == EntityType::ENTITY_PROJECTILE) {
		m_bCritProjectile = IsProjectileCrit(this);
		m_iTeam = CE_INT(this, netvar.iTeamNum);
		m_bEnemy = (m_iTeam != g_pLocalPlayer->team);
	}

	if (m_Type == EntityType::ENTITY_PLAYER) {
		m_bAlivePlayer = !(NET_BYTE(RAW_ENT(this), netvar.iLifeState));
		g_IEngine->GetPlayerInfo(m_IDX, &player_info);
		m_iTeam = CE_INT(this, netvar.iTeamNum); // TODO
		m_bEnemy = (m_iTeam != g_pLocalPlayer->team);
		m_iHealth = CE_INT(this, netvar.iHealth);
		m_iMaxHealth = g_pPlayerResource->GetMaxHealth(this);
	}
	if (m_Type == EntityType::ENTITY_BUILDING) {
		m_iTeam = CE_INT(this, netvar.iTeamNum); // TODO
		m_bEnemy = (m_iTeam != g_pLocalPlayer->team);
		m_iHealth = CE_INT(this, netvar.iBuildingHealth);
		m_iMaxHealth = CE_INT(this, netvar.iBuildingMaxHealth);
	}
	SEGV_END_INFO("Updating entity");
}

static CatVar fast_vischeck(CV_SWITCH, "fast_vischeck", "0", "Fast VisCheck", "VisCheck only certain player hitboxes");

bool CachedEntity::IsVisible() {
	static constexpr int optimal_hitboxes[] = { hitbox_t::head, hitbox_t::foot_L, hitbox_t::hand_R, hitbox_t::spine_1 };
	static bool vischeck0, vischeck;

	PROF_SECTION(CE_IsVisible);
	if (m_bVisCheckComplete) return m_bAnyHitboxVisible;

	vischeck0 = false;
	SAFE_CALL(vischeck0 = IsEntityVectorVisible(this, m_vecOrigin));

	if (vischeck0) {
		m_bAnyHitboxVisible = true;
		m_bVisCheckComplete = true;
		return true;
	}

	if (m_Type == ENTITY_PLAYER && fast_vischeck) {
		for (int i = 0; i < 4; i++) {
			if (hitboxes.VisibilityCheck(optimal_hitboxes[i])) {
				m_bAnyHitboxVisible = true;
				m_bVisCheckComplete = true;
				return true;
			}
		}
		m_bAnyHitboxVisible = false;
		m_bVisCheckComplete = true;
		return false;
	}

	for (int i = 0; i < hitboxes.m_nNumHitboxes; i++) {
		vischeck = false;
		SAFE_CALL(vischeck = hitboxes.VisibilityCheck(i));
		if (vischeck) {
			m_bAnyHitboxVisible = true;
			m_bVisCheckComplete = true;
			return true;
		}
	}
	m_bAnyHitboxVisible = false;
	m_bVisCheckComplete = true;

	return false;
}

static CatEnum setupbones_time_enum({ "ZERO",  "CURTIME", "LP SERVERTIME", "SIMTIME" });
static CatVar setupbones_time(setupbones_time_enum, "setupbones_time", "3", "Setupbones", "Defines setupbones 4th argument, change it if your aimbot misses, idk!!");

matrix3x4_t* CachedEntity::GetBones() {
	static float bones_setup_time = 0.0f;
	switch ((int)setupbones_time) {
	case 1:
		bones_setup_time = g_GlobalVars->curtime;
		break;
	case 2:
		if (CE_GOOD(LOCAL_E))
			bones_setup_time = g_GlobalVars->interval_per_tick * CE_INT(LOCAL_E, netvar.nTickBase);
		break;
	case 3:
		if (CE_GOOD(this))
			bones_setup_time = CE_FLOAT(this, netvar.m_flSimulationTime);
	}
	if (!m_bBonesSetup) {
		m_bBonesSetup = RAW_ENT(this)->SetupBones(m_Bones, MAXSTUDIOBONES, 0x100, bones_setup_time); // gvars->curtime
	}
	return m_Bones;
}

namespace entity_cache {

CachedEntity array[MAX_ENTITIES] {};

void Update() {
	max = g_IEntityList->GetHighestEntityIndex();
	if (max >= MAX_ENTITIES) max = MAX_ENTITIES - 1;
	for (int i = 0; i <= max; i++) {
		array[i].Update();
	}
}

void Invalidate() {
	for (auto& ent : array) {
		// pMuch useless line!
		// ent.m_pEntity = nullptr;
		ent.Reset();
	}
}

int max = 0;

}
