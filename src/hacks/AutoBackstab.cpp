/*
 * AutoBackstab.cpp
 *
 *  Created on: Apr 14, 2017
 *      Author: nullifiedcat
 */

#include "common.hpp"
#include "Backtrack.hpp"

namespace hacks
{
namespace tf2
{
namespace autobackstab
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

static CatVar enabled(CV_SWITCH, XORSTR("autobackstab"), XORSTR("0"), XORSTR("Auto Backstab"),
                      XORSTR("Does not depend on triggerbot!"));
static CatVar value(CV_INT, XORSTR("autobackstab_range"), XORSTR("75.0f"),
                    XORSTR("Set Detection Distance to this much"));
bool found;
// TODO improve
void CreateMove()
{
    if (!enabled)
        return;
    if (!CE_GOOD(LOCAL_E))
        return;
    if (!LOCAL_E->m_bAlivePlayer())
        return;
    if (g_pLocalPlayer->weapon()->m_iClassID() != CL_CLASS(CTFKnife))
        return;
    if (CE_BYTE(g_pLocalPlayer->weapon(), netvar.m_bReadyToBackstab))
        g_pUserCmd->buttons |= IN_ATTACK;
    else
    {
        if (!hacks::shared::backtrack::enable)
            return;
        if (hacks::shared::backtrack::iBestTarget == -1)
            return;
        int iBestTarget = hacks::shared::backtrack::iBestTarget;
        int BestTick    = hacks::shared::backtrack::BestTick;

        float scr =
            abs(g_pLocalPlayer->v_OrigViewangles.y -
                hacks::shared::backtrack::headPositions[iBestTarget][BestTick]
                    .viewangles);

        if (scr < 40.0f &&
            hacks::shared::backtrack::headPositions[iBestTarget][BestTick]
                    .origin.DistTo(g_pLocalPlayer->v_Eye) <=
                re::C_TFWeaponBaseMelee::GetSwingRange(RAW_ENT(LOCAL_W)))
        {
            g_pUserCmd->tick_count =
                hacks::shared::backtrack::headPositions[iBestTarget][BestTick]
                    .tickcount;
            g_pUserCmd->buttons |= IN_ATTACK;
        }
    }
}
}
}
}
