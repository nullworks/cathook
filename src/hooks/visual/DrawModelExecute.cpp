/*
  Created by Jenny White on 29.04.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#include <MiscTemporary.hpp>
#include <settings/Bool.hpp>
#include "HookedMethods.hpp"
#include "Backtrack.hpp"
#include <visual/EffectChams.hpp>
#include <visual/EffectGlow.hpp>
#include "AntiAim.hpp"

static settings::Boolean no_arms{ "remove.arms", "false" };
static settings::Boolean no_hats{ "remove.hats", "false" };

namespace effect_glow
{
extern settings::Boolean enable;
} // namespace effect_glow
namespace effect_chams
{
extern settings::Boolean enable;
} // namespace effect_chams
namespace hooked_methods
{
// Global scope so we can deconstruct on shutdown
static bool init_mat = false;
static CMaterialReference mat_dme_chams;
static InitRoutine init_dme([]() {
    EC::Register(
        EC::LevelShutdown,
        []() {
            if (init_mat)
            {
                mat_dme_chams.Shutdown();
                init_mat = false;
            }
        },
        "dme_lvl_shutdown");
});
bool aa_draw = false;
DEFINE_HOOKED_METHOD(DrawModelExecute, void, IVModelRender *this_, const DrawModelState_t &state, const ModelRenderInfo_t &info, matrix3x4_t *bone)
{
    if (!isHackActive())
        return original::DrawModelExecute(this_, state, info, bone);

    if (!(hacks::tf2::backtrack::isBacktrackEnabled /*|| (hacks::shared::antiaim::force_fakelag && hacks::shared::antiaim::isEnabled())*/ || spectator_target || no_arms || no_hats || (*clean_screenshots && g_IEngine->IsTakingScreenshot()) || CE_BAD(LOCAL_E) || !LOCAL_E->m_bAlivePlayer()))
    {
        return original::DrawModelExecute(this_, state, info, bone);
    }
    if (effect_glow::g_EffectGlow.drawing || effect_chams::g_EffectChams.drawing)
        return original::DrawModelExecute(this_, state, info, bone);

    PROF_SECTION(DrawModelExecute);

    if (no_arms || no_hats)
    {
        if (info.pModel)
        {
            const char *name = g_IModelInfo->GetModelName(info.pModel);
            if (name)
            {
                std::string sname = name;
                if (no_arms && sname.find("arms") != std::string::npos)
                {
                    return;
                }
                else if (no_hats && sname.find("player/items") != std::string::npos)
                {
                    return;
                }
            }
        }
    }

    // Used for fakes and for backtrack chams/glow
    if (!init_mat)
    {
        KeyValues *kv = new KeyValues("UnlitGeneric");
        kv->SetString("$basetexture", "vgui/white_additive");
        kv->SetInt("$ignorez", 0);
        mat_dme_chams.Init("__cathook_glow_unlit", kv);
        init_mat = true;
    }

    // Maybe one day i'll get this working
    /*if (aa_draw && info.entity_index == g_pLocalPlayer->entity_idx)
    {
        CMatRenderContextPtr ptr(GET_RENDER_CONTEXT);
        rgba_t mod_original;
        // Save color just in case, then set to white
        g_IVRenderView->GetColorModulation(mod_original.rgba);
        g_IVRenderView->SetColorModulation(colors::white);
        // Important for Depth
        ptr->DepthRange(0.0f, 1.0f);
        // Apply our material
        g_IVModelRender->ForcedMaterialOverride(mat_unlit);
        // Run Original
        original::DrawModelExecute(this_, state, info, bone);
        // Revert
        g_IVRenderView->SetColorModulation(mod_original.rgba);
        g_IVModelRender->ForcedMaterialOverride(nullptr);
        return;
    }
    if (hacks::shared::antiaim::force_fakelag && hacks::shared::antiaim::isEnabled() && info.entity_index == g_pLocalPlayer->entity_idx)
    {
        float fake     = hacks::shared::antiaim::used_yaw;
        Vector &angles = CE_VECTOR(LOCAL_E, netvar.m_angEyeAngles);
        float backup   = angles.y;
        angles.y       = fake;
        aa_draw        = true;
        RAW_ENT(LOCAL_E)->DrawModel(1);
        aa_draw  = false;
        angles.y = backup;
    }*/
    if (hacks::tf2::backtrack::chams && hacks::tf2::backtrack::isBacktrackEnabled)
    {
        const char *name = g_IModelInfo->GetModelName(info.pModel);
        if (name)
        {
            std::string sname = name;
            if (sname.find("models/player") || sname.find("models/weapons") || sname.find("models/workshop/player") || sname.find("models/workshop/weapons"))
            {
                if (IDX_GOOD(info.entity_index) && info.entity_index <= g_IEngine->GetMaxClients() && info.entity_index != g_IEngine->GetLocalPlayer())
                {
                    CachedEntity *ent = ENTITY(info.entity_index);
                    if (CE_GOOD(ent) && ent->m_bAlivePlayer())
                    {

                        // Get Backtrack data for target entity
                        auto good_ticks = hacks::tf2::backtrack::getGoodTicks(info.entity_index);

                        // Check if valid
                        if (!good_ticks.empty())
                        {
                            // Make our own Chamsish Material
                            // Render Chams/Glow stuff
                            CMatRenderContextPtr ptr(GET_RENDER_CONTEXT);
                            // Backup Blend
                            float orig_blend = g_IVRenderView->GetBlend();
                            // Make Backtrack stuff Use chams alpha
                            g_IVRenderView->SetBlend((*hacks::tf2::backtrack::chams_color).a);

                            rgba_t mod_original;
                            // Save color just in case, then set to team color
                            g_IVRenderView->GetColorModulation(mod_original.rgba);
                            g_IVRenderView->SetColorModulation(*hacks::tf2::backtrack::chams_color);
                            // Important for Depth
                            ptr->DepthRange(0.0f, 1.0f);
                            // Apply our material
                            if (hacks::tf2::backtrack::chams_solid)
                                g_IVModelRender->ForcedMaterialOverride(mat_dme_chams);

                            // Draw as many ticks as desired
                            for (unsigned i = 0; i <= (unsigned) std::max(*hacks::tf2::backtrack::chams_ticks, 1); i++)
                            {
                                // Can't draw more than we have
                                if (i >= good_ticks.size())
                                    break;
                                if (!good_ticks[i].bones.empty())
                                    original::DrawModelExecute(this_, state, info, &good_ticks[i].bones[0]);
                            }
                            // Revert
                            g_IVRenderView->SetColorModulation(mod_original.rgba);
                            g_IVModelRender->ForcedMaterialOverride(nullptr);
                            g_IVRenderView->SetBlend(orig_blend);
                        }
                    }
                }
            }
        }
    }
    IClientUnknown *unk = info.pRenderable->GetIClientUnknown();
    if (unk)
    {
        IClientEntity *ent = unk->GetIClientEntity();
        if (ent)
            if (ent->entindex() == spectator_target)
                return;
    }

    // Don't do it when we are trying to enforce backtrack chams
    if (!hacks::tf2::backtrack::isDrawing)
        return original::DrawModelExecute(this_, state, info, bone);
} // namespace hooked_methods
} // namespace hooked_methods
