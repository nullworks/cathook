/*
 * entityhitboxcache.hpp
 *
 *  Created on: May 25, 2017
 *      Author: nullifiedcat
 */

#pragma once

#include <cdll_int.h>
#include <studio.h>
#include <stdexcept>
#include <memory>
#include <vector>

// Forward declaration from entitycache.hpp
class CachedEntity;
constexpr int CACHE_MAX_HITBOXES = 64;

namespace hitbox_cache
{

struct CachedHitbox
{
    Vector min;
    Vector max;
    Vector center;
    mstudiobbox_t *bbox;
};

class EntityHitboxCache
{
private:
    int hit_idx;
    bool m_bModelSet      = false;
    bool m_bInit          = false;
    bool m_bSuccess       = false;
    model_t *m_pLastModel = nullptr;
    CachedEntity *parent_ref;

    uint_fast64_t m_VisCheckValidationFlags = 0;
    uint_fast64_t m_VisCheck                = 0;
    void Init();

public:
    EntityHitboxCache() = default;
    EntityHitboxCache(int in_IDX) : hit_idx(in_IDX)
    {
    }

    CachedHitbox *GetHitbox(int id);
    uint_fast64_t m_CacheValidationFlags = 0;
    void InvalidateCache()
    {
        bones_setup               = false;
        m_CacheValidationFlags    = 0;
        m_VisCheckValidationFlags = 0;
        m_bInit                   = false;
        m_bSuccess                = false;
    }
    bool VisibilityCheck(int id);
    int GetNumHitboxes()
    {
        if (!m_bInit)
            Init();
        if (!m_bSuccess)
            return 0;
        return m_nNumHitboxes;
    }
    matrix3x4_t *GetBones(int numbones = -1);

    // for "fixing" bones to use the reconstructed ones
    void UpdateBones();
    int m_nNumHitboxes = 0;
    std::vector<CachedHitbox> m_CacheInternal;

    std::vector<matrix3x4_t> bones;
    bool bones_setup{ false };
};
} // namespace hitbox_cache
