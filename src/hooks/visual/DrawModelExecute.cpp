/*
  Created by Jenny White on 29.04.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

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

/* Cham target rvars */
static settings::Boolean flat{ "chams.flat", "false" };
static settings::Boolean health{ "chams.health", "false" };
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
static settings::Boolean arms_chams_team_color{ "chams.arms.team-colors", "true" };
static settings::Boolean arms_chams_wireframe{ "chams.arms.wireframe", "false" };
static settings::Boolean arm_chams_overlay_chams{ "chams.overlay.arms", "true" };
static settings::Rgba arm_overlaychams_color{ "chams.arms.overlaycolor", "000000ff" };
static settings::Rgba arm_basechams_color{ "chams.arms.basecolor", "00000055" };

/* Local weapon chams */
static settings::Boolean local_weapon_chams{ "chams.local-weapon", "false" };
static settings::Boolean local_weapon_chams_team_color{ "chams.local-weapon.team-colors", "true" };
static settings::Boolean local_weapon_chams_wireframe{ "chams.local-weapon.wireframe", "false" };
static settings::Boolean local_weapon_chams_overlay_chams{ "chams.overlay.local-weapon", "true" };
static settings::Rgba local_weapon_overlaychams_color{ "chams.local-weapon.overlaycolor", "000000ff" };
static settings::Rgba local_weapon_basechams_color{ "chams.local-weapon.basecolor", "000000ff" };

class Materials
{
public:
    CMaterialReference mat_dme_unlit;
    CMaterialReference mat_dme_lit;
    CMaterialReference mat_dme_unlit_overlay_base;
    CMaterialReference mat_dme_lit_overlay;

    void Shutdown()
    {
        mat_dme_unlit.Shutdown();
        mat_dme_lit.Shutdown();
        mat_dme_unlit_overlay_base.Shutdown();
        mat_dme_lit_overlay.Shutdown();
    }
};

