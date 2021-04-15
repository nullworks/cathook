/*
    This file is part of Cathook.

    Cathook is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Cathook is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Cathook. If not, see <https://www.gnu.org/licenses/>.
*/

// Codeowners: aUniqueUser

#include <PlayerTools.hpp>
#include "common.hpp"
#include "HookedMethods.hpp"
#include "MiscTemporary.hpp"
#include "Backtrack.hpp"
#include "EffectGlow.hpp"
#include "Aimbot.hpp"

/* World visual rvars */
static settings::Boolean no_arms{ "remove.arms", "false" };
static settings::Boolean no_hats{ "remove.hats", "false" };
static settings::Boolean blend_zoom{ "zoom.blend", "false" };

static settings::Boolean enable{ "chams.enable", "false" };
static settings::Boolean render_original{ "chams.original", "false" };

/* Cham target rvars */
static settings::Boolean health{ "chams.health", "false" };
static settings::Boolean aimbot_color{ "chams.aimbot.color", "true" };
static settings::Boolean teammates{ "chams.show.teammates", "false" };
static settings::Boolean disguised{ "chams.show.disguised", "true" };
static settings::Boolean players{ "chams.show.players", "true" };
static settings::Boolean medkits{ "chams.show.medkits", "false" };
static settings::Boolean ammobox{ "chams.show.ammoboxes", "false" };
static settings::Boolean buildings{ "chams.show.buildings", "true" };
static settings::Boolean stickies{ "chams.show.stickies", "true" };
static settings::Boolean stickies_local{ "chams.show.stickies.local", "true" };
static settings::Boolean pipes{ "chams.show.pipes", "true" };
static settings::Boolean pipes_local{ "chams.show.pipes.local", "true" };
static settings::Boolean teammate_buildings{ "chams.show.teammate-buildings", "false" };
static settings::Boolean recursive{ "chams.recursive", "true" };
static settings::Boolean legit{ "chams.legit", "false" };
static settings::Boolean singlepass{ "chams.single-pass", "false" };
static settings::Boolean chamsself{ "chams.self", "true" };
static settings::Boolean weapons{ "chams.weapons", "false" };
static settings::Rgba weapons_base{ "chams.weapons.basecolor", "ffffffff" };
static settings::Rgba weapons_overlay{ "chams.weapons.overlaycolor", "ffffffff" };
static settings::Float cham_alpha{ "chams.alpha", "1" };
static settings::Boolean overlay_chams{ "chams.overlay", "false" };

/* Lighting rvars */
static settings::Boolean phong_enable{ "chams.phong", "true" };
static settings::Boolean halfambert{ "chams.halfambert", "true" };
static settings::Boolean phong_fresnelrange{ "chams.phonefresnelranges", "true" };
static settings::Int phong_boost{ "chams.phongboost", "2" };
static settings::Int additive{ "chams.additive", "1" };
static settings::Int pearlescent{ "chams.pearlescent", "8" };

static settings::Float phong_exponent{ "chams.phongexponent", "25" };
static settings::Float phong_fresnelrange_1{ "chams.phongfresnelranges.1", "0" };
static settings::Float phong_fresnelrange_2{ "chams.phongfresnelranges.2", "3" };
static settings::Float phong_fresnelrange_3{ "chams.phongfresnelranges.3", "15" };

static settings::Boolean rimlighting{ "chams.rimlighting", "true" };
static settings::Float rimlighting_boost{ "chams.rimlighting.boost", "1" };
static settings::Float rimlighting_exponent{ "chams.rimlighting.exponent", "4" };

/* Customization of envmap */
static settings::Boolean envmap{ "chams.envmap", "true" };
static settings::Boolean envmap_matt{ "chams.envmap.matt", "false" };
static settings::Float envmapfresnel{ "chams.envmapfresnel", "1" };
static settings::Boolean envmap_tint{ "chams.envmap.tint", "true" };
static settings::Float envmap_tint_red_r{ "chams.envmap.tint.red.r", "4" };
static settings::Float envmap_tint_red_g{ "chams.envmap.tint.red.g", "0" };
static settings::Float envmap_tint_red_b{ "chams.envmap.tint.red.b", "2" };
static settings::Float envmap_tint_blu_r{ "chams.envmap.tint.blu.r", "0" };
static settings::Float envmap_tint_blu_g{ "chams.envmap.tint.blu.g", "2" };
static settings::Float envmap_tint_blu_b{ "chams.envmap.tint.blu.b", "4" };
static settings::Float envmap_tint_weapons_r{ "chams.envmap.tint.weapons.r", "1" };
static settings::Float envmap_tint_weapons_g{ "chams.envmap.tint.weapons.g", "1" };
static settings::Float envmap_tint_weapons_b{ "chams.envmap.tint.weapons.b", "1" };
static settings::Float envmap_tint_local_weapon_r{ "chams.envmap.tint.local-weapon.r", "1" };
static settings::Float envmap_tint_local_weapon_g{ "chams.envmap.tint.local-weapon.g", "1" };
static settings::Float envmap_tint_local_weapon_b{ "chams.envmap.tint.local-weapon.b", "1" };
static settings::Float envmap_tint_arms_r{ "chams.envmap.tint.arms.r", "1" };
static settings::Float envmap_tint_arms_g{ "chams.envmap.tint.arms.g", "1" };
static settings::Float envmap_tint_arms_b{ "chams.envmap.tint.arms.b", "1" };

