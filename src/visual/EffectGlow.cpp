/*
 * EffectGlow.cpp
 *
 *  Created on: Apr 13, 2017
 *      Author: nullifiedcat
 */

#include <visual/EffectGlow.hpp>
#include <MiscTemporary.hpp>
#include <hacks/Aimbot.hpp>
#include <settings/Bool.hpp>
#include "common.hpp"

IMaterialSystem *materials = nullptr;

CScreenSpaceEffectRegistration *CScreenSpaceEffectRegistration::s_pHead = NULL;
IScreenSpaceEffectManager *g_pScreenSpaceEffects                        = nullptr;
CScreenSpaceEffectRegistration **g_ppScreenSpaceRegistrationHead        = nullptr;
CScreenSpaceEffectRegistration::CScreenSpaceEffectRegistration(const char *pName, IScreenSpaceEffect *pEffect)
{
    logging::Info("Creating new effect '%s', head: 0x%08x", pName, *g_ppScreenSpaceRegistrationHead);
    m_pEffectName                    = pName;
    m_pEffect                        = pEffect;
    m_pNext                          = *g_ppScreenSpaceRegistrationHead;
    *g_ppScreenSpaceRegistrationHead = this;
    logging::Info("New head: 0x%08x", *g_ppScreenSpaceRegistrationHead);
}

namespace effect_glow
{

static settings::Boolean health{ "glow.health", "false" };
static settings::Boolean aimbot_color{ "glow.aimbot.color", "true" };
static settings::Boolean teammates{ "glow.show.teammates", "false" };
static settings::Boolean disguised{ "glow.show.disguised", "true" };
static settings::Boolean players{ "glow.show.players", "true" };
static settings::Boolean medkits{ "glow.show.medkits", "false" };
static settings::Boolean ammobox{ "glow.show.ammoboxes", "false" };
static settings::Boolean buildings{ "glow.show.buildings", "true" };
static settings::Boolean stickies{ "glow.show.stickies", "true" };
static settings::Boolean teammate_buildings{ "glow.show.teammate-buildings", "false" };
static settings::Boolean show_powerups{ "glow.show.powerups", "true" };
static settings::Boolean weapons_white{ "glow.white-weapons", "true" };
static settings::Boolean glowself{ "glow.self", "true" };
static settings::Boolean rainbow{ "glow.self-rainbow", "true" };
static settings::Int blur_scale{ "glow.blur-scale", "5" };
// https://puu.sh/vobH4/5da8367aef.png
static settings::Int solid_when{ "glow.solid-when", "0" };
settings::Boolean enable{ "glow.enable", "false" };

struct ShaderStencilState_t
{
    bool m_bEnable;
    StencilOperation_t m_FailOp;
    StencilOperation_t m_ZFailOp;
    StencilOperation_t m_PassOp;
    StencilComparisonFunction_t m_CompareFunc;
    int m_nReferenceValue;
    uint32 m_nTestMask;
    uint32 m_nWriteMask;

    ShaderStencilState_t()
    {
        Reset();
    }

    inline void Reset()
    {
        m_bEnable = false;
        m_PassOp = m_FailOp = m_ZFailOp = STENCILOPERATION_KEEP;
        m_CompareFunc                   = STENCILCOMPARISONFUNCTION_ALWAYS;
        m_nReferenceValue               = 0;
        m_nTestMask = m_nWriteMask = 0xFFFFFFFF;
    }