class ChamColors
{
public:
    float envmap_r, envmap_g, envmap_b;
    rgba_t rgba;

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
static InitRoutine init_dme([]() {
    EC::Register(
        EC::LevelShutdown,
        []() {
            if (init_mat)
            {
                mats.Shutdown();
                init_mat = false;
            }
        },
        "dme_lvl_shutdown");

    halfambert.installChangeCallback([](settings::VariableBase<bool> &var, bool after) { init_mat = false; });
    additive.installChangeCallback([](settings::VariableBase<int> &var, int after) { init_mat = false; });
    pearlescent.installChangeCallback([](settings::VariableBase<int> &var, int after) { init_mat = false; });

    phong_enable.installChangeCallback([](settings::VariableBase<bool> &var, bool after) { init_mat = false; });
    phong_boost.installChangeCallback([](settings::VariableBase<bool> &var, int after) { init_mat = false; });
    phong_exponent.installChangeCallback([](settings::VariableBase<float> &var, float after) { init_mat = false; });
    phong_fresnelrange.installChangeCallback([](settings::VariableBase<bool> &var, bool after) { init_mat = false; });
    phong_fresnelrange_1.installChangeCallback([](settings::VariableBase<float> &var, float after) { init_mat = false; });
    phong_fresnelrange_2.installChangeCallback([](settings::VariableBase<float> &var, float after) { init_mat = false; });
    phong_fresnelrange_3.installChangeCallback([](settings::VariableBase<float> &var, float after) { init_mat = false; });

    rimlighting.installChangeCallback([](settings::VariableBase<bool> &var, bool after) { init_mat = false; });
    rimlighting_boost.installChangeCallback([](settings::VariableBase<float> &var, float after) { init_mat = false; });
    rimlighting_exponent.installChangeCallback([](settings::VariableBase<float> &var, float after) { init_mat = false; });

    envmap.installChangeCallback([](settings::VariableBase<bool> &var, bool after) { init_mat = false; });
    envmapfresnel.installChangeCallback([](settings::VariableBase<float> &var, float after) { init_mat = false; });
    envmap_tint.installChangeCallback([](settings::VariableBase<bool> &var, bool after) { init_mat = false; });
});

bool ShouldRenderChams(IClientEntity *entity)
{
    if (!enable || CE_BAD(LOCAL_E))
        return false;
    if (entity->entindex() < 0)
        return false;
    CachedEntity *ent = ENTITY(entity->entindex());
    if (!chamsself && ent->m_IDX == LOCAL_E->m_IDX)
        return false;
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

static ChamColors GetChamColors(IClientEntity *entity, bool ignorez)
{
    CachedEntity *ent = ENTITY(entity->entindex());

    if (CE_BAD(ent))
        return ChamColors(colors::white);
    if (ent == hacks::shared::aimbot::CurrentTarget())
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

void RenderChamsRecursive(IClientEntity *entity, IVModelRender *this_, const DrawModelState_t &state, const ModelRenderInfo_t &info, matrix3x4_t *bone)
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
        if (attach->ShouldDraw())
        {
            if (entity->GetClientClass()->m_ClassID == RCC_PLAYER && re::C_BaseCombatWeapon::IsBaseCombatWeapon(attach))
            {
                if (weapons)
                {
                    // TODO: Use the envmap_tint_weapons for the envmap tint, also fix the flickering

                    rgba_t mod_original;
                    g_IVRenderView->GetColorModulation(mod_original.rgba);
                    g_IVRenderView->SetColorModulation(*weapons_base);
                    g_IVRenderView->SetBlend((*weapons_base).a);
                    attach->DrawModel(1);

                    if (overlay_chams)
                    {
                        g_IVRenderView->SetColorModulation(*weapons_overlay);
                        g_IVRenderView->SetBlend((*weapons_overlay).a);
                        attach->DrawModel(1);
                    }

                    g_IVRenderView->SetColorModulation(mod_original.rgba);
                }
                else
                {
                    attach->DrawModel(1);
                }
            }
            else
                attach->DrawModel(1);
        }
        attach = g_IEntityList->GetClientEntity(*(int *) ((uintptr_t) attach + netvar.m_Collision - 20) & 0xFFF);
    }
#endif
}

DEFINE_HOOKED_METHOD(DrawModelExecute, void, IVModelRender *this_, const DrawModelState_t &state, const ModelRenderInfo_t &info, matrix3x4_t *bone)
{
    if (!isHackActive() || effect_glow::g_EffectGlow.drawing || (*clean_screenshots && g_IEngine->IsTakingScreenshot()) || CE_BAD(LOCAL_E) || (!enable && !no_hats && !no_arms && !blend_zoom && !arms_chams && !local_weapon_chams && !(hacks::tf2::backtrack::chams && hacks::tf2::backtrack::isBacktrackEnabled)))
        return original::DrawModelExecute(this_, state, info, bone);

    PROF_SECTION(DrawModelExecute);

    if (!init_mat)
    {
        {
            auto *kv = new KeyValues("UnlitGeneric");
            kv->SetString("$basetexture", "white");
            mats.mat_dme_unlit.Init("__cathook_dme_chams_unlit", kv);
        }
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
                kv->SetString("$envmap", "env_cubemap");
                kv->SetFloat("$envmapfresnel", *envmapfresnel);
                kv->SetInt("$normalmapalphaenvmapmask", 1);
                kv->SetInt("$selfillum", 1);
                if (envmap_tint)
                {
                    char buffer[100];
                    snprintf(buffer, 100, "[%.2f %.2f %.2f]", 1.0f, 1.0f, 1.0f);
                    kv->SetString("$envmaptint", buffer);
                }
            }
            kv->SetBool("$rimlight", *rimlighting);
            kv->SetFloat("$rimlightexponent", *rimlighting_exponent);
            kv->SetFloat("$rimlightboost", *phong_boost);
            mats.mat_dme_lit.Init("__cathook_dme_chams_lit", kv);
        }
        {
            auto *kv = new KeyValues("UnlitGeneric");
            kv->SetString("$basetexture", "white");
            kv->SetBool("$flat", true);
            mats.mat_dme_unlit_overlay_base.Init("__cathook_dme_chams_lit_overlay_base", kv);
        }
        {
            auto *kv = new KeyValues("VertexLitGeneric");
            kv->SetString("$basetexture", "white");
            kv->SetString("$bumpmap", "water/tfwater001_normal");
            kv->SetString("$lightwarptexture", "models/player/pyro/pyro_lightwarp");
            kv->SetBool("$phong", *phong_enable);
            kv->SetFloat("$phongexponent", *phong_exponent);
            kv->SetFloat("$phongboost", *phong_boost);
            if (phong_fresnelrange)
            {
                char buffer[100];
                snprintf(buffer, 100, "[%.2f %.2f %.2f]", *phong_fresnelrange_1, *phong_fresnelrange_2, *phong_fresnelrange_3);
                kv->SetString("$phongfresnelranges", buffer);
            }
            kv->SetBool("$halflambert", *halfambert);
            kv->SetBool("$rimlight", *rimlighting);
            kv->SetFloat("$rimlightboost", *rimlighting_boost);
            kv->SetFloat("$rimlightexponent", *rimlighting_exponent);
            kv->SetInt("$additive", *additive);
            kv->SetInt("$pearlescent", *pearlescent);
            kv->SetBool("$flat", false);
            if (envmap)
            {
                kv->SetString("$envmap", "env_cubemap");
                kv->SetFloat("$envmapfresnel", *envmapfresnel);
                kv->SetInt("$normalmapalphaenvmapmask", 1);
                kv->SetInt("$selfillum", 1);
                if (envmap_tint)
                {
                    char buffer[100];
                    snprintf(buffer, 100, "[%.2f %.2f %.2f]", 1.0f, 1.0f, 1.0f);
                    kv->SetString("$envmaptint", buffer);
                }
            }
            mats.mat_dme_lit_overlay.Init("__cathook_dme_chams_lit_overlay", kv);
        }
        init_mat = true;
    }

    if (info.pModel)
    {
        const char *name = g_IModelInfo->GetModelName(info.pModel);
        if (name)
        {
            std::string sname = name;
            if ((sname.find("arms") != std::string::npos && sname.find("yeti") == std::string::npos) || sname.find("c_engineer_gunslinger") != std::string::npos)
            {
                if (no_arms)
                    return;

                if (arms_chams)
                {
                    rgba_t original_color;
                    g_IVRenderView->GetColorModulation(original_color);
                    original_color.a = g_IVRenderView->GetBlend();

                    static auto &mat = arm_chams_overlay_chams ? mats.mat_dme_unlit_overlay_base : mats.mat_dme_lit;
                    auto colors      = GetChamColors(LOCAL_E->InternalEntity(), false);

                    g_IVRenderView->SetColorModulation(arms_chams_team_color ? colors.rgba : *arm_basechams_color);
                    g_IVRenderView->SetBlend((*arm_basechams_color).a);
                    if (envmap && envmap_tint)
                    {
                        if (arms_chams_team_color)
                            mat->FindVar("$envmaptint", nullptr)->SetVecValue(colors.envmap_r, colors.envmap_g, colors.envmap_b);
                        else
                            mat->FindVar("$envmaptint", nullptr)->SetVecValue(*envmap_tint_arms_r, *envmap_tint_arms_g, *envmap_tint_arms_b);
                    }
                    mat->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
                    mat->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, *arms_chams_wireframe);
                    g_IVModelRender->ForcedMaterialOverride(mat);
                    original::DrawModelExecute(this_, state, info, bone);
                    if (arm_chams_overlay_chams)
                    {
                        if (arms_chams_team_color)
                        {
                            g_IVRenderView->SetColorModulation(LOCAL_E->m_iTeam() == TEAM_RED ? *chams_overlay_color_red : LOCAL_E->m_iTeam() == TEAM_BLU ? *chams_overlay_color_blu : colors::white);
                            g_IVRenderView->SetBlend((*arm_overlaychams_color).a);
                        }
                        else
                        {
                            g_IVRenderView->SetColorModulation(*arm_overlaychams_color);
                            g_IVRenderView->SetBlend((*arm_overlaychams_color).a);
                        }
                        static auto &mat_overlay = mats.mat_dme_lit_overlay;
                        mat_overlay->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
                        if (envmap && envmap_tint)
                        {
                            if (arms_chams_team_color)
                                mat_overlay->FindVar("$envmaptint", nullptr)->SetVecValue(colors.envmap_r, colors.envmap_g, colors.envmap_b);
                            else
                                mat_overlay->FindVar("$envmaptint", nullptr)->SetVecValue(*envmap_tint_arms_r, *envmap_tint_arms_g, *envmap_tint_arms_b);
                        }
                        g_IVModelRender->ForcedMaterialOverride(mat_overlay);
                        original::DrawModelExecute(this_, state, info, bone);
                    }
                    g_IVModelRender->ForcedMaterialOverride(nullptr);
                    g_IVRenderView->SetColorModulation(original_color);
                    g_IVRenderView->SetBlend(original_color.a);
                    return;
                }
            }
            if (local_weapon_chams && info.entity_index == -1 && sname.find("arms") == std::string::npos && (sname.find("models/weapons") != std::string::npos || sname.find("models/workshop/weapons") != std::string::npos || sname.find("models/workshop_partner/weapons") != std::string::npos))
            {
                rgba_t original_color;
                g_IVRenderView->GetColorModulation(original_color);
                original_color.a = g_IVRenderView->GetBlend();

                static auto &mat = local_weapon_chams_overlay_chams ? mats.mat_dme_unlit_overlay_base : mats.mat_dme_lit;
                auto colors      = GetChamColors(LOCAL_E->InternalEntity(), false);

                g_IVRenderView->SetColorModulation(local_weapon_chams_team_color ? colors.rgba : *local_weapon_basechams_color);
                g_IVRenderView->SetBlend((*local_weapon_basechams_color).a);
                if (envmap && envmap_tint)
                {
                    if (local_weapon_chams_team_color)
                        mat->FindVar("$envmaptint", nullptr)->SetVecValue(colors.envmap_r, colors.envmap_g, colors.envmap_b);
                    else
                        mat->FindVar("$envmaptint", nullptr)->SetVecValue(*envmap_tint_local_weapon_r, *envmap_tint_local_weapon_g, *envmap_tint_local_weapon_b);
                }
                mat->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
                mat->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, *local_weapon_chams_wireframe);
                g_IVModelRender->ForcedMaterialOverride(mat);
                original::DrawModelExecute(this_, state, info, bone);
                if (local_weapon_chams_overlay_chams)
                {
                    if (local_weapon_chams_team_color)
                    {
                        g_IVRenderView->SetColorModulation(LOCAL_E->m_iTeam() == TEAM_RED ? *chams_overlay_color_red : LOCAL_E->m_iTeam() == TEAM_BLU ? *chams_overlay_color_blu : colors::white);
                        g_IVRenderView->SetBlend((*local_weapon_overlaychams_color).a);
                    }
                    else
                    {
                        g_IVRenderView->SetColorModulation(*local_weapon_overlaychams_color);
                        g_IVRenderView->SetBlend((*local_weapon_overlaychams_color).a);
                    }
                    static auto &mat_overlay = mats.mat_dme_lit_overlay;
                    mat_overlay->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
                    g_IVModelRender->ForcedMaterialOverride(mat_overlay);
                    if (envmap && envmap_tint)
                    {
                        if (local_weapon_chams_team_color)
                            mat_overlay->FindVar("$envmaptint", nullptr)->SetVecValue(colors.envmap_r, colors.envmap_g, colors.envmap_b);
                        else
                            mat_overlay->FindVar("$envmaptint", nullptr)->SetVecValue(*envmap_tint_local_weapon_r, *envmap_tint_local_weapon_g, *envmap_tint_local_weapon_b);
                    }
                    original::DrawModelExecute(this_, state, info, bone);
                }
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
                IClientEntity *entity = g_IEntityList->GetClientEntity(info.entity_index);
                if (ShouldRenderChams(entity))
                {
                    CachedEntity *ent = ENTITY(info.entity_index);
                    if (CE_GOOD(ent))
                    {
                        rgba_t original_color;
                        g_IVRenderView->GetColorModulation(original_color);
                        original_color.a = g_IVRenderView->GetBlend();
                        static auto &mat = overlay_chams ? mats.mat_dme_unlit_overlay_base : mats.mat_dme_lit;
                        if (!legit)
                        {
                            auto colors = GetChamColors(entity, true);
                            g_IVRenderView->SetColorModulation(colors.rgba);
                            g_IVRenderView->SetBlend(*cham_alpha);
                            if (envmap && envmap_tint)
                                mat->FindVar("$envmaptint", nullptr)->SetVecValue(colors.envmap_r, colors.envmap_g, colors.envmap_b);

                            mat->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
                            mat->SetMaterialVarFlag(MATERIAL_VAR_FLAT, *flat);
                            g_IVModelRender->ForcedMaterialOverride(mat);
                            RenderChamsRecursive(entity, this_, state, info, bone);
                            if (overlay_chams)
                            {
                                if (ent->m_Type() != ENTITY_PLAYER && ent->m_Type() != ENTITY_PROJECTILE && ent->m_Type() != ENTITY_BUILDING)
                                {
                                    g_IVRenderView->SetColorModulation(colors::white);
                                    g_IVRenderView->SetBlend((colors::white).a);
                                }
                                else
                                {
                                    g_IVRenderView->SetColorModulation(ent->m_iTeam() == TEAM_RED ? *chams_overlay_color_red : ent->m_iTeam() == TEAM_BLU ? *chams_overlay_color_blu : colors::white);
                                    g_IVRenderView->SetBlend((ent->m_iTeam() == TEAM_RED ? *chams_overlay_color_red : ent->m_iTeam() == TEAM_BLU ? *chams_overlay_color_blu : colors::white).a);
                                }
                                static auto &mat_overlay = mats.mat_dme_lit_overlay;
                                mat->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);

                                g_IVModelRender->ForcedMaterialOverride(mat_overlay);
                                RenderChamsRecursive(entity, this_, state, info, bone);
                            }
                        }
                        if (legit || !singlepass)
                        {
                            auto colors = GetChamColors(entity, false);
                            g_IVRenderView->SetColorModulation(colors.rgba);
                            g_IVRenderView->SetBlend(*cham_alpha);
                            if (envmap && envmap_tint)
                                mat->FindVar("$envmaptint", nullptr)->SetVecValue(colors.envmap_r, colors.envmap_g, colors.envmap_b);

                            mat->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
                            mat->SetMaterialVarFlag(MATERIAL_VAR_FLAT, *flat);
                            g_IVModelRender->ForcedMaterialOverride(mat);
                            RenderChamsRecursive(entity, this_, state, info, bone);
                            if (overlay_chams)
                            {
                                if (ent->m_Type() != ENTITY_PLAYER && ent->m_Type() != ENTITY_PROJECTILE && ent->m_Type() != ENTITY_BUILDING)
                                {
                                    g_IVRenderView->SetColorModulation(colors::white);
                                    g_IVRenderView->SetBlend((colors::white).a);
                                }
                                else
                                {
                                    g_IVRenderView->SetColorModulation(ent->m_iTeam() == TEAM_RED ? *chams_overlay_color_red : ent->m_iTeam() == TEAM_BLU ? *chams_overlay_color_blu : colors::white);
                                    g_IVRenderView->SetBlend((ent->m_iTeam() == TEAM_RED ? *chams_overlay_color_red : ent->m_iTeam() == TEAM_BLU ? *chams_overlay_color_blu : colors::white).a);
                                }
                                static auto &mat_overlay = mats.mat_dme_lit_overlay;
                                mat_overlay->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
                                g_IVModelRender->ForcedMaterialOverride(mat_overlay);
                                RenderChamsRecursive(entity, this_, state, info, bone);
                            }
                        }
                        // Backtrack chams
                        using namespace hacks::tf2;
                        if (backtrack::chams && backtrack::isBacktrackEnabled)
                        {
                            if (ent->m_bAlivePlayer())
                            {
                                auto good_ticks = backtrack::getGoodTicks(info.entity_index);
                                if (!good_ticks.empty())
                                {
                                    rgba_t original_color;
                                    g_IVRenderView->GetColorModulation(original_color);
                                    original_color.a = g_IVRenderView->GetBlend();
                                    static auto &mat = backtrack::chams_overlay ? mats.mat_dme_unlit_overlay_base : mats.mat_dme_lit;

                                    mat->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, *backtrack::chams_wireframe);
                                    mat->SetMaterialVarFlag(MATERIAL_VAR_FLAT, *backtrack::chams_flat);
                                    mat->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
                                    if (envmap && envmap_tint)
                                        mats.mat_dme_lit_overlay->FindVar("$envmaptint", nullptr)->SetVecValue(*backtrack::chams_envmap_tint_r, *backtrack::chams_envmap_tint_g, *backtrack::chams_envmap_tint_b);

                                    g_IVModelRender->ForcedMaterialOverride(mat);
                                    g_IVRenderView->SetColorModulation(*backtrack::chams_color);
                                    g_IVRenderView->SetBlend((*backtrack::chams_color).a);
                                    for (unsigned i = 0; i <= (unsigned) std::max(*backtrack::chams_ticks, 1); i++)
                                    {
                                        // Can't draw more than we have
                                        if (i >= good_ticks.size())
                                            break;
                                        if (!good_ticks[i].bones.empty())
                                            RenderChamsRecursive(entity, this_, state, info, &good_ticks[i].bones[0]);
                                    }

                                    if (backtrack::chams_overlay)
                                    {
                                        mats.mat_dme_lit_overlay->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
                                        if (envmap && envmap_tint)
                                            mats.mat_dme_lit_overlay->FindVar("$envmaptint", nullptr)->SetVecValue(*backtrack::chams_envmap_tint_r, *backtrack::chams_envmap_tint_g, *backtrack::chams_envmap_tint_b);

                                        g_IVRenderView->SetColorModulation(*backtrack::chams_color_overlay);
                                        g_IVRenderView->SetBlend((*backtrack::chams_color_overlay).a);
                                        g_IVModelRender->ForcedMaterialOverride(mats.mat_dme_lit_overlay);
                                        for (unsigned i = 0; i <= (unsigned) std::max(*backtrack::chams_ticks, 1); i++)
                                        {
                                            // Can't draw more than we have
                                            if (i >= good_ticks.size())
                                                break;
                                            if (!good_ticks[i].bones.empty())
                                                RenderChamsRecursive(entity, this_, state, info, &good_ticks[i].bones[0]);
                                        }
                                    }
                                }
                            }
                        }
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
    if (!hacks::tf2::backtrack::isDrawing)
        return original::DrawModelExecute(this_, state, info, bone);
} // namespace hooked_methods
} // namespace hooked_methods
