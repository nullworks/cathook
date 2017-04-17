/*
 * Chams.cpp
 *
 *  Created on: Apr 15, 2017
 *      Author: nullifiedcat
 */

#include "../common.h"

namespace hacks { namespace shared { namespace chams {

/*CatVar enable(CV_SWITCH, "chams_enable", "0", "Enable");
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

static bool init = false;

CMaterialReference mat_unlit;
CMaterialReference mat_unlit_z;
CMaterialReference mat_lit;
CMaterialReference mat_lit_z;

void Init() {
	{
		KeyValues* kv = new KeyValues("UnlitGeneric");
		kv->SetString("$basetexture", "vgui/white_additive");
		kv->SetInt("$ignorez", 0);
		mat_unlit.Init("__cathook_chams_unlit", kv);
	}
	{
		KeyValues* kv = new KeyValues("UnlitGeneric");
		kv->SetString("$basetexture", "vgui/white_additive");
		kv->SetInt("$ignorez", 1);
		mat_unlit_z.Init("__cathook_chams_unlit_z", kv);
	}
	{
		KeyValues* kv = new KeyValues("VertexLitGeneric");
		kv->SetString("$basetexture", "vgui/white_additive");
		kv->SetInt("$ignorez", 0);
		kv->SetInt("$halflambert", 1);
		mat_lit.Init("__cathook_chams_lit", kv);
	}
	{
		KeyValues* kv = new KeyValues("VertexLitGeneric");
		kv->SetString("$basetexture", "vgui/white_additive");
		kv->SetInt("$ignorez", 1);
		kv->SetInt("$halflambert", 1);
		mat_lit_z.Init("__cathook_chams_lit_z", kv);
	}
	init = true;
}

int GetChamColor(int idx) {
	if (idx < 0 || idx > g_IEntityList->GetHighestEntityIndex()) return colors::white;
	CachedEntity* ent = ENTITY(idx);
	if (CE_BAD(ent)) return colors::white;
	IClientEntity* entity = RAW_ENT(ent);
	if (vfunc<bool(*)(IClientEntity*)>(entity, 0xBE, 0)(entity)) {
		IClientEntity* owner = vfunc<IClientEntity*(*)(IClientEntity*)>(entity, 0x1C3, 0)(entity);
		if (owner) {
			return GetChamColor(owner->entindex());
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

bool ShouldCham(int idx) {
	if (idx < 0 || idx > HIGHEST_ENTITY) return false;
	CachedEntity* ent = ENTITY(idx);
	if (CE_BAD(ent)) return false;
	IClientEntity* entity = RAW_ENT(ent);
	if (weapons && vfunc<bool(*)(IClientEntity*)>(entity, 0xBE, 0)(entity)) {
		IClientEntity* owner = vfunc<IClientEntity*(*)(IClientEntity*)>(entity, 0x1C3, 0)(entity);
		if (owner) {
			return ShouldCham(owner->entindex());
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

void DrawModelExecute(IVModelRender* _this, const DrawModelState_t& state, const ModelRenderInfo_t& info, matrix3x4_t* matrix) {
	if (!enable) return;
	if (!init) {
		Init();
	}
	//std::string name(g_IModelInfo->GetModelName(info.pModel));
	IClientUnknown* unknown = info.pRenderable->GetIClientUnknown();
	if (unknown) {
		IClientEntity* entity = unknown->GetIClientEntity();
		//logging::Info("entity: 0x%08x", entity);
		if (entity && !entity->IsDormant()) {
			if (ShouldCham(entity->entindex())) {
				int color = GetChamColor(entity->entindex());
				unsigned char _b = (color >> 16) & 0xFF;
				unsigned char _g = (color >> 8)  & 0xFF;
				unsigned char _r = (color) & 0xFF;
				float color_1[] = { (float)_r / 255.0f, (float)_g / 255.0f, (float)_b / 255.0f };
				float color_2[] = { color_1[0] * 0.6f, color_1[1] * 0.6f, color_1[2] * 0.6f };
				mat_unlit_z->AlphaModulate(1.0f);
				g_IVRenderView->SetColorModulation(color_1);
				g_IVModelRender->ForcedMaterialOverride(flat ? mat_unlit_z : mat_lit_z);
				((DrawModelExecute_t)(hooks::hkIVModelRender->GetMethod(hooks::offDrawModelExecute)))(_this, state, info, matrix);
				mat_unlit->AlphaModulate(1.0f);
				g_IVRenderView->SetColorModulation(color_2);
				g_IVModelRender->ForcedMaterialOverride(flat ? mat_unlit : mat_lit);
			}
			//logging::Info("lit player");
			// IsBaseCombatWeapon
			/*if (vfunc<bool(*)(IClientEntity*)>(entity, 0xBE, 0)(entity)) {
				IClientEntity* owner = vfunc<IClientEntity*(*)(IClientEntity*)>(entity, 0x1C3, 0)(entity);
				if (owner) {
					int color = colors::EntityF(ENTITY(owner->entindex()));
					unsigned char _b = (color >> 16) & 0xFF;
					unsigned char _g = (color >> 8)  & 0xFF;
					unsigned char _r = (color) & 0xFF;
					float color_1[] = { (float)_r / 255.0f, (float)_g / 255.0f, (float)_b / 255.0f };
					float color_2[] = { color_1[0] * 0.6f, color_1[1] * 0.6f, color_1[2] * 0.6f };
					mat_unlit_z->AlphaModulate(1.0f);
					g_IVRenderView->SetColorModulation(color_1);
					g_IVModelRender->ForcedMaterialOverride(flat ? mat_unlit_z : mat_lit_z);
					((DrawModelExecute_t)(hooks::hkIVModelRender->GetMethod(hooks::offDrawModelExecute)))(_this, state, info, matrix);
					mat_unlit->AlphaModulate(1.0f);
					g_IVRenderView->SetColorModulation(color_2);
					g_IVModelRender->ForcedMaterialOverride(flat ? mat_unlit : mat_lit);
				}
			} else if (entity->GetClientClass()->m_ClassID == g_pClassID->C_Player) {
				int color = colors::EntityF(ENTITY(entity->entindex()));
				unsigned char _b = (color >> 16) & 0xFF;
				unsigned char _g = (color >> 8)  & 0xFF;
				unsigned char _r = (color) & 0xFF;
				float color_1[] = { (float)_r / 255.0f, (float)_g / 255.0f, (float)_b / 255.0f };
				float color_2[] = { color_1[0] * 0.6f, color_1[1] * 0.6f, color_1[2] * 0.6f };
				mat_unlit_z->AlphaModulate(1.0f);
				g_IVRenderView->SetColorModulation(color_1);
				g_IVModelRender->ForcedMaterialOverride(flat ? mat_unlit_z : mat_lit_z);
				((DrawModelExecute_t)(hooks::hkIVModelRender->GetMethod(hooks::offDrawModelExecute)))(_this, state, info, matrix);
				mat_unlit->AlphaModulate(1.0f);
				g_IVRenderView->SetColorModulation(color_2);
				g_IVModelRender->ForcedMaterialOverride(flat ? mat_unlit : mat_lit);
			}


		}
	}
}*/

}}}
