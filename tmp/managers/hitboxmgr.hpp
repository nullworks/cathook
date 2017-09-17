/*
 * entityhitboxcache.hpp
 *
 *  Created on: May 25, 2017
 *      Author: nullifiedcat
 */

#ifndef ENTITYHITBOXCACHE_HPP_
#define ENTITYHITBOXCACHE_HPP_

#include "entitycache.h"

#include <mathlib/vector.h>
#include <mathlib/mathlib.h>
#include <cdll_int.h>
#include <studio.h>

#define CACHE_MAX_HITBOXES 64

namespace hitbox_cache {

struct CachedHitbox {
	Vector min;
	Vector max;
	Vector center;
	mstudiobbox_t* bbox;
};

class EntityHitboxCache {
public:
	EntityHitboxCache();
	~EntityHitboxCache();

	CachedHitbox* GetHitbox(int id);
	void Update();
	void InvalidateCache();
	bool VisibilityCheck(int id);
	void Init();
	int GetNumHitboxes();
	void Reset();
	matrix3x4_t* GetBones();

	int m_nNumHitboxes;
	bool m_bModelSet;
	bool m_bInit;
	bool m_bSuccess;

	mstudiohitboxset_t* m_pHitboxSet;
	model_t* m_pLastModel;
	CachedEntity* parent_ref; // TODO FIXME turn this into an actual reference

	bool m_VisCheckValidationFlags[CACHE_MAX_HITBOXES] { false };
	bool m_VisCheck[CACHE_MAX_HITBOXES] { false };
	bool m_CacheValidationFlags[CACHE_MAX_HITBOXES] { false };
	CachedHitbox m_CacheInternal[CACHE_MAX_HITBOXES] {};

	matrix3x4_t bones[128];
	bool bones_setup { false };
};

extern EntityHitboxCache array[2048];
inline EntityHitboxCache& Get(unsigned i) {
	if (i > 2048) throw std::out_of_range("Requested out-of-range entity hitbox cache entry!");
	return array[i];
}

}

#endif /* ENTITYHITBOXCACHE_HPP_ */
