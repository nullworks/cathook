/*
 * entitycache.h
 *
 *  Created on: Nov 7, 2016
 *      Author: nullifiedcat
 */

#ifndef ENTITYCACHE_H_
#define ENTITYCACHE_H_

#include "enums.h"
#include "itemtypes.h"
#include "fixsdk.h"
#include <mathlib/vector.h>
#include <mathlib/mathlib.h>
#include <cdll_int.h>

struct matrix3x4_t;

class IClientEntity;
struct player_info_s;
struct model_t;
struct mstudiohitboxset_t;
struct mstudiobbox_t;

#define MAX_STRINGS 16
#define MAX_ENTITIES 2048

#define PROXY_ENTITY true

#if PROXY_ENTITY == true
#define RAW_ENT(ce) ((ce) ? (ce)->InternalEntity() : nullptr)
#else
#define RAW_ENT(ce) ce->m_pEntity
#endif

#define CE_VAR(entity, offset, type) \
	NET_VAR(RAW_ENT(entity), offset, type)

#define CE_INT(entity, offset) CE_VAR(entity, offset, int)
#define CE_FLOAT(entity, offset) CE_VAR(entity, offset, float)
#define CE_BYTE(entity, offset) CE_VAR(entity, offset, unsigned char)
#define CE_VECTOR(entity, offset) CE_VAR(entity, offset, Vector)

#define CE_GOOD(entity) (!g_Settings.bInvalid && dynamic_cast<CachedEntity*>(entity) && entity->m_iClassID && RAW_ENT(entity) && !RAW_ENT(entity)->IsDormant())
#define CE_BAD(entity) (!CE_GOOD(entity))

#define IDX_GOOD(idx) (idx >= 0 && idx <= HIGHEST_ENTITY && idx < MAX_ENTITIES)
#define IDX_BAD(idx) !IDX_GOOD(idx)

#define HIGHEST_ENTITY (entity_cache::max)
#define ENTITY(idx) (&entity_cache::Get(idx))

struct CachedHitbox {
	Vector min;
	Vector max;
	Vector center;
	mstudiobbox_t* bbox;
};

#define CACHE_MAX_HITBOXES 64

class EntityHitboxCache {
public:
	EntityHitboxCache(CachedEntity* parent);
	~EntityHitboxCache();

	CachedHitbox* GetHitbox(int id);
	void Update();
	void InvalidateCache();
	bool VisibilityCheck(int id);
	void Init();
	int GetNumHitboxes();
	void Reset();

	bool m_VisCheckValidationFlags[CACHE_MAX_HITBOXES] { false };
	bool m_VisCheck[CACHE_MAX_HITBOXES] { false };
	bool m_CacheValidationFlags[CACHE_MAX_HITBOXES] { false };
	CachedHitbox m_CacheInternal[CACHE_MAX_HITBOXES] {};

	int m_nNumHitboxes;
	bool m_bModelSet;
	bool m_bInit;
	bool m_bSuccess;

	mstudiohitboxset_t* m_pHitboxSet;
	model_t* m_pLastModel;
	CachedEntity* parent_ref; // TODO FIXME turn this into an actual reference
};

class CachedEntity {
public:
	CachedEntity();
	~CachedEntity();

	void Update();
	bool IsVisible();
	matrix3x4_t* GetBones();
	IClientEntity* InternalEntity();
	void Reset();

	// Entity fields start here
	EntityType m_Type { ENTITY_GENERIC };

	int m_iClassID { 0 };
	float m_flDistance { 0.0f };

	bool m_bCritProjectile { false };
	bool m_bGrenadeProjectile { false };

	bool m_bAnyHitboxVisible { false };
	bool m_bVisCheckComplete { false };

	Vector m_vecOrigin { 0 };

	k_EItemType m_ItemType { ITEM_NONE };
	int  m_iTeam { 0 };
	bool m_bAlivePlayer { false };
	bool m_bEnemy { false };
	int m_iMaxHealth { 0 };
	int m_iHealth { 0 };

	unsigned long m_lSeenTicks { 0 };
	unsigned long m_lLastSeen { 0 };

	player_info_s player_info {};
	matrix3x4_t m_Bones[128]; // MAXSTUDIOBONES
	bool m_bBonesSetup { false };

	// Players, Buildings, Stickies


	// Entity fields end here.

	const int m_IDX;
	Vector m_vecVOrigin { 0 };
	Vector m_vecVelocity { 0 };
	Vector m_vecAcceleration { 0 };
	float m_fLastUpdate { 0.0f };
	EntityHitboxCache hitboxes;
#if PROXY_ENTITY != true
	IClientEntity* m_pEntity { nullptr };
#endif
};

namespace entity_cache {

extern CachedEntity array[MAX_ENTITIES]; // b1g fat array in
inline CachedEntity& Get(int idx) {
	if (idx < 0 || idx >= 2048) throw std::out_of_range("Entity index out of range!");
	return array[idx];
}
void Update();
void Invalidate();
extern int max;

}

#endif /* ENTITYCACHE_H_ */
