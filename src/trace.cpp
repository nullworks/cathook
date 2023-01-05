/*
 * trace.cpp
 *
 *  Created on: Oct 10, 2016
 *      Author: nullifiedcat
 */

#include "common.hpp"

// This file is a mess. I need to fix it. TODO

/* Default Filter */

trace::FilterDefault::FilterDefault()
{
    m_pSelf = nullptr;
}

trace::FilterDefault::~FilterDefault()
{
}

void trace::FilterDefault::SetSelf(IClientEntity *self)
{
    if (self == nullptr)
    {
        logging::Info("nullptr in FilterDefault::SetSelf");
    }
    m_pSelf = self;
}

bool trace::FilterDefault::ShouldHitEntity(IHandleEntity *handle, int mask)
{
    IClientEntity *entity;
    ClientClass *clazz;

    if (!handle)
        return false;
    entity = (IClientEntity *) handle;
    clazz  = entity->GetClientClass();
    /* Ignore invisible entities that we don't wanna hit */
    switch (clazz->m_ClassID)
    {
    // TODO magic numbers: invisible entity ids
    case CL_CLASS(CFuncRespawnRoomVisualizer):
    case CL_CLASS(CTFMedigunShield):
    case CL_CLASS(CFuncAreaPortalWindow):
        return false;
    // Sniper rifles can shoot through teammates!
    case CL_CLASS(CTFPlayer):
    {
        if (m_pSelf)
        {
            // If what we hit is an enemy it does not matter
            if (m_pSelf && entity && entity->entindex() != 0 && IDX_GOOD(entity->entindex()) && CE_VALID(ENTITY(entity->entindex())) && ENTITY(entity->entindex())->m_iTeam() == ENTITY(m_pSelf->entindex())->m_iTeam())
            {
                auto ent = ENTITY(m_pSelf->entindex());
                if (CE_GOOD(ent) && ent->m_bAlivePlayer())
                {
                    // Get held weapon
                    auto weapon_idx = HandleToIDX(CE_INT(ent, netvar.hActiveWeapon));
                    // Check if weapon is valid
                    if (IDX_GOOD(weapon_idx))
                    {
                        auto weapon = ENTITY(weapon_idx);
                        // If holding sniper rifle
                        if (!weapon || weapon->m_iClassID() == CL_CLASS(CTFSniperRifle) || weapon->m_iClassID() == CL_CLASS(CTFSniperRifleDecap))
                            return false;
                    }
                }
            }
        }
        break;
    }
    }
    /* Do not hit yourself. Idiot. */
    if (entity == m_pSelf)
        return false;
    return true;
}

TraceType_t trace::FilterDefault::GetTraceType() const
{
    return TRACE_EVERYTHING;
}

/* No-Player filter */

trace::FilterNoPlayer::FilterNoPlayer()
{
    m_pSelf = nullptr;
}

trace::FilterNoPlayer::~FilterNoPlayer(){};

void trace::FilterNoPlayer::SetSelf(IClientEntity *self)
{
    if (self == nullptr)
    {
        logging::Info("nullptr in FilterNoPlayer::SetSelf");
        return;
    }
    m_pSelf = self;
}

bool trace::FilterNoPlayer::ShouldHitEntity(IHandleEntity *handle, int mask)
{
    IClientEntity *entity;
    ClientClass *clazz;

    if (!handle)
        return false;
    entity = (IClientEntity *) handle;
    clazz  = entity->GetClientClass();

    /* Ignore invisible entities that we don't wanna hit */
    switch (clazz->m_ClassID)
    {
    // TODO magic numbers: invisible entity ids
    case CL_CLASS(CTFPlayerResource):
    case CL_CLASS(CTFPlayer):
    case CL_CLASS(CFuncRespawnRoomVisualizer):
    case CL_CLASS(CTFMedigunShield):
    case CL_CLASS(CFuncAreaPortalWindow):
        return false;
    }
    /* Do not hit yourself. Idiot. */
    if (entity == m_pSelf)
        return false;
    return true;
}

TraceType_t trace::FilterNoPlayer::GetTraceType() const
{
    return TRACE_EVERYTHING;
}

/* Navigation filter */

trace::FilterNavigation::FilterNavigation(){};

trace::FilterNavigation::~FilterNavigation(){};

