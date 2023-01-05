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
static settings::Float nightmode_skybox{ "visual.night-mode.skybox", "0" };
static settings::Rgba nightmode_gui_color{ "visual.night-mode.gui-color", "000000FF" };
static settings::Rgba nightmode_world_color{ "visual.night-mode.world-color", "000000FF" };
static settings::Rgba nightmode_skybox_color{ "visual.night-mode.skybox-color", "000000FF" };
static settings::Boolean no_shake{ "visual.no-shake", "true" };
static settings::Boolean override_textures{ "visual.override-textures", "false" };
static settings::String override_textures_texture{ "visual.override-textures.custom-texture", "dev/dev_measuregeneric01b" };

// Should we update?
static bool update_nightmode         = false;
static bool update_override_textures = false;

// Which strings trigger this nightmode option
std::vector<std::string> world_strings         = { "World" };
std::vector<std::string> skybox_strings        = { "SkyBox" };
std::vector<std::string> gui_strings           = { "Other", "VGUI" };
std::vector<std::string> dont_override_strings = { "glass", "door", "water", "tools", "player", "wall28", "wall26" };
std::vector<std::string> nodraw_strings        = { "decal", "overlay", "hay" };

namespace hooked_methods
{
#include "reclasses.hpp"
DEFINE_HOOKED_METHOD(FrameStageNotify, void, void *this_, ClientFrameStage_t stage)
{
    if (!isHackActive())
        return original::FrameStageNotify(this_, stage);

    PROF_SECTION(FrameStageNotify_TOTAL);

    if (update_override_textures)
    {
        if (override_textures)
        {
            for (MaterialHandle_t i = g_IMaterialSystem->FirstMaterial(); i != g_IMaterialSystem->InvalidMaterial(); i = g_IMaterialSystem->NextMaterial(i))
            {
                IMaterial *pMaterial = g_IMaterialSystem->GetMaterial(i);
                if (!pMaterial)
                    continue;

                auto name = std::string(pMaterial->GetTextureGroupName());
                auto path = std::string(pMaterial->GetName());

                // Ensure world mat
                if (name.find("World") == std::string::npos)
                    continue;
                if (pMaterial->IsErrorMaterial() || !pMaterial->IsPrecached() || pMaterial->IsTranslucent() || pMaterial->IsSpriteCard())
                    continue;
                // Don't override this stuff
                bool good = true;
                for (auto const &entry : dont_override_strings)
                    if (path.find(entry) != path.npos)
                    {
                        good = false;
                    }
                // Don't draw this stuff
                for (auto const &entry : nodraw_strings)
                    if (path.find(entry) != path.npos)
                    {
                        pMaterial->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, true);
                        good = false;
                    }
                if (!good)
                    continue;

                if (!pMaterial->GetMaterialVarFlag(MATERIAL_VAR_NO_DRAW))
                {
                    auto *kv = new KeyValues("LightmappedGeneric" /*pMaterial->GetShaderName()*/);
                    kv->SetString("$basetexture", (*override_textures_texture).c_str());
                    // kv->SetString("$basetexturetransform", "center .5 .5 scale 6 6 rotate 0 translate 0 0");
                    kv->SetString("$surfaceprop", "concrete");
                    pMaterial->SetShaderAndParams(kv);
                }
            }
        }
        update_override_textures = false;
    }

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

            // 0 = do not filter, 1 = Gui filter, 2 = World filter, 3 = Skybox filter
            int should_filter = 0;
            auto name         = std::string(pMaterial->GetTextureGroupName());

            for (auto const &entry : gui_strings)
                if (name.find(entry) != name.npos)
                    should_filter = 1;

            for (auto const &entry : world_strings)
                if (name.find(entry) != name.npos)
                    should_filter = 2;

            for (auto const &entry : skybox_strings)
                if (name.find(entry) != name.npos)
                    should_filter = 3;

