/*
 * EffectGlow.cpp
 *
 *  Created on: Apr 13, 2017
 *      Author: nullifiedcat
 */

#include <visual/EffectGlow.hpp>
#include <MiscTemporary.hpp>
#include <hacks/Aimbot.hpp>
#include "common.hpp"

IMaterialSystem *materials = nullptr;

CScreenSpaceEffectRegistration *CScreenSpaceEffectRegistration::s_pHead = NULL;
IScreenSpaceEffectManager *g_pScreenSpaceEffects                 = nullptr;
CScreenSpaceEffectRegistration **g_ppScreenSpaceRegistrationHead = nullptr;
CScreenSpaceEffectRegistration::CScreenSpaceEffectRegistration(
    const char *pName, IScreenSpaceEffect *pEffect)
{
    logging::Info(XORSTR("Creating new effect '%s', head: 0x%08x"), pName,
                  *g_ppScreenSpaceRegistrationHead);
    m_pEffectName                    = pName;
    m_pEffect                        = pEffect;
    m_pNext                          = *g_ppScreenSpaceRegistrationHead;
    *g_ppScreenSpaceRegistrationHead = this;
    logging::Info(XORSTR("New head: 0x%08x"), *g_ppScreenSpaceRegistrationHead);
}

namespace effect_glow
{

static CatVar enable(CV_SWITCH, XORSTR("glow_enable"), XORSTR("0"), XORSTR("Enable"),
                     XORSTR("Main glow switch"));
static CatVar health(CV_SWITCH, XORSTR("glow_health"), XORSTR("0"), XORSTR("Health"),
                     XORSTR("Change glow color based on their health"));
static CatVar teammates(CV_SWITCH, XORSTR("glow_teammates"), XORSTR("0"), XORSTR("Teammates"),
                        XORSTR("Render glow on teammates"));
static CatVar players(CV_SWITCH, XORSTR("glow_players"), XORSTR("1"), XORSTR("Players"),
                      XORSTR("Render glow on player models"));
static CatVar medkits(CV_SWITCH, XORSTR("glow_medkits"), XORSTR("0"), XORSTR("Medkits"),
                      XORSTR("Render glow on medkits"));
static CatVar ammobox(CV_SWITCH, XORSTR("glow_ammo"), XORSTR("0"), XORSTR("Ammoboxes"),
                      XORSTR("Render glow on ammoboxes"));
static CatVar buildings(CV_SWITCH, XORSTR("glow_buildings"), XORSTR("0"), XORSTR("Buildings"),
                        XORSTR("Render glow on buildings"));
static CatVar stickies(CV_SWITCH, XORSTR("glow_stickies"), XORSTR("0"), XORSTR("Stickies"),
                       XORSTR("Render glow on stickybombs"));
static CatVar teammate_buildings(CV_SWITCH, XORSTR("glow_teammate_buildings"), XORSTR("0"),
                                 XORSTR("Teammate Buildings"),
                                 XORSTR("Render glow on teammates buildings"));
static CatVar powerups(CV_SWITCH, XORSTR("glow_powerups"), XORSTR("1"), XORSTR("Powerups"));
static CatVar weapons_white(CV_SWITCH, XORSTR("glow_weapons_white"), XORSTR("1"),
                            XORSTR("White Weapon Glow"), XORSTR("Weapons will glow white"));
static CatVar glowself(CV_SWITCH, XORSTR("glow_self"), XORSTR("0"), XORSTR("Enable glow on self"));
static CatVar rainbow(CV_SWITCH, XORSTR("glow_self_rainbow"), XORSTR("1"),
                      XORSTR("Enable rainbow glow on self"),
                      XORSTR("Only visible in thirdperson!"));
static CatVar glowteam(CV_SWITCH, XORSTR("glow_self_team"), XORSTR("0"), XORSTR("Team chams color"));
static CatVar glowR(CV_INT, XORSTR("glow_self_r"), XORSTR("200"), XORSTR("Self glow red"), XORSTR(""), 0, 255);
static CatVar glowG(CV_INT, XORSTR("glow_self_g"), XORSTR("200"), XORSTR("Self glow green"), XORSTR(""), 0,
                    255);
static CatVar glowB(CV_INT, XORSTR("glow_self_b"), XORSTR("200"), XORSTR("Self glow blue"), XORSTR(""), 0, 255);

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
        fullframe = g_IMaterialSystem->FindTexture(XORSTR("_rt_FullFrameFB"),
                                                   TEXTURE_GROUP_RENDER_TARGET);
        else fullframe = g_IMaterialSystemHL->FindTexture(
            XORSTR("_rt_FullFrameFB"), TEXTURE_GROUP_RENDER_TARGET);
        char *newname    = new char[32];
        std::string name = format(XORSTR("_cathook_buff"), i);
        strncpy(newname, name.c_str(), 30);
        logging::Info(XORSTR("Creating new buffer %d with size %dx%d %s"), i,
                      fullframe->GetActualWidth(), fullframe->GetActualHeight(),
                      newname);