#define MOVEMENT_COLLISION_GROUP 8
#define RED_CONTENTS_MASK 0x800
#define BLU_CONTENTS_MASK 0x1000
bool trace::FilterNavigation::ShouldHitEntity(IHandleEntity *handle, int mask)
{
    IClientEntity *entity;
    ClientClass *clazz;

    if (!handle)
        return false;
    entity = (IClientEntity *) handle;
    clazz  = entity->GetClientClass();

    // Ignore everything that is not the world or a CBaseEntity
    if (entity->entindex() != 0 && clazz->m_ClassID != CL_CLASS(CBaseEntity))
    {
        // Besides respawn room areas, we want to explicitly ignore those if they are not on our team
        if (clazz->m_ClassID == CL_CLASS(CFuncRespawnRoomVisualizer))
            if (CE_GOOD(LOCAL_E) && (g_pLocalPlayer->team == TEAM_RED || g_pLocalPlayer->team == TEAM_BLU))
            {
                // If we can't collide, hit it
                if (!re::C_BaseEntity::ShouldCollide(entity, MOVEMENT_COLLISION_GROUP, g_pLocalPlayer->team == TEAM_RED ? RED_CONTENTS_MASK : BLU_CONTENTS_MASK))
                    return true;
            }
        return false;
    }
    return true;
}

TraceType_t trace::FilterNavigation::GetTraceType() const
{
    return TRACE_EVERYTHING;
}

/* No-Entity filter */

trace::FilterNoEntity::FilterNoEntity()
{
    m_pSelf = nullptr;
}

trace::FilterNoEntity::~FilterNoEntity(){};

void trace::FilterNoEntity::SetSelf(IClientEntity *self)
{
    if (self == nullptr)
    {
        logging::Info("nullptr in FilterNoPlayer::SetSelf");
        return;
    }
    m_pSelf = self;
}

bool trace::FilterNoEntity::ShouldHitEntity(IHandleEntity *handle, int mask)
{
    IClientEntity *entity;
    ClientClass *clazz;

    if (!handle)
        return false;
    entity = (IClientEntity *) handle;
    clazz  = entity->GetClientClass();

    // Hit doors, carts, etc
    switch (clazz->m_ClassID)
    {
    case CL_CLASS(CWorld):
    case CL_CLASS(CPhysicsProp):
    case CL_CLASS(CDynamicProp):
    case CL_CLASS(CBaseDoor):
    case CL_CLASS(CBaseEntity):
        return true;
    }
    return false;
}

TraceType_t trace::FilterNoEntity::GetTraceType() const
{
    return TRACE_EVERYTHING;
}
/* Penetration Filter */

trace::FilterPenetration::FilterPenetration()
{
    m_pSelf = nullptr;
}

trace::FilterPenetration::~FilterPenetration(){};

void trace::FilterPenetration::SetSelf(IClientEntity *self)
{
    if (self == nullptr)
    {
        logging::Info("nullptr in FilterPenetration::SetSelf");
    }
    m_pSelf = self;
}

bool trace::FilterPenetration::ShouldHitEntity(IHandleEntity *handle, int mask)
{
    IClientEntity *entity;
    ClientClass *clazz;

    if (!handle)
        return false;
    entity = (IClientEntity *) handle;
    clazz  = entity->GetClientClass();
    /* Ignore invisible entities that we don't wanna hit */
    switch (clazz->m_ClassID)
    {
    // TODO magic numbers: invisible entity ids
    case CL_CLASS(CFuncRespawnRoomVisualizer):
    case CL_CLASS(CTFMedigunShield):
    case CL_CLASS(CFuncAreaPortalWindow):
        return false;
    case CL_CLASS(CTFPlayer):
        if (!m_pIgnoreFirst && (entity != m_pSelf))
        {
            m_pIgnoreFirst = entity;
        }
    }
    /* Do not hit yourself. Idiot. */
    if (entity == m_pSelf)
        return false;
    if (entity == m_pIgnoreFirst)
        return false;
    return true;
}

TraceType_t trace::FilterPenetration::GetTraceType() const
{
    return TRACE_EVERYTHING;
}

void trace::FilterPenetration::Reset()
{
    m_pIgnoreFirst = 0;
}

trace::FilterDefault trace::filter_default{};
trace::FilterNoPlayer trace::filter_no_player{};
trace::FilterNavigation trace::filter_navigation{};
trace::FilterNoEntity trace::filter_no_entity{};
trace::FilterPenetration trace::filter_penetration{};