            if (should_filter)
            {
                if (should_filter == 1 && *nightmode_gui > 0.0f)
                {
                    // Map to PI/2 so we get full color scale
                    rgba_t draw_color = colors::Fade(colors::white, *nightmode_gui_color, (*nightmode_gui / 100.0f) * (PI / 2), 1.0f);

                    // Check for change
                    float r, g, b, a;
                    pMaterial->GetColorModulation(&r, &g, &b);
                    a = pMaterial->GetAlphaModulation();

                    if (r != draw_color.r || g != draw_color.g || b != draw_color.b)
                        pMaterial->ColorModulate(draw_color.r, draw_color.g, draw_color.b);
                    if (a != draw_color.a)
                        pMaterial->AlphaModulate((*nightmode_gui_color).a);
                }
                else if (should_filter == 2 && *nightmode_world > 0.0f)
                {
                    // Map to PI/2 so we get full color scale
                    rgba_t draw_color = colors::Fade(colors::white, *nightmode_world_color, (*nightmode_world / 100.0f) * (PI / 2), 1.0f);

                    // Check for change
                    float r, g, b, a;
                    pMaterial->GetColorModulation(&r, &g, &b);
                    a = pMaterial->GetAlphaModulation();
                    if (r != draw_color.r || g != draw_color.g || b != draw_color.b)
                        pMaterial->ColorModulate(draw_color.r, draw_color.g, draw_color.b);
                    if (a != draw_color.a)
                        pMaterial->AlphaModulate((*nightmode_world_color).a);
                }
                else if (should_filter == 3 && *nightmode_skybox > 0.0f)
                {
                    // Map to PI/2 so we get full color scale
                    rgba_t draw_color = colors::Fade(colors::white, *nightmode_skybox_color, (*nightmode_skybox / 100.0f) * (PI / 2), 1.0f);

                    // Check for change
                    float r, g, b, a;
                    pMaterial->GetColorModulation(&r, &g, &b);
                    a = pMaterial->GetAlphaModulation();
                    if (r != draw_color.r || g != draw_color.g || b != draw_color.b)
                        pMaterial->ColorModulate(draw_color.r, draw_color.g, draw_color.b);
                    if (a != draw_color.a)
                        pMaterial->AlphaModulate((*nightmode_skybox_color).a);
                }
                else
                {
                    float r, g, b, a;
                    pMaterial->GetColorModulation(&r, &g, &b);
                    a = pMaterial->GetAlphaModulation();
                    if (r != 1.0f || g != 1.0f || b != 1.0f)
                        pMaterial->ColorModulate(1.0f, 1.0f, 1.0f);
                    if (a != 1.0f)
                        pMaterial->AlphaModulate(1.0f);
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
    std::optional<Vector> backup_punch;
    if (isHackActive() && !g_Settings.bInvalid && stage == FRAME_RENDER_START)
    {
        IF_GAME(IsTF())
        {
            if (no_shake && CE_GOOD(LOCAL_E) && LOCAL_E->m_bAlivePlayer())
            {
                backup_punch                                       = NET_VECTOR(RAW_ENT(LOCAL_E), netvar.vecPunchAngle);
                NET_VECTOR(RAW_ENT(LOCAL_E), netvar.vecPunchAngle) = { 0.0f, 0.0f, 0.0f };
            }
        }
        hacks::tf::thirdperson::frameStageNotify();
    }
    original::FrameStageNotify(this_, stage);
    if (backup_punch)
        NET_VECTOR(RAW_ENT(LOCAL_E), netvar.vecPunchAngle) = *backup_punch;
}
template <typename T> void rvarCallback(settings::VariableBase<T> &, T)
{
    update_nightmode = true;
}
static InitRoutine init_fsn(
    []()
    {
        nightmode_gui.installChangeCallback(rvarCallback<float>);
        nightmode_world.installChangeCallback(rvarCallback<float>);
        nightmode_skybox.installChangeCallback(rvarCallback<float>);
        nightmode_gui_color.installChangeCallback(rvarCallback<rgba_t>);
        nightmode_world_color.installChangeCallback(rvarCallback<rgba_t>);
        nightmode_skybox_color.installChangeCallback(rvarCallback<rgba_t>);
        override_textures.installChangeCallback([](settings::VariableBase<bool> &, bool after) { update_override_textures = true; });
        override_textures_texture.installChangeCallback([](settings::VariableBase<std::string> &, std::string after) { update_override_textures = true; });
        EC::Register(
            EC::LevelInit,
            []()
            {
                update_nightmode         = true;
                update_override_textures = true;
            },
            "levelinit_fsn");
    });
} // namespace hooked_methods