    inline void SetStencilState(CMatRenderContextPtr &pRenderContext) const
    {
        pRenderContext->SetStencilEnable(m_bEnable);
        pRenderContext->SetStencilFailOperation(m_FailOp);
        pRenderContext->SetStencilZFailOperation(m_ZFailOp);
        pRenderContext->SetStencilPassOperation(m_PassOp);
        pRenderContext->SetStencilCompareFunction(m_CompareFunc);
        pRenderContext->SetStencilReferenceValue(m_nReferenceValue);
        pRenderContext->SetStencilTestMask(m_nTestMask);
        pRenderContext->SetStencilWriteMask(m_nWriteMask);
    }
};

static CTextureReference buffers[4]{};

ITexture *GetBuffer(int i)
{
    if (!buffers[i])
    {
        ITexture *fullframe;
        IF_GAME(IsTF2())
        fullframe      = g_IMaterialSystem->FindTexture("_rt_FullFrameFB", TEXTURE_GROUP_RENDER_TARGET);
        else fullframe = g_IMaterialSystemHL->FindTexture("_rt_FullFrameFB", TEXTURE_GROUP_RENDER_TARGET);
        // char *newname    = new char[32];
        std::unique_ptr<char[]> newname(new char[32]);
        std::string name = format("_cathook_buff", i);
        strncpy(newname.get(), name.c_str(), 30);
        logging::Info("Creating new buffer %d with size %dx%d %s", i, fullframe->GetActualWidth(), fullframe->GetActualHeight(), newname.get());

        int textureFlags      = TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT | TEXTUREFLAGS_EIGHTBITALPHA;
        int renderTargetFlags = CREATERENDERTARGETFLAGS_HDR;

        ITexture *texture;
        IF_GAME(IsTF2())
        {
            texture = g_IMaterialSystem->CreateNamedRenderTargetTextureEx(newname.get(), fullframe->GetActualWidth(), fullframe->GetActualHeight(), RT_SIZE_LITERAL, IMAGE_FORMAT_RGBA8888, MATERIAL_RT_DEPTH_SEPARATE, textureFlags, renderTargetFlags);
        }
        else
        {
            texture = g_IMaterialSystemHL->CreateNamedRenderTargetTextureEx(newname.get(), fullframe->GetActualWidth(), fullframe->GetActualHeight(), RT_SIZE_LITERAL, IMAGE_FORMAT_RGBA8888, MATERIAL_RT_DEPTH_SEPARATE, textureFlags, renderTargetFlags);
        }
        buffers[i].Init(texture);
    }
    return buffers[i];
}

static ShaderStencilState_t SS_NeverSolid{};
static ShaderStencilState_t SS_SolidInvisible{};
static ShaderStencilState_t SS_Null{};
static ShaderStencilState_t SS_Drawing{};

CatCommand fix_black_glow("fix_black_glow", "Fix Black Glow",
                          []()
                          {
                              effect_glow::g_EffectGlow.Shutdown();
                              effect_glow::g_EffectGlow.Init();
                          });

void EffectGlow::Init()
{
#if !ENFORCE_STREAM_SAFETY
    if (init)
        return;
    logging::Info("Init Glow...");
    {
        KeyValues *kv = new KeyValues("UnlitGeneric");
        kv->SetString("$basetexture", "vgui/white_additive");
        kv->SetInt("$ignorez", 0);
        mat_unlit.Init("__cathook_glow_unlit", kv);
    }
    {
        KeyValues *kv = new KeyValues("UnlitGeneric");
        kv->SetString("$basetexture", "vgui/white_additive");
        kv->SetInt("$ignorez", 1);
        mat_unlit_z.Init("__cathook_glow_unlit_z", kv);
    }
    // Initialize 2 buffers
    GetBuffer(1);
    GetBuffer(2);
    {
        KeyValues *kv = new KeyValues("UnlitGeneric");
        kv->SetString("$basetexture", "_cathook_buff1");
        kv->SetInt("$additive", 1);
        mat_blit.Init("__cathook_glow_blit", TEXTURE_GROUP_CLIENT_EFFECTS, kv);
        mat_blit->Refresh();
    }
    {
        KeyValues *kv = new KeyValues("BlurFilterX");
        kv->SetString("$basetexture", "_cathook_buff1");
        kv->SetInt("$ignorez", 1);
        kv->SetInt("$translucent", 1);
        kv->SetInt("$alphatest", 1);
        mat_blur_x.Init("_cathook_blurx", kv);
        mat_blur_x->Refresh();
    }
    {
        KeyValues *kv = new KeyValues("BlurFilterY");
        kv->SetString("$basetexture", "_cathook_buff2");
        kv->SetInt("$bloomamount", 5);
        kv->SetInt("$ignorez", 1);
        kv->SetInt("$translucent", 1);
        kv->SetInt("$alphatest", 1);
        mat_blur_y.Init("_cathook_blury", kv);
        mat_blur_y->Refresh();
    }
    {
        SS_NeverSolid.m_bEnable         = true;
        SS_NeverSolid.m_PassOp          = STENCILOPERATION_REPLACE;
        SS_NeverSolid.m_FailOp          = STENCILOPERATION_KEEP;
        SS_NeverSolid.m_ZFailOp         = STENCILOPERATION_KEEP;
        SS_NeverSolid.m_CompareFunc     = STENCILCOMPARISONFUNCTION_ALWAYS;
        SS_NeverSolid.m_nWriteMask      = 1;
        SS_NeverSolid.m_nReferenceValue = 1;
    }
    {
        SS_SolidInvisible.m_bEnable         = true;
        SS_SolidInvisible.m_PassOp          = STENCILOPERATION_REPLACE;
        SS_SolidInvisible.m_FailOp          = STENCILOPERATION_KEEP;
        SS_SolidInvisible.m_ZFailOp         = STENCILOPERATION_KEEP;
        SS_SolidInvisible.m_CompareFunc     = STENCILCOMPARISONFUNCTION_ALWAYS;
        SS_SolidInvisible.m_nWriteMask      = 1;
        SS_SolidInvisible.m_nReferenceValue = 1;
    }
    /*case 3: https://puu.sh/vobH4/5da8367aef.png*/
    {
        SS_Drawing.m_bEnable         = true;
        SS_Drawing.m_nReferenceValue = 0;
        SS_Drawing.m_nTestMask       = 1;
        SS_Drawing.m_CompareFunc     = STENCILCOMPARISONFUNCTION_EQUAL;
        SS_Drawing.m_PassOp          = STENCILOPERATION_ZERO;
    }

    logging::Info("Init done!");
    init = true;
#endif
}

void EffectGlow::Shutdown()
{
    if (init)
    {
        mat_unlit.Shutdown();
        mat_unlit_z.Shutdown();
        mat_blit.Shutdown();
        mat_blur_x.Shutdown();
        mat_blur_y.Shutdown();
        init = false;
        logging::Info("Shutdown glow");
    }
}

rgba_t EffectGlow::GlowColor(IClientEntity *entity)
{
    static CachedEntity *ent;
    static IClientEntity *owner;

    ent = ENTITY(entity->entindex());
    if (CE_BAD(ent))
        return colors::white;
    if (ent == hacks::shared::aimbot::CurrentTarget() && aimbot_color)
        return colors::target;
    if (re::C_BaseCombatWeapon::IsBaseCombatWeapon(entity))
    {
        owner = re::C_TFWeaponBase::GetOwnerViaInterface(entity);
        if (owner)
        {
            return GlowColor(owner);
        }
    }
    switch (ent->m_Type())
    {
    case ENTITY_BUILDING:
        if (health)
        {
            return colors::Health_dimgreen(ent->m_iHealth(), ent->m_iMaxHealth());
        }
        break;
    case ENTITY_PLAYER:
        if (health && playerlist::IsDefault(ent))
        {
            return colors::Health_dimgreen(ent->m_iHealth(), ent->m_iMaxHealth());
        }
        else if (!playerlist::IsDefault(ent))
            return playerlist::Color(ent);
    default:
        break;
    }

    return colors::EntityF(ent);
}

bool EffectGlow::ShouldRenderGlow(IClientEntity *entity)
{
#if ENFORCE_STREAM_SAFETY
    return false;
#endif
    static CachedEntity *ent;
    if (entity->entindex() < 0)
        return false;
    ent = ENTITY(entity->entindex());
    if (CE_BAD(ent))
        return false;
    if (ent->m_IDX == LOCAL_E->m_IDX && !glowself)
        return false;
    switch (ent->m_Type())
    {
    case ENTITY_BUILDING:
        if (!buildings)
            return false;
        if (!ent->m_bEnemy() && !teammate_buildings)
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
        if (CE_BYTE(ent, netvar.iLifeState) != LIFE_ALIVE)
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
        const auto &type = ent->m_ItemType();
        if (type >= ITEM_HEALTH_SMALL && type <= ITEM_HEALTH_LARGE)
        {
            return *medkits;
        }
        else if (type >= ITEM_AMMO_SMALL && type <= ITEM_AMMO_LARGE)
        {
            return *ammobox;
        }
        else if (type >= ITEM_POWERUP_FIRST && type <= ITEM_POWERUP_LAST)
        {
            return *show_powerups;
        }
        break;
    }
    return false;
}

void EffectGlow::BeginRenderGlow()
{
#if !ENFORCE_STREAM_SAFETY
    drawing = true;
    CMatRenderContextPtr ptr(GET_RENDER_CONTEXT);
    ptr->ClearColor4ub(0, 0, 0, 0);
    ptr->PushRenderTargetAndViewport();
    ptr->SetRenderTarget(GetBuffer(1));
    ptr->OverrideAlphaWriteEnable(true, true);
    g_IVRenderView->SetBlend(0.99f);
    ptr->ClearBuffers(true, false);
    mat_unlit_z->AlphaModulate(1.0f);
    ptr->DepthRange(0.0f, 0.01f);
#endif
}

void EffectGlow::EndRenderGlow()
{
#if !ENFORCE_STREAM_SAFETY
    drawing = false;
    CMatRenderContextPtr ptr(GET_RENDER_CONTEXT);
    ptr->DepthRange(0.0f, 1.0f);
    g_IVModelRender->ForcedMaterialOverride(nullptr);
    ptr->PopRenderTargetAndViewport();
#endif
}

void EffectGlow::StartStenciling()
{
#if !ENFORCE_STREAM_SAFETY
    static ShaderStencilState_t state;
    state.Reset();
    state.m_bEnable = true;
    CMatRenderContextPtr ptr(GET_RENDER_CONTEXT);
    switch (*solid_when)
    {
    case 0:
        SS_NeverSolid.SetStencilState(ptr);
        break;
    case 2:
        SS_SolidInvisible.SetStencilState(ptr);
        break;
        /*case 3: https://puu.sh/vobH4/5da8367aef.png*/
    default:
        break;
    }
    if (!*solid_when)
    {
        ptr->DepthRange(0.0f, 0.01f);
    }
    else
    {
        ptr->DepthRange(0.0f, 1.0f);
    }
    g_IVRenderView->SetBlend(0.0f);
    mat_unlit->AlphaModulate(1.0f);
    g_IVModelRender->ForcedMaterialOverride(*solid_when ? mat_unlit : mat_unlit_z);
#endif
}

void EffectGlow::EndStenciling()
{
#if !ENFORCE_STREAM_SAFETY
    static ShaderStencilState_t state;
    state.Reset();
    g_IVModelRender->ForcedMaterialOverride(nullptr);
    CMatRenderContextPtr ptr(GET_RENDER_CONTEXT);
    state.SetStencilState(ptr);
    ptr->DepthRange(0.0f, 1.0f);
    g_IVRenderView->SetBlend(1.0f);
#endif
}

void EffectGlow::DrawToStencil(IClientEntity *entity)
{
#if !ENFORCE_STREAM_SAFETY
    DrawEntity(entity);
#endif
}

void EffectGlow::DrawToBuffer(IClientEntity *entity)
{
}

void EffectGlow::DrawEntity(IClientEntity *entity)
{
#if !ENFORCE_STREAM_SAFETY
    static IClientEntity *attach;
    static int passes;
    passes = 0;

    entity->DrawModel(1);
    attach = g_IEntityList->GetClientEntity(HandleToIDX(*(int *) ((uintptr_t) entity + netvar.m_Collision - 24)));
    while (attach && passes++ < 32)
    {
        if (attach->ShouldDraw())
        {
            if (weapons_white && entity->GetClientClass()->m_ClassID == RCC_PLAYER && re::C_BaseCombatWeapon::IsBaseCombatWeapon(attach))
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
        attach = g_IEntityList->GetClientEntity(HandleToIDX(*(int *) ((uintptr_t) attach + netvar.m_Collision - 20)));
    }
#endif
}

void EffectGlow::RenderGlow(IClientEntity *entity)
{
#if !ENFORCE_STREAM_SAFETY
    CMatRenderContextPtr ptr(GET_RENDER_CONTEXT);
    g_IVRenderView->SetColorModulation(GlowColor(entity));
    g_IVModelRender->ForcedMaterialOverride(mat_unlit_z);
    DrawEntity(entity);
#endif
}

void EffectGlow::Render(int x, int y, int w, int h)
{
#if !ENFORCE_STREAM_SAFETY
    if (!enable)
        return;
    if (!isHackActive() || (clean_screenshots && g_IEngine->IsTakingScreenshot()) || g_Settings.bInvalid || disable_visuals)
        return;
    static ITexture *orig;
    static IMaterialVar *blury_bloomamount;
    if (!init)
        Init();
    CMatRenderContextPtr ptr(GET_RENDER_CONTEXT);
    orig = ptr->GetRenderTarget();
    BeginRenderGlow();
    for (auto const &ent_non_raw : entity_cache::valid_ents)
    {
        auto ent = RAW_ENT(ent_non_raw);
        if (ent && ShouldRenderGlow(ent))
        {
            RenderGlow(ent);
        }
    }
    EndRenderGlow();
    if (*solid_when != 1)
    {
        ptr->ClearStencilBufferRectangle(x, y, w, h, 0);
        StartStenciling();
        for (auto &non_raw : entity_cache::valid_ents)
        {
            auto ent = RAW_ENT(non_raw);
            if (ent && ShouldRenderGlow(ent))
            {
                DrawToStencil(ent);
            }
        }
        EndStenciling();
    }
    ptr->SetRenderTarget(GetBuffer(2));
    ptr->Viewport(x, y, w, h);
    ptr->ClearBuffers(true, false);
    ptr->DrawScreenSpaceRectangle(mat_blur_x, x, y, w, h, 0, 0, w - 1, h - 1, w, h);
    ptr->SetRenderTarget(GetBuffer(1));
    blury_bloomamount = mat_blur_y->FindVar("$bloomamount", nullptr);
    blury_bloomamount->SetIntValue(*blur_scale);
    ptr->DrawScreenSpaceRectangle(mat_blur_y, x, y, w, h, 0, 0, w - 1, h - 1, w, h);
    ptr->Viewport(x, y, w, h);
    ptr->SetRenderTarget(orig);
    g_IVRenderView->SetBlend(0.0f);
    if (*solid_when != 1)
    {
        SS_Drawing.SetStencilState(ptr);
    }
    ptr->DrawScreenSpaceRectangle(mat_blit, x, y, w, h, 0, 0, w - 1, h - 1, w, h);
    if (*solid_when != -1)
    {
        SS_Null.SetStencilState(ptr);
    }
#endif
}

EffectGlow g_EffectGlow;
CScreenSpaceEffectRegistration *g_pEffectGlow = nullptr;

static InitRoutine init(
    []()
    {
        EC::Register(
            EC::LevelShutdown, []() { g_EffectGlow.Shutdown(); }, "glow");
        if (g_ppScreenSpaceRegistrationHead && g_pScreenSpaceEffects)
        {
            effect_glow::g_pEffectGlow = new CScreenSpaceEffectRegistration("_cathook_glow", &effect_glow::g_EffectGlow);
            g_pScreenSpaceEffects->EnableScreenSpaceEffect("_cathook_glow");
        }
    } // namespace effect_glow
);
} // namespace effect_glow
