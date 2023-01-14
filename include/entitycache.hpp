/*
 * entitycache.h
 *
 *  Created on: Nov 7, 2016
 *      Author: nullifiedcat
 */

#pragma once

#include "entityhitboxcache.hpp"
#include "averager.hpp"
#include <mathlib/vector.h>
#include <icliententity.h>
#include <icliententitylist.h>
#include <cdll_int.h>
#include <enums.hpp>
#include <core/interfaces.hpp>
#include <itemtypes.hpp>
#include "localplayer.hpp"
#include <core/netvars.hpp>
#include "playerresource.h"
#include "globals.h"
#include "classinfo/classinfo.hpp"
#include "client_class.h"
#include "Constants.hpp"
#include <optional>
#include <boost/unordered/unordered_flat_map.hpp>
#include <soundcache.hpp>

struct matrix3x4_t;

class IClientEntity;
struct player_info_s;
struct model_t;
struct mstudiohitboxset_t;
struct mstudiobbox_t;

constexpr int MAX_STRINGS = 16;

#define PROXY_ENTITY true

#if PROXY_ENTITY == true
#define RAW_ENT(ce) ce->InternalEntity()
#else
#define RAW_ENT(ce) ce->m_pEntity
#endif

#define CE_VAR(entity, offset, type) NET_VAR(RAW_ENT(entity), offset, type)

#define CE_INT(entity, offset) CE_VAR(entity, offset, int)
#define CE_FLOAT(entity, offset) CE_VAR(entity, offset, float)
#define CE_BYTE(entity, offset) CE_VAR(entity, offset, unsigned char)
#define CE_VECTOR(entity, offset) CE_VAR(entity, offset, Vector)

#define CE_GOOD(entity) (entity && !g_Settings.bInvalid && entity->Good())
#define CE_BAD(entity) (!CE_GOOD(entity))
#define CE_VALID(entity) (entity && !g_Settings.bInvalid && entity->Valid())
#define CE_INVALID(entity) (!CE_VALID(entity))

#define IDX_GOOD(idx) (idx >= 0 && idx <= HIGHEST_ENTITY && idx < MAX_ENTITIES)
#define IDX_BAD(idx) !IDX_GOOD(idx)

#define HIGHEST_ENTITY (entity_cache::max)
#define ENTITY(idx) (entity_cache::Get(idx))

class CachedEntity
{
public:
    typedef CachedEntity ThisClass;
    CachedEntity();
    CachedEntity(u_int16_t idx);
    ~CachedEntity();

    __attribute__((hot)) void Update();

    bool IsVisible();
    __attribute__((always_inline, hot, const)) IClientEntity *InternalEntity() const
    {
        return g_IEntityList->GetClientEntity(m_IDX);
    }
    __attribute__((always_inline, hot, const)) bool Good() const
    {
        if (!RAW_ENT(this) || !RAW_ENT(this)->GetClientClass()->m_ClassID)
            return false;
        IClientEntity *const entity = InternalEntity();
        return entity && !entity->IsDormant();
    }
    __attribute__((always_inline, hot, const)) bool Valid() const
    {
        if (!RAW_ENT(this) || !RAW_ENT(this)->GetClientClass()->m_ClassID)
            return false;
        IClientEntity *const entity = InternalEntity();
        return entity;
    }
    template <typename T> __attribute__((always_inline, hot, const)) T &var(uintptr_t offset) const
    {
        return *reinterpret_cast<T *>(uintptr_t(RAW_ENT(this)) + offset);
    }

    const int m_IDX;

    int m_iClassID()
    {
        if (this && RAW_ENT(this))
            if (RAW_ENT(this)->GetClientClass())
                if (RAW_ENT(this)->GetClientClass()->m_ClassID)
                    return RAW_ENT(this)->GetClientClass()->m_ClassID;
        return 0;
    };
    Vector m_vecOrigin()
    {
        return RAW_ENT(this)->GetAbsOrigin();
    };
    std::optional<Vector> m_vecDormantOrigin()
    {
        if (!RAW_ENT(this)->IsDormant())
            return m_vecOrigin();
        auto vec = soundcache::GetSoundLocation(this->m_IDX);
        if (vec)
            return *vec;
        return std::nullopt;
    }
    int m_iTeam()
    {
        return NET_INT(RAW_ENT(this), netvar.iTeamNum);
    };
    bool m_bAlivePlayer()
    {
        return !(NET_BYTE(RAW_ENT(this), netvar.iLifeState));
    };
    bool m_bEnemy()
    {
        if (CE_BAD(g_pLocalPlayer->entity))
            return true;
        return m_iTeam() != g_pLocalPlayer->team;
    };
    int m_iMaxHealth()
    {
        if (m_Type() == ENTITY_PLAYER)
            return g_pPlayerResource->GetMaxHealth(this);
        else if (m_Type() == ENTITY_BUILDING)
            return NET_INT(RAW_ENT(this), netvar.iBuildingMaxHealth);
        else
            return 0.0f;
    };
    int m_iHealth()
    {
        if (m_Type() == ENTITY_PLAYER)
            return NET_INT(RAW_ENT(this), netvar.iHealth);
        else if (m_Type() == ENTITY_BUILDING)
            return NET_INT(RAW_ENT(this), netvar.iBuildingHealth);
        else
            return 0.0f;
    };
    Vector &m_vecAngle()
    {
        return CE_VECTOR(this, netvar.m_angEyeAngles);
    };

