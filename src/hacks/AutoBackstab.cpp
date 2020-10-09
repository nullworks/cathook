/*
 * AutoBackstab.cpp
 *
 *  Created on: Apr 14, 2017
 *      Author: nullifiedcat
 */

#include "common.hpp"
#include "PlayerTools.hpp"
#include "Trigger.hpp"
#include "Backtrack.hpp"

namespace hacks::tf2::autobackstab
{
static settings::Boolean enabled("autobackstab.enabled", "false");
static settings::Int mode("autobackstab.mode", "0");

// Function to find the closest hitbox to the v_Eye for a given ent
int ClosestDistanceHitbox(CachedEntity *target)
{
    int closest        = -1;
    float closest_dist = FLT_MAX, dist = 0.0f;
    for (int i = pelvis; i < spine_3; i++)
    {
        auto hitbox = target->hitboxes.GetHitbox(i);
        if (!hitbox)
            continue;
        dist = g_pLocalPlayer->v_Eye.DistTo(hitbox->center);
        if (dist < closest_dist)
        {
            closest      = i;
            closest_dist = dist;
        }
    }
    return closest;
}
int ClosestDistanceHitbox(hacks::tf2::backtrack::BacktrackData btd)
{
    int closest        = -1;
    float closest_dist = FLT_MAX, dist = 0.0f;
    for (int i = pelvis; i < spine_3; i++)
    {
        dist = g_pLocalPlayer->v_Eye.DistTo(btd.hitboxes.at(i).center);
        if (dist < closest_dist)
        {
            closest      = i;
            closest_dist = dist;
        }
    }
    return closest;
}

bool canFaceStab(CachedEntity *ent)
{
    return ent->m_iHealth() <= 40.0f;
}

bool angleCheck(CachedEntity *from, CachedEntity *to, std::optional<Vector> target_pos, Vector from_angle)
{
    Vector tarAngle = CE_VECTOR(to, netvar.m_angEyeAngles);

    Vector vecToTarget;
    if (target_pos)
        vecToTarget = *target_pos - from->m_vecOrigin();
    else
        vecToTarget = to->m_vecOrigin() - from->m_vecOrigin();
    vecToTarget.z = 0;
    vecToTarget.NormalizeInPlace();

    Vector vecOwnerForward;
    AngleVectors2(VectorToQAngle(from_angle), &vecOwnerForward);
    vecOwnerForward.z = 0;
    vecOwnerForward.NormalizeInPlace();

    Vector vecTargetForward;
    AngleVectors2(VectorToQAngle(tarAngle), &vecTargetForward);
    vecTargetForward.z = 0;
    vecTargetForward.NormalizeInPlace();

    if (DotProduct(vecToTarget, vecTargetForward) <= 0.0f)
        return false;
    if (DotProduct(vecToTarget, vecOwnerForward) <= 0.5f)
        return false;
    if (DotProduct(vecOwnerForward, vecTargetForward) <= -0.3f)
        return false;
    return true;
}

static bool angleCheck(CachedEntity *target, std::optional<Vector> target_pos, Vector local_angle)
{
    Vector tarAngle = CE_VECTOR(target, netvar.m_angEyeAngles);

    // Get a vector from owner origin to target origin
    Vector vecToTarget;

    Vector local_worldspace;
    VectorLerp(RAW_ENT(LOCAL_E)->GetCollideable()->OBBMins(), RAW_ENT(LOCAL_E)->GetCollideable()->OBBMaxs(), 0.5f, local_worldspace);
    local_worldspace += LOCAL_E->m_vecOrigin();
    if (target_pos)
        vecToTarget = *target_pos - local_worldspace;
    else
    {
        Vector target_worldspace;
        VectorLerp(RAW_ENT(target)->GetCollideable()->OBBMins(), RAW_ENT(target)->GetCollideable()->OBBMaxs(), 0.5f, target_worldspace);
        target_worldspace += target->m_vecOrigin();
        vecToTarget = target_worldspace - local_worldspace;
    }

    vecToTarget.z = 0;
    vecToTarget.NormalizeInPlace();

    // Get owner forward view vector
    Vector vecOwnerForward;
    AngleVectors2(VectorToQAngle(local_angle), &vecOwnerForward);
    vecOwnerForward.z = 0;
    vecOwnerForward.NormalizeInPlace();

    // Get target forward view vector
    Vector vecTargetForward;
    AngleVectors2(VectorToQAngle(tarAngle), &vecTargetForward);
    vecTargetForward.z = 0;
    vecTargetForward.NormalizeInPlace();

    // Make sure owner is behind, facing and aiming at target's back
    float flPosVsTargetViewDot = DotProduct(vecToTarget, vecTargetForward);     // Behind?
    float flPosVsOwnerViewDot  = DotProduct(vecToTarget, vecOwnerForward);      // Facing?
    float flViewAnglesDot      = DotProduct(vecTargetForward, vecOwnerForward); // Facestab?

    return (flPosVsTargetViewDot > 0.f && flPosVsOwnerViewDot > 0.5 && flViewAnglesDot > -0.3f);
}

static bool doLegitBackstab()
{
    trace_t trace;
    if (!re::C_TFWeaponBaseMelee::DoSwingTrace(RAW_ENT(LOCAL_W), &trace))
        return false;
    if (!trace.m_pEnt)
        return false;
    int index = reinterpret_cast<IClientEntity *>(trace.m_pEnt)->entindex();
    auto ent  = ENTITY(index);
    if (index == 0 || index > g_IEngine->GetMaxClients() || !ent->m_bEnemy() || !player_tools::shouldTarget(ent) || IsPlayerInvulnerable(ent))
        return false;
    if (angleCheck(ENTITY(index), std::nullopt, g_pLocalPlayer->v_OrigViewangles) || canFaceStab(ENTITY(index)))
    {
        current_user_cmd->buttons |= IN_ATTACK;
        return true;
    }
    return false;
}

static bool doRageBackstab()
{
    if (doLegitBackstab())
        return true;
    float swingrange = re::C_TFWeaponBaseMelee::GetSwingRange(RAW_ENT(LOCAL_W));
    // AimAt Autobackstab
    {
        for (int i = 1; i <= g_IEngine->GetMaxClients(); i++)
        {
            auto ent = ENTITY(i);
            if (CE_BAD(ent) || ent->m_flDistance() > swingrange * 4 || !ent->m_bEnemy() || !ent->m_bAlivePlayer() || g_pLocalPlayer->entity_idx == ent->m_IDX || IsPlayerInvulnerable(ent))
                continue;
            if (!player_tools::shouldTarget(ent))
                continue;
            auto hitbox = ClosestDistanceHitbox(ent);
            if (hitbox == -1)
                continue;
            auto angle = GetAimAtAngles(g_pLocalPlayer->v_Eye, ent->hitboxes.GetHitbox(hitbox)->center, LOCAL_E);
            if (!angleCheck(ent, std::nullopt, angle) && !canFaceStab(ent))
                continue;

            trace_t trace;
            Ray_t ray;
            trace::filter_default.SetSelf(RAW_ENT(g_pLocalPlayer->entity));
            ray.Init(g_pLocalPlayer->v_Eye, GetForwardVector(g_pLocalPlayer->v_Eye, angle, swingrange, LOCAL_E));
            g_ITrace->TraceRay(ray, MASK_SOLID, &trace::filter_default, &trace);
            if (trace.m_pEnt)
            {
                int index = reinterpret_cast<IClientEntity *>(trace.m_pEnt)->entindex();
                if (index == ent->m_IDX)
                {
                    current_user_cmd->buttons |= IN_ATTACK;
                    g_pLocalPlayer->bUseSilentAngles = true;
                    current_user_cmd->viewangles     = angle;
                    *bSendPackets                    = true;
                    return true;
                }
            }
        }
    }

    // Rotating Autobackstab
    {
        Vector newangle = { 0, 0, g_pLocalPlayer->v_OrigViewangles.z };
        std::vector<float> yangles;
        for (newangle.y = -180.0f; newangle.y < 180.0f; newangle.y += 10.0f)
        {
            trace_t trace;
            Ray_t ray;
            trace::filter_default.SetSelf(RAW_ENT(g_pLocalPlayer->entity));
            ray.Init(g_pLocalPlayer->v_Eye, GetForwardVector(g_pLocalPlayer->v_Eye, newangle, swingrange, LOCAL_E));
            g_ITrace->TraceRay(ray, MASK_SOLID, &trace::filter_default, &trace);
            if (trace.m_pEnt)
            {
                int index = reinterpret_cast<IClientEntity *>(trace.m_pEnt)->entindex();
                auto ent  = ENTITY(index);
                if (index == 0 || index > PLAYER_ARRAY_SIZE || !ent->m_bEnemy() || !player_tools::shouldTarget(ent) || IsPlayerInvulnerable(ent))
                    continue;
                if (angleCheck(ent, std::nullopt, newangle))
                {
                    yangles.push_back(newangle.y);
                }
            }
        }
        if (!yangles.empty())
        {
            newangle.y = yangles.at(std::floor((float) yangles.size() / 2));
            current_user_cmd->buttons |= IN_ATTACK;
            current_user_cmd->viewangles     = newangle;
            g_pLocalPlayer->bUseSilentAngles = true;
            *bSendPackets                    = true;
            return true;
        }
    }
    return false;
}
// Make accessible to the filter
static bool legit_stab = false;
static Vector newangle_apply;

bool backtrackFilter(CachedEntity *ent, hacks::tf2::backtrack::BacktrackData tick, std::optional<hacks::tf2::backtrack::BacktrackData> &best_tick)
{
    float swingrange = re::C_TFWeaponBaseMelee::GetSwingRange(RAW_ENT(LOCAL_W));

    Vector target_worldspace;
    VectorLerp(tick.m_vecMins, tick.m_vecMaxs, 0.5f, target_worldspace);
    target_worldspace += tick.m_vecOrigin;
    Vector distcheck = target_worldspace;
    distcheck.z      = g_pLocalPlayer->v_Eye.z;

    // Angle check
    Vector newangle;
    if (legit_stab)
        newangle = g_pLocalPlayer->v_OrigViewangles;
    else
        newangle = GetAimAtAngles(g_pLocalPlayer->v_Eye, distcheck, LOCAL_E);
    if (!angleCheck(ent, target_worldspace, newangle) && !canFaceStab(ent))
        return false;

    Vector min = tick.m_vecMins + tick.m_vecOrigin;
    Vector max = tick.m_vecMaxs + tick.m_vecOrigin;
    Vector hit;

    // Check if we can hit the enemies hitbox
    if (hacks::shared::triggerbot::CheckLineBox(min, max, g_pLocalPlayer->v_Eye, GetForwardVector(g_pLocalPlayer->v_Eye, newangle, swingrange * 0.95f), hit))
    {
        // Check if this tick is closer
        if (!best_tick || (*best_tick).m_vecOrigin.DistTo(LOCAL_E->m_vecOrigin()) > tick.m_vecOrigin.DistTo(LOCAL_E->m_vecOrigin()))
        {
            newangle_apply = newangle;
            return true;
        }
    }

    return false;
}

static bool doBacktrackStab(bool legit = false)
{
    CachedEntity *stab_ent = nullptr;
    hacks::tf2::backtrack::BacktrackData stab_data;
    // Set for our filter
    legit_stab = legit;
    // Get the Best tick
    for (int i = 0; i <= g_IEngine->GetMaxClients(); i++)
    {
        CachedEntity *ent = ENTITY(i);
        // Targeting checks
        if (CE_BAD(ent) || !ent->m_bAlivePlayer() || !ent->m_bEnemy() || !player_tools::shouldTarget(ent) || IsPlayerInvulnerable(ent))
            continue;

        // Get the best tick for that ent
        auto tick_data = hacks::tf2::backtrack::getBestTick(ent, backtrackFilter);

        // We found something matching the criterias, break out
        if (tick_data)
        {
            stab_data = *tick_data;
            stab_ent  = ent;
            break;
        }
    }

    // We found a good ent
    if (stab_ent)
    {
        hacks::tf2::backtrack::SetBacktrackData(stab_ent, stab_data);
        current_user_cmd->buttons |= IN_ATTACK;
        current_user_cmd->viewangles     = newangle_apply;
        g_pLocalPlayer->bUseSilentAngles = true;
        *bSendPackets                    = true;
        return true;
    }
    return false;
}

inline bool HasKnife()
{
    if (re::C_TFWeaponBase::GetWeaponID(RAW_ENT(LOCAL_W)) == 7)
        return true;
    return false;
}

void CreateMove()
{
    if (!enabled)
        return;
    if (CE_BAD(LOCAL_E) || g_pLocalPlayer->life_state || CE_BAD(LOCAL_W) || !HasKnife() || IsPlayerInvisible(LOCAL_E) || CE_BYTE(LOCAL_E, netvar.m_bFeignDeathReady))
        return;
    if (!CanShoot())
        return;
    switch (*mode)
    {
    case 0:
        doLegitBackstab();
        break;
    case 1:
        doRageBackstab();
        break;
    case 2:
        if (hacks::tf2::backtrack::isBacktrackEnabled)
        {
            if (*hacks::tf2::backtrack::latency <= 190 && doRageBackstab())
                break;
            doBacktrackStab(false);
        }
        else
        {
            doRageBackstab();
        }
        break;
    case 3:
        if (hacks::tf2::backtrack::isBacktrackEnabled)
        {
            if (*hacks::tf2::backtrack::latency <= 190 && doLegitBackstab())
                break;
            doBacktrackStab(true);
        }
        else
        {
            doLegitBackstab();
        }
        break;
    default:
        break;
    }
}

static InitRoutine EC([]() { EC::Register(EC::CreateMove, CreateMove, "autobackstab", EC::average); });
} // namespace hacks::tf2::autobackstab
