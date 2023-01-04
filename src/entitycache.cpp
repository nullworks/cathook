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

bool IsProjectileACrit(CachedEntity *ent)
{
    if (ent->m_bGrenadeProjectile())
        return CE_BYTE(ent, netvar.Grenade_bCritical);
    return CE_BYTE(ent, netvar.Rocket_bCritical);
}
// This method of const'ing the index is weird.
CachedEntity::CachedEntity() : m_IDX(int(((unsigned) this - (unsigned) &entity_cache::array) / sizeof(CachedEntity))), hitboxes(hitbox_cache::Get(unsigned(m_IDX)))
{
#if PROXY_ENTITY != true
    m_pEntity = nullptr;
#endif
    m_fLastUpdate = 0.0f;
}

void CachedEntity::Reset()
{
    m_bAnyHitboxVisible = false;
    m_bVisCheckComplete = false;
    m_lLastSeen         = 0;
    m_lSeenTicks        = 0;
    memset(&player_info, 0, sizeof(player_info_s));
    m_vecAcceleration.Zero();
    m_vecVOrigin.Zero();
    m_vecVelocity.Zero();
    m_fLastUpdate = 0;
}

CachedEntity::~CachedEntity()
{
}

static settings::Float ve_window{ "debug.ve.window", "0" };
static settings::Boolean ve_smooth{ "debug.ve.smooth", "true" };
static settings::Int ve_averager_size{ "debug.ve.averaging", "0" };

void CachedEntity::Update()
{
    auto raw = RAW_ENT(this);

    if (!raw)
        return;
#if PROXY_ENTITY != true
    m_pEntity = g_IEntityList->GetClientEntity(idx);
    if (!m_pEntity)
    {
        return;
    }
#endif
    m_lSeenTicks = 0;
    m_lLastSeen  = 0;

    hitboxes.Update();

    m_bVisCheckComplete = false;

    if (m_Type() == EntityType::ENTITY_PLAYER)
        GetPlayerInfo(m_IDX, &player_info);
}

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

std::optional<Vector> CachedEntity::m_vecDormantOrigin()
{
    if (!RAW_ENT(this)->IsDormant())
        return m_vecOrigin();
    auto vec = soundcache::GetSoundLocation(this->m_IDX);
    if (vec)
        return *vec;
    return std::nullopt;
}
namespace entity_cache
{
CachedEntity array[MAX_ENTITIES]{};
std::vector<CachedEntity *> valid_ents;
std::map<Vector, CachedEntity *> proj_map;
std::vector<CachedEntity *> skip_these;
void Update()
{
    max = g_IEntityList->GetHighestEntityIndex();
    valid_ents.clear(); // Reserving isn't necessary as this doesn't reallocate it
    if (max >= MAX_ENTITIES)
        max = MAX_ENTITIES - 1;
    for (int i = 0; i <= max; ++i)
    {
        array[i].Update();

        if (CE_GOOD((&array[i])))
        {
            array[i].hitboxes.UpdateBones();
            valid_ents.push_back(&array[i]);
            if ((bool) hacks::tf2::warp::dodge_projectile && CE_GOOD(g_pLocalPlayer->entity) && array[i].m_Type() == ENTITY_PROJECTILE && array[i].m_bEnemy() && std::find(skip_these.begin(), skip_these.end(), &array[i]) == skip_these.end())
            {
                Vector eav;
                CachedEntity *proj_ptr = &array[i];
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
                    {
                        proj_map.insert({ eav, proj_ptr });
                        skip_these.push_back(proj_ptr);
                    }
                    else
                        skip_these.push_back(proj_ptr);
                }
            }
        }
        else if ((bool) hacks::tf2::warp::dodge_projectile)
        {
            auto iter = std::find(skip_these.begin(), skip_these.end(), &array[i]);
            if (iter != skip_these.end())
                skip_these.erase(iter);
        }
    }
}

void Invalidate()
{
    for (auto &ent : array)
    {
        // pMuch useless line!
        // ent.m_pEntity = nullptr;
        ent.Reset();
    }
}

void Shutdown()
{
    for (auto &ent : array)
    {
        ent.Reset();
        ent.hitboxes.Reset();
    }
}

int max = 0;
} // namespace entity_cache
