/*
 * entityhitboxcache.cpp
 *
 *  Created on: Dec 23, 2016
 *      Author: nullifiedcat
 */

#include <settings/Int.hpp>
#include "common.hpp"
#include "MiscTemporary.hpp"

namespace hitbox_cache
{

EntityHitboxCache::EntityHitboxCache() : parent_ref(&entity_cache::Get(((unsigned) this - (unsigned) &hitbox_cache::array) / sizeof(EntityHitboxCache)))
{
    Reset();
}

int EntityHitboxCache::GetNumHitboxes()
{
    if (!m_bInit)
        Init();
    if (!m_bSuccess)
        return 0;
    return m_nNumHitboxes;
}

EntityHitboxCache::~EntityHitboxCache()
{
}

void EntityHitboxCache::InvalidateCache()
{
    bones_setup = false;
    for (int i = 0; i < CACHE_MAX_HITBOXES; i++)
    {
        m_CacheValidationFlags[i]    = false;
        m_VisCheckValidationFlags[i] = false;
    }
    m_bInit    = false;
    m_bSuccess = false;
}

void EntityHitboxCache::Update()
{
    InvalidateCache();
}

void EntityHitboxCache::Init()
{
    model_t *model;
    studiohdr_t *shdr;
    mstudiohitboxset_t *set;

    m_bInit = true;
    if (CE_BAD(parent_ref))
        return;
    model = (model_t *) RAW_ENT(parent_ref)->GetModel();
    if (!model)
        return;
    if (!m_bModelSet || model != m_pLastModel)
    {
        shdr = g_IModelInfo->GetStudiomodel(model);
        if (!shdr)
            return;
        set = shdr->pHitboxSet(CE_INT(parent_ref, netvar.iHitboxSet));
        if (!dynamic_cast<mstudiohitboxset_t *>(set))
            return;
        m_pLastModel   = model;
        m_nNumHitboxes = 0;
        if (set)
        {
            m_nNumHitboxes = set->numhitboxes;
        }
        if (m_nNumHitboxes > CACHE_MAX_HITBOXES)
            m_nNumHitboxes = CACHE_MAX_HITBOXES;
        m_bModelSet = true;
    }
    m_bSuccess = true;
}

bool EntityHitboxCache::VisibilityCheck(int id)
{
    CachedHitbox *hitbox;

    if (!m_bInit)
        Init();
    if (id < 0 || id >= m_nNumHitboxes)
        return false;
    if (!m_bSuccess)
        return false;
    if (m_VisCheckValidationFlags[id])
        return m_VisCheck[id];
    // TODO corners
    hitbox = GetHitbox(id);
    if (!hitbox)
        return false;
    m_VisCheck[id]                = (IsEntityVectorVisible(parent_ref, hitbox->center, true));
    m_VisCheckValidationFlags[id] = true;
    return m_VisCheck[id];
}

static settings::Int setupbones_time{ "source.setupbones-time", "2" };
static settings::Boolean bonecache_enabled{ "source.use-bone-cache", "false" };

static std::mutex setupbones_mutex;

matrix3x4_t *EntityHitboxCache::GetBones(int numbones)
{
    static float bones_setup_time = 0.0f;
    switch (*setupbones_time)
    {
    case 0:
        bones_setup_time = 0.0f;
        break;
    case 1:
        bones_setup_time = g_GlobalVars->curtime;
        break;
    case 2:
        if (CE_GOOD(LOCAL_E))
            bones_setup_time = g_GlobalVars->interval_per_tick * CE_INT(LOCAL_E, netvar.nTickBase);
        break;
    case 3:
        if (CE_GOOD(parent_ref))
            bones_setup_time = CE_FLOAT(parent_ref, netvar.m_flSimulationTime);
    }
    if (!bones_setup)
    {
        // Reset game cache
        if (!bonecache_enabled && CE_GOOD(parent_ref))
            re::C_BaseAnimating::InvalidateBoneCache(RAW_ENT(parent_ref));

        // If numbones is not set, get it from some terrible and unnamed variable
        if (numbones == -1)
        {
            if (parent_ref->m_Type() == ENTITY_PLAYER)
                numbones = CE_INT(parent_ref, 0x844);
            else
                numbones = MAXSTUDIOBONES;
        }

        if (bones.size() != (size_t) numbones)
            bones.resize(numbones);
        if (g_Settings.is_create_move)
        {
#if !ENABLE_TEXTMODE
            if (!*bonecache_enabled || parent_ref->m_Type() != ENTITY_PLAYER || IsPlayerInvisible(parent_ref))
            {
                PROF_SECTION(bone_setup);
                bones_setup = RAW_ENT(parent_ref)->SetupBones(bones.data(), numbones, 0x7FF00, bones_setup_time);
            }
            else
            {
                PROF_SECTION(bone_cache);
                auto to_copy = CE_VAR(parent_ref, 0x838, matrix3x4_t *);
                if (to_copy)
                {
                    // This is catastrophically bad, don't do this. Someone needs to fix this.
                    memcpy(bones.data(), to_copy, sizeof(matrix3x4_t) * numbones);
                    bones_setup = true;
                }
                else
                {
                    PROF_SECTION(bone_setup);
                    bones_setup = RAW_ENT(parent_ref)->SetupBones(bones.data(), numbones, 0x7FF00, bones_setup_time);
                }
            }
#else
            // Textmode bots miss/shoot at nothing when the tf2 bonecache is used
            PROF_SECTION(bone_setup);
            bones_setup = RAW_ENT(parent_ref)->SetupBones(bones.data(), numbones, 0x7FF00, bones_setup_time);
#endif
        }
    }
    return bones.data();
}

void EntityHitboxCache::Reset()
{
    memset(m_VisCheck, 0, sizeof(bool) * CACHE_MAX_HITBOXES);
    memset(m_VisCheckValidationFlags, 0, sizeof(bool) * CACHE_MAX_HITBOXES);
    memset(m_CacheValidationFlags, 0, sizeof(bool) * CACHE_MAX_HITBOXES);
    m_CacheInternal.clear();
    m_CacheInternal.shrink_to_fit();
    bones.clear();
    bones.shrink_to_fit();
    m_nNumHitboxes = 0;
    m_bInit        = false;
    m_bModelSet    = false;
    m_bSuccess     = false;
    m_pLastModel   = nullptr;
    bones_setup    = false;
}

CachedHitbox *EntityHitboxCache::GetHitbox(int id)
{
    if (m_CacheValidationFlags[id])
        return &m_CacheInternal[id];
    mstudiobbox_t *box;

    if (!m_bInit)
        Init();
    if (id < 0 || id >= m_nNumHitboxes)
        return nullptr;
    if (!m_bSuccess)
        return nullptr;
    if (CE_BAD(parent_ref))
        return nullptr;
    auto model = (const model_t *) RAW_ENT(parent_ref)->GetModel();
    if (!model)
        return nullptr;
    auto shdr = g_IModelInfo->GetStudiomodel(model);
    if (!shdr)
        return nullptr;
    auto set = shdr->pHitboxSet(CE_INT(parent_ref, netvar.iHitboxSet));
    if (!dynamic_cast<mstudiohitboxset_t *>(set))
        return nullptr;
    if (m_nNumHitboxes > m_CacheInternal.size())
        m_CacheInternal.resize(m_nNumHitboxes);
    box = set->pHitbox(id);
    if (!box)
        return nullptr;
    if (box->bone < 0 || box->bone >= MAXSTUDIOBONES)
        return nullptr;
    VectorTransform(box->bbmin, GetBones(shdr->numbones)[box->bone], m_CacheInternal[id].min);
    VectorTransform(box->bbmax, GetBones(shdr->numbones)[box->bone], m_CacheInternal[id].max);
    m_CacheInternal[id].bbox   = box;
    m_CacheInternal[id].center = (m_CacheInternal[id].min + m_CacheInternal[id].max) / 2;
    m_CacheValidationFlags[id] = true;
    return &m_CacheInternal[id];
}

EntityHitboxCache array[MAX_ENTITIES]{};

void Update()
{
}

void Invalidate()
{
}
} // namespace hitbox_cache
