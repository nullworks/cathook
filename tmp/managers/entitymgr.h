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
#include "interfaces.h"
#include "entityhitboxcache.hpp"
#include "fixsdk.h"

#include "beforecheaders.h"
#include "averager.hpp"
#include "aftercheaders.h"

#include <mathlib/vector.h>
#include <mathlib/mathlib.h>
#include <icliententity.h>
#include <icliententitylist.h>
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
#define RAW_ENT(ce) ce->InternalEntity()
#else
#define RAW_ENT(ce) ce->m_pEntity
#endif

#define CE_VAR(entity, offset, type) \
	NET_VAR(RAW_ENT(entity), offset, type)

#define CE_INT(entity, offset) CE_VAR(entity, offset, int)
#define CE_FLOAT(entity, offset) CE_VAR(entity, offset, float)
#define CE_BYTE(entity, offset) CE_VAR(entity, offset, unsigned char)
#define CE_VECTOR(entity, offset) CE_VAR(entity, offset, Vector)

#define CE_GOOD(entity) (entity && !g_Settings.bInvalid && entity->Good())
#define CE_BAD(entity) (!CE_GOOD(entity))

#define IDX_GOOD(idx) (idx >= 0 && idx <= HIGHEST_ENTITY && idx < MAX_ENTITIES)
#define IDX_BAD(idx) !IDX_GOOD(idx)

#define HIGHEST_ENTITY (entity_cache::max)
#define ENTITY(idx) (&entity_cache::Get(idx))

class CachedEntity {
public:
	CachedEntity();
	~CachedEntity();

	__attribute__((hot)) void Update();
	bool IsVisible();
	void Reset();
	__attribute__((always_inline, hot, const)) IClientEntity* InternalEntity() const {
		return g_IEntityList->GetClientEntity(m_IDX);
	}
	__attribute__((always_inline, hot, const)) inline bool Good() const {
		if (!m_iClassID) return false;
		IClientEntity* const entity = InternalEntity();
		return entity && !entity->IsDormant();
	}
	template<typename T>
	__attribute__((always_inline, hot, const)) inline T& var(uintptr_t offset) const {
		return *reinterpret_cast<T*>(uintptr_t(RAW_ENT(this)) + offset);
	}

	const int m_IDX;

	int m_iClassID { 0 };

	Vector m_vecOrigin { 0 };
	int  m_iTeam { 0 };
	bool m_bAlivePlayer { false };
	bool m_bEnemy { false };
	int m_iMaxHealth { 0 };
	int m_iHealth { 0 };

	// Entity fields start here
	EntityType m_Type { ENTITY_GENERIC };

	float m_flDistance { 0.0f };

	bool m_bCritProjectile { false };
	bool m_bGrenadeProjectile { false };

	bool m_bAnyHitboxVisible { false };
	bool m_bVisCheckComplete { false };

	k_EItemType m_ItemType { ITEM_NONE };

	unsigned long m_lSeenTicks { 0 };
	unsigned long m_lLastSeen { 0 };
	Vector m_vecVOrigin { 0 };
	Vector m_vecVelocity { 0 };
	Vector m_vecAcceleration { 0 };
	float m_fLastUpdate { 0.0f };
	hitbox_cache::EntityHitboxCache& hitboxes;
	player_info_s player_info {};
	Averager<float> velocity_averager { 8 };
	bool was_dormant { true };
	bool velocity_is_valid { false };
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