        int textureFlags = TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT |
                           TEXTUREFLAGS_EIGHTBITALPHA;
        int renderTargetFlags = CREATERENDERTARGETFLAGS_HDR;

        ITexture *texture;
        IF_GAME(IsTF2())
        {
            texture = g_IMaterialSystem->CreateNamedRenderTargetTextureEx(
                newname, fullframe->GetActualWidth(),
                fullframe->GetActualHeight(), RT_SIZE_LITERAL,
                IMAGE_FORMAT_RGBA8888, MATERIAL_RT_DEPTH_SEPARATE, textureFlags,
                renderTargetFlags);
        }
        else
        {
            texture = g_IMaterialSystemHL->CreateNamedRenderTargetTextureEx(
                newname, fullframe->GetActualWidth(),
                fullframe->GetActualHeight(), RT_SIZE_LITERAL,
                IMAGE_FORMAT_RGBA8888, MATERIAL_RT_DEPTH_SEPARATE, textureFlags,
                renderTargetFlags);
        }
        delete newname;
        buffers[i].Init(texture);
    }
    return buffers[i];
}

static ShaderStencilState_t SS_NeverSolid{};
static ShaderStencilState_t SS_SolidInvisible{};
static ShaderStencilState_t SS_Null{};
static ShaderStencilState_t SS_Drawing{};

void EffectGlow::Init()
{
    logging::Info(XORSTR("Init Glow..."));
    {
        KeyValues *kv = new KeyValues(XORSTR("UnlitGeneric"));
        kv->SetString(XORSTR("$basetexture"), XORSTR("vgui/white_additive"));
        kv->SetInt(XORSTR("$ignorez"), 0);
        mat_unlit.Init(XORSTR("__cathook_glow_unlit"), kv);
    }
    {
        KeyValues *kv = new KeyValues(XORSTR("UnlitGeneric"));
        kv->SetString(XORSTR("$basetexture"), XORSTR("vgui/white_additive"));
        kv->SetInt(XORSTR("$ignorez"), 1);
        mat_unlit_z.Init(XORSTR("__cathook_glow_unlit_z"), kv);
    }
    // Initialize 2 buffers
    GetBuffer(1);
    GetBuffer(2);
    {
        KeyValues *kv = new KeyValues(XORSTR("UnlitGeneric"));
        kv->SetString(XORSTR("$basetexture"), XORSTR("_cathook_buff1"));
        kv->SetInt(XORSTR("$additive"), 1);
        mat_blit.Init(XORSTR("__cathook_glow_blit"), TEXTURE_GROUP_CLIENT_EFFECTS, kv);
        mat_blit->Refresh();
    }
    {
        KeyValues *kv = new KeyValues(XORSTR("BlurFilterX"));
        kv->SetString(XORSTR("$basetexture"), XORSTR("_cathook_buff1"));
        kv->SetInt(XORSTR("$ignorez"), 1);
        kv->SetInt(XORSTR("$translucent"), 1);
        kv->SetInt(XORSTR("$alphatest"), 1);
        mat_blur_x.Init(XORSTR("_cathook_blurx"), kv);
        mat_blur_x->Refresh();
    }
    {
        KeyValues *kv = new KeyValues(XORSTR("BlurFilterY"));
        kv->SetString(XORSTR("$basetexture"), XORSTR("_cathook_buff2"));
        kv->SetInt(XORSTR("$bloomamount"), 5);
        kv->SetInt(XORSTR("$ignorez"), 1);
        kv->SetInt(XORSTR("$translucent"), 1);
        kv->SetInt(XORSTR("$alphatest"), 1);
        mat_blur_y.Init(XORSTR("_cathook_blury"), kv);
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

    logging::Info(XORSTR("Init done!"));
    init = true;
}

rgba_t EffectGlow::GlowColor(IClientEntity *entity)
{
    static CachedEntity *ent;
    static IClientEntity *owner;

    ent = ENTITY(entity->entindex());
    if (CE_BAD(ent))
        return colors::white;
    if (ent == hacks::shared::aimbot::CurrentTarget())
        return colors::pink;
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
            return colors::Health(ent->m_iHealth(), ent->m_iMaxHealth());
        }
        break;
    case ENTITY_PLAYER:
        if (ent->m_IDX == LOCAL_E->m_IDX && glowteam)
            if (LOCAL_E->m_iTeam() == TEAM_BLU)
                return colors::blu;
            else
                return colors::red;
        if (ent->m_IDX == LOCAL_E->m_IDX && glowself && !rainbow)
            return colors::FromRGBA8(glowR, glowG, glowB, 255);
        if (health && playerlist::IsDefault(ent))
        {
            return colors::Health(ent->m_iHealth(), ent->m_iMaxHealth());
        }
        break;
    }

    return colors::EntityF(ent);
}

