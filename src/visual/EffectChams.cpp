/*
 * EffectChams.cpp
 *
 *  Created on: Apr 16, 2017
 *      Author: nullifiedcat
 */

#include <visual/EffectChams.hpp>
#include <MiscTemporary.hpp>
#include <settings/Bool.hpp>
#include "common.hpp"
#include "Backtrack.hpp"

static settings::Bool enable{ "chams.enable", "false" };
static settings::Bool flat{ "chams.flat", "false" };
static settings::Bool health{ "chams.health", "false" };
static settings::Bool teammates{ "chams.show.teammates", "false" };
static settings::Bool players{ "chams.show.players", "true" };
static settings::Bool medkits{ "chams.show.medkits", "false" };
static settings::Bool ammobox{ "chams.show.ammoboxes", "false" };
static settings::Bool buildings{ "chams.show.buildings", "true" };
static settings::Bool stickies{ "chams.show.stickies", "true" };
static settings::Bool teammate_buildings{ "chams.show.teammate-buildings",
                                          "false" };
static settings::Bool recursive{ "chams.recursive", "true" };
static settings::Bool weapons_white{ "chams.white-weapons", "true" };
static settings::Bool legit{ "chams.legit", "false" };
static settings::Bool singlepass{ "chams.single-pass", "false" };
static settings::Bool chamsself{ "chams.self", "true" };
static settings::Bool rainbow{ "chams.self-rainbow", "true" };
static settings::Bool disco_chams{ "chams.disco", "false" };

namespace effect_chams
{

void EffectChams::Init()
{
    logging::Info("Init EffectChams...");
    {
        KeyValues *kv = new KeyValues("UnlitGeneric");
        kv->SetString("$basetexture", "vgui/white_additive");
        kv->SetInt("$ignorez", 0);
        mat_unlit.Init("__cathook_echams_unlit", kv);
    }
    {
        KeyValues *kv = new KeyValues("UnlitGeneric");
        kv->SetString("$basetexture", "vgui/white_additive");
        kv->SetInt("$ignorez", 1);
        mat_unlit_z.Init("__cathook_echams_unlit_z", kv);
    }
    {
        KeyValues *kv = new KeyValues("VertexLitGeneric");
        kv->SetString("$basetexture", "vgui/white_additive");
        kv->SetInt("$ignorez", 0);
        kv->SetInt("$halflambert", 1);
        mat_lit.Init("__cathook_echams_lit", kv);
    }
    {
        KeyValues *kv = new KeyValues("VertexLitGeneric");
        kv->SetString("$basetexture", "vgui/white_additive");
        kv->SetInt("$ignorez", 1);
        kv->SetInt("$halflambert", 1);
        mat_lit_z.Init("__cathook_echams_lit_z", kv);
    }
    logging::Info("Init done!");
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
Timer t{};
int prevcolor = -1;
rgba_t EffectChams::ChamsColor(IClientEntity *entity)
{
    CachedEntity *ent = ENTITY(entity->entindex());
    if (disco_chams)
    {
        static rgba_t disco = { 0, 0, 0, 0 };
        if (t.test_and_set(200))
        {
            int color = rand() % 20;
            while (color == prevcolor)
                color = rand() % 20;
            prevcolor = color;
            switch (color)
            {
            case 2:
                disco = colors::pink;
                break;
            case 3:
                disco = colors::red;
                break;
            case 4:
                disco = colors::blu;
                break;
            case 5:
                disco = colors::red_b;
                break;
            case 6:
                disco = colors::blu_b;
                break;
            case 7:
                disco = colors::red_v;
                break;
            case 8:
                disco = colors::blu_v;
                break;
            case 9:
                disco = colors::red_u;
                break;
            case 10:
                disco = colors::blu_u;
                break;
            case 0:
            case 1:
            case 11:
            case 12:
            case 13:
            case 14:
            case 15:
            case 16:
            case 17:
            case 18:
            case 19:
                float color1 = rand() % 256;
                float color2 = rand() % 256;
                float color3 = rand() % 256;
                disco        = { color1, color2, color3, 255.0f };
            }
        }
        return disco;
    }
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
            ent != LOCAL_E)
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
            return *medkits;
        case ITEM_AMMO_LARGE:
        case ITEM_AMMO_MEDIUM:
        case ITEM_AMMO_SMALL:
            return *ammobox;
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

void EffectChams::RenderChams(IClientEntity *entity)
{
    CMatRenderContextPtr ptr(GET_RENDER_CONTEXT);
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
            g_IVModelRender->ForcedMaterialOverride(flat ? mat_unlit : mat_lit);
            RenderChamsRecursive(entity);
        }
    }
}

void EffectChams::Render(int x, int y, int w, int h)
{
    PROF_SECTION(DRAW_chams);
    if (!enable)
        return;
    if (!init)
        Init();
    if (!isHackActive() ||
        (g_IEngine->IsTakingScreenshot() && clean_screenshots))
        return;
    CMatRenderContextPtr ptr(GET_RENDER_CONTEXT);
    BeginRenderChams();
    for (int i = 1; i < HIGHEST_ENTITY; i++)
    {
        IClientEntity *entity = g_IEntityList->GetClientEntity(i);
        if (!entity || entity->IsDormant() || CE_BAD(ENTITY(i)))
            return;
        RenderChams(entity);
    }
    EndRenderChams();
}

EffectChams g_EffectChams;
CScreenSpaceEffectRegistration *g_pEffectChams = nullptr;
} // namespace effect_chams
