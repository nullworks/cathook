/*
 * EffectGlow.cpp
 *
 *  Created on: Apr 13, 2017
 *      Author: nullifiedcat
 */

#include "common.h"
#include "EffectGlow.hpp"

IMaterialSystem* materials = nullptr;

CScreenSpaceEffectRegistration *CScreenSpaceEffectRegistration::s_pHead = NULL;
IScreenSpaceEffectManager* g_pScreenSpaceEffects = nullptr;
CScreenSpaceEffectRegistration** g_ppScreenSpaceRegistrationHead = nullptr;
CScreenSpaceEffectRegistration::CScreenSpaceEffectRegistration( const char *pName, IScreenSpaceEffect *pEffect )
{
	logging::Info("Creating new effect '%s', head: 0x%08x", pName, *g_ppScreenSpaceRegistrationHead);
	m_pEffectName = pName;
	m_pEffect = pEffect;
	m_pNext = *g_ppScreenSpaceRegistrationHead;
	*g_ppScreenSpaceRegistrationHead = this;
	logging::Info("New head: 0x%08x", *g_ppScreenSpaceRegistrationHead);
}

namespace effect_glow {

CatVar enable(CV_SWITCH, "glow_enable", "0", "Enable");
static CatVar health(CV_SWITCH, "glow_health", "0", "Health");
static CatVar teammates(CV_SWITCH, "glow_teammates", "0", "Teammates");
static CatVar players(CV_SWITCH, "glow_players", "1", "Players");
static CatVar medkits(CV_SWITCH, "glow_medkits", "0", "Medkits");
static CatVar ammobox(CV_SWITCH, "glow_ammo", "0", "Ammoboxes");
static CatVar buildings(CV_SWITCH, "glow_buildings", "0", "Buildings");
static CatVar stickies(CV_SWITCH, "glow_stickies", "0", "Stickies");
static CatVar teammate_buildings(CV_SWITCH, "glow_teammate_buildings", "0", "Teammate Buildings");


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

	ShaderStencilState_t() {
		m_bEnable = false;
		m_PassOp = m_FailOp = m_ZFailOp = STENCILOPERATION_KEEP;
		m_CompareFunc = STENCILCOMPARISONFUNCTION_ALWAYS;
		m_nReferenceValue = 0;
		m_nTestMask = m_nWriteMask = 0xFFFFFFFF;
	}

	void SetStencilState(CMatRenderContextPtr &pRenderContext) const {
		pRenderContext->SetStencilEnable( m_bEnable );
		pRenderContext->SetStencilFailOperation( m_FailOp );
		pRenderContext->SetStencilZFailOperation( m_ZFailOp );
		pRenderContext->SetStencilPassOperation( m_PassOp );
		pRenderContext->SetStencilCompareFunction( m_CompareFunc );
		pRenderContext->SetStencilReferenceValue( m_nReferenceValue );
		pRenderContext->SetStencilTestMask( m_nTestMask );
		pRenderContext->SetStencilWriteMask( m_nWriteMask );
	}
};

static CTextureReference buffers[4] {};

ITexture* GetBuffer(int i) {
	if (!buffers[i]) {
		ITexture* fullframe;
		if (TF2)
			fullframe = g_IMaterialSystem->FindTexture("_rt_FullFrameFB", TEXTURE_GROUP_RENDER_TARGET);
		else
			fullframe = g_IMaterialSystemHL->FindTexture("_rt_FullFrameFB", TEXTURE_GROUP_RENDER_TARGET);
		char* newname = new char[32];
		std::string name = format("_cathook_buff", i);
		strncpy(newname, name.c_str(), 30);
		logging::Info("Creating new buffer %d with size %dx%d %s", i, fullframe->GetActualWidth(), fullframe->GetActualHeight(), newname);

		int textureFlags = TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT | TEXTUREFLAGS_EIGHTBITALPHA;
		int renderTargetFlags = CREATERENDERTARGETFLAGS_HDR;

		ITexture* texture;
		if (TF2) {
			texture = g_IMaterialSystem->CreateNamedRenderTargetTextureEx( newname, fullframe->GetActualWidth(), fullframe->GetActualHeight(), RT_SIZE_LITERAL, IMAGE_FORMAT_RGBA8888,
					MATERIAL_RT_DEPTH_SEPARATE, textureFlags, renderTargetFlags );
		} else {
			texture = g_IMaterialSystemHL->CreateNamedRenderTargetTextureEx( newname, fullframe->GetActualWidth(), fullframe->GetActualHeight(), RT_SIZE_LITERAL, IMAGE_FORMAT_RGBA8888,
					MATERIAL_RT_DEPTH_SEPARATE, textureFlags, renderTargetFlags );
		}

		buffers[i].Init(texture);
	}
	return buffers[i];
}

