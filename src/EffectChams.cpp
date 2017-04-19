/*
 * EffectChams.cpp
 *
 *  Created on: Apr 16, 2017
 *      Author: nullifiedcat
 */

#include "common.h"
#include "EffectChams.hpp"

//static CatVar chams_experimental(CV_SWITCH, "chams_effect", "0", "Experimental Chams");

namespace effect_chams {

CatVar enable(CV_SWITCH, "chams_enable", "0", "Enable");
static CatVar flat(CV_SWITCH, "chams_flat", "0", "Flat");
static CatVar health(CV_SWITCH, "chams_health", "0", "Health");
static CatVar teammates(CV_SWITCH, "chams_teammates", "0", "Teammates");
static CatVar players(CV_SWITCH, "chams_players", "1", "Players");
static CatVar medkits(CV_SWITCH, "chams_medkits", "0", "Medkits");
static CatVar ammobox(CV_SWITCH, "chams_ammo", "0", "Ammoboxes");
static CatVar buildings(CV_SWITCH, "chams_buildings", "0", "Buildings");
static CatVar stickies(CV_SWITCH, "chams_stickies", "0", "Stickies");
static CatVar teammate_buildings(CV_SWITCH, "chams_teammate_buildings", "0", "Teammate Buildings");
static CatVar weapons(CV_SWITCH, "chams_weapons", "1", "Weapons");

void EffectChams::Init() {
	logging::Info("Init EffectChams...");
	{
		KeyValues* kv = new KeyValues("UnlitGeneric");
		kv->SetString("$basetexture", "vgui/white_additive");
		kv->SetInt("$ignorez", 0);
		mat_unlit.Init("__cathook_echams_unlit", kv);
	}
	{
		KeyValues* kv = new KeyValues("UnlitGeneric");
		kv->SetString("$basetexture", "vgui/white_additive");
		kv->SetInt("$ignorez", 1);
		mat_unlit_z.Init("__cathook_echams_unlit_z", kv);
	}
	{
		KeyValues* kv = new KeyValues("VertexLitGeneric");
		kv->SetString("$basetexture", "vgui/white_additive");
		kv->SetInt("$ignorez", 0);
		kv->SetInt("$halflambert", 1);
		mat_lit.Init("__cathook_echams_lit", kv);
	}
	{
		KeyValues* kv = new KeyValues("VertexLitGeneric");
		kv->SetString("$basetexture", "vgui/white_additive");
		kv->SetInt("$ignorez", 1);
		kv->SetInt("$halflambert", 1);
		mat_lit_z.Init("__cathook_echams_lit_z", kv);
	}
	logging::Info("Init done!");
	init = true;
}

void EffectChams::BeginRenderChams() {
	drawing = true;
	CMatRenderContextPtr ptr(GET_RENDER_CONTEXT);
	g_IVRenderView->SetBlend(1.0f);
}

void EffectChams::EndRenderChams() {
	drawing = false;
	CMatRenderContextPtr ptr(GET_RENDER_CONTEXT);
	g_IVModelRender->ForcedMaterialOverride(nullptr);
}

int  EffectChams::ChamsColor(IClientEntity* entity) {
	CachedEntity* ent = ENTITY(entity->entindex());
	if (CE_BAD(ent)) return colors::white;
	if (vfunc<bool(*)(IClientEntity*)>(entity, 0xBE, 0)(entity)) {
		IClientEntity* owner = vfunc<IClientEntity*(*)(IClientEntity*)>(entity, 0x1C3, 0)(entity);
		if (owner) {
			return ChamsColor(owner);
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

bool EffectChams::ShouldRenderChams(IClientEntity* entity) {
	if (!enable) return false;
	if (entity->entindex() < 0) return false;
	CachedEntity* ent = ENTITY(entity->entindex());
	if (CE_BAD(ent)) return false;
	if (weapons && vfunc<bool(*)(IClientEntity*)>(entity, 0xBE, 0)(entity)) {
		IClientEntity* owner = vfunc<IClientEntity*(*)(IClientEntity*)>(entity, 0x1C3, 0)(entity);
		if (owner) {
			return ShouldRenderChams(owner);
		}
	}
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

void EffectChams::RenderChams(int idx) {
	CMatRenderContextPtr ptr(GET_RENDER_CONTEXT);
	IClientEntity* entity = g_IEntityList->GetClientEntity(idx);
	if (entity && !entity->IsDormant()) {
		if (ShouldRenderChams(entity)) {
			int color = ChamsColor(entity);
			unsigned char _b = (color >> 16) & 0xFF;
			unsigned char _g = (color >> 8)  & 0xFF;
			unsigned char _r = (color) & 0xFF;
			float color_1[] = { (float)_r / 255.0f, (float)_g / 255.0f, (float)_b / 255.0f };
			float color_2[] = { color_1[0] * 0.6f, color_1[1] * 0.6f, color_1[2] * 0.6f };
			mat_unlit_z->AlphaModulate(1.0f);
			ptr->DepthRange(0.0f, 0.01f);
			g_IVRenderView->SetColorModulation(color_1);
			g_IVModelRender->ForcedMaterialOverride(flat ? mat_unlit_z : mat_lit_z);
			entity->DrawModel(1);
			//((DrawModelExecute_t)(hooks::hkIVModelRender->GetMethod(hooks::offDrawModelExecute)))(_this, state, info, matrix);
			mat_unlit->AlphaModulate(1.0f);
			g_IVRenderView->SetColorModulation(color_2);
			ptr->DepthRange(0.0f, 1.0f);
			g_IVModelRender->ForcedMaterialOverride(flat ? mat_unlit : mat_lit);
			entity->DrawModel(1);
		}
	}
}

void EffectChams::Render(int x, int y, int w, int h) {
	if (!init) Init();
	if (!cathook || (g_IEngine->IsTakingScreenshot() && clean_screenshots)) return;
	if (!enable) return;
	CMatRenderContextPtr ptr(GET_RENDER_CONTEXT);
	BeginRenderChams();
	for (int i = 1; i < HIGHEST_ENTITY; i++) {
		IClientEntity* ent = g_IEntityList->GetClientEntity(i);
		if (ent && !ent->IsDormant()) {
			RenderChams(i);
		}
	}
	EndRenderChams();
}

EffectChams g_EffectChams;
CScreenSpaceEffectRegistration* g_pEffectChams = nullptr;

}
