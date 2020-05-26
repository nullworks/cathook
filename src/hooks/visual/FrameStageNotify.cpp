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

static settings::Float nightmode_gui{ "visual.night-mode.gui", "0" };
static settings::Float nightmode_world{ "visual.night-mode.world", "0" };
static settings::Rgba nightmode_gui_color{ "visual.night-mode.gui-color", "000000FF" };
static settings::Rgba nightmode_world_color{ "visual.night-mode.world-color", "000000FF" };
static settings::Boolean no_shake{ "visual.no-shake", "true" };

// Should we update nightmode?
static bool update_nightmode = false;

// Which strings trigger this nightmode option
std::vector<std::string> world_strings = { "SkyBox", "World" };
std::vector<std::string> gui_strings   = { "Other", "VGUI" };

namespace hooked_methods
{
#include "reclasses.hpp"
DEFINE_HOOKED_METHOD(FrameStageNotify, void, void *this_, ClientFrameStage_t stage)
{
    if (!isHackActive())
        return original::FrameStageNotify(this_, stage);

    PROF_SECTION(FrameStageNotify_TOTAL);

    if (update_nightmode)
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

            // 0 = do not filter, 1 = Gui filter, 2 = World filter
            int should_filter = 0;
            auto name         = std::string(pMaterial->GetTextureGroupName());

            for (auto &entry : gui_strings)
                if (name.find(entry) != name.npos)
                    should_filter = 1;

            for (auto &entry : world_strings)
                if (name.find(entry) != name.npos)
                    should_filter = 2;

            if (should_filter)
            {

                if (should_filter == 1 && *nightmode_gui > 0.0f)
                {
                    // Map to PI/2 so we get full color scale
                    rgba_t draw_color = colors::Fade(colors::white, *nightmode_gui_color, (*nightmode_gui / 100.0f) * (PI / 2), 1.0f);

                    pMaterial->ColorModulate(draw_color.r, draw_color.g, draw_color.b);
                    pMaterial->AlphaModulate((*nightmode_gui_color).a);
                }
                else if (should_filter == 2 && *nightmode_world > 0.0f)
                {
                    // Map to PI/2 so we get full color scale
                    rgba_t draw_color = colors::Fade(colors::white, *nightmode_world_color, (*nightmode_world / 100.0f) * (PI / 2), 1.0f);

                    pMaterial->ColorModulate(draw_color.r, draw_color.g, draw_color.b);
                    pMaterial->AlphaModulate((*nightmode_world_color).a);
                }
                else
                {
                    pMaterial->ColorModulate(1.0f, 1.0f, 1.0f);
                }
            }
        }
        update_nightmode = false;
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
}
template <typename T> void rvarCallback(settings::VariableBase<T> &, T)
{
    update_nightmode = true;
}
static InitRoutine init_fsn([]() {
    nightmode_gui.installChangeCallback(rvarCallback<float>);
    nightmode_world.installChangeCallback(rvarCallback<float>);
    nightmode_gui_color.installChangeCallback(rvarCallback<rgba_t>);
    nightmode_world_color.installChangeCallback(rvarCallback<rgba_t>);
});
} // namespace hooked_methods
