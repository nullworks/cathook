/*
 * EffectChams.cpp
 *
 *  Created on: Apr 16, 2017
 *      Author: nullifiedcat
 */

#include <visual/EffectChams.hpp>
#include <MiscTemporary.hpp>
#include "common.hpp"

// static CatVar chams_experimental(CV_SWITCH, XORSTR("chams_effect"), XORSTR("0"),
// XORSTR("Experimental Chams"));

namespace effect_chams
{

static CatVar enable(CV_SWITCH, XORSTR("chams_enable"), XORSTR("0"), XORSTR("Enable"),
                     XORSTR("Main chams switch"));
static CatVar flat(CV_SWITCH, XORSTR("chams_flat"), XORSTR("0"), XORSTR("Flat"),
                   XORSTR("Makes chams brighter and more full"));
static CatVar health(CV_SWITCH, XORSTR("chams_health"), XORSTR("0"), XORSTR("Health"),
                     XORSTR("Change chams color based on their health"));
static CatVar teammates(CV_SWITCH, XORSTR("chams_teammates"), XORSTR("0"), XORSTR("Teammates"),
                        XORSTR("Render chams on teammates"));
static CatVar players(CV_SWITCH, XORSTR("chams_players"), XORSTR("1"), XORSTR("Players"),
                      XORSTR("Render chams on player models"));
static CatVar medkits(CV_SWITCH, XORSTR("chams_medkits"), XORSTR("0"), XORSTR("Medkits"),
                      XORSTR("Render chams on medkits"));
static CatVar ammobox(CV_SWITCH, XORSTR("chams_ammo"), XORSTR("0"), XORSTR("Ammoboxes"),
                      XORSTR("Render chams on ammoboxes"));
static CatVar buildings(CV_SWITCH, XORSTR("chams_buildings"), XORSTR("0"), XORSTR("Buildings"),
                        XORSTR("Render chams on buildings"));
static CatVar stickies(CV_SWITCH, XORSTR("chams_stickies"), XORSTR("0"), XORSTR("Stickies"),
                       XORSTR("Render chams on stickybombs"));
static CatVar teammate_buildings(CV_SWITCH, XORSTR("chams_teammate_buildings"), XORSTR("0"),
                                 XORSTR("Teammate Buildings"),
                                 XORSTR("Render chams on teammates buildings"));
static CatVar recursive(CV_SWITCH, XORSTR("chams_recursive"), XORSTR("1"), XORSTR("Recursive"),
                        XORSTR("Render chams on weapons and cosmetics"));
static CatVar weapons_white(CV_SWITCH, XORSTR("chams_weapons_white"), XORSTR("1"),
                            XORSTR("White Weapons"),
                            XORSTR("Should chams on weapons be white"));
static CatVar legit(CV_SWITCH, XORSTR("chams_legit"), XORSTR("0"), XORSTR("Legit chams"),
                    XORSTR("Don't show chams through walls"));
static CatVar singlepass(CV_SWITCH, XORSTR("chams_singlepass"), XORSTR("0"), XORSTR("Single-pass"),
                         XORSTR("Render chams only once (this disables 'darker' chams ")
                         XORSTR("on invisible parts of player"));
static CatVar chamsself(CV_SWITCH, XORSTR("chams_self"), XORSTR("0"), XORSTR("Enable chams on self"),
                        XORSTR(""));
static CatVar rainbow(CV_SWITCH, XORSTR("chams_self_rainbow"), XORSTR("1"),
                      XORSTR("Enable rainbow chams on self"),
                      XORSTR("Only visible in thirdperson!"));
static CatVar chamsteam(CV_SWITCH, XORSTR("chams_self_team"), XORSTR("0"), XORSTR("Team chams color"));
static CatVar chamsR(CV_INT, XORSTR("chams_self_r"), XORSTR("0"), XORSTR("Self chams red"), XORSTR(""), 0, 255);
static CatVar chamsG(CV_INT, XORSTR("chams_self_g"), XORSTR("0"), XORSTR("Self chams green"), XORSTR(""), 0,
                     255);
static CatVar chamsB(CV_INT, XORSTR("chams_self_b"), XORSTR("0"), XORSTR("Self chams blue"), XORSTR(""), 0,
                     255);
void EffectChams::Init()
{
    logging::Info(XORSTR("Init EffectChams..."));
    {
        KeyValues *kv = new KeyValues(XORSTR("UnlitGeneric"));
        kv->SetString(XORSTR("$basetexture"), XORSTR("vgui/white_additive"));
        kv->SetInt(XORSTR("$ignorez"), 0);
        mat_unlit.Init(XORSTR("__cathook_echams_unlit"), kv);
    }
    {
        KeyValues *kv = new KeyValues(XORSTR("UnlitGeneric"));
        kv->SetString(XORSTR("$basetexture"), XORSTR("vgui/white_additive"));
        kv->SetInt(XORSTR("$ignorez"), 1);
        mat_unlit_z.Init(XORSTR("__cathook_echams_unlit_z"), kv);
    }
    {
        KeyValues *kv = new KeyValues(XORSTR("VertexLitGeneric"));
        kv->SetString(XORSTR("$basetexture"), XORSTR("vgui/white_additive"));
        kv->SetInt(XORSTR("$ignorez"), 0);
        kv->SetInt(XORSTR("$halflambert"), 1);
        mat_lit.Init(XORSTR("__cathook_echams_lit"), kv);
    }
    {
        KeyValues *kv = new KeyValues(XORSTR("VertexLitGeneric"));
        kv->SetString(XORSTR("$basetexture"), XORSTR("vgui/white_additive"));
        kv->SetInt(XORSTR("$ignorez"), 1);
        kv->SetInt(XORSTR("$halflambert"), 1);
        mat_lit_z.Init(XORSTR("__cathook_echams_lit_z"), kv);
    }
    logging::Info(XORSTR("Init done!"));
    init = true;
}

void EffectChams::BeginRenderChams()
{
    drawing = true;
    CMatRenderContextPtr ptr(GET_RENDER_CONTEXT);
    g_IVRenderView->SetBlend(1.0f);
}

void EffectChams::EndRenderChams()
{
    drawing = false;
    CMatRenderContextPtr ptr(GET_RENDER_CONTEXT);
    g_IVModelRender->ForcedMaterialOverride(nullptr);
}
bool data[32] = {};
void EffectChams::SetEntityColor(CachedEntity *ent, rgba_t color)
{
    data[ent->m_IDX] = color;
}
rgba_t EffectChams::ChamsColor(IClientEntity *entity)
{
    CachedEntity *ent = ENTITY(entity->entindex());
    if (data[entity->entindex()])
    {
        data[entity->entindex()] = false;
        return colors::pink;
    }
    if (CE_BAD(ent))
        return colors::white;
    if (re::C_BaseCombatWeapon::IsBaseCombatWeapon(entity))
    {
        IClientEntity *owner = re::C_TFWeaponBase::GetOwnerViaInterface(entity);
        if (owner)
        {
            return ChamsColor(owner);
        }
    }
    switch (ent->m_Type())
    {
    case ENTITY_BUILDING:
        if (!ent->m_bEnemy() && !(teammates || teammate_buildings) &&
            (!(chamsR || chamsB || chamsG) && ent != LOCAL_E))
        {
            return colors::empty;
        }
        if (health)
        {
            return colors::Health(ent->m_iHealth(), ent->m_iMaxHealth());
        }
        break;
    case ENTITY_PLAYER:
        if (!players)
            return colors::empty;
        if (ent->m_IDX == LOCAL_E->m_IDX && chamsteam)
            if (LOCAL_E->m_iTeam() == TEAM_BLU)
                return colors::blu;
            else
                return colors::red;
        if (ent->m_IDX == LOCAL_E->m_IDX && !rainbow)
            return colors::FromRGBA8(chamsR, chamsG, chamsB, 255);
        if (health)
        {
            return colors::Health(ent->m_iHealth(), ent->m_iMaxHealth());
        }
        break;
    }
    return colors::EntityF(ent);
}

bool EffectChams::ShouldRenderChams(IClientEntity *entity)
{
    if (!enable)
        return false;
    if (entity->entindex() < 0)
        return false;
    CachedEntity *ent = ENTITY(entity->entindex());
    if (CE_BAD(ent))
        return false;
    if (ent->m_IDX == LOCAL_E->m_IDX && !chamsself)
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
        if (CE_BYTE(LOCAL_E, netvar.m_bCarryingObject) &&
            LOCAL_E->m_vecOrigin().DistTo(ent->m_vecOrigin()) <= 100.0f)
            return false;
        return true;
    case ENTITY_PLAYER:
        if (!players)
            return false;
        if (!teammates && !ent->m_bEnemy() && playerlist::IsDefault(ent))
            return false;
        if (CE_BYTE(ent, netvar.iLifeState) != LIFE_ALIVE)
            return false;
        return true;
        break;
    case ENTITY_PROJECTILE:
        if (!ent->m_bEnemy())
            return false;
        if (stickies &&
            ent->m_iClassID() == CL_CLASS(CTFGrenadePipebombProjectile))
        {
            return true;
        }
        break;
    case ENTITY_GENERIC:
        switch (ent->m_ItemType())
        {
        case ITEM_HEALTH_LARGE:
        case ITEM_HEALTH_MEDIUM:
        case ITEM_HEALTH_SMALL:
            return medkits;
        case ITEM_AMMO_LARGE:
        case ITEM_AMMO_MEDIUM:
        case ITEM_AMMO_SMALL:
            return ammobox;
        }
        break;
    }
    return false;
}

