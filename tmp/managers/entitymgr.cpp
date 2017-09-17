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
	m_IDX(int(((unsigned)this - (unsigned)&entity_cache::array) / sizeof(CachedEntity))),
	hitboxes(hitbox_cache::Get(unsigned(m_IDX))) {
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
	m_vecAcceleration.Zero();
	m_vecVOrigin.Zero();
	m_vecVelocity.Zero();
	m_fLastUpdate = 0;
}

CachedEntity::~CachedEntity() {}

static CatVar ve_window(CV_FLOAT, "debug_ve_window", "0", "VE Window");
static CatVar ve_smooth(CV_SWITCH, "debug_ve_smooth", "1", "VE Smoothing");
static CatVar ve_averager_size(CV_INT, "debug_ve_averaging", "8", "VE Averaging");

void CachedEntity::Update() {
	SEGV_BEGIN

	auto raw = RAW_ENT(this);

	if (!raw) return;
#if PROXY_ENTITY != true
	m_pEntity = g_IEntityList->GetClientEntity(idx);
	if (!m_pEntity) {
		return;
	}
#endif
	bool dormant = raw->IsDormant();
	bool dormant_state_changed = dormant != was_dormant;
	was_dormant = dormant;
	m_iClassID = raw->GetClientClass()->m_ClassID;
	m_vecOrigin = raw->GetAbsOrigin();
	/*float simtime = CE_FLOAT(this, netvar.m_flSimulationTime);
	float deltat = (simtime - m_fLastUpdate);
	if (ve_smooth) {
		//
		if (dormant_state_changed) {
			velocity_averager.reset(0);
			velocity_is_valid = false;
		}
		if (size_t(int(ve_averager_size)) != velocity_averager.size()) {
			velocity_averager.resize(size_t(int(ve_averager_size)));
			velocity_averager.reset(0);
		}
	}
	if (!dormant && deltat > (float)ve_window) {
		ICollideable* ca = RAW_ENT(this)->GetCollideable();
		Vector origin = m_vecOrigin;
		if (ca) {
			origin = ca->GetCollisionOrigin();
		}
		Vector delta = origin - m_vecVOrigin;
		Vector velnew = delta / deltat;
		m_vecAcceleration = velnew - m_vecVelocity;
		if (ve_smooth) {
			if (velocity_is_valid) {
				static Vector zero {0.0f, 0.0f, 0.0f};
				float length = velnew.Length();
				velocity_averager.push(length);
				Vector normalized = (length ? (velnew / length) : zero);
				m_vecVelocity = normalized * velocity_averager.average();
				//m_vecVelocity = velocity_averager.average();
			} else {
				EstimateAbsVelocity(RAW_ENT(this), m_vecVelocity);
				//velocity_averager.push(m_vecVelocity);
				velocity_is_valid = true;
			}
		} else
			m_vecVelocity = velnew;
		m_vecVOrigin = origin;
		m_fLastUpdate = simtime;
	}*/

	m_ItemType = ITEM_NONE;

	m_lSeenTicks = 0;
	m_lLastSeen = 0;

	hitboxes.Update();

	m_bGrenadeProjectile = false;
	m_bVisCheckComplete = false;

	if (m_iClassID == RCC_PLAYER) {
		m_Type = EntityType::ENTITY_PLAYER;
	} else if (m_iClassID == CL_CLASS(CTFGrenadePipebombProjectile) ||
			   m_iClassID == CL_CLASS(CTFProjectile_Cleaver) ||
			   m_iClassID == CL_CLASS(CTFProjectile_Jar) ||
			   m_iClassID == CL_CLASS(CTFProjectile_JarMilk)) {
		m_Type = EntityType::ENTITY_PROJECTILE;
		m_bGrenadeProjectile = true;
	} else if (m_iClassID == CL_CLASS(CObjectTeleporter) ||
			   m_iClassID == CL_CLASS(CObjectSentrygun) ||
			   m_iClassID == CL_CLASS(CObjectDispenser)) {
		m_Type = EntityType::ENTITY_BUILDING;
	} else if (m_iClassID == CL_CLASS(CTFProjectile_Arrow) ||
			   m_iClassID == CL_CLASS(CTFProjectile_EnergyBall) ||
			   m_iClassID == CL_CLASS(CTFProjectile_EnergyRing) ||
			   m_iClassID == CL_CLASS(CTFProjectile_GrapplingHook) ||
			   m_iClassID == CL_CLASS(CTFProjectile_HealingBolt) ||
			   m_iClassID == CL_CLASS(CTFProjectile_Rocket) ||
			   m_iClassID == CL_CLASS(CTFProjectile_SentryRocket) ||
			   m_iClassID == CL_CLASS(CTFProjectile_Flare)) {
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
		m_iTeam = NET_INT(raw, netvar.iTeamNum);
		m_bEnemy = (m_iTeam != g_pLocalPlayer->team);
	}

	if (m_Type == EntityType::ENTITY_PLAYER) {
		m_bAlivePlayer = !(NET_BYTE(raw, netvar.iLifeState));
		g_IEngine->GetPlayerInfo(m_IDX, &player_info);
		m_iTeam = NET_INT(raw, netvar.iTeamNum); // TODO
		m_bEnemy = (m_iTeam != g_pLocalPlayer->team);
		m_iHealth = NET_INT(raw, netvar.iHealth);
		m_iMaxHealth = g_pPlayerResource->GetMaxHealth(this);
	}
	if (m_Type == EntityType::ENTITY_BUILDING) {
		m_iTeam = NET_INT(raw, netvar.iTeamNum); // TODO
		m_bEnemy = (m_iTeam != g_pLocalPlayer->team);
		m_iHealth = NET_INT(raw, netvar.iBuildingHealth);
		m_iMaxHealth = NET_INT(raw, netvar.iBuildingMaxHealth);
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
