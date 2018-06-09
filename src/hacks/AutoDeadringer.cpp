/*
 * AutoDeadringer.cpp
 *
 *  Created on: Apr 12, 2018
 *      Author: bencat07
 */
#include "common.hpp"
namespace hacks
{
namespace shared
{
namespace deadringer
{
static CatVar
    enabled(CV_SWITCH, XORSTR("deadringer_auto"), XORSTR("0"), XORSTR("Auto deadringer"),
            XORSTR("automatically pull out DR on low health or projectile nearby"));

bool IsProjectile(CachedEntity *ent)
{
    return (ent->m_iClassID() == CL_CLASS(CTFProjectile_Rocket) ||
            ent->m_iClassID() == CL_CLASS(CTFProjectile_Flare) ||
            ent->m_iClassID() == CL_CLASS(CTFProjectile_EnergyBall) ||
            ent->m_iClassID() == CL_CLASS(CTFProjectile_HealingBolt) ||
            ent->m_iClassID() == CL_CLASS(CTFProjectile_Arrow) ||
            ent->m_iClassID() == CL_CLASS(CTFProjectile_SentryRocket) ||
            ent->m_iClassID() == CL_CLASS(CTFProjectile_Cleaver) ||
            ent->m_iClassID() == CL_CLASS(CTFGrenadePipebombProjectile) ||
            ent->m_iClassID() == CL_CLASS(CTFProjectile_EnergyRing));
}
int NearbyEntities()
{
    int ret = 0;
    if (CE_BAD(LOCAL_E))
        return ret;
    for (int i = 0; i < HIGHEST_ENTITY; i++)
    {
        CachedEntity *ent = ENTITY(i);
        if (CE_BAD(ent))
            continue;
        if (ent == LOCAL_E)
            continue;
        if (!ent->m_bAlivePlayer())
            continue;
        if (ent->m_flDistance() <= 300.0f)
            ret++;
    }
    return ret;
}
void CreateMove()
{
    if (CE_BAD(LOCAL_E))
        return;
    if (!HasWeapon(LOCAL_E, 59))
        return;
    if (CE_INT(LOCAL_E, netvar.iHealth) < 30 && NearbyEntities() > 2)
        g_pUserCmd->buttons |= IN_ATTACK2;
    for (int i = 0; i < HIGHEST_ENTITY; i++)
    {
        CachedEntity *ent = ENTITY(i);
        if (CE_BAD(ent))
            continue;
        if (!IsProjectile(ent))
            continue;
        if (ent->m_bCritProjectile())
            g_pUserCmd->buttons |= IN_ATTACK2;
        if (ent->m_flDistance() < 100.0f)
            g_pUserCmd->buttons |= IN_ATTACK2;
    }
}
}
}
}
