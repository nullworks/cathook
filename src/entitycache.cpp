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

CachedEntity::CachedEntity() {
#if PROXY_ENTITY != true
	m_pEntity = nullptr;
#endif
	m_Bones = 0;
	m_Bones = new matrix3x4_t[MAXSTUDIOBONES];
	m_pHitboxCache = new EntityHitboxCache(this);
	m_pPlayerInfo = 0;
	m_fLastUpdate = 0.0f;
}

CachedEntity::~CachedEntity() {
	delete [] m_Bones;
	delete m_pHitboxCache;
}

IClientEntity* CachedEntity::InternalEntity() {
	return g_IEntityList->GetClientEntity(m_IDX);
}

void EntityCache::Invalidate() {
	delete [] m_pArray;
	m_pArray = new CachedEntity[MAX_ENTITIES]();
}

void CachedEntity::Update(int idx) {
	SEGV_BEGIN
	m_IDX = idx;
	if (!RAW_ENT(this)) return;
#if PROXY_ENTITY != true
	m_pEntity = g_IEntityList->GetClientEntity(idx);
	if (!m_pEntity) {
		return;
	}
#endif
	m_iClassID = RAW_ENT(this)->GetClientClass()->m_ClassID;
	Vector origin = RAW_ENT(this)->GetAbsOrigin();
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
	m_vecOrigin = origin;

	m_ItemType = ITEM_NONE;

	m_lSeenTicks = 0;
	m_lLastSeen = 0;

	m_bGrenadeProjectile = false;
	m_bBonesSetup = false;

	m_bVisCheckComplete = false;
	if (m_pHitboxCache) {
		SAFE_CALL(m_pHitboxCache->Update());
	}
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

	if (CE_BAD(g_pLocalPlayer->entity)) return;

	if (m_Type == EntityType::ENTITY_PROJECTILE) {
		m_bCritProjectile = IsProjectileCrit(this);
		m_iTeam = CE_INT(this, netvar.iTeamNum);
		m_bEnemy = (m_iTeam != g_pLocalPlayer->team);
	}

	if (m_Type == EntityType::ENTITY_PLAYER) {
		m_bAlivePlayer = !(NET_BYTE(RAW_ENT(this), netvar.iLifeState));
		if (m_pPlayerInfo) {
			delete m_pPlayerInfo;
			m_pPlayerInfo = 0;
		}
		m_pPlayerInfo = new player_info_s;
		g_IEngine->GetPlayerInfo(m_IDX, m_pPlayerInfo);
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
	PROF_SECTION(CE_IsVisible);
	if (m_bVisCheckComplete) return m_bAnyHitboxVisible;

	bool vischeck0 = false;
	SAFE_CALL(vischeck0 = IsEntityVectorVisible(this, m_vecOrigin));

	if (vischeck0) {
		m_bAnyHitboxVisible = true;
		m_bVisCheckComplete = true;
		return true;
	}

	if (m_Type == ENTITY_PLAYER && fast_vischeck) {
		static const int hitboxes[] = { hitbox_t::head, hitbox_t::foot_L, hitbox_t::hand_R, hitbox_t::spine_1 };
		for (int i = 0; i < 4; i++) {
			if (m_pHitboxCache->VisibilityCheck(hitboxes[i])) {
				m_bAnyHitboxVisible = true;
				m_bVisCheckComplete = true;
				return true;
			}
		}
		m_bAnyHitboxVisible = false;
		m_bVisCheckComplete = true;
		return false;
	}

	for (int i = 0; i < m_pHitboxCache->m_nNumHitboxes; i++) {
		bool vischeck = false;
		SAFE_CALL(vischeck = m_pHitboxCache->VisibilityCheck(i));
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

matrix3x4_t* CachedEntity::GetBones() {
	if (!m_bBonesSetup) {
		m_bBonesSetup = RAW_ENT(this)->SetupBones(m_Bones, MAXSTUDIOBONES, 0x100, 0); // gvars->curtime
	}
	return m_Bones;
}

EntityCache::EntityCache() {
	m_pArray = new CachedEntity[MAX_ENTITIES]();
}

EntityCache::~EntityCache() {
	delete [] m_pArray;
}

void EntityCache::Update() {
	m_nMax = g_IEntityList->GetHighestEntityIndex();
	// idk that can break something i guess
	for (int i = 0; i <= m_nMax && i < MAX_ENTITIES; i++) {
		m_pArray[i].Update(i);
	}
}

CachedEntity* EntityCache::GetEntity(int idx) {
	if (idx < 0 || idx > m_nMax) {
		logging::Info("Requested invalid entity: %i max %i", idx, m_nMax);
	}
	//logging::Info("Request entity: %i, 0x%08x", idx, m_pArray[idx].m_pEntity);
	return &(m_pArray[idx]);
}

EntityCache gEntityCache;
