/*
  Created by Jenny White on 29.04.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#include <MiscTemporary.hpp>
#include <hacks/hacklist.hpp>
#include <settings/Bool.hpp>
#include <hacks/Thirdperson.hpp>
#include "HookedMethods.hpp"
#include "hacks/Backtrack.hpp"
#include "AntiAntiAim.hpp"

static settings::Float nightmode{ "visual.night-mode", "0" };
static settings::Boolean no_shake{ "visual.no-shake", "true" };

static float old_nightmode{ 0.0f };

namespace hooked_methods
{
#include "reclasses.hpp"
DEFINE_HOOKED_METHOD(FrameStageNotify, void, void *this_, ClientFrameStage_t stage)
{
    if (!isHackActive())
        return original::FrameStageNotify(this_, stage);

    PROF_SECTION(FrameStageNotify_TOTAL);

    if (old_nightmode != *nightmode)
    {
        static ConVar *r_DrawSpecificStaticProp = g_ICvar->FindVar("r_DrawSpecificStaticProp");
        if (!r_DrawSpecificStaticProp)
        {
            r_DrawSpecificStaticProp = g_ICvar->FindVar("r_DrawSpecificStaticProp");
            return;
        }
        r_DrawSpecificStaticProp->SetValue(0);

        for (MaterialHandle_t i = g_IMaterialSystem->FirstMaterial(); i != g_IMaterialSystem->InvalidMaterial(); i = g_IMaterialSystem->NextMaterial(i))
        {
            IMaterial *pMaterial = g_IMaterialSystem->GetMaterial(i);

            if (!pMaterial)
                continue;
            if (strstr(pMaterial->GetTextureGroupName(), "World") || strstr(pMaterial->GetTextureGroupName(), "StaticProp"))
            {
                if (float(nightmode) > 0.0f)
                {
                    if (strstr(pMaterial->GetTextureGroupName(), "StaticProp"))
                        pMaterial->ColorModulate(1.0f - float(nightmode) / 100.0f, 1.0f - float(nightmode) / 100.0f, 1.0f - float(nightmode) / 100.0f);
                    else
                        pMaterial->ColorModulate((1.0f - float(nightmode) / 100.0f) / 6.0f, (1.0f - float(nightmode) / 100.0f) / 6.0f, (1.0f - float(nightmode) / 100.0f) / 6.0f);
                }
                else
                    pMaterial->ColorModulate(1.0f, 1.0f, 1.0f);
            }
        }
        old_nightmode = *nightmode;
    }

    if (!g_IEngine->IsInGame())
        g_Settings.bInvalid = true;
    {
        PROF_SECTION(FSN_antiantiaim);
        hacks::shared::anti_anti_aim::frameStageNotify(stage);
    }
    {
        PROF_SECTION(FSN_skinchanger);
        hacks::tf2::skinchanger::FrameStageNotify(stage);
    }
    /*if (hacks::tf2::seedprediction::prediction && CE_GOOD(LOCAL_E)) {
        C_BaseTempEntity *fire = C_TEFireBullets::GTEFireBullets();
        while (fire) {
            logging::Info("0x%08X", (uintptr_t) fire);
            C_TEFireBullets *fire2 = nullptr;
            if (!fire->IsDormant() &&
    fire->GetClientNetworkable()->GetClientClass() &&
    fire->GetClientNetworkable()->GetClientClass()->m_ClassID ==
    CL_CLASS(CTEFireBullets))
                fire2 = (C_TEFireBullets *) fire;
            if (fire2 && !hooks::IsHooked((void *) fire2)) {
                hooks::firebullets.Set(fire2);
                hooks::firebullets.HookMethod(HOOK_ARGS(PreDataUpdate));
                hooks::firebullets.Apply();
            }
            if (fire2)
                logging::Info("%d", fire2->m_iSeed());
            fire = fire->m_pNext;
        }
    }*/

    if (isHackActive() && !g_Settings.bInvalid && stage == FRAME_RENDER_START)
    {
        if (vm_aimbot)
        {
            if (CE_GOOD(LOCAL_E) && LOCAL_E->m_bAlivePlayer())
                if (ViewmodelAimbotAngle.IsValid() && !ViewmodelAimbotAngle.IsZero())
                {
                    auto Viewmodel = (IClientEntity *) g_IEntityList->GetClientEntityFromHandle(CE_INT(LOCAL_E, netvar.hViewModel));

                    if (Viewmodel)
                    {
                        QAngle &write = const_cast<QAngle &>(Viewmodel->GetAbsAngles());
                        write         = VectorToQAngle(ViewmodelAimbotAngle);
                    }
                }
        }
        IF_GAME(IsTF())
        {
            if (no_shake && CE_GOOD(LOCAL_E) && LOCAL_E->m_bAlivePlayer())
            {
                NET_VECTOR(RAW_ENT(LOCAL_E), netvar.vecPunchAngle)    = { 0.0f, 0.0f, 0.0f };
                NET_VECTOR(RAW_ENT(LOCAL_E), netvar.vecPunchAngleVel) = { 0.0f, 0.0f, 0.0f };
            }
        }
        hacks::tf::thirdperson::frameStageNotify();
    }
    return original::FrameStageNotify(this_, stage);
} // namespace hooked_methods
} // namespace hooked_methods