bool EffectGlow::ShouldRenderGlow(IClientEntity *entity)
{
    static CachedEntity *ent;

    if (!enable)
        return false;
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
        if (!ent->m_bEnemy() && !(teammate_buildings || teammates))
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
        const auto &type = ent->m_ItemType();
        if (type >= ITEM_HEALTH_SMALL && type <= ITEM_HEALTH_LARGE)
        {
            return medkits;
        }
        else if (type >= ITEM_AMMO_SMALL && type <= ITEM_AMMO_SMALL)
        {
            return ammobox;
        }
        else if (type >= ITEM_POWERUP_FIRST && type <= ITEM_POWERUP_LAST)
        {
            return powerups;
        }
        break;
    }
    return false;
}

void EffectGlow::BeginRenderGlow()
{
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
}

void EffectGlow::EndRenderGlow()
{
    drawing = false;
    CMatRenderContextPtr ptr(GET_RENDER_CONTEXT);
    ptr->DepthRange(0.0f, 1.0f);
    g_IVModelRender->ForcedMaterialOverride(nullptr);
    ptr->PopRenderTargetAndViewport();
}

// https://puu.sh/vobH4/5da8367aef.png
static CatEnum solid_when_enum({ XORSTR("Never"), XORSTR("Always"), XORSTR("Invisible") });
static CatVar blur_scale(CV_INT, XORSTR("glow_blur_scale"), XORSTR("5"), XORSTR("Blur amount"),
                         XORSTR("Ammount to blur the glow"));
static CatVar solid_when(solid_when_enum, XORSTR("glow_solid_when"), XORSTR("0"), XORSTR("Solid when"),
                         XORSTR("Glow will be solid when entity is..."));

void EffectGlow::StartStenciling()
{
    static ShaderStencilState_t state;
    state.Reset();
    state.m_bEnable = true;
    CMatRenderContextPtr ptr(GET_RENDER_CONTEXT);
    switch ((int) solid_when)
    {
    case 0:
        SS_NeverSolid.SetStencilState(ptr);
        break;
    case 2:
        SS_SolidInvisible.SetStencilState(ptr);
        break;
        /*case 3: https://puu.sh/vobH4/5da8367aef.png*/
    }
    if (!solid_when)
    {
        ptr->DepthRange(0.0f, 0.01f);
    }
    else
    {
        ptr->DepthRange(0.0f, 1.0f);
    }
    g_IVRenderView->SetBlend(0.0f);
    mat_unlit->AlphaModulate(1.0f);
    g_IVModelRender->ForcedMaterialOverride(solid_when ? mat_unlit
                                                       : mat_unlit_z);
}

void EffectGlow::EndStenciling()
{
    static ShaderStencilState_t state;
    state.Reset();
    g_IVModelRender->ForcedMaterialOverride(nullptr);
    CMatRenderContextPtr ptr(GET_RENDER_CONTEXT);
    state.SetStencilState(ptr);
    ptr->DepthRange(0.0f, 1.0f);
    g_IVRenderView->SetBlend(1.0f);
}

