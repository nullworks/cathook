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
#include "conditions.hpp"
#include "helpers.hpp"
#include "DetourHook.hpp"

namespace hacks::tf2::autobackstab
{
static settings::Boolean enabled("autobackstab.enabled", "false");
static settings::Boolean decrease_range("autobackstab.decrease-range", "false");
static settings::Int mode("autobackstab.mode", "0");

DetourHook melee_range_hook{};

typedef int (*GetSwingRange_o)(IClientEntity *);
int GetSwingRange_hook(IClientEntity *_this)
{
    float return_val = ((GetSwingRange_o) melee_range_hook.GetOriginalFunc())(_this);
    if (decrease_range)
        return_val *= 0.5f;
    melee_range_hook.RestorePatch();
    return return_val;
}

void ApplySwingHook()
{
    static bool inited = false;
    if (!inited)
    {
        inited = true;
        melee_range_hook.Init((uintptr_t) vfunc<GetSwingRange_o>(RAW_ENT(LOCAL_W), offsets::PlatformOffset(521, offsets::undefined, 521), 0), (void *) GetSwingRange_hook);
    }
    melee_range_hook.RestorePatch();
}

void RemoveSwingHook()
{
    melee_range_hook.Shutdown();
}

// Function to do swing trace in a specific direction
bool doSwingTraceAngle(Vector angles, trace_t &trace)
{
    Vector original_angles = LOCAL_E->m_vecAngle();

    re::C_BasePlayer::GetEyeAngles(RAW_ENT(LOCAL_E)) = angles;
    ApplySwingHook();
    bool hit_trace = re::C_TFWeaponBaseMelee::DoSwingTrace(RAW_ENT(LOCAL_W), &trace);
    RemoveSwingHook();

    re::C_BasePlayer::GetEyeAngles(RAW_ENT(LOCAL_E)) = original_angles;
    return hit_trace && (IClientEntity *) trace.m_pEnt != g_IEntityList->GetClientEntity(0);
}

// Function to do a swing trace against an entity from an arbitary position
bool doMovedSwingTrace(CachedEntity *target, Vector new_target_origin)
{
    // Back up data
    Vector target_origin = target->m_vecOrigin();

    Vector angles_to_target = GetAimAtAngles(LOCAL_E->m_vecOrigin(), new_target_origin);

    // Set Our and entity data to match the target origin
    const_cast<Vector &>(RAW_ENT(target)->GetAbsOrigin()) = new_target_origin;

    // We need to update their positions so the rays actually work, this requires some hacky stuff
    uintptr_t collisionprop = (uintptr_t) RAW_ENT(target) + netvar.m_Collision;

    typedef void (*MarkSurroundingBoundsDirty_t)(uintptr_t prop);
    static auto sig_mark                                              = gSignatures.GetClientSignature("55 89 E5 56 53 83 EC 10 8B 5D ? 8B 43 ? 81 88 ? ? ? ? 00 40 00 00");
    static MarkSurroundingBoundsDirty_t MarkSurroundingBoundsDirty_fn = (MarkSurroundingBoundsDirty_t) sig_mark;

    typedef void (*UpdateParition_t)(uintptr_t prop);
    static auto sig_update                     = gSignatures.GetClientSignature("55 89 E5 57 56 53 83 EC 3C 8B 5D ? 8B 43 ? 8B 90");
    static UpdateParition_t UpdatePartition_fn = (UpdateParition_t) sig_update;

    // Mark for update
    MarkSurroundingBoundsDirty_fn(collisionprop);
    // Update
    UpdatePartition_fn(collisionprop);

    trace_t trace;

    // TF2 swing trace
    bool did_trace_hit = doSwingTraceAngle(angles_to_target, trace);

    // Restore data
    const_cast<Vector &>(RAW_ENT(target)->GetAbsOrigin()) = target_origin;

    // Once again update the ray traces
    MarkSurroundingBoundsDirty_fn(collisionprop);
    UpdatePartition_fn(collisionprop);

    return did_trace_hit && (IClientEntity *) trace.m_pEnt == RAW_ENT(target);
}

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
    int w_index = CE_INT(LOCAL_W, netvar.iItemDefinitionIndex);

    if (w_index == 40000)
    {
        return false;
    }

    if (HasCondition<TFCond_MiniCritOnKill>(g_pLocalPlayer->entity) || HasCondition<TFCond_Jarated>(ent))
    {
        return ent->m_iHealth() <= 54.0f;
    }

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
    ApplySwingHook();
    if (!re::C_TFWeaponBaseMelee::DoSwingTrace(RAW_ENT(LOCAL_W), &trace))
    {
        RemoveSwingHook();
        return false;
    }
    RemoveSwingHook();

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
    trace_t trace;
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

            Vector aim_pos = ent->m_vecOrigin();
            aim_pos.z      = ent->hitboxes.GetHitbox(hitbox)->center.z;
            auto angle     = GetAimAtAngles(g_pLocalPlayer->v_Eye, aim_pos, LOCAL_E);
            if (!angleCheck(ent, std::nullopt, angle) && !canFaceStab(ent))
                continue;
            if (doSwingTraceAngle(angle, trace))
            {
                current_user_cmd->buttons |= IN_ATTACK;
                g_pLocalPlayer->bUseSilentAngles = true;
                current_user_cmd->viewangles     = angle;
                *bSendPackets                    = true;
                return true;
            }
        }
    }
    // Rotating Autobackstab, only use if we hit the world
    if (trace.DidHit() && (IClientEntity *) trace.m_pEnt == g_IEntityList->GetClientEntity(0))
    {
        Vector newangle = { 0, 0, g_pLocalPlayer->v_OrigViewangles.z };
        std::vector<float> yangles;
        for (newangle.y = -180.0f; newangle.y < 180.0f; newangle.y += 10.0f)
        {
            if (doSwingTraceAngle(newangle, trace))
            {
                int index = reinterpret_cast<IClientEntity *>(trace.m_pEnt)->entindex();
                auto ent  = ENTITY(index);
                if (index == 0 || index > PLAYER_ARRAY_SIZE || !ent->m_bEnemy() || !player_tools::shouldTarget(ent) || IsPlayerInvulnerable(ent))
                    continue;
                if (angleCheck(ent, std::nullopt, newangle))
                    yangles.push_back(newangle.y);
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

    // Check if we can hit the enemy
    if (doMovedSwingTrace(ent, tick.m_vecOrigin))
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

static InitRoutine EC([]() {
    EC::Register(EC::CreateMove, CreateMove, "autobackstab", EC::average);
    EC::Register(EC::CreateMoveWarp, CreateMove, "autobackstab_w", EC::average);
});
} // namespace hacks::tf2::autobackstab