/* Overlay chams team highlight colors */
static settings::Rgba chams_overlay_color_blu{ "chams.overlay.overlaycolor.blu", "000000ff" };
static settings::Rgba chams_overlay_color_red{ "chams.overlay.overlaycolor.red", "000000ff" };

/* Seperate cham settings when ignorez */
static settings::Boolean novis{ "chams.novis", "true" };
static settings::Rgba novis_team_red{ "chams.novis.red", "ff8800ff" };
static settings::Rgba novis_team_blu{ "chams.novis.blu", "bc00ffff" };

/* Customization of novis envmap */
static settings::Float envmap_tint_red_r_novis{ "chams.novis.envmap.tint.red.r", "4" };
static settings::Float envmap_tint_red_g_novis{ "chams.novis.envmap.tint.red.g", "4" };
static settings::Float envmap_tint_red_b_novis{ "chams.novis.envmap.tint.red.b", "1" };
static settings::Float envmap_tint_blu_r_novis{ "chams.novis.envmap.tint.blu.r", "4" };
static settings::Float envmap_tint_blu_g_novis{ "chams.novis.envmap.tint.blu.g", "1" };
static settings::Float envmap_tint_blu_b_novis{ "chams.novis.envmap.tint.blu.b", "4" };

/* Overlay chams novis team highlight colors */
static settings::Rgba chams_overlay_color_blu_novis{ "chams.novis.overlay.overlaycolor.blu", "ff00ffff" };
static settings::Rgba chams_overlay_color_red_novis{ "chams.novis.overlay.overlaycolor.red", "ff0000ff" };

/* Arm chams */
static settings::Boolean arms_chams{ "chams.arms", "false" };
static settings::Boolean arm_chams_original{ "chams.arms.original", "false" };
static settings::Boolean arms_chams_team_color{ "chams.arms.team-colors", "true" };
static settings::Boolean arms_chams_wireframe{ "chams.arms.wireframe", "false" };
static settings::Boolean arm_chams_overlay_chams{ "chams.overlay.arms", "true" };
static settings::Rgba arm_overlaychams_color{ "chams.arms.overlaycolor", "000000ff" };
static settings::Rgba arm_basechams_color{ "chams.arms.basecolor", "00000055" };

/* Local weapon chams */
static settings::Boolean local_weapon_chams{ "chams.local-weapon", "false" };
static settings::Boolean local_weapon_chams_original{ "chams.local-weapon.original", "false" };
static settings::Boolean local_weapon_chams_team_color{ "chams.local-weapon.team-colors", "true" };
static settings::Boolean local_weapon_chams_wireframe{ "chams.local-weapon.wireframe", "false" };
static settings::Boolean local_weapon_chams_overlay_chams{ "chams.overlay.local-weapon", "true" };
static settings::Rgba local_weapon_overlaychams_color{ "chams.local-weapon.overlaycolor", "000000ff" };
static settings::Rgba local_weapon_basechams_color{ "chams.local-weapon.basecolor", "000000ff" };

// Can we render arms/weapon chams right now? We need to draw on some player atleast once before it works without flat
// chams.
static bool should_draw_fp_chams = false;
static Timer should_draw_fp_chams_timer;

class Materials
{
public:
    CMaterialReference mat_dme_unlit;
    CMaterialReference mat_dme_lit;
    CMaterialReference mat_dme_unlit_overlay_base;
    CMaterialReference mat_dme_lit_overlay;

    // Sadly necessary ):
    CMaterialReference mat_dme_lit_fp;
    CMaterialReference mat_dme_unlit_overlay_base_fp;
    CMaterialReference mat_dme_lit_overlay_fp;

    void Shutdown()
    {
        mat_dme_unlit.Shutdown();
        mat_dme_lit.Shutdown();
        mat_dme_unlit_overlay_base.Shutdown();
        mat_dme_lit_overlay.Shutdown();

        mat_dme_lit_fp.Shutdown();
        mat_dme_unlit_overlay_base_fp.Shutdown();
        mat_dme_lit_overlay_fp.Shutdown();
    }
};

