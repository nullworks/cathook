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
int ClosestDistanceHitbox(hacks::shared::backtrack::BacktrackData btd)
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

    Vector wsc_spy_to_victim;
    if (target_pos)
        wsc_spy_to_victim = *target_pos - from->m_vecOrigin();
    else
        wsc_spy_to_victim = to->m_vecOrigin() - from->m_vecOrigin();
    wsc_spy_to_victim.z = 0;
    wsc_spy_to_victim.NormalizeInPlace();

    Vector eye_spy;
    AngleVectors2(VectorToQAngle(from_angle), &eye_spy);
    eye_spy.z = 0;
    eye_spy.NormalizeInPlace();

    Vector eye_victim;
    AngleVectors2(VectorToQAngle(tarAngle), &eye_victim);
    eye_victim.z = 0;
    eye_victim.NormalizeInPlace();

    if (DotProduct(wsc_spy_to_victim, eye_victim) <= 0.0f)
        return false;
    if (DotProduct(wsc_spy_to_victim, eye_spy) <= 0.5f)
        return false;
    if (DotProduct(eye_spy, eye_victim) <= -0.3f)
        return false;
    return true;
}

static bool angleCheck(CachedEntity *target, std::optional<Vector> target_pos, Vector local_angle)
{
    Vector tarAngle = CE_VECTOR(target, netvar.m_angEyeAngles);

    Vector wsc_spy_to_victim;
    if (target_pos)
        wsc_spy_to_victim = *target_pos - LOCAL_E->m_vecOrigin();
    else
        wsc_spy_to_victim = target->m_vecOrigin() - LOCAL_E->m_vecOrigin();
    wsc_spy_to_victim.z = 0;
    wsc_spy_to_victim.NormalizeInPlace();

    Vector eye_spy;
    AngleVectors2(VectorToQAngle(local_angle), &eye_spy);
    eye_spy.z = 0;
    eye_spy.NormalizeInPlace();

    Vector eye_victim;
    AngleVectors2(VectorToQAngle(tarAngle), &eye_victim);
    eye_victim.z = 0;
    eye_victim.NormalizeInPlace();

    if (DotProduct(wsc_spy_to_victim, eye_victim) <= 0.0f)
        return false;
    if (DotProduct(wsc_spy_to_victim, eye_spy) <= 0.5f)
        return false;
    if (DotProduct(eye_spy, eye_victim) <= -0.3f)
        return false;
    return true;
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
        for (int i = 1; i < g_IEngine->GetMaxClients(); i++)
        {
            auto ent = ENTITY(i);
            if (CE_BAD(ent) || ent->m_flDistance() > swingrange * 4 || !ent->m_bEnemy() || !ent->m_bAlivePlayer() || g_pLocalPlayer->entity_idx == ent->m_IDX || IsPlayerInvulnerable(ent))
                continue;
            if (!player_tools::shouldTarget(ent))
                continue;
            auto hitbox = ClosestDistanceHitbox(ent);
            if (hitbox == -1)
                continue;
            auto angle = GetAimAtAngles(g_pLocalPlayer->v_Eye, ent->hitboxes.GetHitbox(hitbox)->center);
            if (!angleCheck(ent, std::nullopt, angle) && !canFaceStab(ent))
                continue;

            trace_t trace;
            Ray_t ray;
            trace::filter_default.SetSelf(RAW_ENT(g_pLocalPlayer->entity));
            ray.Init(g_pLocalPlayer->v_Eye, GetForwardVector(g_pLocalPlayer->v_Eye, angle, swingrange));
            g_ITrace->TraceRay(ray, MASK_SHOT_HULL, &trace::filter_default, &trace);
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
            ray.Init(g_pLocalPlayer->v_Eye, GetForwardVector(g_pLocalPlayer->v_Eye, newangle, swingrange));
            g_ITrace->TraceRay(ray, MASK_SHOT_HULL, &trace::filter_default, &trace);
            if (trace.m_pEnt)
            {
                int index = reinterpret_cast<IClientEntity *>(trace.m_pEnt)->entindex();
                auto ent  = ENTITY(index);
                if (index == 0 || index > g_IEngine->GetMaxClients() || !ent->m_bEnemy() || !player_tools::shouldTarget(ent) || IsPlayerInvulnerable(ent))
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

static bool doBacktrackStab()
{
    float swingrange = re::C_TFWeaponBaseMelee::GetSwingRange(RAW_ENT(LOCAL_W));
    CachedEntity *ent;
    if (hacks::shared::backtrack::iBestTarget < 1)
        return false;
    // Get the best ent decided by backtrack (no reason to do work twice)
    ent = ENTITY(hacks::shared::backtrack::iBestTarget);
    if (!ent->m_bEnemy() || !player_tools::shouldTarget(ent) || IsPlayerInvulnerable(ent))
        return false;
    // Get the ent's backtrack ticks
    auto &btd = hacks::shared::backtrack::headPositions[ent->m_IDX];

    for (int i = 0; i < 66; i++)
    {
        auto &btp = btd[i];
        // Check if the backtrack positions are valid
        if (!hacks::shared::backtrack::ValidTick(btp, ent))
            continue;
        Vector distcheck = btp.entorigin;
        distcheck.z      = g_pLocalPlayer->v_Eye.z;
        // dont stab while inside the enemy
        if (distcheck.DistTo(g_pLocalPlayer->v_Eye) < 20.0f)
            continue;
        // Get and calculate an  angle to use to backstab the ent
        auto hitbox = ClosestDistanceHitbox(btp);
        if (hitbox == -1)
            continue;
        Vector newangle = GetAimAtAngles(g_pLocalPlayer->v_Eye, btp.hitboxes.at(hitbox).center);
        if (!angleCheck(ent, btp.entorigin, newangle) && !canFaceStab(ent))
            continue;
        Vector &min = btp.collidable.min;
        Vector &max = btp.collidable.max;
        Vector hit;
        // Check if we can hit the enemies hitbox
        if (hacks::shared::triggerbot::CheckLineBox(min, max, g_pLocalPlayer->v_Eye, GetForwardVector(g_pLocalPlayer->v_Eye, newangle, swingrange * 0.95f), hit))
        {
            current_user_cmd->tick_count = btp.tickcount;
            current_user_cmd->viewangles = newangle;
            current_user_cmd->buttons |= IN_ATTACK;
            g_pLocalPlayer->bUseSilentAngles = true;
            *bSendPackets                    = true;
            return true;
        }
    }
    return false;
}

// TODO: Don't duplicate these loops
static bool doLegitBacktrackStab() // lol
{
    float swingrange = re::C_TFWeaponBaseMelee::GetSwingRange(RAW_ENT(LOCAL_W));
    CachedEntity *ent;
    if (hacks::shared::backtrack::iBestTarget < 1)
        return false;
    ent = ENTITY(hacks::shared::backtrack::iBestTarget);
    if (!ent->m_bEnemy() || !player_tools::shouldTarget(ent) || IsPlayerInvulnerable(ent))
        return false;
    auto &btd       = hacks::shared::backtrack::headPositions[ent->m_IDX];
    Vector newangle = g_pLocalPlayer->v_OrigViewangles;
    std::vector<float> yangles;
    float best_scr = FLT_MAX;
    hacks::shared::backtrack::BacktrackData *best_tick;
    for (int ii = 0; ii < 66; ii++)
    {
        std::vector<float> yangles_tmp;
        auto &i = btd[ii];

        Vector distcheck = i.entorigin;
        distcheck.z      = g_pLocalPlayer->v_Eye.z;
        if (distcheck.DistTo(g_pLocalPlayer->v_Eye) < best_scr && distcheck.DistTo(g_pLocalPlayer->v_Eye) > 20.0f)
        {
            if (!hacks::shared::backtrack::ValidTick(i, ent))
                continue;
            if (!angleCheck(ent, i.entorigin, newangle) && !canFaceStab(ent))
                continue;

            Vector &min = i.collidable.min;
            Vector &max = i.collidable.max;

            // Get the min and max for the hitbox
            Vector minz(fminf(min.x, max.x), fminf(min.y, max.y), fminf(min.z, max.z));
            Vector maxz(fmaxf(min.x, max.x), fmaxf(min.y, max.y), fmaxf(min.z, max.z));

            // Shrink the hitbox here
            Vector size = maxz - minz;
            Vector smod = { size.x * 0.20f, size.y * 0.20f, 0 };

            // Save the changes to the vectors
            minz += smod;
            maxz -= smod;
            maxz.z += 20.0f;

            Vector hit;
            if (hacks::shared::triggerbot::CheckLineBox(minz, maxz, g_pLocalPlayer->v_Eye, GetForwardVector(g_pLocalPlayer->v_Eye, newangle, swingrange), hit))
                yangles_tmp.push_back(newangle.y);

            if (!yangles_tmp.empty())
            {
                yangles.clear();
                best_scr  = distcheck.DistTo(g_pLocalPlayer->v_Eye);
                best_tick = &i;
                yangles   = yangles_tmp;
            }
        }
    }
    if (!yangles.empty() && best_tick)
    {
        newangle.y                   = yangles.at(std::floor((float) yangles.size() / 2));
        current_user_cmd->tick_count = best_tick->tickcount;
        current_user_cmd->viewangles = newangle;
        current_user_cmd->buttons |= IN_ATTACK;
        g_pLocalPlayer->bUseSilentAngles = true;
        *bSendPackets                    = true;
        return true;
    }
    return false;
}

void CreateMove()
{
    if (!enabled)
        return;
    if (CE_BAD(LOCAL_E) || g_pLocalPlayer->life_state || g_pLocalPlayer->clazz != tf_spy || CE_BAD(LOCAL_W) || GetWeaponMode() != weapon_melee || IsPlayerInvisible(LOCAL_E) || CE_BYTE(LOCAL_E, netvar.m_bFeignDeathReady))
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
        if (hacks::shared::backtrack::isBacktrackEnabled)
        {
            if (*hacks::shared::backtrack::latency <= 190 && doRageBackstab())
                break;
            doBacktrackStab();
        }
        else
        {
            doRageBackstab();
        }
        break;
    case 3:
        if (hacks::shared::backtrack::isBacktrackEnabled)
        {
            if (*hacks::shared::backtrack::latency <= 190 && doLegitBackstab())
                break;
            doLegitBacktrackStab();
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
