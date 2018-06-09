/*
  Created by Jenny White on 29.04.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#include <MiscTemporary.hpp>
#include <hacks/hacklist.hpp>
#include "HookedMethods.hpp"
#if not LAGBOT_MODE
#include "Backtrack.hpp"
#endif
static CatVar resolver(CV_SWITCH, XORSTR("resolver"), XORSTR("0"), XORSTR("Resolve angles"));
static CatVar nightmode(CV_SWITCH, XORSTR("nightmode"), XORSTR("0"), XORSTR("Enable nightmode"), XORSTR(""));
namespace hooked_methods
{

DEFINE_HOOKED_METHOD(FrameStageNotify, void, void *this_,
                     ClientFrameStage_t stage)
{
    if (nightmode)
    {
        static int OldNightmode = 0;
        if (OldNightmode != (int) nightmode)
        {

            static ConVar *r_DrawSpecificStaticProp =
                g_ICvar->FindVar(XORSTR("r_DrawSpecificStaticProp"));
            if (!r_DrawSpecificStaticProp)
            {
                r_DrawSpecificStaticProp =
                    g_ICvar->FindVar(XORSTR("r_DrawSpecificStaticProp"));
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
                if (strstr(pMaterial->GetTextureGroupName(), XORSTR("World")) ||
                    strstr(pMaterial->GetTextureGroupName(), XORSTR("StaticProp")))
                {
                    if (nightmode)
                        if (strstr(pMaterial->GetTextureGroupName(),
                                   XORSTR("StaticProp")))
                            pMaterial->ColorModulate(0.3f, 0.3f, 0.3f);
                        else
                            pMaterial->ColorModulate(0.05f, 0.05f, 0.05f);
                    else
                        pMaterial->ColorModulate(1.0f, 1.0f, 1.0f);
                }
            }
            OldNightmode = nightmode;
        }
    }
    static IClientEntity *ent;

    PROF_SECTION(FrameStageNotify_TOTAL);

    if (!g_IEngine->IsInGame())
        g_Settings.bInvalid = true;
    {
        PROF_SECTION(FSN_skinchanger);
        hacks::tf2::skinchanger::FrameStageNotify(stage);
    }
    if (resolver && cathook && !g_Settings.bInvalid &&
        stage == FRAME_NET_UPDATE_POSTDATAUPDATE_START)
    {
        PROF_SECTION(FSN_resolver);
        for (int i = 1; i < 32 && i < HIGHEST_ENTITY; i++)
        {
            if (i == g_IEngine->GetLocalPlayer())
                continue;
            ent = g_IEntityList->GetClientEntity(i);
            if (ent && !ent->IsDormant() && !NET_BYTE(ent, netvar.iLifeState))
            {
                Vector &angles = NET_VECTOR(ent, netvar.m_angEyeAngles);
                if (angles.x >= 90)
                    angles.x = -89;
                if (angles.x <= -90)
                    angles.x = 89;
                angles.y     = fmod(angles.y + 180.0f, 360.0f);
                if (angles.y < 0)
                    angles.y += 360.0f;
                angles.y -= 180.0f;
            }
        }
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
                    g_Settings.last_angles.x;
                CE_FLOAT(g_pLocalPlayer->entity, netvar.deadflag + 8) =
                    g_Settings.last_angles.y;
            }
        }
    }
    return original::FrameStageNotify(this_, stage);
}
}
