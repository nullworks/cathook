/*
 * entityhitboxcache.cpp
 *
 *  Created on: Dec 23, 2016
 *      Author: nullifiedcat
 */

#include "common.h"

EntityHitboxCache::EntityHitboxCache(CachedEntity* parent) {
	InvalidateCache();
	parent_ref = parent;
	m_bModelSet = false;
	m_nNumHitboxes = 0;
}

int EntityHitboxCache::GetNumHitboxes() {
	if (!m_bInit) Init();
	if (!m_bSuccess) return 0;
	return m_nNumHitboxes;
}

EntityHitboxCache::~EntityHitboxCache() {}

void EntityHitboxCache::InvalidateCache() {
	for (int i = 0; i < CACHE_MAX_HITBOXES; i++) {
		m_CacheValidationFlags[i] = false;
		m_VisCheckValidationFlags[i] = false;
	}
	m_bInit = false;
	m_bSuccess = false;
}

void EntityHitboxCache::Update() {
	SAFE_CALL(InvalidateCache());
	if (CE_BAD(parent_ref)) return;
}

void EntityHitboxCache::Init() {
	static model_t *model;
	static studiohdr_t *shdr;
	static mstudiohitboxset_t *set;

	m_bInit = true;
	model = 0;
	if (CE_BAD(parent_ref)) return;
	SAFE_CALL(model = (model_t*)RAW_ENT(parent_ref)->GetModel());
	if (!model) return;
	if (!m_bModelSet || model != m_pLastModel) {
		shdr = g_IModelInfo->GetStudiomodel(model);
		if (!shdr) return;
		set = shdr->pHitboxSet(CE_INT(parent_ref, netvar.iHitboxSet));
		if (!dynamic_cast<mstudiohitboxset_t*>(set)) return;
		m_pLastModel = model;
		m_pHitboxSet = set;
		m_nNumHitboxes = 0;
		if (set) {
			SAFE_CALL(m_nNumHitboxes = set->numhitboxes);
		}
		if (m_nNumHitboxes > CACHE_MAX_HITBOXES) m_nNumHitboxes = CACHE_MAX_HITBOXES;
		m_bModelSet = true;
	}
	m_bSuccess = true;
}

bool EntityHitboxCache::VisibilityCheck(int id) {
	static CachedHitbox *hitbox;

	if (!m_bInit) Init();
	if (id < 0 || id >= m_nNumHitboxes) return 0;
	if (!m_bSuccess) return 0;
	if (m_VisCheckValidationFlags[id]) return m_VisCheck[id];
	// TODO corners
	hitbox = GetHitbox(id);
	if (!hitbox) return 0;
	SAFE_CALL(m_VisCheck[id] = (IsEntityVectorVisible(parent_ref, hitbox->center)));
	m_VisCheckValidationFlags[id] = true;
	return m_VisCheck[id];
}

void EntityHitboxCache::Reset() {
	memset(m_VisCheck, 0, sizeof(bool) * CACHE_MAX_HITBOXES);
	memset(m_VisCheckValidationFlags, 0, sizeof(bool) * CACHE_MAX_HITBOXES);
	memset(m_CacheValidationFlags, 0, sizeof(bool) * CACHE_MAX_HITBOXES);
	memset(m_CacheInternal, 0, sizeof(CachedHitbox) * CACHE_MAX_HITBOXES);
	m_nNumHitboxes = 0;
	m_bInit = false;
	m_bModelSet = false;
	m_bSuccess = false;
	m_pHitboxSet = nullptr;
	m_pLastModel = nullptr;
}

CachedHitbox* EntityHitboxCache::GetHitbox(int id) {
	static mstudiobbox_t *box;

	if (!m_bInit) Init();
	if (id < 0 || id >= m_nNumHitboxes) return 0;
	if (!m_bSuccess) return 0;
	if (!m_CacheValidationFlags[id]) {
		box = m_pHitboxSet->pHitbox(id);
		if (!box) return 0;
		if (box->bone < 0 || box->bone >= MAXSTUDIOBONES) return 0;
		VectorTransform(box->bbmin, parent_ref->GetBones()[box->bone], m_CacheInternal[id].min);
		VectorTransform(box->bbmax, parent_ref->GetBones()[box->bone], m_CacheInternal[id].max);
		m_CacheInternal[id].bbox = box;
		m_CacheInternal[id].center = (m_CacheInternal[id].min + m_CacheInternal[id].max) / 2;
		m_CacheValidationFlags[id] = true;
	}
	return &m_CacheInternal[id];
}
