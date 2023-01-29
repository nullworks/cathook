/*
 * AntiBackstab.cpp
 *
 *  Created on: Apr 14, 2017
 *      Author: nullifiedcat
 */

#include <settings/Bool.hpp>
#include "common.hpp"
#include "hack.hpp"

namespace hacks::tf2::autobackstab
{
extern bool angleCheck(CachedEntity *from, CachedEntity *to, std::optional<Vector> target_pos, Vector from_angle);
}
namespace hacks::tf2::antibackstab
{
static settings::Boolean enable{ "antibackstab.enable", "0" };
static settings::Float distance{ "antibackstab.distance", "200" };
static settings::Boolean sayno{ "antibackstab.nope", "0" };
bool noaa = false;

void SayNope()
{
    static float last_say = 0.0f;
    if (g_GlobalVars->curtime < last_say)
        last_say = 0.0f;
    if (g_GlobalVars->curtime - last_say < 1.5f)
        return;
    hack::ExecuteCommand("voicemenu 0 7");
    last_say = g_GlobalVars->curtime;
}

float GetAngle(CachedEntity *spy)
{
    float yaw, yaw2, anglediff;
    Vector diff;
    yaw             = g_pLocalPlayer->v_OrigViewangles.y;
    const Vector &A = LOCAL_E->m_vecOrigin();
    const Vector &B = spy->m_vecOrigin();
    diff            = (A - B);
    yaw2            = acos(diff.x / diff.Length()) * 180.0f / PI;
    if (diff.y < 0)
        yaw2 = -yaw2;
    anglediff = yaw - yaw2;
    if (anglediff > 180)
        anglediff -= 360;
    if (anglediff < -180)
        anglediff += 360;
    // logging::Info("Angle: %.2f | %.2f | %.2f | %.2f", yaw, yaw2, anglediff,
    // yaw - yaw2);
    return anglediff;
}

CachedEntity *ClosestSpy()
{
    CachedEntity *closest;
    float closest_dist, dist;

    closest      = nullptr;
    closest_dist = 0.0f;

    for (auto const &ent: entity_cache::player_cache)
    {
        
        bool ispyro  = false;
        bool isheavy = false;
        if (CE_INT(ent, netvar.iClass) != tf_class::tf_spy)
        {
            if (CE_INT(ent, netvar.iClass) != tf_class::tf_pyro && CE_INT(ent, netvar.iClass) != tf_class::tf_heavy)
                continue;
            int idx = HandleToIDX(CE_INT(ent, netvar.hActiveWeapon));
            if (IDX_BAD(idx))
                continue;
            CachedEntity *pyro_weapon = ENTITY(idx);
            if (CE_BAD(pyro_weapon))
                continue;
            int widx = CE_INT(pyro_weapon, netvar.iItemDefinitionIndex);
            if (widx != 40 && widx != 1146 && widx != 656)
                continue;
            if (widx == 656)
                isheavy = true;
            ispyro = true;
        }
        if (CE_INT(ent, netvar.iTeamNum) == g_pLocalPlayer->team)
            continue;
        if (IsPlayerInvisible(ent))
            continue;
        dist = ent->m_flDistance();
        if ((ispyro && !isheavy && fabs(GetAngle(ent)) > 90.0f) || (isheavy && fabs(GetAngle(ent)) > 132.0f))
        {
            break;
            // logging::Info("Backstab???");
        }
        if ((((!ispyro && dist < (float) distance)) || (ispyro && !isheavy && dist < 314.0f) || (isheavy && dist < 120.0f)) && (dist < closest_dist || !closest_dist))
        {
            closest_dist = dist;
            closest      = ent;
        }
    }
    return closest;
}

static void CreateMove()
{
    CachedEntity *spy;
    Vector diff;

    if (!enable || CE_BAD(LOCAL_E))
        return;
    spy = ClosestSpy();
    if (spy)
    {
        noaa = true;
        if (current_user_cmd->buttons & IN_ATTACK)
            return;
        Vector spy_angle;
        spy_angle = CE_VECTOR(spy, netvar.m_angEyeAngles);
        fClampAngle(spy_angle);
        bool couldbebackstabbed = hacks::tf2::autobackstab::angleCheck(spy, LOCAL_E, std::nullopt, spy_angle);
        if (couldbebackstabbed || CE_INT(spy, netvar.iClass) == tf_class::tf_heavy)
            current_user_cmd->viewangles.x = 150.0f;
        g_pLocalPlayer->bUseSilentAngles = true;
        if (sayno)
            SayNope();
        *bSendPackets = true;
    }
    else
        noaa = false;
}

static InitRoutine EC(
    []()
    {
        EC::Register(EC::CreateMove, CreateMove, "antibackstab", EC::late);
        EC::Register(EC::CreateMoveWarp, CreateMove, "antibackstab_w", EC::late);
    });
} // namespace hacks::tf2::antibackstab