    // Entity fields start here
    EntityType m_Type()
    {

        int classid = m_iClassID();
        if (classid == CL_CLASS(CTFPlayer))
            return ENTITY_PLAYER;
        else if (classid == CL_CLASS(CTFGrenadePipebombProjectile) || classid == CL_CLASS(CTFProjectile_Cleaver) || classid == CL_CLASS(CTFProjectile_Jar) || classid == CL_CLASS(CTFProjectile_JarMilk) || classid == CL_CLASS(CTFProjectile_Arrow) || classid == CL_CLASS(CTFProjectile_EnergyBall) || classid == CL_CLASS(CTFProjectile_EnergyRing) || classid == CL_CLASS(CTFProjectile_GrapplingHook) || classid == CL_CLASS(CTFProjectile_HealingBolt) || classid == CL_CLASS(CTFProjectile_Rocket) || classid == CL_CLASS(CTFProjectile_SentryRocket) || classid == CL_CLASS(CTFProjectile_BallOfFire) || classid == CL_CLASS(CTFProjectile_Flare))
            return ENTITY_PROJECTILE;
        else if (classid == CL_CLASS(CObjectTeleporter) || classid == CL_CLASS(CObjectSentrygun) || classid == CL_CLASS(CObjectDispenser))
            return ENTITY_BUILDING;
        else if (classid == CL_CLASS(CZombie) || classid == CL_CLASS(CTFTankBoss) || classid == CL_CLASS(CMerasmus) || classid == CL_CLASS(CMerasmusDancer) || classid == CL_CLASS(CEyeballBoss) || classid == CL_CLASS(CHeadlessHatman))
            return ENTITY_NPC;
        else
            return ENTITY_GENERIC;
    };

    float m_flDistance()
    {
        if (CE_GOOD(g_pLocalPlayer->entity))
            return g_pLocalPlayer->v_Origin.DistTo(m_vecOrigin());
        else
            return FLT_MAX;
    };
    bool IsProjectileACrit(CachedEntity *ent)
    {
        if (ent->m_bGrenadeProjectile())
            return CE_BYTE(ent, netvar.Grenade_bCritical);
        return CE_BYTE(ent, netvar.Rocket_bCritical);
    }
    bool m_bCritProjectile()
    {
        if (m_Type() == EntityType::ENTITY_PROJECTILE)
            return IsProjectileACrit(this);
        else
            return false;
    };
    bool m_bGrenadeProjectile()
    {
        return m_iClassID() == CL_CLASS(CTFGrenadePipebombProjectile) || m_iClassID() == CL_CLASS(CTFProjectile_Cleaver) || m_iClassID() == CL_CLASS(CTFProjectile_Jar) || m_iClassID() == CL_CLASS(CTFProjectile_JarMilk);
    };

    bool m_bAnyHitboxVisible{ false };
    bool m_bVisCheckComplete{ false };

    k_EItemType m_ItemType()
    {
        if (m_Type() == ENTITY_GENERIC)
            return g_ItemManager.GetItemType(this);
        else
            return ITEM_NONE;
    };

    unsigned long m_lSeenTicks{ 0 };
    unsigned long m_lLastSeen{ 0 };
    Vector m_vecVOrigin{ 0 };
    Vector m_vecVelocity{ 0 };
    Vector m_vecAcceleration{ 0 };
    float m_fLastUpdate{ 0.0f };
    hitbox_cache::EntityHitboxCache hitboxes;
    player_info_s player_info{};
    Averager<float> velocity_averager{ 8 };
    void Reset()
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

    bool was_dormant()
    {
        return RAW_ENT(this)->IsDormant();
    };
    bool velocity_is_valid{ false };
#if PROXY_ENTITY != true
    IClientEntity *m_pEntity{ nullptr };
#endif
};

namespace entity_cache
{

// b1g fat array in
extern u_int16_t max;
extern u_int16_t previous_max;
extern std::vector<CachedEntity *> valid_ents;
extern boost::unordered_flat_map<u_int16_t, CachedEntity> array;
extern std::vector<std::tuple<Vector, CachedEntity *>> proj_map;
extern std::vector<CachedEntity *> player_cache;
inline CachedEntity *Get(const u_int16_t &idx)
{
    auto test = array.find(idx);
    if (test == array.end())
        return nullptr;
    else
        return &test->second;
}
void dodgeProj(CachedEntity *proj_ptr);
void Update();
void Invalidate();
void Shutdown();

} // namespace entity_cache
