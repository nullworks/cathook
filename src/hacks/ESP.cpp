/*
 * HEsp.cpp
 *
 *  Created on: Oct 6, 2016
 *      Author: nullifiedcat
 */

#include "ESP.h"

#include "../common.h"
#include "../sdk.h"

namespace hacks { namespace shared { namespace esp {

CatVar show_weapon(CV_SWITCH, "esp_weapon", "1", "Show weapon name", "Show which weapon does the enemy use");
CatVar local_esp(CV_SWITCH, "esp_local", "1", "ESP Local Player", "Shows local player ESP in thirdperson");
CatVar buildings(CV_SWITCH, "esp_buildings", "1", "Building ESP", "Show buildings");
CatVar enabled(CV_SWITCH, "esp_enabled", "0", "ESP", "Master ESP switch");
CatVar entity_info(CV_SWITCH, "esp_entity", "0", "Entity ESP", "Show entity info (debug)");
CatVar teammates(CV_SWITCH, "esp_teammates", "0", "ESP Teammates", "Teammate ESP");
CatVar item_esp(CV_SWITCH, "esp_item", "1", "Item ESP", "Master Item ESP switch (health packs, etc.)");
CatVar show_bot_id(CV_SWITCH, "esp_followbot_id", "1", "Followbot ESP", "Show followbot ID");
CatVar item_dropped_weapons(CV_SWITCH, "esp_item_weapons", "0", "Dropped weapons", "Show dropped weapons");
CatVar item_ammo_packs(CV_SWITCH, "esp_item_ammo", "0", "Ammo packs", "Show ammo packs");
CatVar item_health_packs(CV_SWITCH, "esp_item_health", "1", "Health packs", "Show health packs");
CatVar item_powerups(CV_SWITCH, "esp_item_powerups", "1", "Powerups", "Show powerups");
CatVar item_money(CV_SWITCH, "esp_money", "1", "MvM money", "Show MvM money");
CatVar item_money_red(CV_SWITCH, "esp_money_red", "1", "Red MvM money", "Show red MvM money");
CatVar entity_id(CV_SWITCH, "esp_entity_id", "1", "Entity ID", "Used with Entity ESP. Shows entityID");
CatVar tank(CV_SWITCH, "esp_show_tank", "1", "Show tank", "Show tank");
CatVar box_esp(CV_SWITCH, "esp_box", "1", "Box", "Draw 2D box with healthbar. fancy.");
CatVar show_distance(CV_SWITCH, "esp_distance", "1", "Distance ESP", "Show distance to target");
CatVar show_name(CV_SWITCH, "esp_name", "1", "Name ESP", "Show name");
CatVar show_class(CV_SWITCH, "esp_class", "1", "Class ESP", "Show class");
CatVar show_conditions(CV_SWITCH, "esp_conds", "1", "Conditions ESP", "Show conditions");
CatVar vischeck(CV_SWITCH, "esp_vischeck", "1", "VisCheck", "ESP visibility check - makes enemy info behind walls darker, disable this if you get FPS drops");
CatVar legit(CV_SWITCH, "esp_legit", "0", "Legit Mode", "Don't show invisible enemies");
CatVar show_health(CV_SWITCH, "esp_health_num", "1", "Health numbers", "Show health in numbers");
CatEnum proj_esp_enum({ "OFF", "ALL", "CRIT" });
CatVar proj_rockets(proj_esp_enum, "esp_proj_rockets", "1", "Rockets", "Rockets");
CatVar proj_arrows(proj_esp_enum, "esp_proj_arrows", "1", "Arrows", "Arrows");
CatVar proj_pipes(proj_esp_enum, "esp_proj_pipes", "1", "Pipes", "Pipebombs");
CatVar proj_stickies(proj_esp_enum, "esp_proj_stickies", "1", "Stickies", "Stickybombs");
CatVar proj_enemy(CV_SWITCH, "esp_proj_enemy", "1", "Only enemy projectiles", "Don't show friendly projectiles");
CatVar proj_esp(CV_SWITCH, "esp_proj", "1", "Projectile ESP", "Projectile ESP");
CatVar entity_model(CV_SWITCH, "esp_model_name", "0", "Model name ESP", "Model name esp (DEBUG ONLY)");
CatVar item_weapon_spawners(CV_SWITCH, "esp_weapon_spawners", "1", "Show weapon spawners", "TF2C deathmatch weapon spawners");
CatVar item_adrenaline(CV_SWITCH, "esp_item_adrenaline", "0", "Show Adrenaline", "TF2C adrenaline pills");

std::array<ESPData, 2048> data;

void ResetEntityStrings() {
	for (auto& i : data) {
		i.string_count = 0;
		i.color = 0;
	}
}

void SetEntityColor(CachedEntity* entity, int color) {
	data[entity->m_IDX].color = color;
}

void AddEntityString(CachedEntity* entity, const std::string& string, int color) {
	ESPData& entity_data = data[entity->m_IDX];
	entity_data.strings[entity_data.string_count].data = string;
	entity_data.strings[entity_data.string_count].color = color;
	entity_data.string_count++;
}

void CreateMove() {
	static int limit;
	static int max_clients = g_IEngine->GetMaxClients();
	static CachedEntity* ent;

	ResetEntityStrings();
	limit = HIGHEST_ENTITY;
	if (!buildings && !proj_esp && !item_esp) limit = min(max_clients, HIGHEST_ENTITY);
	for (int i = 0; i < limit; i++) {
		ent = ENTITY(i);
		ProcessEntity(ent);
		if (data[i].string_count) {
			SetEntityColor(ent, colors::EntityF(ent));
			if (show_distance) {
				AddEntityString(ent, format((int)(ENTITY(i)->m_flDistance / 64 * 1.22f), 'm'));
			}
		}
	}
}

void Draw() {
	static int limit;
	static int max_clients = g_IEngine->GetMaxClients();
	limit = HIGHEST_ENTITY;
	if (!buildings && !proj_esp && !item_esp) limit = min(max_clients, HIGHEST_ENTITY);
	for (int i = 0; i < limit; i++) {
		ProcessEntityPT(ENTITY(i));
	}
}

static CatEnum esp_box_text_position_enum({"TOP RIGHT", "BOTTOM RIGHT", "CENTER", "ABOVE", "BELOW" });
static CatVar esp_box_text_position(esp_box_text_position_enum, "esp_box_text_position", "0", "Text position", "Defines text position");
static CatVar esp_3d_box_health(CV_SWITCH, "esp_3d_box_health", "1", "3D box health color", "Adds a health bar to 3d esp box");
static CatVar esp_3d_box_thick(CV_SWITCH, "esp_3d_box_thick", "1", "Thick 3D box", "Makes the 3d box thicker\nMost times, easier to see");
static CatVar esp_3d_box_expand_rate(CV_FLOAT, "esp_3d_box_expand_size", "10", "3D Box Expand Size", "Expand 3D box by X units", 50.0f);
static CatVar esp_3d_box_expand(CV_SWITCH, "esp_3d_box_expand", "1", "Expand 3D box", "Makes the 3d bigger");
static CatEnum esp_3d_box_smoothing_enum({"None", "Origin offset", "Bone update (NOT IMPL)"});
static CatVar esp_3d_box_smoothing(esp_3d_box_smoothing_enum, "esp_3d_box_smoothing", "1", "3D box smoothing", "3D box smoothing method");
static CatVar esp_3d_box_nodraw(CV_SWITCH, "esp_3d_box_nodraw", "0", "Invisible 3D box", "Don't draw 3d box");
static CatVar esp_3d_box_healthbar(CV_SWITCH, "esp_3d_box_healthbar", "1", "Health bar", "Adds a health bar to the esp");

void Draw3DBox(CachedEntity* ent, int clr, bool healthbar, int health, int healthmax) {
	static Vector mins, maxs;
	static Vector points_r[8];
	static Vector points[8];
	static bool set, success, cloak;
	static float x, y, z;
	static int hbh, max_x, max_y, min_x, min_y;
	static CachedHitbox* hb;

	set = false;
	success = true;

	for (int i = 0; i < ent->hitboxes.GetNumHitboxes(); i++) {
		hb = ent->hitboxes.GetHitbox(i);
		if (!hb) return;
		if (!set || hb->min.x < mins.x) mins.x = hb->min.x;
		if (!set || hb->min.y < mins.y) mins.y = hb->min.y;
		if (!set || hb->min.z < mins.z) mins.z = hb->min.z;
		if (!set || hb->max.x > maxs.x) maxs.x = hb->max.x;
		if (!set || hb->max.y > maxs.y) maxs.y = hb->max.y;
		if (!set || hb->max.z > maxs.z) maxs.z = hb->max.z;
		set = true;
	}

	// This is weird, smoothing only makes it worse on local servers
	if ((int)esp_3d_box_smoothing == 1) {
		mins += (RAW_ENT(ent)->GetAbsOrigin() - ent->m_vecOrigin);
		maxs += (RAW_ENT(ent)->GetAbsOrigin() - ent->m_vecOrigin);
	}

	//static const Vector expand_vector(8.5, 8.5, 8.5);

	if (esp_3d_box_expand) {
		const float& exp = (float)esp_3d_box_expand_rate;
		maxs.x += exp;
		maxs.y += exp;
		maxs.z += exp;
		mins.x -= exp;
		mins.y -= exp;
		mins.z -= exp;
	}

	x = maxs.x - mins.x;
	y = maxs.y - mins.y;
	z = maxs.z - mins.z;
	points_r[0] = mins;
	points_r[1] = mins + Vector(x, 0, 0);
	points_r[2] = mins + Vector(x, y, 0);
	points_r[3] = mins + Vector(0, y, 0);
	points_r[4] = mins + Vector(0, 0, z);
	points_r[5] = mins + Vector(x, 0, z);
	points_r[6] = mins + Vector(x, y, z);
	points_r[7] = mins + Vector(0, y, z);
	success = true;
	for (int i = 0; i < 8; i++) {
		if (!draw::WorldToScreen(points_r[i], points[i])) success = false;
	}
	if (!success) return;
	max_x = -1;
	max_y = -1;
	min_x = 65536;
	min_y = 65536;
	for (int i = 0; i < 8; i++) {
		if (points[i].x > max_x) max_x = points[i].x;
		if (points[i].y > max_y) max_y = points[i].y;
		if (points[i].x < min_x) min_x = points[i].x;
		if (points[i].y < min_y) min_y = points[i].y;
	}
	data.at(ent->m_IDX).esp_origin.Zero();
	switch ((int)esp_box_text_position) {
	case 0: { // TOP RIGHT
		data.at(ent->m_IDX).esp_origin = Vector(max_x + 1, min_y, 0);
	} break;
	case 1: { // BOTTOM RIGHT
		data.at(ent->m_IDX).esp_origin = Vector(max_x + 1, max_y - data.at(ent->m_IDX).string_count * ((int)fonts::esp_height - 3), 0);
	} break;
	case 2: { // CENTER
	} break;
	case 3: { // ABOVE
		data.at(ent->m_IDX).esp_origin = Vector(min_x + 1, min_y - data.at(ent->m_IDX).string_count * ((int)fonts::esp_height - 3), 0);
	} break;
	case 4: { // BELOW
		data.at(ent->m_IDX).esp_origin = Vector(min_x + 1, max_y, 0);
	}
	}
	//draw::String(fonts::ESP, points[0].x, points[0].y, clr, 1, "MIN");
	//draw::String(fonts::ESP, points[6].x, points[6].y, clr, 1, "MAX");
	constexpr int indices[][2] = {
		{ 0, 1 }, { 0, 3 }, { 1, 2 }, { 2, 3 },
		{ 4, 5 }, { 4, 7 }, { 5, 6 }, { 6, 7 },
		{ 0, 4 }, { 1, 5 }, { 2, 6 }, { 3, 7 }
	};

	if (esp_3d_box_health) clr = colors::Health(health, healthmax);
	cloak = ent->m_iClassID == g_pClassID->C_Player && IsPlayerInvisible(ent);
	if (cloak) clr = colors::Transparent(clr, 0.2);
	if (esp_3d_box_healthbar) {
		draw::OutlineRect(min_x - 6, min_y, 6, max_y - min_y, colors::black);
		hbh = (max_y - min_y - 2) * min((float)health / (float)healthmax, 1.0f);
		draw::DrawRect(min_x - 5, max_y - 1 - hbh, 4, hbh, colors::Health(health, healthmax));
	}
	if (!esp_3d_box_nodraw) {
		for (int i = 0; i < 12; i++) {
			// I'll let compiler optimize this.
			const Vector& p1 = points[indices[i][0]];
			const Vector& p2 = points[indices[i][1]];
			draw::DrawLine(p1.x, p1.y, p2.x - p1.x, p2.y - p1.y, clr);
			if (esp_3d_box_thick) {
				draw::DrawLine(p1.x + 1, p1.y, p2.x - p1.x, p2.y - p1.y, clr);
				draw::DrawLine(p1.x, p1.y + 1, p2.x - p1.x, p2.y - p1.y, clr);
				draw::DrawLine(p1.x + 1, p1.y + 1, p2.x - p1.x, p2.y - p1.y, clr);
			}
		}
	}
}

static CatVar box_nodraw(CV_SWITCH, "esp_box_nodraw", "0", "Invisible 2D Box", "Don't draw 2D box");
static CatVar box_expand(CV_INT, "esp_box_expand", "0", "Expand 2D Box", "Expand 2D box by N units");

void DrawBox(CachedEntity* ent, int clr, float widthFactor, float addHeight, bool healthbar, int health, int healthmax) {
	static Vector min, max, origin, so, omin, omax, smin, smax;
	static float height, width, trf;
	static bool cloak;
	static int min_x, min_y, max_x, max_y, border, hp, hbh;
	static unsigned char alpha;

	if (CE_BAD(ent)) return;

	cloak = ent->m_iClassID == g_pClassID->C_Player && IsPlayerInvisible(ent);
	RAW_ENT(ent)->GetRenderBounds(min, max);
	origin = RAW_ENT(ent)->GetAbsOrigin();
	draw::WorldToScreen(origin, so);
	//if (!a) return;
	//logging::Info("%f %f", so.x, so.y);
	omin = origin + Vector(0, 0, min.z);
	omax = origin + Vector(0, 0, max.z + addHeight);
	bool a = draw::WorldToScreen(omin, smin);
	a = a && draw::WorldToScreen(omax, smax);
	if (!a) return;
	height = abs(smax.y - smin.y);
	//logging::Info("height: %f", height);
	width = height / widthFactor;
	//bool a = draw::WorldToScreen(omin, smin);
	//a = a && draw::WorldToScreen(omax, smax);
	//if (!a) return;
	//draw::DrawString(min(smin.x, smax.x), min(smin.y, smax.y), clr, false, "min");
	//draw::DrawString(max(smin.x, smax.x), max(smin.y, smax.y), clr, false, "max");
	//draw::DrawString((int)so.x, (int)so.y, draw::white, false, "origin");
	data.at(ent->m_IDX).esp_origin.Zero();
	max_x = so.x + width / 2 + 1;
	min_y = so.y - height;
	max_y = so.y;
	min_x = so.x - width / 2;

	if (box_expand) {
		const float& exp = (float)box_expand;
		max_x += exp;
		max_y += exp;
		min_x -= exp;
		min_y -= exp;
	}

	switch ((int)esp_box_text_position) {
	case 0: { // TOP RIGHT
		data.at(ent->m_IDX).esp_origin = Vector(max_x, min_y, 0);
	} break;
	case 1: { // BOTTOM RIGHT
		data.at(ent->m_IDX).esp_origin = Vector(max_x, max_y - data.at(ent->m_IDX).string_count * ((int)fonts::esp_height - 3), 0);
	} break;
	case 2: { // CENTER
	} break;
	case 3: { // ABOVE
		data.at(ent->m_IDX).esp_origin = Vector(min_x + 1, min_y - data.at(ent->m_IDX).string_count * ((int)fonts::esp_height - 3), 0);
	} break;
	case 4: { // BELOW
		data.at(ent->m_IDX).esp_origin = Vector(min_x + 1, max_y, 0);
	}
	}
	alpha = clr >> 24;
	trf = (float)((float)alpha / 255.0f);
	border = cloak ? colors::Create(160, 160, 160, alpha) : colors::Transparent(colors::black, trf);
	if (!box_nodraw) {
		draw::OutlineRect(min_x, min_y, max_x - min_x, max_y - min_y, border);
		draw::OutlineRect(min_x + 1, min_y + 1, max_x - min_x - 2, max_y - min_y - 2, clr);
		draw::OutlineRect(min_x + 2, min_y + 2, max_x - min_x - 4, max_y - min_y - 4, border);
	}

	if (healthbar) {
		hp = colors::Transparent(colors::Health(health, healthmax), trf);
		hbh = (max_y - min_y - 2) * min((float)health / (float)healthmax, 1.0f);
		draw::OutlineRect(min_x - 6, min_y, 7, max_y - min_y, border);
		draw::DrawRect(min_x - 5, max_y - hbh - 1, 5, hbh, hp);
	}
}

void ProcessEntity(CachedEntity* ent) {
	static const model_t* model;
	static int string_count_backup, level, pclass;
	static bool shown;
	static player_info_s info;
	static powerup_type power;
	static CachedEntity* weapon;
	static const char* weapon_name;

	if (!enabled) return;
	if (CE_BAD(ent)) return;

	if (entity_info) {
		AddEntityString(ent, format(RAW_ENT(ent)->GetClientClass()->m_pNetworkName, " [", ent->m_iClassID, "]"));
		if (entity_id) {
			AddEntityString(ent, std::to_string(ent->m_IDX));
		}
		if (entity_model) {
			model = RAW_ENT(ent)->GetModel();
			if (model) AddEntityString(ent, std::string(g_IModelInfo->GetModelName(model)));
		}
	}

	if (ent->m_Type == ENTITY_PROJECTILE && proj_esp && (ent->m_bEnemy || (teammates && !proj_enemy))) {
		if (ent->m_iClassID == g_pClassID->CTFProjectile_Rocket || ent->m_iClassID ==  g_pClassID->CTFProjectile_SentryRocket) {
			if (proj_rockets) {
				if ((int)proj_rockets != 2 || ent->m_bCritProjectile) {
					AddEntityString(ent, "[ ==> ]");
				}
			}
		} else if (ent->m_iClassID == g_pClassID->CTFGrenadePipebombProjectile) {
			switch (CE_INT(ent, netvar.iPipeType)) {
			case 0:
				if (!proj_pipes) break;
				if ((int)proj_pipes == 2 && !ent->m_bCritProjectile) break;
				AddEntityString(ent, "[ (PP) ]");
				break;
			case 1:
				if (!proj_stickies) break;
				if ((int)proj_stickies == 2 && !ent->m_bCritProjectile) break;
				AddEntityString(ent, "[ {*} ]");
			}
		} else if (ent->m_iClassID == g_pClassID->CTFProjectile_Arrow) {
			if ((int)proj_arrows != 2 || ent->m_bCritProjectile) {
				AddEntityString(ent, "[ >>---> ]");
			}
		}
	}

	if (HL2DM) {
		if (item_esp && item_dropped_weapons) {
			if (CE_BYTE(ent, netvar.hOwner) == (unsigned char)-1) {
				string_count_backup = data[ent->m_IDX].string_count;
				if (ent->m_iClassID == g_pClassID->CWeapon_SLAM) AddEntityString(ent, "SLAM");
				else if (ent->m_iClassID == g_pClassID->CWeapon357) AddEntityString(ent, ".357");
				else if (ent->m_iClassID == g_pClassID->CWeaponAR2) AddEntityString(ent, "AR2");
				else if (ent->m_iClassID == g_pClassID->CWeaponAlyxGun) AddEntityString(ent, "Alyx Gun");
				else if (ent->m_iClassID == g_pClassID->CWeaponAnnabelle) AddEntityString(ent, "Annabelle");
				else if (ent->m_iClassID == g_pClassID->CWeaponBinoculars) AddEntityString(ent, "Binoculars");
				else if (ent->m_iClassID == g_pClassID->CWeaponBugBait) AddEntityString(ent, "Bug Bait");
				else if (ent->m_iClassID == g_pClassID->CWeaponCrossbow) AddEntityString(ent, "Crossbow");
				else if (ent->m_iClassID == g_pClassID->CWeaponShotgun) AddEntityString(ent, "Shotgun");
				else if (ent->m_iClassID == g_pClassID->CWeaponSMG1) AddEntityString(ent, "SMG");
				else if (ent->m_iClassID == g_pClassID->CWeaponRPG) AddEntityString(ent, "RPG");
				if (string_count_backup != data[ent->m_IDX].string_count) {
					SetEntityColor(ent, colors::yellow);
				}
			}
		}
	}

	if (ent->m_iClassID == g_pClassID->CTFTankBoss && tank) {
		AddEntityString(ent, "Tank");
	} else if (ent->m_iClassID == g_pClassID->CTFDroppedWeapon && item_esp && item_dropped_weapons) {
		AddEntityString(ent, format("WEAPON ", RAW_ENT(ent)->GetClientClass()->GetName()));
	} else if (ent->m_iClassID == g_pClassID->CCurrencyPack && item_money) {
		if (CE_BYTE(ent, netvar.bDistributed)) {
			if (item_money_red) {
				AddEntityString(ent, "~$~");
			}
		} else {
			AddEntityString(ent, "$$$");
		}
	} else if (ent->m_ItemType != ITEM_NONE && item_esp) {
		shown = false;
		if (item_health_packs && (ent->m_ItemType >= ITEM_HEALTH_SMALL && ent->m_ItemType <= ITEM_HEALTH_LARGE || ent->m_ItemType == ITEM_HL_BATTERY)) {
			if (ent->m_ItemType == ITEM_HEALTH_SMALL) AddEntityString(ent, "[+]");
			if (ent->m_ItemType == ITEM_HEALTH_MEDIUM) AddEntityString(ent, "[++]");
			if (ent->m_ItemType == ITEM_HEALTH_LARGE) AddEntityString(ent, "[+++]");
			if (ent->m_ItemType == ITEM_HL_BATTERY) AddEntityString(ent, "[Z]");
		} else if (item_adrenaline && ent->m_ItemType == ITEM_TF2C_PILL) {
			AddEntityString(ent, "[a]");
		} else if (item_ammo_packs && ent->m_ItemType >= ITEM_AMMO_SMALL && ent->m_ItemType <= ITEM_AMMO_LARGE) {
			if (ent->m_ItemType == ITEM_AMMO_SMALL) AddEntityString(ent, "{i}");
			if (ent->m_ItemType == ITEM_AMMO_MEDIUM) AddEntityString(ent, "{ii}");
			if (ent->m_ItemType == ITEM_AMMO_LARGE) AddEntityString(ent, "{iii}");
		} else if (item_powerups && ent->m_ItemType >= ITEM_POWERUP_FIRST && ent->m_ItemType <= ITEM_POWERUP_LAST) {
			AddEntityString(ent, format(powerups[ent->m_ItemType - ITEM_POWERUP_FIRST], " PICKUP"));
		} else if (item_weapon_spawners && ent->m_ItemType >= ITEM_TF2C_W_FIRST && ent->m_ItemType <= ITEM_TF2C_W_LAST) {
			AddEntityString(ent, format(tf2c_weapon_names[ent->m_ItemType - ITEM_TF2C_W_FIRST], " SPAWNER"));
			if (CE_BYTE(ent, netvar.bRespawning)) AddEntityString(ent, "-- RESPAWNING --");
		}
	} else if (ent->m_Type == ENTITY_BUILDING && buildings) {
		if (!ent->m_bEnemy && !teammates) return;
		level = CE_INT(ent, netvar.iUpgradeLevel);
		const std::string& name = (ent->m_iClassID == g_pClassID->CObjectTeleporter ? "Teleporter" : (ent->m_iClassID == g_pClassID->CObjectSentrygun ? "Sentry Gun" : "Dispenser"));
		if (legit && ent->m_iTeam != g_pLocalPlayer->team) {
			/*if (ent->m_lLastSeen > v_iLegitSeenTicks->GetInt()) {
				return;
			}*/
		}
		if (show_name || show_class) AddEntityString(ent, format("LV ", level, ' ', name));
		if (show_health) {
			AddEntityString(ent, format(ent->m_iHealth, '/', ent->m_iMaxHealth, " HP"), colors::Health(ent->m_iHealth, ent->m_iMaxHealth));
		}
		return;
	} else if (ent->m_Type == ENTITY_PLAYER && ent->m_bAlivePlayer) {
		if (!(local_esp && g_IInput->CAM_IsThirdPerson()) &&
			ent->m_IDX == g_IEngine->GetLocalPlayer()) return;
		pclass = CE_INT(ent, netvar.iClass);
		if (!g_IEngine->GetPlayerInfo(ent->m_IDX, &info)) return;
		power = GetPowerupOnPlayer(ent);
		// If target is enemy, always show powerups, if player is teammate, show powerups
		// only if bTeammatePowerup or bTeammates is true
		if (legit && ent->m_iTeam != g_pLocalPlayer->team && playerlist::IsDefault(info.friendsID)) {
			if (IsPlayerInvisible(ent)) return;
			/*if (ent->m_lLastSeen > (unsigned)v_iLegitSeenTicks->GetInt()) {
				return;
			}*/
		}
		if (power >= 0) {
			// FIXME Disabled powerup ESP.
			//AddEntityString(ent, format("HAS ", powerups[power]));
		}
		if (ent->m_bEnemy || teammates || !playerlist::IsDefault(info.friendsID)) {
			if (show_name)
				AddEntityString(ent, std::string(info.name));
			if (show_class) {
				if (pclass > 0 && pclass < 10)
					AddEntityString(ent, classes[pclass - 1]);
			}
#ifdef IPC_ENABLED
			if (show_bot_id && ipc::peer && ent != LOCAL_E) {
				for (unsigned i = 1; i < cat_ipc::max_peers; i++) {
					if (!ipc::peer->memory->peer_data[i].free && ipc::peer->memory->peer_user_data[i].friendid == info.friendsID) {
						AddEntityString(ent, format("BOT #", i));
						break;
					}
				}
			}
#endif
			if (show_health) {
				AddEntityString(ent, format(ent->m_iHealth, '/', ent->m_iMaxHealth, " HP"), colors::Health(ent->m_iHealth, ent->m_iMaxHealth));
			}
			if (show_conditions && TF) {
				if (IsPlayerInvisible(ent)) {
					AddEntityString(ent, "INVISIBLE");
				}
				if (IsPlayerInvulnerable(ent)) {
					AddEntityString(ent, "INVULNERABLE");
				}
				if (HasCondition(ent, TFCond_UberBulletResist)) {
					AddEntityString(ent, "VACCINATOR ACTIVE");
				}
				if (HasCondition(ent, TFCond_SmallBulletResist)) {
					AddEntityString(ent, "VACCINATOR PASSIVE");
				}
				if (IsPlayerCritBoosted(ent)) {
					AddEntityString(ent, "CRIT BOOSTED");
				}
			}
			if (IsHoovy(ent)) AddEntityString(ent, "Hoovy");
			weapon = ENTITY(CE_INT(ent, netvar.hActiveWeapon) & 0xFFF);
			if (CE_GOOD(weapon)) {
				if (show_weapon) {
					weapon_name = vfunc<const char*(*)(IClientEntity*)>(RAW_ENT(weapon), 398, 0)(RAW_ENT(weapon));
					if (weapon_name) AddEntityString(ent, std::string(weapon_name));
				}
			}
		}
		return;
	}
}

static CatVar esp_3d_box(CV_SWITCH, "esp_3d_box", "0", "3D box");
static CatVar box_healthbar(CV_SWITCH, "esp_box_healthbar", "1", "Box Healthbar");

void ProcessEntityPT(CachedEntity* ent) {
	static int fg, color;
	static bool transparent, cloak, origin_is_zero;
	static Vector screen, origin_screen, draw_point;

	if (!enabled) return;
	if (CE_BAD(ent)) return;


	transparent = false;

	if (!(local_esp && g_IInput->CAM_IsThirdPerson()) &&
		ent->m_IDX == g_IEngine->GetLocalPlayer()) return;

	const ESPData& ent_data = data[ent->m_IDX];
	fg = ent_data.color;

	if (!draw::EntityCenterToScreen(ent, screen) && !draw::WorldToScreen(ent->m_vecOrigin, origin_screen)) return;

	if (box_esp) {
		switch (ent->m_Type) {
		case ENTITY_PLAYER: {
			cloak = IsPlayerInvisible(ent);
			if (legit && ent->m_iTeam != g_pLocalPlayer->team && playerlist::IsDefault(ent)) {
				if (cloak) return;
				/*if (ent->m_lLastSeen > v_iLegitSeenTicks->GetInt()) {
					return;
				}*/
			}

			if (!ent->m_bEnemy && !teammates && playerlist::IsDefault(ent)) break;
			if (!ent->m_bAlivePlayer) break;
			if (vischeck && !ent->IsVisible()) transparent = true;
			if (!fg) fg = colors::EntityF(ent);
			if (transparent) fg = colors::Transparent(fg);
			if (esp_3d_box) {
				Draw3DBox(ent, fg, true, CE_INT(ent, netvar.iHealth), ent->m_iMaxHealth);
			} else {
				DrawBox(ent, fg, 3.0f, -15.0f, static_cast<bool>(box_healthbar), CE_INT(ent, netvar.iHealth), ent->m_iMaxHealth);
			}
		break;
		}
		case ENTITY_BUILDING: {
			if (legit && ent->m_iTeam != g_pLocalPlayer->team) {
				/*if (ent->m_lLastSeen > v_iLegitSeenTicks->GetInt()) {
					return;
				}*/
			}
			if (CE_INT(ent, netvar.iTeamNum) == g_pLocalPlayer->team && !teammates) break;
			if (!transparent && vischeck && !ent->IsVisible()) transparent = true;
			if (!fg) fg = colors::EntityF(ent);
			if (transparent) fg = colors::Transparent(fg);
			if (esp_3d_box) {
				Draw3DBox(ent, fg, true, CE_INT(ent, netvar.iBuildingHealth), CE_INT(ent, netvar.iBuildingMaxHealth));
			} else {
				DrawBox(ent, fg, 1.0f, 0.0f, static_cast<bool>(box_healthbar), CE_INT(ent, netvar.iBuildingHealth), CE_INT(ent, netvar.iBuildingMaxHealth));
			}
		break;
		}
		}
	}

	if (ent_data.string_count) {
		origin_is_zero = !box_esp || ent_data.esp_origin.IsZero(1.0f);
		if (vischeck && !ent->IsVisible()) transparent = true;
		draw_point = origin_is_zero ? screen : ent_data.esp_origin;
		for (int j = 0; j < ent_data.string_count; j++) {
			const ESPString& string = ent_data.strings[j];
			color = string.color ? string.color : ent_data.color;
			if (transparent) color = colors::Transparent(color);
			if (!origin_is_zero) {
				draw::String(fonts::ESP, draw_point.x, draw_point.y, color, 2, string.data);
				draw_point.y += (int)fonts::esp_height - 3;
			} else {
				auto l = draw::GetStringLength(fonts::ESP, string.data);
				draw::String(fonts::ESP, draw_point.x - l.first / 2, draw_point.y, color, 2, string.data);
				draw_point.y += (int)fonts::esp_height - 3;
			}
		}
	}

}

}}}
