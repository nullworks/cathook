/*
 * AntiBackstab.cpp
 *
 *  Created on: Apr 14, 2017
 *      Author: nullifiedcat
 */

#include "common.hpp"
#include "hack.hpp"

namespace hacks
{
namespace tf2
{
namespace antibackstab
{
bool noaa = false;
static CatVar enabled(CV_SWITCH, XORSTR("antibackstab"), XORSTR("0"), XORSTR("Enable"),
                      XORSTR("Main anti-backstab switch"));
static CatVar distance(CV_FLOAT, XORSTR("antibackstab_distance"), XORSTR("200"), XORSTR("Distance"),
                       XORSTR("Distance Until anti-backstab reacts"));
static CatVar silent(CV_SWITCH, XORSTR("antibackstab_silent"), XORSTR("1"), XORSTR("Silent"),
                     XORSTR("Works silently without moving your view"));
static CatVar angle(CV_FLOAT, XORSTR("antibackstab_angle"), XORSTR("90"), XORSTR("Detection Angle"));
static CatVar sayno(CV_SWITCH, XORSTR("antibackstab_nope"), XORSTR("0"), XORSTR("Nope!"), XORSTR("Memes"));

void SayNope()
{
    static float last_say = 0.0f;
    if (g_GlobalVars->curtime < last_say)
        last_say = 0.0f;
    if (g_GlobalVars->curtime - last_say < 1.5f)
        return;
    hack::ExecuteCommand(XORSTR("voicemenu 0 7"));
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
        yaw2  = -yaw2;
    anglediff = yaw - yaw2;
    if (anglediff > 180)
        anglediff -= 360;
    if (anglediff < -180)
        anglediff += 360;
    // logging::Info(XORSTR("Angle: %.2f | %.2f | %.2f | %.2f"), yaw, yaw2, anglediff,
    // yaw - yaw2);
    return anglediff;
}

CachedEntity *ClosestSpy()
{
    CachedEntity *closest, *ent;
    float closest_dist, dist;

    closest      = nullptr;
    closest_dist = 0.0f;

    for (int i = 1; i < 32 && i < g_IEntityList->GetHighestEntityIndex(); i++)
    {
        ent = ENTITY(i);
        if (CE_BAD(ent))
            continue;
        if (CE_BYTE(ent, netvar.iLifeState))
            continue;
        if (CE_INT(ent, netvar.iClass) != tf_class::tf_spy)
            continue;
        if (CE_INT(ent, netvar.iTeamNum) == g_pLocalPlayer->team)
            continue;
        if (IsPlayerInvisible(ent))
            continue;
        dist = ent->m_flDistance();
        if (fabs(GetAngle(ent)) > (float) angle)
        {
            break;
            // logging::Info(XORSTR("Backstab???"));
        }
        if (dist < (float) distance && (dist < closest_dist || !closest_dist))
        {
            closest_dist = dist;
            closest      = ent;
        }
    }
    return closest;
}

void CreateMove()
{
    CachedEntity *spy;
    Vector diff;

    if (!enabled)
        return;
    spy                   = ClosestSpy();
    ConVar *pitchdown     = g_ICvar->FindVar(XORSTR("cl_pitchdown"));
    static int normal_val = pitchdown->GetInt();
    if (spy)
    {
        noaa = true;
        pitchdown->SetValue(180);
        g_pUserCmd->viewangles.x = 140.0f;
        if (silent)
            g_pLocalPlayer->bUseSilentAngles = true;
        if (sayno)
            SayNope();
    }
    else
    {
        pitchdown->SetValue(normal_val);
        noaa = false;
    }
}
}
}
}
