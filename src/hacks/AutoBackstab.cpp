/*
 * AutoBackstab.cpp
 *
 *  Created on: Apr 14, 2017
 *      Author: nullifiedcat
 */

#include <settings/Bool.hpp>
#include "common.hpp"
#include "hacks/Backtrack.hpp"
#include "hacks/Aimbot.hpp"

static settings::Bool enable{ "autobackstab.enable", "0" };
static settings::Bool silent{ "autobackstab.silent", "1" };

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

std::pair<Vector, Vector> GetHitboxBounds(CachedEntity *it, int hitbox)
{
	std::pair<Vector, Vector> result(it->hitboxes.GetHitbox(hitbox)->min,it->hitboxes.GetHitbox(hitbox)->max);
	return result;
}

void traceEntity(int *result_eindex, Vector *result_pos, QAngle angle,
                 Vector loc)
{
    Ray_t ray;
    Vector forward;
    float sp, sy, cp, cy;
    trace_t trace;

    trace::filter_default.SetSelf(RAW_ENT(g_pLocalPlayer->entity));
    sy        = sinf(DEG2RAD(angle[1]));
    cy        = cosf(DEG2RAD(angle[1]));
    sp        = sinf(DEG2RAD(angle[0]));
    cp        = cosf(DEG2RAD(angle[0]));
    forward.x = cp * cy;
    forward.y = cp * sy;
    forward.z = -sp;
    forward   = forward * 8192.0f + loc;
    ray.Init(loc, forward);
    g_ITrace->TraceRay(ray, MASK_SHOT_HULL, &trace::filter_default, &trace);
    if (result_pos)
        *result_pos = trace.endpos;
    if (result_eindex)
    {
        *result_eindex = 0;
    }
    if (trace.m_pEnt && result_eindex)
    {
        *result_eindex = ((IClientEntity *) (trace.m_pEnt))->entindex();
    }
}

bool canBackstab(CachedEntity *tar, Vector angle, Vector loc, Vector hitboxLoc)
{
    float meleeRange = re::C_TFWeaponBaseMelee::GetSwingRange(RAW_ENT(LOCAL_W));
    Vector targetAngle = NET_VECTOR(RAW_ENT(tar), netvar.m_angEyeAngles);
    if (fabsf(angle.y - targetAngle.y) < 80)
    {
        int IDX;
        Vector hitLoc;
        traceEntity(&IDX, &hitLoc, QAngle(angle.x, angle.y, angle.z), loc);
        if (IDX == tar->m_IDX)
        {
            if (loc.DistTo(hitboxLoc) <= meleeRange)
                return true;
        }
    }
    return false;
}

void CreateMove()
{
    if (!enable)
        return;
    if (CE_BAD(LOCAL_E) || CE_BAD(LOCAL_W) || !LOCAL_E->m_bAlivePlayer())
        return;
    if (g_pLocalPlayer->weapon()->m_iClassID() != CL_CLASS(CTFKnife))
        return;
    CachedEntity *besttarget = nullptr;

    for (int i = 0; i < g_IEngine->GetMaxClients(); i++)
    {
        CachedEntity *target = ENTITY(i);
        if (CE_BAD(target))
            continue;
        if (target == LOCAL_E || target->m_iTeam() == LOCAL_E->m_iTeam() ||
            !target->m_bAlivePlayer() || target->m_Type() != ENTITY_PLAYER)
            continue;
        if (target->hitboxes.GetHitbox(spine_2)->center.DistTo(
                g_pLocalPlayer->v_Eye) <= 300.0f)
        {
            if (CE_GOOD(besttarget))
            {
                if (target->hitboxes.GetHitbox(spine_2)->center.DistTo(
                        g_pLocalPlayer->v_Eye) <
                    besttarget->hitboxes.GetHitbox(spine_2)->center.DistTo(
                        g_pLocalPlayer->v_Eye))
                    besttarget = target;
            }
            else
            {
                besttarget = target;
            }
        }
    }
    if (CE_GOOD(besttarget))
    {
        Vector hitboxLoc =
            besttarget->hitboxes.GetHitbox(spine_2)
                ->center;
        Vector angle = NET_VECTOR(RAW_ENT(LOCAL_E), netvar.m_angEyeAngles);
            for (angle.y = -180.0f; angle.y < 180.0f; angle.y += 1.0f)
            {
                // Get angles
                Vector tr = (hitboxLoc - g_pLocalPlayer->v_Eye);
                Vector xAngle;
                VectorAngles(tr, xAngle);
                // Clamping is important
                fClampAngle(xAngle);
                angle.x                = xAngle.x;
                if (canBackstab(besttarget, angle, g_pLocalPlayer->v_Eye, hitboxLoc))
                {
                    current_user_cmd->viewangles = angle;
                    current_user_cmd->buttons |= IN_ATTACK;
                    besttarget = nullptr;
                    if (silent)
                        g_pLocalPlayer->bUseSilentAngles = true;
                    return;
                }
        }
    }
}
} // namespace hacks::tf2::autobackstab