void EffectChams::RenderChamsRecursive(IClientEntity *entity)
{
    entity->DrawModel(1);

    if (!recursive)
        return;

    IClientEntity *attach;
    int passes = 0;

    attach = g_IEntityList->GetClientEntity(
        *(int *) ((uintptr_t) entity + netvar.m_Collision - 24) & 0xFFF);
    while (attach && passes++ < 32)
    {
        if (attach->ShouldDraw())
        {
            if (entity->GetClientClass()->m_ClassID == RCC_PLAYER &&
                re::C_BaseCombatWeapon::IsBaseCombatWeapon(attach))
            {
                if (weapons_white)
                {
                    rgba_t mod_original;
                    g_IVRenderView->GetColorModulation(mod_original.rgba);
                    g_IVRenderView->SetColorModulation(colors::white);
                    attach->DrawModel(1);
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
        attach = g_IEntityList->GetClientEntity(
            *(int *) ((uintptr_t) attach + netvar.m_Collision - 20) & 0xFFF);
    }
}

void EffectChams::RenderChams(int idx)
{
    CMatRenderContextPtr ptr(GET_RENDER_CONTEXT);
    IClientEntity *entity = g_IEntityList->GetClientEntity(idx);
    if (entity && !entity->IsDormant())
    {
        if (ShouldRenderChams(entity))
        {
            rgba_t color   = ChamsColor(entity);
            rgba_t color_2 = color * 0.6f;
            if (!legit)
            {
                mat_unlit_z->AlphaModulate(1.0f);
                ptr->DepthRange(0.0f, 0.01f);
                g_IVRenderView->SetColorModulation(color_2);
                g_IVModelRender->ForcedMaterialOverride(flat ? mat_unlit_z
                                                             : mat_lit_z);

                RenderChamsRecursive(entity);
            }

            if (legit || !singlepass)
            {
                mat_unlit->AlphaModulate(1.0f);
                g_IVRenderView->SetColorModulation(color);
                ptr->DepthRange(0.0f, 1.0f);
                g_IVModelRender->ForcedMaterialOverride(flat ? mat_unlit
                                                             : mat_lit);
                RenderChamsRecursive(entity);
            }
        }
    }
}

void EffectChams::Render(int x, int y, int w, int h)
{
    if (!init)
        Init();
    if (!cathook || (g_IEngine->IsTakingScreenshot() && clean_screenshots))
        return;
    if (!enable)
        return;
    CMatRenderContextPtr ptr(GET_RENDER_CONTEXT);
    BeginRenderChams();
    for (int i = 1; i < HIGHEST_ENTITY; i++)
    {
        IClientEntity *ent = g_IEntityList->GetClientEntity(i);
        if (ent && !ent->IsDormant())
        {
            RenderChams(i);
        }
    }
    EndRenderChams();
}

EffectChams g_EffectChams;
CScreenSpaceEffectRegistration *g_pEffectChams = nullptr;
}
