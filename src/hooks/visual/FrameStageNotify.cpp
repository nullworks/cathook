/*
  Created by Jenny White on 29.04.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#include <MiscTemporary.hpp>
#include <hacks/hacklist.hpp>
#include "HookedMethods.hpp"
#if not LAGBOT_MODE
#include "hacks/Backtrack.hpp"
#endif

static CatVar nightmode(CV_FLOAT, "nightmode", "0", "Enable nightmode", "");
namespace hooked_methods
{
#include "reclasses.hpp"
#include "C_TEFireBullets.hpp"
DEFINE_HOOKED_METHOD(FrameStageNotify, void, void *this_,
                     ClientFrameStage_t stage)
{
    static float OldNightmode = 0.0f;
    if (OldNightmode != (float) nightmode)
    {

        static ConVar *r_DrawSpecificStaticProp =
            g_ICvar->FindVar("r_DrawSpecificStaticProp");
        if (!r_DrawSpecificStaticProp)
        {
            r_DrawSpecificStaticProp =
                g_ICvar->FindVar("r_DrawSpecificStaticProp");
            return;
        }
        r_DrawSpecificStaticProp->SetValue(0);

        for (MaterialHandle_t i = g_IMaterialSystem->FirstMaterial();
             i != g_IMaterialSystem->InvalidMaterial();
             i = g_IMaterialSystem->NextMaterial(i))
        {
            IMaterial *pMaterial = g_IMaterialSystem->GetMaterial(i);

            if (!pMaterial)
                continue;
            if (strstr(pMaterial->GetTextureGroupName(), "World") ||
                strstr(pMaterial->GetTextureGroupName(), "StaticProp"))
            {
                if (float(nightmode) > 0.0f)
                {
                    if (strstr(pMaterial->GetTextureGroupName(), "StaticProp"))
                        pMaterial->ColorModulate(
                            1.0f - float(nightmode) / 100.0f,
                            1.0f - float(nightmode) / 100.0f,
                            1.0f - float(nightmode) / 100.0f);
                    else
                        pMaterial->ColorModulate(
                            (1.0f - float(nightmode) / 100.0f) / 6.0f,
                            (1.0f - float(nightmode) / 100.0f) / 6.0f,
                            (1.0f - float(nightmode) / 100.0f) / 6.0f);
                }
                else
                    pMaterial->ColorModulate(1.0f, 1.0f, 1.0f);
            }
        }
        OldNightmode = nightmode;
    }

    PROF_SECTION(FrameStageNotify_TOTAL);

    if (!g_IEngine->IsInGame())
        g_Settings.bInvalid = true;
    {
        PROF_SECTION(FSN_skinchanger);
        hacks::tf2::skinchanger::FrameStageNotify(stage);
    }
    if (cathook && stage == FRAME_RENDER_START)
    {
        INetChannel *ch;
        ch = (INetChannel *) g_IEngine->GetNetChannelInfo();
        if (ch && !hooks::IsHooked((void *) ch))
        {
            hooks::netchannel.Set(ch);
            hooks::netchannel.HookMethod(HOOK_ARGS(SendDatagram));
            hooks::netchannel.HookMethod(HOOK_ARGS(CanPacket));
            hooks::netchannel.HookMethod(HOOK_ARGS(SendNetMsg));
            hooks::netchannel.HookMethod(HOOK_ARGS(Shutdown));
            hooks::netchannel.Apply();
#if ENABLE_IPC
            ipc::UpdateServerAddress();
#endif
        }
        C_TEFireBullets *fire = C_TEFireBullets::GTEFireBullets();
        if (fire && !hooks::IsHooked((void *)fire))
        {
        	hooks::firebullets.Set(fire);
        	hooks::firebullets.HookMethod(HOOK_ARGS(PreDataUpdate));
        	hooks::firebullets.Apply();
        }
    }
    if (cathook && !g_Settings.bInvalid && stage == FRAME_RENDER_START)
    {
        IF_GAME(IsTF())
        {
            if (CE_GOOD(LOCAL_E) && no_zoom)
                RemoveCondition<TFCond_Zoomed>(LOCAL_E);
        }
        if (force_thirdperson && !g_pLocalPlayer->life_state &&
            CE_GOOD(g_pLocalPlayer->entity))
        {
            CE_INT(g_pLocalPlayer->entity, netvar.nForceTauntCam) = 1;
        }
        if (stage == 5 && show_antiaim && g_IInput->CAM_IsThirdPerson())
        {
            if (CE_GOOD(g_pLocalPlayer->entity))
            {
                CE_FLOAT(g_pLocalPlayer->entity, netvar.deadflag + 4) =
                    g_Settings.brute.last_angles[LOCAL_E->m_IDX].x;
                CE_FLOAT(g_pLocalPlayer->entity, netvar.deadflag + 8) =
                    g_Settings.brute.last_angles[LOCAL_E->m_IDX].y;
            }
        }
    }
    return original::FrameStageNotify(this_, stage);
}
}