class ChamColors
{
public:
    float envmap_r, envmap_g, envmap_b;
    rgba_t rgba;

    rgba_t rgba_overlay = colors::empty;

    ChamColors(rgba_t col = colors::empty, float r = 1.0f, float g = 1.0f, float b = 1.0f)
    {
        rgba     = col;
        envmap_r = r;
        envmap_g = g;
        envmap_b = b;
    }
};

namespace hooked_methods
{
static bool init_mat = false;
static Materials mats;

template <typename T> void rvarCallback(settings::VariableBase<T> &, T)
{
    init_mat = false;
}

class DrawEntry
{
public:
    int entidx;
    int parentidx;
    DrawEntry()
    {
    }
    DrawEntry(int own_idx, int parent_idx)
    {
        entidx    = own_idx;
        parentidx = parent_idx;
    }
};

std::vector<DrawEntry> attachment_draw_list;

static InitRoutine init_dme([]() {
    EC::Register(
        EC::LevelShutdown,
        []() {
            if (init_mat)
            {
                mats.Shutdown();
                init_mat = false;
            }
            attachment_draw_list.clear();
        },
        "dme_lvl_shutdown");

    halfambert.installChangeCallback(rvarCallback<bool>);
    additive.installChangeCallback(rvarCallback<int>);
    pearlescent.installChangeCallback(rvarCallback<int>);

    phong_enable.installChangeCallback(rvarCallback<bool>);
    phong_boost.installChangeCallback(rvarCallback<int>);
    phong_exponent.installChangeCallback(rvarCallback<float>);
    phong_fresnelrange.installChangeCallback(rvarCallback<bool>);
    phong_fresnelrange_1.installChangeCallback(rvarCallback<float>);
    phong_fresnelrange_2.installChangeCallback(rvarCallback<float>);
    phong_fresnelrange_3.installChangeCallback(rvarCallback<float>);

    rimlighting.installChangeCallback(rvarCallback<bool>);
    rimlighting_boost.installChangeCallback(rvarCallback<float>);
    rimlighting_exponent.installChangeCallback(rvarCallback<float>);

    envmap.installChangeCallback(rvarCallback<bool>);
    envmapfresnel.installChangeCallback(rvarCallback<float>);
    envmap_tint.installChangeCallback(rvarCallback<bool>);
    envmap_matt.installChangeCallback(rvarCallback<bool>);
});

// Purpose => Returns true if we should render provided internal entity
bool ShouldRenderChams(IClientEntity *entity)
{
    if (CE_BAD(LOCAL_E))
        return false;
    if ((arms_chams || local_weapon_chams) && !should_draw_fp_chams)
    {
        should_draw_fp_chams = true;
        should_draw_fp_chams_timer.update();
        return true;
    }
    if (!enable)
        return false;
    if (entity->entindex() < 0)
        return false;
    CachedEntity *ent = ENTITY(entity->entindex());
    if (ent->m_IDX == LOCAL_E->m_IDX)
        return *chamsself;
    switch (ent->m_Type())
    {
    case ENTITY_BUILDING:
        if (!buildings)
            return false;
        if (!ent->m_bEnemy() && !(teammate_buildings || teammates))
            return false;
        if (ent->m_iHealth() == 0 || !ent->m_iHealth())
            return false;
        if (CE_BYTE(LOCAL_E, netvar.m_bCarryingObject) && ent->m_IDX == HandleToIDX(CE_INT(LOCAL_E, netvar.m_hCarriedObject)))
            return false;
        return true;
    case ENTITY_PLAYER:
        if (!players)
            return false;
        if (!disguised && IsPlayerDisguised(ent))
            return false;
        if (!teammates && !ent->m_bEnemy() && playerlist::IsDefault(ent))
            return false;
        if (CE_BYTE(ent, netvar.iLifeState))
            return false;
        return true;
    case ENTITY_PROJECTILE:
        if (ent->m_iClassID() == CL_CLASS(CTFGrenadePipebombProjectile))
            if (stickies || pipes)
            {
                if (CE_INT(ent, netvar.iPipeType) != 1)
                {
                    if (pipes)
                    {
                        if (pipes_local && chamsself)
                            if ((CE_INT(ent, netvar.hThrower) & 0xFFF) == g_pLocalPlayer->entity->m_IDX) // Check if the sticky is the players own
                                return true;
                        if (ent->m_bEnemy())
                            return true;
                    }
                    else
                        return false;
                }
                if (stickies_local && chamsself)
                    if ((CE_INT(ent, netvar.hThrower) & 0xFFF) == g_pLocalPlayer->entity->m_IDX) // Check if the sticky is the players own
                        return true;
                if (ent->m_bEnemy())
                    return true;
            }
        break;
    case ENTITY_GENERIC:
        switch (ent->m_ItemType())
        {
        case ITEM_HEALTH_LARGE:
        case ITEM_HEALTH_MEDIUM:
        case ITEM_HEALTH_SMALL:
            return *medkits;
        case ITEM_AMMO_LARGE:
        case ITEM_AMMO_MEDIUM:
        case ITEM_AMMO_SMALL:
            return *ammobox;
        default:
            break;
        }
        break;
    default:
        break;
    }
    return false;
}

// Purpose => Get ChamColors struct from internal entity
static ChamColors GetChamColors(IClientEntity *entity, bool ignorez)
{
    CachedEntity *ent = ENTITY(entity->entindex());

    if (CE_BAD(ent))
        return ChamColors(colors::white);
    if (ent == hacks::shared::aimbot::CurrentTarget() && aimbot_color)
        return ChamColors(colors::target);
    if (re::C_BaseCombatWeapon::IsBaseCombatWeapon(entity))
    {
        IClientEntity *owner = re::C_TFWeaponBase::GetOwnerViaInterface(entity);
        if (owner)
            return GetChamColors(owner, ignorez);
    }
    switch (ent->m_Type())
    {
    case ENTITY_BUILDING:
        if (!ent->m_bEnemy() && !(teammates || teammate_buildings) && ent != LOCAL_E)
            return ChamColors();
        if (health)
            return ChamColors(colors::Health_dimgreen(ent->m_iHealth(), ent->m_iMaxHealth()));
        break;
    case ENTITY_PLAYER:
        if (!players)
            return ChamColors();
        if (health)
            return ChamColors(colors::Health_dimgreen(ent->m_iHealth(), ent->m_iMaxHealth()));
        break;
    default:
        break;
    }
    if (ent->m_Type() == ENTITY_PLAYER || ent->m_Type() == ENTITY_BUILDING || ent->m_Type() == ENTITY_PROJECTILE)
    {
        ChamColors result;

        if (ent->m_iTeam() == TEAM_BLU)
            result = ChamColors(colors::blu, *envmap_tint_blu_r, *envmap_tint_blu_g, *envmap_tint_blu_b);
        else if (ent->m_iTeam() == TEAM_RED)
            result = ChamColors(colors::red, *envmap_tint_red_r, *envmap_tint_red_g, *envmap_tint_red_b);
        if (novis && ignorez)
        {
            if (ent->m_iTeam() == TEAM_BLU)
                result = ChamColors(*novis_team_blu, *envmap_tint_blu_r_novis, *envmap_tint_blu_g_novis, *envmap_tint_blu_b_novis);
            else if (ent->m_iTeam() == TEAM_RED)
                result = ChamColors(*novis_team_red, *envmap_tint_red_r_novis, *envmap_tint_red_g_novis, *envmap_tint_red_b_novis);
        }
        if (ent->m_Type() == ENTITY_PLAYER)
        {
            if (IsPlayerInvulnerable(ent))
            {
                if (ent->m_iTeam() == TEAM_BLU)
                    result = ChamColors(colors::blu_u, *envmap_tint_blu_r, *envmap_tint_blu_g, *envmap_tint_blu_b);
                else if (ent->m_iTeam() == TEAM_RED)
                    result = ChamColors(colors::red_u, *envmap_tint_red_r, *envmap_tint_red_g, *envmap_tint_red_b);
            }
            if (HasCondition<TFCond_UberBulletResist>(ent))
            {
                if (ent->m_iTeam() == TEAM_BLU)
                    result = ChamColors(colors::blu_v, *envmap_tint_blu_r, *envmap_tint_blu_g, *envmap_tint_blu_b);
                else if (ent->m_iTeam() == TEAM_RED)
                    result = ChamColors(colors::red_v, *envmap_tint_red_r, *envmap_tint_red_g, *envmap_tint_red_b);
            }
        }
        auto o = player_tools::forceEspColor(ent);
        if (o.has_value())
            result = ChamColors(*o);

        return result;
    }
    return ChamColors(colors::EntityF(ent));
}

// Purpose => Render entity attachments (weapons, hats)
void RenderAttachment(IClientEntity *entity, IClientEntity *attach, CMaterialReference &mat)
{
    if (attach->ShouldDraw())
    {
        attachment_draw_list.emplace_back(attach->entindex(), entity->entindex());
        if (entity->GetClientClass()->m_ClassID == RCC_PLAYER && re::C_BaseCombatWeapon::IsBaseCombatWeapon(attach))
        {
            // If separate weapon settings is used, apply them
            if (weapons)
            {
                // Backup original color
                rgba_t original;
                g_IVRenderView->GetColorModulation(original.rgba);
                g_IVRenderView->SetColorModulation(*weapons_base);

                // Setup material
                g_IVRenderView->SetBlend((*weapons_base).a);
                if (mat && envmap)
                    mat->FindVar("$envmaptint", nullptr)->SetVecValue(*envmap_tint_weapons_r, *envmap_tint_weapons_g, *envmap_tint_weapons_b);

                // Render
                attach->DrawModel(1);

                if (overlay_chams)
                {
                    // Setup material
                    g_IVRenderView->SetColorModulation(*weapons_overlay);
                    g_IVRenderView->SetBlend((*weapons_overlay).a);
                    if (mat && envmap)
                        mat->FindVar("$envmaptint", nullptr)->SetVecValue(*envmap_tint_weapons_r, *envmap_tint_weapons_g, *envmap_tint_weapons_b);

                    // Render
                    attach->DrawModel(1);
                }

                // Reset it!
                g_IVRenderView->SetColorModulation(original.rgba);
            }
            else
            {
                attach->DrawModel(1);
            }
        }
        else
            attach->DrawModel(1);
    }
}

// Locked from drawing
bool chams_attachment_drawing = false;

// Purpose => Render overriden model and and attachments
void RenderChamsRecursive(IClientEntity *entity, CMaterialReference &mat, IVModelRender *this_, const DrawModelState_t &state, const ModelRenderInfo_t &info, matrix3x4_t *bone)
{
#if !ENFORCE_STREAM_SAFETY
    if (!enable)
        return;
    original::DrawModelExecute(this_, state, info, bone);

    if (!*recursive)
        return;

    IClientEntity *attach;
    int passes = 0;

    attach = g_IEntityList->GetClientEntity(*(int *) ((uintptr_t) entity + netvar.m_Collision - 24) & 0xFFF);
    while (attach && passes++ < 32)
    {
        chams_attachment_drawing = true;
        RenderAttachment(entity, attach, mat);
        chams_attachment_drawing = false;
        attach                   = g_IEntityList->GetClientEntity(*(int *) ((uintptr_t) attach + netvar.m_Collision - 20) & 0xFFF);
    }
#endif
}

// Purpose => Apply and render chams according to settings
void ApplyChams(ChamColors colors, bool recurse, bool render_original, bool overlay, bool ignorez, bool wireframe, bool firstperson, IClientEntity *entity, IVModelRender *this_, const DrawModelState_t &state, const ModelRenderInfo_t &info, matrix3x4_t *bone)
{
    static auto &mat = firstperson ? overlay ? mats.mat_dme_unlit_overlay_base_fp : mats.mat_dme_lit_fp : overlay ? mats.mat_dme_unlit_overlay_base : mats.mat_dme_lit;
    if (render_original)
        recurse ? RenderChamsRecursive(entity, mat, this_, state, info, bone) : original::DrawModelExecute(this_, state, info, bone);

    // Setup material
    g_IVRenderView->SetColorModulation(colors.rgba);
    g_IVRenderView->SetBlend((colors.rgba).a);
    mat->AlphaModulate((colors.rgba).a);
    if (envmap && envmap_tint)
        mat->FindVar("$envmaptint", nullptr)->SetVecValue(colors.envmap_r, colors.envmap_g, colors.envmap_b);

    // Setup wireframe and ignorez using material vars
    mat->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, ignorez);
    mat->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, wireframe);

    // Override
    g_IVModelRender->ForcedMaterialOverride(mat);

    // Apply our new material
    recurse ? RenderChamsRecursive(entity, mat, this_, state, info, bone) : original::DrawModelExecute(this_, state, info, bone);
    if (overlay)
    {
        // Use white if no color was supplied
        if (colors.rgba_overlay == colors::empty && entity && IDX_GOOD(entity->entindex()))
        {
            CachedEntity *ent = ENTITY(entity->entindex());
            if (ent->m_Type() != ENTITY_PLAYER && ent->m_Type() != ENTITY_PROJECTILE && ent->m_Type() != ENTITY_BUILDING)
                colors.rgba_overlay = colors::white;
            else
                colors.rgba_overlay = ent->m_iTeam() == TEAM_RED ? *chams_overlay_color_red : ent->m_iTeam() == TEAM_BLU ? *chams_overlay_color_blu : colors::white;
        }
        // Setup material
        g_IVRenderView->SetColorModulation(colors.rgba_overlay);
        g_IVRenderView->SetBlend((colors.rgba_overlay).a);

        static auto &mat_overlay = mats.mat_dme_lit_overlay;
        if (envmap && envmap_tint)
            mat_overlay->FindVar("$envmaptint", nullptr)->SetVecValue(colors.envmap_r, colors.envmap_g, colors.envmap_b);

        mat_overlay->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, ignorez);
        mat_overlay->AlphaModulate((colors.rgba_overlay).a);

        // Override and apply
        g_IVModelRender->ForcedMaterialOverride(mat_overlay);
        recurse ? RenderChamsRecursive(entity, mat, this_, state, info, bone) : original::DrawModelExecute(this_, state, info, bone);
    }
}

