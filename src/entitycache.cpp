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

    if (m_Type() == EntityType::ENTITY_PLAYER)
        GetPlayerInfo(m_IDX, &player_info);
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

// FIXME maybe disable this by default
static settings::Boolean fast_vischeck{ "debug.fast-vischeck", "true" };

bool CachedEntity::IsVisible()
{
    static constexpr int optimal_hitboxes[] = { hitbox_t::head, hitbox_t::foot_L, hitbox_t::hand_R, hitbox_t::spine_1 };
    static bool vischeck0, vischeck;

    PROF_SECTION(CE_IsVisible);
    if (m_bVisCheckComplete)
        return m_bAnyHitboxVisible;

    vischeck0 = IsEntityVectorVisible(this, m_vecOrigin(), true);

    if (vischeck0)
    {
        m_bAnyHitboxVisible = true;
        m_bVisCheckComplete = true;
        return true;
    }

    if (m_Type() == ENTITY_PLAYER && fast_vischeck)
    {
        for (int i = 0; i < 4; i++)
        {
            if (hitboxes.VisibilityCheck(optimal_hitboxes[i]))
            {
                m_bAnyHitboxVisible = true;
                m_bVisCheckComplete = true;
                return true;
            }
        }
        m_bAnyHitboxVisible = false;
        m_bVisCheckComplete = true;
        return false;
    }

    for (int i = 0; i < hitboxes.m_nNumHitboxes; i++)
    {
        vischeck = false;
        vischeck = hitboxes.VisibilityCheck(i);
        if (vischeck)
        {
            m_bAnyHitboxVisible = true;
            m_bVisCheckComplete = true;
            return true;
        }
    }
    m_bAnyHitboxVisible = false;
    m_bVisCheckComplete = true;

    return false;
}
namespace entity_cache
{
boost::unordered_flat_map<u_int16_t, CachedEntity> array;
std::vector<CachedEntity *> valid_ents;
std::vector<std::tuple<Vector, CachedEntity *>> proj_map;
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
            if (CE_GOOD((&val)))
            {
                val.hitboxes.UpdateBones();
                valid_ents.emplace_back(&val);
                if (val.m_Type() == ENTITY_PLAYER && val.m_bAlivePlayer())
                    player_cache.emplace_back(&val);
                if ((bool) hacks::tf2::warp::dodge_projectile && CE_GOOD(g_pLocalPlayer->entity) && val.m_Type() == ENTITY_PROJECTILE && val.m_bEnemy() && std::find_if(proj_map.begin(), proj_map.end(), [=](const auto &item) { return std::get<1>(item) == &val; }) == proj_map.end())
                    dodgeProj(&val);
            }
        }
    }
    else
    {
        for (u_int16_t i = 0; i <= max; ++i)
        {
            if (g_Settings.bInvalid || !(g_IEntityList->GetClientEntity(i)) || !(g_IEntityList->GetClientEntity(i)->GetClientClass()->m_ClassID))
                continue;
            array.try_emplace(i, CachedEntity{ i });
            array[i].Update();

            if (CE_GOOD((&array[i])))
            {
                array[i].hitboxes.UpdateBones();
                valid_ents.emplace_back(&array[i]);
                if (array[i].m_Type() == ENTITY_PLAYER && array[i].m_bAlivePlayer())
                    player_cache.emplace_back(&(array[i]));
                if ((bool) hacks::tf2::warp::dodge_projectile && CE_GOOD(g_pLocalPlayer->entity) && array[i].m_Type() == ENTITY_PROJECTILE && array[i].m_bEnemy() && std::find_if(proj_map.begin(), proj_map.end(), [=](const auto &item) { return std::get<1>(item) == &array[i]; }) == proj_map.end())
                    dodgeProj(&array[i]);
            }
        }
    }
    previous_max = max;
    previous_ent = current_ents;
}

void dodgeProj(CachedEntity *proj_ptr)
{

    Vector eav;

    velocity::EstimateAbsVelocity(RAW_ENT(proj_ptr), eav);
    // Sometimes EstimateAbsVelocity returns completely BS values (as in 0 for everything on say a rocket)
    // The ent could also be an in-place sticky which we don't care about - we want to catch it while it's in the air
    if (1 < eav.Length())
    {
        Vector proj_pos   = RAW_ENT(proj_ptr)->GetAbsOrigin();
        Vector player_pos = RAW_ENT(LOCAL_E)->GetAbsOrigin();

        float displacement      = proj_pos.DistToSqr(player_pos);
        float displacement_temp = displacement - 1;
        float min_displacement  = displacement_temp - 1;
        float multipler         = 0.01f;
        bool add_grav           = false;
        float curr_grav         = g_ICvar->FindVar("sv_gravity")->GetFloat();
        if (proj_ptr->m_Type() == ENTITY_PROJECTILE)
            add_grav = true;
        // Couldn't find a cleaner way to get the projectiles gravity based on just having a pointer to the projectile itself
        curr_grav = curr_grav * ProjGravMult(proj_ptr->m_iClassID(), eav.Length());
        // Optimization loop. Just checks if the projectile can possibly hit within ~141HU
        while (displacement_temp < displacement)
        {

            Vector temp_pos = (eav * multipler) + proj_pos;
            if (add_grav)
                temp_pos.z = temp_pos.z - 0.5 * curr_grav * multipler * multipler;
            displacement_temp = temp_pos.DistToSqr(player_pos);
            if (displacement_temp < min_displacement)
                min_displacement = displacement_temp;
            else
                break;

            multipler += 0.01f;
        }
        if (min_displacement < 20000)
            proj_map.emplace_back((std::make_tuple(eav, proj_ptr)));
        else
            proj_map.emplace_back((std::make_tuple(Vector{ 0, 0, 0 }, proj_ptr)));
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