void EffectGlow::DrawToStencil(IClientEntity *entity)
{
    DrawEntity(entity);
}

void EffectGlow::DrawToBuffer(IClientEntity *entity)
{
}

void EffectGlow::DrawEntity(IClientEntity *entity)
{
    static IClientEntity *attach;
    static int passes;
    passes = 0;

    entity->DrawModel(1);
    attach = g_IEntityList->GetClientEntity(
        *(int *) ((uintptr_t) entity + netvar.m_Collision - 24) & 0xFFF);
    while (attach && passes++ < 32)
    {
        if (attach->ShouldDraw())
        {
            if (weapons_white &&
                entity->GetClientClass()->m_ClassID == RCC_PLAYER &&
                re::C_BaseCombatWeapon::IsBaseCombatWeapon(attach))
            {
                rgba_t mod_original;
                g_IVRenderView->GetColorModulation(mod_original.rgba);
                g_IVRenderView->SetColorModulation(colors::white);
                attach->DrawModel(1);
                g_IVRenderView->SetColorModulation(mod_original.rgba);
            }
            else
                attach->DrawModel(1);
        }
        attach = g_IEntityList->GetClientEntity(
            *(int *) ((uintptr_t) attach + netvar.m_Collision - 20) & 0xFFF);
    }
}

void EffectGlow::RenderGlow(IClientEntity *entity)
{
    CMatRenderContextPtr ptr(GET_RENDER_CONTEXT);
    g_IVRenderView->SetColorModulation(GlowColor(entity));
    g_IVModelRender->ForcedMaterialOverride(mat_unlit_z);
    DrawEntity(entity);
}

void EffectGlow::Render(int x, int y, int w, int h)
{
    static ITexture *orig;
    static IClientEntity *ent;
    static IMaterialVar *blury_bloomamount;

    if (!init)
        Init();
    if (!cathook || (g_IEngine->IsTakingScreenshot() && clean_screenshots) ||
        g_Settings.bInvalid)
        return;
    if (!enable)
        return;
    CMatRenderContextPtr ptr(GET_RENDER_CONTEXT);
    orig = ptr->GetRenderTarget();
    BeginRenderGlow();
    for (int i = 1; i < HIGHEST_ENTITY; i++)
    {
        ent = g_IEntityList->GetClientEntity(i);
        if (ent && !ent->IsDormant() && ShouldRenderGlow(ent))
        {
            RenderGlow(ent);
        }
    }
    EndRenderGlow();
    if ((int) solid_when != 1)
    {
        ptr->ClearStencilBufferRectangle(x, y, w, h, 0);
        StartStenciling();
        for (int i = 1; i < HIGHEST_ENTITY; i++)
        {
            ent = g_IEntityList->GetClientEntity(i);
            if (ent && !ent->IsDormant() && ShouldRenderGlow(ent))
            {
                DrawToStencil(ent);
            }
        }
        EndStenciling();
    }
    ptr->SetRenderTarget(GetBuffer(2));
    ptr->Viewport(x, y, w, h);
    ptr->ClearBuffers(true, false);
    ptr->DrawScreenSpaceRectangle(mat_blur_x, x, y, w, h, 0, 0, w - 1, h - 1, w,
                                  h);
    ptr->SetRenderTarget(GetBuffer(1));
    blury_bloomamount = mat_blur_y->FindVar(XORSTR("$bloomamount"), nullptr);
    blury_bloomamount->SetIntValue((int) blur_scale);
    ptr->DrawScreenSpaceRectangle(mat_blur_y, x, y, w, h, 0, 0, w - 1, h - 1, w,
                                  h);
    ptr->Viewport(x, y, w, h);
    ptr->SetRenderTarget(orig);
    g_IVRenderView->SetBlend(0.0f);
    if ((int) solid_when != 1)
    {
        SS_Drawing.SetStencilState(ptr);
    }
    ptr->DrawScreenSpaceRectangle(mat_blit, x, y, w, h, 0, 0, w - 1, h - 1, w,
                                  h);
    if ((int) solid_when != -1)
    {
        SS_Null.SetStencilState(ptr);
    }
}

EffectGlow g_EffectGlow;
CScreenSpaceEffectRegistration *g_pEffectGlow = nullptr;
}
