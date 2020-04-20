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
namespace hacks::shared::backtrack
{
extern settings::Boolean backtrack_chams_glow;
}
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

    if (!(hacks::shared::backtrack::isBacktrackEnabled /*|| (hacks::shared::antiaim::force_fakelag && hacks::shared::antiaim::isEnabled())*/ || spectator_target || no_arms || no_hats || (*clean_screenshots && g_IEngine->IsTakingScreenshot()) || CE_BAD(LOCAL_E) || !LOCAL_E->m_bAlivePlayer()))
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
    if (hacks::shared::backtrack::isBacktrackEnabled && hacks::shared::backtrack::backtrack_chams_glow)
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
                        // Backup Blend
                        float orig_blend = g_IVRenderView->GetBlend();
                        // Make Backtrack stuff seethrough
                        g_IVRenderView->SetBlend(1.0f);
                        // Get Backtrack data for target entity
                        auto head_pos = hacks::shared::backtrack::headPositions[info.entity_index];
                        // Usable vector instead of ptr to c style array, also used to filter valid and invalid ticks
                        std::vector<hacks::shared::backtrack::BacktrackData> usable;
                        for (int i = 0; i < 66; i++)
                        {
                            if (hacks::shared::backtrack::ValidTick(head_pos[i], ent))
                                usable.push_back(head_pos[i]);
                        }
                        // Crash much?
                        if (usable.size())
                        {
                            // Sort
                            std::sort(usable.begin(), usable.end(), [](hacks::shared::backtrack::BacktrackData &a, hacks::shared::backtrack::BacktrackData &b) { return a.tickcount < b.tickcount; });
                            // Make our own Chamsish Material
                            // Render Chams/Glow stuff
                            CMatRenderContextPtr ptr(GET_RENDER_CONTEXT);
                            rgba_t mod_original;
                            // Save color just in case, then set to white
                            g_IVRenderView->GetColorModulation(mod_original.rgba);
                            g_IVRenderView->SetColorModulation(colors::white);
                            // Important for Depth
                            ptr->DepthRange(0.0f, 1.0f);
                            // Apply our material
                            g_IVModelRender->ForcedMaterialOverride(mat_dme_chams);
                            // Run Original
                            original::DrawModelExecute(this_, state, info, usable[0].bones);
                            // Revert
                            g_IVRenderView->SetColorModulation(mod_original.rgba);
                            g_IVModelRender->ForcedMaterialOverride(nullptr);
                        }
                        g_IVRenderView->SetBlend(orig_blend);
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

    return original::DrawModelExecute(this_, state, info, bone);
} // namespace hooked_methods
} // namespace hooked_methods