static ShaderStencilState_t SS_NeverSolid {};
static ShaderStencilState_t SS_SolidInvisible {};
static ShaderStencilState_t SS_Null {};
static ShaderStencilState_t SS_Drawing {};

void EffectGlow::Init() {
	logging::Info("Init Glow...");
	{
		KeyValues* kv = new KeyValues("UnlitGeneric");
		kv->SetString("$basetexture", "vgui/white_additive");
		kv->SetInt("$ignorez", 0);
		mat_unlit.Init("__cathook_glow_unlit", kv);
	}
	{
		KeyValues* kv = new KeyValues("UnlitGeneric");
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
		KeyValues* kv = new KeyValues("BlurFilterX");
		kv->SetString("$basetexture", "_cathook_buff1");
		kv->SetInt("$ignorez", 1);
		kv->SetInt("$translucent", 1);
		kv->SetInt("$alphatest", 1);
		mat_blur_x.Init("_cathook_blurx", kv);
		mat_blur_x->Refresh();
	}
	{
		KeyValues* kv = new KeyValues("BlurFilterY");
		kv->SetString("$basetexture", "_cathook_buff2");
		kv->SetInt("$bloomamount", 5);
		kv->SetInt("$ignorez", 1);
		kv->SetInt("$translucent", 1);
		kv->SetInt("$alphatest", 1);
		mat_blur_y.Init("_cathook_blury", kv);
		mat_blur_y->Refresh();
	}
	{
		SS_NeverSolid.m_bEnable = true;
		SS_NeverSolid.m_PassOp = STENCILOPERATION_REPLACE;
		SS_NeverSolid.m_FailOp = STENCILOPERATION_KEEP;
		SS_NeverSolid.m_ZFailOp = STENCILOPERATION_KEEP;
		SS_NeverSolid.m_CompareFunc = STENCILCOMPARISONFUNCTION_ALWAYS;
		SS_NeverSolid.m_nWriteMask = 1;
		SS_NeverSolid.m_nReferenceValue = 1;
	}
	{
		SS_SolidInvisible.m_bEnable = true;
		SS_SolidInvisible.m_PassOp = STENCILOPERATION_REPLACE;
		SS_SolidInvisible.m_FailOp = STENCILOPERATION_KEEP;
		SS_SolidInvisible.m_ZFailOp = STENCILOPERATION_KEEP;
		SS_SolidInvisible.m_CompareFunc = STENCILCOMPARISONFUNCTION_ALWAYS;
		SS_SolidInvisible.m_nWriteMask = 1;
		SS_SolidInvisible.m_nReferenceValue = 1;
	}
	/*case 3: https://puu.sh/vobH4/5da8367aef.png*/
	{
		SS_Drawing.m_bEnable = true;
		SS_Drawing.m_nReferenceValue = 0;
		SS_Drawing.m_nTestMask = 1;
		SS_Drawing.m_CompareFunc = STENCILCOMPARISONFUNCTION_EQUAL;
		SS_Drawing.m_PassOp = STENCILOPERATION_ZERO;
	}


	logging::Info("Init done!");
	init = true;
}

