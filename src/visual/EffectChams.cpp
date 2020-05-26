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

namespace effect_chams
{
static settings::Boolean flat{ "chams.flat", "false" };
static settings::Boolean health{ "chams.health", "false" };
static settings::Boolean teammates{ "chams.show.teammates", "false" };
static settings::Boolean disguised{ "chams.show.disguised", "true" };
static settings::Boolean players{ "chams.show.players", "true" };
static settings::Boolean medkits{ "chams.show.medkits", "false" };
static settings::Boolean ammobox{ "chams.show.ammoboxes", "false" };
static settings::Boolean buildings{ "chams.show.buildings", "true" };
static settings::Boolean stickies{ "chams.show.stickies", "true" };
static settings::Boolean teammate_buildings{ "chams.show.teammate-buildings", "false" };
static settings::Boolean recursive{ "chams.recursive", "true" };
static settings::Boolean weapons_white{ "chams.white-weapons", "true" };
static settings::Boolean legit{ "chams.legit", "false" };
static settings::Boolean singlepass{ "chams.single-pass", "false" };
static settings::Boolean chamsself{ "chams.self", "true" };
static settings::Boolean disco_chams{ "chams.disco", "false" };

settings::Boolean enable{ "chams.enable", "false" };
CatCommand fix_black_chams("fix_black_chams", "Fix Black Chams", []() {
    effect_chams::g_EffectChams.Shutdown();
    effect_chams::g_EffectChams.Init();
});

void EffectChams::Init()
{
#if !ENFORCE_STREAM_SAFETY
    if (init)
        return;
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
#endif
}

void EffectChams::BeginRenderChams()
{
#if !ENFORCE_STREAM_SAFETY
    drawing = true;
    CMatRenderContextPtr ptr(GET_RENDER_CONTEXT);
    g_IVRenderView->SetBlend(1.0f);
#endif
}

void EffectChams::EndRenderChams()
{
#if !ENFORCE_STREAM_SAFETY
    drawing = false;
    CMatRenderContextPtr ptr(GET_RENDER_CONTEXT);
    g_IVModelRender->ForcedMaterialOverride(nullptr);
#endif
}
static rgba_t data[PLAYER_ARRAY_SIZE] = { colors::empty };
void EffectChams::SetEntityColor(CachedEntity *ent, rgba_t color)
{
    if (ent->m_IDX > MAX_PLAYERS || ent->m_IDX < 0)
        return;
    data[ent->m_IDX] = color;
}
static Timer t{};
static int prevcolor = -1;
rgba_t EffectChams::ChamsColor(IClientEntity *entity)
{
    if (!isHackActive() || !*effect_chams::enable)
        return colors::empty;

    CachedEntity *ent = ENTITY(entity->entindex());
    if (disco_chams)
    {
        static rgba_t disco{ 0, 0, 0, 0 };
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
    if (ent->m_IDX <= MAX_PLAYERS && ent->m_IDX >= 0)
    {
        if (data[entity->entindex()] != colors::empty)
        {
            auto toret               = data[entity->entindex()];
            data[entity->entindex()] = colors::empty;
            return toret;
        }
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
        if (!ent->m_bEnemy() && !(teammates || teammate_buildings) && ent != LOCAL_E)
        {
            return colors::empty;
        }
        if (health)
        {
            return colors::Health_dimgreen(ent->m_iHealth(), ent->m_iMaxHealth());
        }
        break;
    case ENTITY_PLAYER:
        if (!players)
            return colors::empty;
        if (health)
        {
            return colors::Health_dimgreen(ent->m_iHealth(), ent->m_iMaxHealth());
        }
        break;
    default:
        break;
    }
    return colors::EntityF(ent);
}

bool EffectChams::ShouldRenderChams(IClientEntity *entity)
{
#if ENFORCE_STREAM_SAFETY
    return false;
#endif
    if (!isHackActive() || !*effect_chams::enable || CE_BAD(LOCAL_E))
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
        break;
    case ENTITY_PROJECTILE:
        if (!ent->m_bEnemy())
            return false;
        if (stickies && ent->m_iClassID() == CL_CLASS(CTFGrenadePipebombProjectile))
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
        default:
            break;
        }
        break;
    default:
        break;
    }
    return false;
}

void EffectChams::RenderChamsRecursive(IClientEntity *entity)
{
#if !ENFORCE_STREAM_SAFETY
    if (!isHackActive() || !*effect_chams::enable)
        return;
    entity->DrawModel(1);

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
        attach = g_IEntityList->GetClientEntity(*(int *) ((uintptr_t) attach + netvar.m_Collision - 20) & 0xFFF);
    }
#endif
}

void EffectChams::RenderChams(IClientEntity *entity)
{
#if !ENFORCE_STREAM_SAFETY
    if (!isHackActive() || !*effect_chams::enable)
        return;
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
            g_IVModelRender->ForcedMaterialOverride(flat ? mat_unlit_z : mat_lit_z);
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
#endif
}
void EffectChams::Render(int x, int y, int w, int h)
{
#if !ENFORCE_STREAM_SAFETY
    PROF_SECTION(DRAW_chams);
    if (!isHackActive() || disable_visuals)
        return;
    if (!effect_chams::enable && !(hacks::tf2::backtrack::chams && hacks::tf2::backtrack::isBacktrackEnabled))
        return;
    if (g_Settings.bInvalid)
        return;
    if (!init && effect_chams::enable)
        Init();
    if (!isHackActive() || (g_IEngine->IsTakingScreenshot() && clean_screenshots))
        return;
    if (hacks::tf2::backtrack::chams && hacks::tf2::backtrack::isBacktrackEnabled)
    {
        CMatRenderContextPtr ptr(GET_RENDER_CONTEXT);
        BeginRenderChams();
        // Don't mark as normal chams drawing
        drawing = false;
        for (int i = 1; i <= g_IEngine->GetMaxClients(); i++)
        {
            CachedEntity *ent = ENTITY(i);
            if (CE_BAD(ent) || i == g_IEngine->GetLocalPlayer() || !ent->m_bAlivePlayer() || ent->m_Type() != ENTITY_PLAYER)
                continue;
            // Entity won't draw in some cases so help the chams a bit
            hacks::tf2::backtrack::isDrawing = true;
            RAW_ENT(ent)->DrawModel(1);
            hacks::tf2::backtrack::isDrawing = false;
        }
        EndRenderChams();
    }
    if (!effect_chams::enable)
        return;
    CMatRenderContextPtr ptr(GET_RENDER_CONTEXT);
    BeginRenderChams();
    for (int i = 1; i <= HIGHEST_ENTITY; i++)
    {
        IClientEntity *entity = g_IEntityList->GetClientEntity(i);
        if (!entity || entity->IsDormant() || CE_BAD(ENTITY(i)))
            continue;
        RenderChams(entity);
    }
    EndRenderChams();
#endif
}
EffectChams g_EffectChams;
CScreenSpaceEffectRegistration *g_pEffectChams = nullptr;

static InitRoutine init([]() {
    EC::Register(
        EC::LevelShutdown, []() { g_EffectChams.Shutdown(); }, "chams");
    if (g_ppScreenSpaceRegistrationHead && g_pScreenSpaceEffects)
    {
        effect_chams::g_pEffectChams = new CScreenSpaceEffectRegistration("_cathook_chams", &effect_chams::g_EffectChams);
        g_pScreenSpaceEffects->EnableScreenSpaceEffect("_cathook_chams");
    }
});
} // namespace effect_chams
