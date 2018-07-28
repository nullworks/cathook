/*
 * AutoBackstab.cpp
 *
 *  Created on: Apr 14, 2017
 *      Author: nullifiedcat
 */

#include "common.hpp"
#include "hacks/Backtrack.hpp"
namespace hacks::tf2::autobackstab
{

// pPaste, thanks to F1ssi0N
const Vector GetWorldSpaceCenter(CachedEntity *ent)
{
    Vector vMin, vMax;
    RAW_ENT(ent)->GetRenderBounds(vMin, vMax);
    Vector vWorldSpaceCenter = RAW_ENT(ent)->GetAbsOrigin();
    vWorldSpaceCenter.z += (vMin.z + vMax.z) / 2;
    return vWorldSpaceCenter;
}

static CatVar enabled(CV_SWITCH, "autobackstab", "0", "Auto Backstab",
                      "Does not depend on triggerbot!");
static CatVar silent(CV_SWITCH, "autobackstab_silent", "1", "Silent");
bool found;
std::pair<Vector, Vector> GetHitboxBounds(CachedEntity *it, int hitbox)
{
	std::pair<Vector, Vector> result(it->hitboxes.GetHitbox(hitbox)->min,it->hitboxes.GetHitbox(hitbox)->max);
	return result;
}
// TODO improve
bool CanBackstab(CachedEntity *tar, Vector Local_ang)
{
    if (CE_BAD(tar))
        return false;
    // Get the forward view vector of the target, ignore Z
    Vector vecVictimForward = NET_VECTOR(RAW_ENT(tar), netvar.m_angEyeAngles);
    vecVictimForward.z      = 0.0f;
    vecVictimForward.NormalizeInPlace();

    // Get a vector from my origin to my targets origin
    Vector vecToTarget;
    vecToTarget   = GetWorldSpaceCenter(tar) - GetWorldSpaceCenter(LOCAL_E);
    vecToTarget.z = 0.0f;
    vecToTarget.NormalizeInPlace();

    // Get a forward vector of the attacker.
    Vector vecOwnerForward = Local_ang;
    vecOwnerForward.z      = 0.0f;
    vecOwnerForward.NormalizeInPlace();

    float flDotOwner  = DotProduct(vecOwnerForward, vecToTarget);
    float flDotVictim = DotProduct(vecVictimForward, vecToTarget);

    // Make sure they're actually facing the target.
    // This needs to be done because lag compensation can place target slightly
    // behind the attacker.
    if (flDotOwner > 0.5)
        return (flDotVictim > -0.1);
    return false;
}
void CreateMove()
{
    if (!enabled)
        return;
    if (!CE_GOOD(LOCAL_E) || !LOCAL_E->m_bAlivePlayer() || !CE_GOOD(LOCAL_W))
        return;
    if (!LOCAL_E->m_bAlivePlayer())
        return;
    if (g_pLocalPlayer->weapon()->m_iClassID() != CL_CLASS(CTFKnife))
        return;
    int eid = -1;
    Vector endpos;
    ICollideable *p    = RAW_ENT(LOCAL_E)->GetCollideable();
    const Vector &max1 = p->OBBMaxs() + RAW_ENT(LOCAL_E)->GetAbsOrigin();
    const Vector &min1 = p->OBBMins() + RAW_ENT(LOCAL_E)->GetAbsOrigin();
    WhatIAmLookingAt(&eid, &endpos);

    CachedEntity *target = nullptr;
    if (eid > -1)
        target = ENTITY(eid);
    if (CE_GOOD(target) && target != LOCAL_E &&
        target->m_iTeam() != LOCAL_E->m_iTeam() && target->m_bAlivePlayer() &&
        target->m_Type() == ENTITY_PLAYER &&
        !hacks::shared::backtrack::enable &&
        CanBackstab(target, g_pLocalPlayer->v_OrigViewangles))
    {
        float swingrange =
            re::C_TFWeaponBaseMelee::GetSwingRange(RAW_ENT(LOCAL_W));
        const Vector &max2 = GetHitboxBounds(target, 1).second +
                             Vector(swingrange, swingrange, swingrange);
        const Vector &min2 = GetHitboxBounds(target, 1).first -
                             Vector(swingrange, swingrange, swingrange);
        if ((min1.x <= max2.x && max1.x >= min2.x) &&
            (min1.y <= max2.y && max1.y >= min2.y) &&
            (min1.z <= max2.z && max1.z >= min2.z))
            g_pUserCmd->buttons |= IN_ATTACK;
    }
    else if (!hacks::shared::backtrack::enable)
    {
        CachedEntity *tar = nullptr;
        float bestscr     = 9999.9f;
        int bestent       = -1;
        for (int i = 0; i < g_IEngine->GetMaxClients(); i++)
        {
            CachedEntity *tmp = ENTITY(i);
            if (CE_BAD(tmp))
                continue;
            if (tmp == LOCAL_E)
                continue;
            if (tmp->m_iTeam() == LOCAL_E->m_iTeam())
                continue;
            if (!tmp->m_bAlivePlayer())
                continue;
            if (tmp->m_Type() != ENTITY_PLAYER)
                continue;
            float swingrange =
                re::C_TFWeaponBaseMelee::GetSwingRange(RAW_ENT(LOCAL_W));
            ICollideable *c    = RAW_ENT(tmp)->GetCollideable();
            const Vector &max2 = c->OBBMaxs() + tmp->m_vecOrigin() +
                                 Vector(swingrange, swingrange, swingrange);
            const Vector &min2 = c->OBBMins() + tmp->m_vecOrigin() -
                                 Vector(swingrange, swingrange, swingrange);
            if ((min1.x <= max2.x && max1.x >= min2.x) &&
                (min1.y <= max2.y && max1.y >= min2.y) &&
                (min1.z <= max2.z && max1.z >= min2.z) &&
                bestscr > tmp->m_flDistance())
            {
                bestent = tmp->m_IDX;
                bestscr = tmp->m_flDistance();
            }
        }
        if (bestent > -1)
            tar = ENTITY(bestent);
        if (CE_BAD(tar))
            return;
        Vector eyeang = g_pLocalPlayer->v_OrigViewangles;
        for (float i = -180.0f; i < 180.0f; i += 30.0f)
        {
            eyeang.y = i;
            if (CanBackstab(tar, eyeang))
            {
                g_pUserCmd->viewangles.y = eyeang.y;
                g_pUserCmd->buttons |= IN_ATTACK;
                if (silent)
                    g_pLocalPlayer->bUseSilentAngles = true;
            }
            break;
        }
    }
    else
    {
        if (!hacks::shared::backtrack::enable)
            return;
        if (hacks::shared::backtrack::iBestTarget == -1)
            return;
        int iBestTarget = hacks::shared::backtrack::iBestTarget;
        int tickcnt     = 0;
        int tickus = (float(hacks::shared::backtrack::latency) > 800.0f || float(hacks::shared::backtrack::latency) < 200.0f) ? 12 : 24;
        for (auto i : hacks::shared::backtrack::headPositions[iBestTarget])
        {
            bool good_tick = false;
            for (int j = 0; j < tickus; ++j)
                if (tickcnt == hacks::shared::backtrack::sorted_ticks[j].tick &&
                    hacks::shared::backtrack::sorted_ticks[j].tickcount !=
                        INT_MAX)
                    good_tick = true;
            tickcnt++;
            if (!good_tick)
                continue;

            float scr = abs(g_pLocalPlayer->v_OrigViewangles.y - i.viewangles);

            if (scr <= 90.0f &&
                i.origin.DistTo(g_pLocalPlayer->v_Eye) <=
                    re::C_TFWeaponBaseMelee::GetSwingRange(RAW_ENT(LOCAL_W)))
            {
                CachedEntity *tar = ENTITY(iBestTarget);
                // ok just in case
                if (CE_BAD(tar))
                    continue;
                Vector &angles =
                    NET_VECTOR(RAW_ENT(tar), netvar.m_angEyeAngles);
                float &simtime =
                    NET_FLOAT(RAW_ENT(tar), netvar.m_flSimulationTime);
                angles.y               = i.viewangles;
                simtime                = i.simtime;
                g_pUserCmd->tick_count = i.tickcount;
                g_pUserCmd->buttons |= IN_ATTACK;
                break;
            }
        }
    }
}
}
