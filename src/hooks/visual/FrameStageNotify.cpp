/*
  Created by Jenny White on 29.04.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#include <MiscTemporary.hpp>
#include <hacks/hacklist.hpp>
#include <settings/Bool.hpp>
#include <hacks/Thirdperson.hpp>
#include "HookedMethods.hpp"
#if not LAGBOT_MODE
#include "hacks/Backtrack.hpp"
#endif

static settings::Float nightmode{ "visual.night-mode", "0" };

static float old_nightmode{ 0.0f };

namespace hooked_methods
{
#include "reclasses.hpp"
DEFINE_HOOKED_METHOD(FrameStageNotify, void, void *this_,
                     ClientFrameStage_t stage)
{
    if (!isHackActive())
        return original::FrameStageNotify(this_, stage);

    PROF_SECTION(FrameStageNotify_TOTAL);

    if (old_nightmode != *nightmode)
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
        old_nightmode = *nightmode;
    }

    if (!g_IEngine->IsInGame())
        g_Settings.bInvalid = true;
    {
        PROF_SECTION(FSN_skinchanger);
        hacks::tf2::skinchanger::FrameStageNotify(stage);
    }
    if (isHackActive() && stage == FRAME_RENDER_START)
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
        IF_GAME(IsTF())
        {
            if (no_zoom && CE_GOOD(LOCAL_E))
                RemoveCondition<TFCond_Zoomed>(LOCAL_E);
        }
        hacks::tf::thirdperson::frameStageNotify();
    }
    return original::FrameStageNotify(this_, stage);
}
} // namespace hooked_methods
