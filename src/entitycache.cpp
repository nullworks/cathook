/*
 * entitycache.cpp
 *
 *  Created on: Nov 7, 2016
 *      Author: nullifiedcat
 */

#include "common.hpp"

#include <time.h>
#include <settings/Float.hpp>
#include "soundcache.hpp"
#include <Warp.hpp>
inline void CachedEntity::Update()
{
#if PROXY_ENTITY != true
    m_pEntity = g_IEntityList->GetClientEntity(idx);
    if (!m_pEntity)
    {
        return;
    }
#endif
    m_lSeenTicks = 0;
    m_lLastSeen  = 0;
    hitboxes.InvalidateCache();
    m_bVisCheckComplete = false;
}

inline CachedEntity::CachedEntity(u_int16_t idx) : m_IDX(idx), hitboxes(hitbox_cache::EntityHitboxCache{ idx })
{
#if PROXY_ENTITY != true
    m_pEntity = nullptr;
#endif
    m_fLastUpdate = 0.0f;
}
CachedEntity::~CachedEntity()
{
}
static settings::Float ve_window{ "debug.ve.window", "0" };
static settings::Boolean ve_smooth{ "debug.ve.smooth", "true" };
static settings::Int ve_averager_size{ "debug.ve.averaging", "0" };

bool CachedEntity::IsVisible()
{
    PROF_SECTION(CE_IsVisible);
    if (m_bVisCheckComplete)
        return m_bAnyHitboxVisible;
    auto hitbox   = hitboxes.GetHitbox(std::max(0, (hitboxes.GetNumHitboxes() >> 1) - 1));
    Vector result = hitbox->center;
    if (!hitbox)
        result = m_vecOrigin();
    // Just check a centered hitbox. This is mostly used for ESP anyway
    if (IsEntityVectorVisible(this, result, true, MASK_SHOT_HULL, nullptr, true))
    {
        m_bAnyHitboxVisible = true;
        m_bVisCheckComplete = true;
        return true;
    }

    m_bAnyHitboxVisible = false;
    m_bVisCheckComplete = true;

    return false;
}
namespace entity_cache
{
boost::unordered_flat_map<u_int16_t, CachedEntity> array;
std::vector<CachedEntity *> valid_ents;
std::vector<CachedEntity *> player_cache;
u_int16_t previous_max = 0;
u_int16_t previous_ent = 0;
void Update()
{
    max                    = g_IEntityList->GetHighestEntityIndex();
    u_int16_t current_ents = g_IEntityList->NumberOfEntities(false);
    valid_ents.clear(); // Reserving isn't necessary as this doesn't reallocate it
    player_cache.clear();
    if (max >= MAX_ENTITIES)
        max = MAX_ENTITIES - 1;
    if (previous_max == max && previous_ent == current_ents)
    {
        for (auto &[key, val] : array)
        {
            val.Update();
            if (val.InternalEntity() && !val.InternalEntity()->IsDormant())
            {
                valid_ents.emplace_back(&val);
                if (val.m_Type() == ENTITY_PLAYER)
                {
                    GetPlayerInfo(val.m_IDX, &val.player_info);
                    if (val.m_bAlivePlayer()) [[likely]]
                    {
                        val.hitboxes.UpdateBones();
                        player_cache.emplace_back(&val);
                    }
                }
            }
        }
        previous_max = max;
        previous_ent = current_ents;
        return;
    }
    else
    {
        for (u_int16_t i = 0; i <= max; ++i)
        {
            if (g_Settings.bInvalid || !(g_IEntityList->GetClientEntity(i)) || !(g_IEntityList->GetClientEntity(i)->GetClientClass()->m_ClassID))
                continue;
            CachedEntity &ent = array.try_emplace(i, CachedEntity{ i }).first->second;
            ent.Update();
            if (ent.InternalEntity() && !ent.InternalEntity()->IsDormant())
            {
                valid_ents.emplace_back(&ent);
                if (ent.m_Type() == ENTITY_PLAYER)
                {
                    GetPlayerInfo(ent.m_IDX, &ent.player_info);
                    if (ent.m_bAlivePlayer()) [[likely]]
                    {
                        ent.hitboxes.UpdateBones();
                        player_cache.emplace_back(&ent);
                    }
                }
            }
        }
        previous_max = max;
        previous_ent = current_ents;
        return;
    }
}

void Invalidate()
{
    array.clear();
}
void Shutdown()
{
    array.clear();
    previous_max = 0;
    max          = -1;
}
u_int16_t max = 1;
} // namespace entity_cache