int  EffectGlow::GlowColor(IClientEntity* entity) {
	CachedEntity* ent = ENTITY(entity->entindex());
	if (CE_BAD(ent)) return colors::white;
	if (ent == hacks::shared::aimbot::target_highest) return colors::pink;
	if (vfunc<bool(*)(IClientEntity*)>(entity, 0xBE, 0)(entity)) {
		IClientEntity* owner = vfunc<IClientEntity*(*)(IClientEntity*)>(entity, 0x1C3, 0)(entity);
		if (owner) {
			return GlowColor(owner);
		}
	}
	switch (ent->m_Type) {
	case ENTITY_BUILDING:
		if (!ent->m_bEnemy && !(teammates || teammate_buildings)) {
			return 0;
		}
		if (health) {
			return colors::Health(ent->m_iHealth, ent->m_iMaxHealth);
		}
		break;
	case ENTITY_PLAYER:
		if (!players) return 0;
		if (!ent->m_bEnemy && !teammates) return 0;
		if (health) {
			return colors::Health(ent->m_iHealth, ent->m_iMaxHealth);
		}
		break;
	}
	return colors::EntityF(ent);
}

bool EffectGlow::ShouldRenderGlow(IClientEntity* entity) {
	if (!enable) return false;
	if (entity->entindex() < 0) return false;
	CachedEntity* ent = ENTITY(entity->entindex());
	if (CE_BAD(ent)) return false;
	/*if (weapons && vfunc<bool(*)(IClientEntity*)>(entity, 0xBE, 0)(entity)) {
		IClientEntity* owner = vfunc<IClientEntity*(*)(IClientEntity*)>(entity, 0x1C3, 0)(entity);
		if (owner) {
			return ShouldRenderChams(owner);
		}
	}*/
	switch (ent->m_Type) {
	case ENTITY_BUILDING:
		if (!buildings) return false;
		if (!ent->m_bEnemy && !(teammate_buildings || teammates)) return false;
		return true;
	case ENTITY_PLAYER:
		if (!players) return false;
		if (!teammates && !ent->m_bEnemy) return false;
		if (CE_BYTE(ent, netvar.iLifeState) != LIFE_ALIVE) return false;
		return true;
		break;
	case ENTITY_PROJECTILE:
		if (!ent->m_bEnemy) return false;
		if (stickies && ent->m_iClassID == g_pClassID->CTFGrenadePipebombProjectile) {
			return true;
		}
		break;
	case ENTITY_GENERIC:
		switch (ent->m_ItemType) {
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

void EffectGlow::BeginRenderGlow() {
	drawing = true;
	CMatRenderContextPtr ptr(GET_RENDER_CONTEXT);
	ptr->ClearColor4ub(0, 0, 0, 0);
	ptr->PushRenderTargetAndViewport();
	ptr->SetRenderTarget(GetBuffer(1));
	ptr->OverrideAlphaWriteEnable( true, true );
	g_IVRenderView->SetBlend(0.99f);
	ptr->ClearBuffers(true, false);
	mat_unlit_z->AlphaModulate(1.0f);
	ptr->DepthRange(0.0f, 0.01f);
}

void EffectGlow::EndRenderGlow() {
	drawing = false;
	CMatRenderContextPtr ptr(GET_RENDER_CONTEXT);
	ptr->DepthRange(0.0f, 1.0f);
	g_IVModelRender->ForcedMaterialOverride(nullptr);
	ptr->PopRenderTargetAndViewport();
}

// https://puu.sh/vobH4/5da8367aef.png
static CatEnum solid_when_enum({"Never", "Always", "Invisible"});
static CatVar blur_scale(CV_INT, "glow_blur_scale", "5", "Blur amount");
static CatVar solid_when(solid_when_enum, "glow_solid_when", "0", "Solid when", "Glow will be solid when entity is...");

void EffectGlow::StartStenciling() {
	ShaderStencilState_t state;
	state.m_bEnable = true;
	CMatRenderContextPtr ptr(GET_RENDER_CONTEXT);
	switch ((int)solid_when) {
	case 0:
		SS_NeverSolid.SetStencilState(ptr);
		break;
	case 2:
		SS_SolidInvisible.SetStencilState(ptr);
		break;
	/*case 3: https://puu.sh/vobH4/5da8367aef.png*/
	}
	if (!solid_when) {
		ptr->DepthRange(0.0f, 0.01f);
	} else {
		ptr->DepthRange(0.0f, 1.0f);
	}
	g_IVRenderView->SetBlend(0.0f);
	mat_unlit->AlphaModulate(1.0f);
	g_IVModelRender->ForcedMaterialOverride(solid_when ? mat_unlit : mat_unlit_z);
}

void EffectGlow::EndStenciling() {
	g_IVModelRender->ForcedMaterialOverride(nullptr);
	CMatRenderContextPtr ptr(GET_RENDER_CONTEXT);
	ShaderStencilState_t state {};
	state.SetStencilState(ptr);
	ptr->DepthRange(0.0f, 1.0f);
	g_IVRenderView->SetBlend(1.0f);
}

void EffectGlow::DrawToStencil(IClientEntity* entity) {
	DrawEntity(entity);
}

void EffectGlow::DrawToBuffer(IClientEntity* entity) {

}

void EffectGlow::DrawEntity(IClientEntity* entity) {
	entity->DrawModel(1);
	IClientEntity* attach = g_IEntityList->GetClientEntity(*(int*)((uintptr_t)entity + netvar.m_Collision - 24) & 0xFFF);
	int passes = 0;
	while (attach && passes++ < 32) {
		if (attach->ShouldDraw()) {
			attach->DrawModel(1);
		}
		attach = g_IEntityList->GetClientEntity(*(int*)((uintptr_t)attach + netvar.m_Collision - 20) & 0xFFF);
	}
}

void EffectGlow::RenderGlow(IClientEntity* entity) {
	CMatRenderContextPtr ptr(GET_RENDER_CONTEXT);
	int color = GlowColor(entity);
	unsigned char _b = (color >> 16) & 0xFF;
	unsigned char _g = (color >> 8)  & 0xFF;
	unsigned char _r = (color) & 0xFF;
	float color_1[] = { (float)_r / 255.0f, (float)_g / 255.0f, (float)_b / 255.0f };
	g_IVRenderView->SetColorModulation(color_1);
	g_IVModelRender->ForcedMaterialOverride(mat_unlit_z);
	DrawEntity(entity);
}

void EffectGlow::Render(int x, int y, int w, int h) {
	if (!init) Init();
	if (!cathook || (g_IEngine->IsTakingScreenshot() && clean_screenshots) || g_Settings.bInvalid) return;
	if (!enable) return;
	CMatRenderContextPtr ptr(GET_RENDER_CONTEXT);
	ITexture* orig = ptr->GetRenderTarget();
	BeginRenderGlow();
	for (int i = 1; i < HIGHEST_ENTITY; i++) {
		IClientEntity* ent = g_IEntityList->GetClientEntity(i);
		if (ent && !ent->IsDormant() && ShouldRenderGlow(ent)) {
			RenderGlow(ent);
		}
	}
	EndRenderGlow();
	if ((int)solid_when != 1) {
		ptr->ClearStencilBufferRectangle(x, y, w, h, 0);
		StartStenciling();
		for (int i = 1; i < HIGHEST_ENTITY; i++) {
			IClientEntity* ent = g_IEntityList->GetClientEntity(i);
			if (ent && !ent->IsDormant() && ShouldRenderGlow(ent)) {
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
	IMaterialVar* blury_bloomamount = mat_blur_y->FindVar("$bloomamount", nullptr);
	blury_bloomamount->SetIntValue((int)blur_scale);
	ptr->DrawScreenSpaceRectangle(mat_blur_y, x, y, w, h, 0, 0, w - 1, h - 1, w, h);
	ptr->Viewport(x, y, w, h);
	ptr->SetRenderTarget(orig);
	g_IVRenderView->SetBlend(0.0f);
	if ((int)solid_when != 1) {
		SS_Drawing.SetStencilState(ptr);
	}
	ptr->DrawScreenSpaceRectangle(mat_blit, x, y, w, h, 0, 0, w - 1, h - 1, w, h);
	if ((int)solid_when != -1) {
		SS_Null.SetStencilState(ptr);
	}
}

EffectGlow g_EffectGlow;
CScreenSpaceEffectRegistration* g_pEffectGlow = nullptr;

}