DEFINE_HOOKED_METHOD(DrawModelExecute, void, IVModelRender *this_, const DrawModelState_t &state, const ModelRenderInfo_t &info, matrix3x4_t *bone)
{
    if (!isHackActive() || effect_glow::g_EffectGlow.drawing || chams_attachment_drawing || (*clean_screenshots && g_IEngine->IsTakingScreenshot()) || CE_BAD(LOCAL_E) || (!enable && !no_hats && !no_arms && !blend_zoom && !arms_chams && !local_weapon_chams /*&& !(hacks::tf2::backtrack::chams && hacks::tf2::backtrack::isBacktrackEnabled)*/))
        return original::DrawModelExecute(this_, state, info, bone);

    PROF_SECTION(DrawModelExecute);

    if (!init_mat)
    {
        const char *cubemap_str = *envmap_matt ? "effects/saxxy/saxxy_gold" : "env_cubemap";
        {
            auto *kv = new KeyValues("UnlitGeneric");
            kv->SetString("$basetexture", "white");
            mats.mat_dme_unlit.Init("__cathook_dme_chams_unlit", kv);
        }
        KeyValues *kv_vertex_lit = nullptr;
        {
            auto *kv = new KeyValues("VertexLitGeneric");
            kv->SetString("$basetexture", "white");
            kv->SetString("$bumpmap", "water/tfwater001_normal");
            kv->SetString("$lightwarptexture", "models/player/pyro/pyro_lightwarp");
            kv->SetBool("$halfambert", *halfambert);
            kv->SetBool("$phong", *phong_enable);
            kv->SetFloat("$phongexponent", *phong_exponent);
            kv->SetFloat("$phongboost", *phong_boost);
            if (phong_fresnelrange)
            {
                char buffer[100];
                snprintf(buffer, 100, "[%.2f %.2f %.2f]", *phong_fresnelrange_1, *phong_fresnelrange_2, *phong_fresnelrange_3);
                kv->SetString("$phongfresnelranges", buffer);
            }
            if (envmap)
            {
                kv->SetString("$envmap", cubemap_str);
                kv->SetFloat("$envmapfresnel", *envmapfresnel);
                kv->SetString("$envmapfresnelminmaxexp", "[0.01 1 2]");
                kv->SetInt("$normalmapalphaenvmapmask", 1);
                kv->SetInt("$selfillum", 1);
                if (envmap_tint)
                    kv->SetString("$envmaptint", "[1 1 1]");
            }
            kv->SetBool("$rimlight", *rimlighting);
            kv->SetFloat("$rimlightexponent", *rimlighting_exponent);
            kv->SetFloat("$rimlightboost", *phong_boost);
            kv_vertex_lit = kv->MakeCopy();
            mats.mat_dme_lit.Init("__cathook_dme_chams_lit", kv);
        }
        {
            auto *kv = new KeyValues("UnlitGeneric");
            kv->SetString("$basetexture", "white");
            mats.mat_dme_unlit_overlay_base.Init("__cathook_dme_chams_lit_overlay_base", kv);
        }
        {
            auto *kv = kv_vertex_lit;
            kv->SetInt("$additive", *additive);
            kv->SetInt("$pearlescent", *pearlescent);
            kv->SetBool("$flat", false);
            mats.mat_dme_lit_overlay.Init("__cathook_dme_chams_lit_overlay", kv);
        }
        KeyValues *kv_vertex_lit_fp = nullptr;
        {
            auto *kv = new KeyValues("VertexLitGeneric");
            kv->SetString("$basetexture", "white");
            kv->SetString("$bumpmap", "water/tfwater001_normal");
            kv->SetString("$lightwarptexture", "models/player/pyro/pyro_lightwarp");
            kv->SetBool("$halfambert", *halfambert);
            kv->SetBool("$phong", *phong_enable);
            kv->SetFloat("$phongexponent", *phong_exponent);
            kv->SetFloat("$phongboost", *phong_boost);
            if (phong_fresnelrange)
            {
                char buffer[100];
                snprintf(buffer, 100, "[%.2f %.2f %.2f]", *phong_fresnelrange_1, *phong_fresnelrange_2, *phong_fresnelrange_3);
                kv->SetString("$phongfresnelranges", buffer);
            }
            if (envmap)
            {
                kv->SetString("$envmap", cubemap_str);
                kv->SetFloat("$envmapfresnel", *envmapfresnel);
                kv->SetString("$envmapfresnelminmaxexp", "[0.01 1 2]");
                kv->SetInt("$normalmapalphaenvmapmask", 1);
                kv->SetInt("$selfillum", 1);
                if (envmap_tint)
                    kv->SetString("$envmaptint", "[1 1 1]");
            }
            kv->SetBool("$rimlight", *rimlighting);
            kv->SetFloat("$rimlightexponent", *rimlighting_exponent);
            kv->SetFloat("$rimlightboost", *phong_boost);
            kv_vertex_lit_fp = kv->MakeCopy();
            mats.mat_dme_lit_fp.Init("__cathook_dme_chams_lit_fp", kv);
        }
        {
            auto *kv = new KeyValues("UnlitGeneric");
            kv->SetString("$basetexture", "white");
            mats.mat_dme_unlit_overlay_base_fp.Init("__cathook_dme_chams_lit_overlay_base_fp", kv);
        }
        {
            auto *kv = kv_vertex_lit_fp;
            kv->SetInt("$additive", *additive);
            kv->SetInt("$pearlescent", *pearlescent);
            kv->SetBool("$flat", false);
            mats.mat_dme_lit_overlay_fp.Init("__cathook_dme_chams_lit_overlay_fp", kv);
        }
        init_mat = true;
    }

    if (info.pModel)
    {
        const char *name = g_IModelInfo->GetModelName(info.pModel);
        if (name)
        {
            std::string sname = name;
            if (should_draw_fp_chams && should_draw_fp_chams_timer.check(500) && ((sname.find("arms") != std::string::npos && sname.find("yeti") == std::string::npos) || sname.find("c_engineer_gunslinger") != std::string::npos))
            {
                if (no_arms)
                    return;

                if (arms_chams)
                {
                    // Backup original colors
                    rgba_t original_color;
                    g_IVRenderView->GetColorModulation(original_color);
                    original_color.a = g_IVRenderView->GetBlend();

                    // Setup according to user settings using the ChamColors struct
                    auto colors         = GetChamColors(LOCAL_E->InternalEntity(), false);
                    colors.rgba_overlay = LOCAL_E->m_iTeam() == TEAM_RED ? *chams_overlay_color_red : LOCAL_E->m_iTeam() == TEAM_BLU ? *chams_overlay_color_blu : colors::white;
                    if (!arms_chams_team_color)
                    {
                        colors              = *arm_basechams_color;
                        colors.rgba_overlay = *arm_overlaychams_color;
                        colors.envmap_r     = *envmap_tint_arms_r;
                        colors.envmap_g     = *envmap_tint_arms_g;
                        colors.envmap_b     = *envmap_tint_arms_b;
                    }
                    colors.rgba.a         = (*arm_basechams_color).a;
                    colors.rgba_overlay.a = (*arm_overlaychams_color).a;

                    // Apply arm chams
                    IClientEntity *entity = g_IEntityList->GetClientEntity(info.entity_index);
                    ApplyChams(colors, false, *arm_chams_original, *arm_chams_overlay_chams, false, *arms_chams_wireframe, true, entity, this_, state, info, bone);

                    // Reset it!
                    g_IVModelRender->ForcedMaterialOverride(nullptr);
                    g_IVRenderView->SetColorModulation(original_color);
                    g_IVRenderView->SetBlend(original_color.a);
                    return;
                }
            }
            // Workaround for attachments flickering
            std::vector<DrawEntry> tmp_list;
            bool do_draw = true;
            for (auto &drawer : attachment_draw_list)
            {
                if (drawer.entidx == info.entity_index)
                {
                    do_draw = false;
                }
                else
                    tmp_list.push_back(drawer);
            }
            attachment_draw_list = std::move(tmp_list);
            if (!do_draw)
                return;

            if (should_draw_fp_chams && should_draw_fp_chams_timer.check(500) && local_weapon_chams && info.entity_index == -1 && sname.find("arms") == std::string::npos && sname.find("arrow") == std::string::npos && (sname.find("models/weapons") != std::string::npos || sname.find("models/workshop/weapons") != std::string::npos || sname.find("models/workshop_partner/weapons") != std::string::npos))
            {
                // Backup original colors
                rgba_t original_color;
                g_IVRenderView->GetColorModulation(original_color);
                original_color.a = g_IVRenderView->GetBlend();

                auto colors           = GetChamColors(LOCAL_E->InternalEntity(), false);
                IClientEntity *entity = g_IEntityList->GetClientEntity(info.entity_index);

                // Setup according to user settings using the ChamColors struct
                if (local_weapon_chams_team_color)
                {
                    colors.rgba_overlay = LOCAL_E->m_iTeam() == TEAM_RED ? *chams_overlay_color_red : LOCAL_E->m_iTeam() == TEAM_BLU ? *chams_overlay_color_blu : colors::white;
                }
                else
                {
                    colors              = *local_weapon_basechams_color;
                    colors.rgba_overlay = *local_weapon_overlaychams_color;
                    colors.envmap_r     = *envmap_tint_local_weapon_r;
                    colors.envmap_g     = *envmap_tint_local_weapon_g;
                    colors.envmap_b     = *envmap_tint_local_weapon_b;
                }
                colors.rgba.a         = (*local_weapon_basechams_color).a;
                colors.rgba_overlay.a = (*local_weapon_overlaychams_color).a;

                // Apply local weapon chams
                ApplyChams(colors, false, *local_weapon_chams_original, *local_weapon_chams_overlay_chams, false, *local_weapon_chams_wireframe, true, entity, this_, state, info, bone);

                // Reset it!
                g_IVModelRender->ForcedMaterialOverride(nullptr);
                g_IVRenderView->SetColorModulation(original_color);
                g_IVRenderView->SetBlend(original_color.a);
                return;
            }

            if (no_hats && sname.find("player/items") != std::string::npos)
                return;

            // Player, entity and backtrack chams
            if (IDX_GOOD(info.entity_index))
            {
                // Get the internal entity from the index
                IClientEntity *entity = g_IEntityList->GetClientEntity(info.entity_index);
                if (ShouldRenderChams(entity))
                {
                    // Ensure a valid entity
                    CachedEntity *ent = ENTITY(info.entity_index);
                    if (CE_GOOD(ent))
                    {
                        // Get original to restore to later
                        rgba_t original_color;
                        g_IVRenderView->GetColorModulation(original_color);
                        original_color.a = g_IVRenderView->GetBlend();

                        // Player and entity chams
                        if (enable)
                        {
                            // First time has ignorez, 2nd time not
                            for (int i = 1; i >= 0; i--)
                            {
                                if (i && legit)
                                    continue;
                                if (!i && singlepass)
                                    continue;

                                // Setup colors
                                auto colors   = GetChamColors(entity, i);
                                colors.rgba.a = *cham_alpha;

                                // Apply chams according to user settings
                                ApplyChams(colors, *recursive, *render_original, *overlay_chams, i, false, false, entity, this_, state, info, bone);
                            }
                        }
                        // Backtrack chams
                        namespace bt = hacks::tf2::backtrack;
                        if (bt::chams && bt::backtrackEnabled())
                        {
                            // TODO: Allow for a fade between the entity's color and a specified color, it would look cool but i'm lazy
                            if (ent->m_bAlivePlayer() && (int) bt::bt_data.size() >= info.entity_index && info.entity_index > 0)
                            {
                                // Get ticks
                                auto ticks = bt::bt_data.at(info.entity_index - 1);

                                auto good_ticks = bt::getGoodTicks(ENTITY(info.entity_index));
                                if (good_ticks)
                                {
                                    // Setup chams according to user settings
                                    ChamColors backtrack_colors;
                                    backtrack_colors.rgba         = *bt::chams_color;
                                    backtrack_colors.rgba_overlay = *bt::chams_color_overlay;
                                    backtrack_colors.envmap_r     = *bt::chams_envmap_tint_r;
                                    backtrack_colors.envmap_g     = *bt::chams_envmap_tint_g;
                                    backtrack_colors.envmap_b     = *bt::chams_envmap_tint_b;

                                    for (unsigned i = 0; i <= (unsigned) std::max(*bt::chams_ticks, 1); i++)
                                    {
                                        // Can't draw more than we have
                                        if (i >= good_ticks->size())
                                            break;
                                        if (!(*good_ticks)[i].bones.empty())
                                            ApplyChams(backtrack_colors, false, false, *bt::chams_overlay, false, *bt::chams_wireframe, false, entity, this_, state, info, (*good_ticks)[i].bones.data());
                                    }
                                }
                            }
                        }

                        // Reset it!
                        g_IVModelRender->ForcedMaterialOverride(nullptr);
                        g_IVRenderView->SetColorModulation(original_color);
                        g_IVRenderView->SetBlend(original_color.a);
                        return;
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
    // if (!hacks::tf2::backtrack::isDrawing)
    return original::DrawModelExecute(this_, state, info, bone);
} // namespace hooked_methods
} // namespace hooked_methods
