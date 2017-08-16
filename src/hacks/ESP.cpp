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
CatEnum tracers_enum({ "OFF", "CENTER", "BOTTOM" });
CatVar tracers(tracers_enum, "esp_tracers", "0", "Tracers", "SDraws a line from the player to a position on your screen");
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
CatVar item_spellbooks(CV_SWITCH, "esp_spellbooks", "1", "Spellbooks", "Spell Books");
CatVar entity_id(CV_SWITCH, "esp_entity_id", "1", "Entity ID", "Used with Entity ESP. Shows entityID");
CatVar tank(CV_SWITCH, "esp_show_tank", "1", "Show tank", "Show tank");
CatVar box_esp(CV_SWITCH, "esp_box", "1", "Box", "Draw 2D box with healthbar. fancy.");
CatVar show_distance(CV_SWITCH, "esp_distance", "1", "Distance ESP", "Show distance to target");
CatVar show_name(CV_SWITCH, "esp_name", "1", "Name ESP", "Show name");
CatVar show_class(CV_SWITCH, "esp_class", "1", "Class ESP", "Show class");
CatVar show_conditions(CV_SWITCH, "esp_conds", "1", "Conditions ESP", "Show conditions");
CatVar show_ubercharge(CV_SWITCH, "esp_ubercharge", "1", "Ubercharge ESP", "Show ubercharge percentage while players medigun is out");
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
static CatVar box_healthbar(CV_SWITCH, "esp_box_healthbar", "1", "Box Healthbar");
static CatVar draw_bones(CV_SWITCH, "esp_bones", "0", "Draw Bone ID's");
static CatVar box_corner_size(CV_INT, "esp_box_corner_size", "10", "Corner Size");
static CatEnum esp_box_text_position_enum({"TOP RIGHT", "BOTTOM RIGHT", "CENTER", "ABOVE", "BELOW" });
static CatVar esp_box_text_position(esp_box_text_position_enum, "esp_box_text_position", "0", "Text position", "Defines text position");
static CatVar box_nodraw(CV_SWITCH, "esp_box_nodraw", "0", "Invisible 2D Box", "Don't draw 2D box");
static CatVar box_expand(CV_INT, "esp_box_expand", "0", "Expand 2D Box", "Expand 2D box by N units");
static CatVar box_corners(CV_SWITCH, "esp_box_corners", "1", "Box Corners");
static CatVar powerup_esp(CV_SWITCH, "esp_powerups", "1", "Powerup ESP");

// Storage arrays for keeping strings and other data
std::mutex threadsafe_mutex;
std::array<ESPData, 2048> data;
	
// Storage vars for entities that need to be re-drawn
std::vector<int> entities_need_repaint {};
std::mutex entities_need_repaint_mutex {};

// Function to reset entitys strings
void ResetEntityStrings() {
	for (auto& i : data) {
		i.string_count = 0;
		i.color = colors::empty;
		i.needs_paint = false;
	}
}

// Sets an entitys esp color
void SetEntityColor(CachedEntity* entity, const rgba_t& color) {
	data[entity->m_IDX].color = color;
}

// Use to add a esp string to an entity
void AddEntityString(CachedEntity* entity, const std::string& string, const rgba_t& color) {
	ESPData& entity_data = data[entity->m_IDX];
	if (entity_data.string_count >= 15) return;
	entity_data.strings[entity_data.string_count].data = string;
	entity_data.strings[entity_data.string_count].color = color;
	entity_data.string_count++;
	entity_data.needs_paint = true;
}


// Function to draw box corners
void BoxCorners(int minx, int miny, int maxx, int maxy, const rgba_t& color, bool transparent) {
	const rgba_t& black = transparent ? colors::Transparent(colors::black) : colors::black;
	const int size = box_corner_size;
	// Black corners
	// Top Left
	drawgl::FilledRect(minx, miny, size, 3, black);
	drawgl::FilledRect(minx, miny + 3, 3, size - 3, black);
	// Top Right
	drawgl::FilledRect(maxx - size + 1, miny, size, 3, black);
	drawgl::FilledRect(maxx - 3 + 1, miny + 3, 3, size - 3, black);
	// Bottom Left
	drawgl::FilledRect(minx, maxy - 3, size, 3, black);
	drawgl::FilledRect(minx, maxy - size, 3, size - 3, black);
	// Bottom Right
	drawgl::FilledRect(maxx - size + 1, maxy - 3, size, 3, black);
	drawgl::FilledRect(maxx - 2, maxy - size, 3, size - 3, black);

	// Colored corners
	// Top Left
	drawgl::Line(minx + 1, miny + 1, size - 2, 0, color);
	drawgl::Line(minx + 1, miny + 1, 0, size - 2, color);
	// Top Right
	drawgl::Line(maxx - 1, miny + 1, -(size - 2), 0, color);
	drawgl::Line(maxx - 1, miny + 1, 0, size - 2, color);
	// Bottom Left
	drawgl::Line(minx + 1, maxy - 2, size - 2, 0, color);
	drawgl::Line(minx + 1, maxy - 2, 0, -(size - 2), color);
	// Bottom Right
	drawgl::Line(maxx - 1, maxy - 2, -(size - 2), 0, color);
	drawgl::Line(maxx - 1, maxy - 2, 0, -(size - 2), color);
}

// Function called on create move
void CreateMove() {
	
	// Check usersettings if enabled
	if (!enabled) return;
	
	// Something
	std::lock_guard<std::mutex> esp_lock(threadsafe_mutex);
	
	ResetEntityStrings(); // Clear any strings entities have
	entities_need_repaint.clear(); // Clear data on entities that need redraw
	
	// TODO, Find the benefit of using max clients with this logic
	// Get max clients and 
	static int max_clients = g_IEngine->GetMaxClients();
	int limit = HIGHEST_ENTITY;
	
	// If not using any other special esp, we lower the min to the max clients
	if (!buildings && !proj_esp && !item_esp)
		limit = min(max_clients, HIGHEST_ENTITY);
	
	{ // I still dont understand the purpose of prof_section and surrounding in brackets
		PROF_SECTION(CM_ESP_EntityLoop);
	
		// Loop through entities
		for (int i = 0; i < limit; i++) {
			// Get an entity from the loop tick and process it
			CachedEntity* ent = ENTITY(i);
			ProcessEntity(ent);
			
			// Dont know what this check is for
			if (data[i].string_count) {
	
				// Set entity color
				SetEntityColor(ent, colors::EntityF(ent));
				
				// If snow distance, add string here
				if (show_distance) {
					AddEntityString(ent, format((int)(ENTITY(i)->m_flDistance / 64 * 1.22f), 'm'));
				}
			}
			// No idea, this is confusing
			if (data[ent->m_IDX].needs_paint) entities_need_repaint.push_back(ent->m_IDX);
		}
	}
}

// Function called on draw
void Draw() {
	std::lock_guard<std::mutex> esp_lock(threadsafe_mutex);
	if (!enabled) return;
	for (auto& i : entities_need_repaint) {
		ProcessEntityPT(ENTITY(i));
	}
}


// TODO, Unknown, find what this does
const Vector dims_player[] = { { -16, -16, -4 }, { 16, 16, 72 } };

// Draw a box around a player
void _FASTCALL DrawBox(CachedEntity* ent, const rgba_t& clr, bool healthbar, int health, int healthmax) {
	PROF_SECTION(PT_esp_drawbox); // Unknown
	
	// Potentially un-used vars
	//Vector so, omin, omax, smin, smax;
	//float height, width;
	
	// Check if ent is bad to prevent crashes
	if (CE_BAD(ent)) return;
	
	// Get collision center, max, and mins
	const Vector& origin = RAW_ENT(ent)->GetCollideable()->GetCollisionOrigin();
	Vector mins = RAW_ENT(ent)->GetCollideable()->OBBMins() + origin;
	Vector maxs = RAW_ENT(ent)->GetCollideable()->OBBMaxs() + origin;

	// Unknown
	data.at(ent->m_IDX).esp_origin.Zero();

	// Create a array for storing box points
	Vector points_r[8]; // World vectors
	Vector points[8]; // Screen vectors

	// If user setting for box expnad is true, spread the max and mins
	if (box_expand) {
		const float& exp = (float)box_expand;
		maxs.x += exp;
		maxs.y += exp;
		maxs.z += exp;
		mins.x -= exp;
		mins.y -= exp;
		mins.z -= exp;
	}

	// Create points for the box based on max and mins
	float x = maxs.x - mins.x;
	float y = maxs.y - mins.y;
	float z = maxs.z - mins.z;
	points_r[0] = mins;
	points_r[1] = mins + Vector(x, 0, 0);
	points_r[2] = mins + Vector(x, y, 0);
	points_r[3] = mins + Vector(0, y, 0);
	points_r[4] = mins + Vector(0, 0, z);
	points_r[5] = mins + Vector(x, 0, z);
	points_r[6] = mins + Vector(x, y, z);
	points_r[7] = mins + Vector(0, y, z);
	
	// Check if any point of the box isnt on the screen
	bool success = true;
	for (int i = 0; i < 8; i++) {
		if (!draw::WorldToScreen(points_r[i], points[i])) success = false;
	}
	// If a point isnt on the screen, return here
	if (!success) return;
	
	// Get max and min of the box using the newly created screen vector
	int max_x = -1;
	int max_y = -1;
	int min_x = 65536;
	int min_y = 65536;
	for (int i = 0; i < 8; i++) {
		if (points[i].x > max_x) max_x = points[i].x;
		if (points[i].y > max_y) max_y = points[i].y;
		if (points[i].x < min_x) min_x = points[i].x;
		if (points[i].y < min_y) min_y = points[i].y;
	}

	// Put Text on the position	with a switch
	switch ((int)esp_box_text_position) {
	case 0: { // TOP RIGHT
		data.at(ent->m_IDX).esp_origin = Vector(max_x + 2, min_y, 0);
	} break;
	case 1: { // BOTTOM RIGHT
		data.at(ent->m_IDX).esp_origin = Vector(max_x + 2, max_y - data.at(ent->m_IDX).string_count * ((int)fonts::ftgl_ESP->height), 0);
	} break;
	case 2: { // CENTER
	} break;
	case 3: { // ABOVE
		data.at(ent->m_IDX).esp_origin = Vector(min_x, min_y - data.at(ent->m_IDX).string_count * ((int)fonts::ftgl_ESP->height), 0);
	} break;
	case 4: { // BELOW
		data.at(ent->m_IDX).esp_origin = Vector(min_x, max_y, 0);
	}
	}
	
	// Depending on whether the player is cloaked, we change the color acordingly
	rgba_t border = ((ent->m_iClassID == RCC_PLAYER) && IsPlayerInvisible(ent)) ? colors::FromRGBA8(160, 160, 160, clr.a * 255.0f) : colors::Transparent(colors::black, clr.a);
	
	// If box nodraw isnt true, we can draw our box
	if (!box_nodraw) {
		// With box corners, we draw differently
		if (box_corners)
			BoxCorners(min_x, min_y, max_x, max_y, clr, (clr.a != 1.0f));
		// Otherwise, we just do simple draw funcs
		else {
			drawgl::Rect(min_x, min_y, max_x - min_x, max_y - min_y, border);
			drawgl::Rect(min_x + 1, min_y + 1, max_x - min_x - 2, max_y - min_y - 2, clr);
			drawgl::Rect(min_x + 2, min_y + 2, max_x - min_x - 4, max_y - min_y - 4, border);
		}
	}

	// If healthbar is enabled, create one here
	if (healthbar) {
		rgba_t hp = colors::Transparent(colors::Health(health, healthmax), clr.a);
		int hbh = (max_y - min_y - 2) * min((float)health / (float)healthmax, 1.0f);
		drawgl::Rect(min_x - 7, min_y, 7, max_y - min_y, border);
		drawgl::FilledRect(min_x - 6, max_y - hbh - 1, 5, hbh, hp);
	}
}
	
// Used to process entities from CreateMove
void _FASTCALL ProcessEntity(CachedEntity* ent) {
	if (!enabled) return; // Esp enable check
	if (CE_BAD(ent)) return; // CE_BAD check to prevent crashes

	// Entity esp
	if (entity_info) {
		AddEntityString(ent, format(RAW_ENT(ent)->GetClientClass()->m_pNetworkName, " [", ent->m_iClassID, "]"));
		if (entity_id) {
			AddEntityString(ent, std::to_string(ent->m_IDX));
		}
		if (entity_model) {
			const model_t* model = RAW_ENT(ent)->GetModel();
			if (model) AddEntityString(ent, std::string(g_IModelInfo->GetModelName(model)));
		}
	}
	
	// Get esp data from current ent
	ESPData& espdata = data[ent->m_IDX];

	// Projectile esp
	if (ent->m_Type == ENTITY_PROJECTILE && proj_esp && (ent->m_bEnemy || (teammates && !proj_enemy))) {
		
		// Rockets
		if (ent->m_iClassID == CL_CLASS(CTFProjectile_Rocket) || ent->m_iClassID ==  CL_CLASS(CTFProjectile_SentryRocket)) {
			if (proj_rockets) {
				if ((int)proj_rockets != 2 || ent->m_bCritProjectile) {
					AddEntityString(ent, "[ ==> ]");
				}
			}
			
		// Pills/Stickys
		} else if (ent->m_iClassID == CL_CLASS(CTFGrenadePipebombProjectile)) {
			// Switch based on pills/stickys
			switch (CE_INT(ent, netvar.iPipeType)) {
			case 0:// Pills
				if (!proj_pipes) break;
				if ((int)proj_pipes == 2 && !ent->m_bCritProjectile) break;
				AddEntityString(ent, "[ (PP) ]");
				break;
			case 1: // Stickys
				if (!proj_stickies) break;
				if ((int)proj_stickies == 2 && !ent->m_bCritProjectile) break;
				AddEntityString(ent, "[ {*} ]");
			}
			
		// Huntsman
		} else if (ent->m_iClassID == CL_CLASS(CTFProjectile_Arrow)) {
			if ((int)proj_arrows != 2 || ent->m_bCritProjectile) {
				AddEntityString(ent, "[ >>---> ]");
			}
		}
	}

	// Hl2DM dropped item esp
	IF_GAME (IsHL2DM()) {
		if (item_esp && item_dropped_weapons) {
			if (CE_BYTE(ent, netvar.hOwner) == (unsigned char)-1) {
				int string_count_backup = data[ent->m_IDX].string_count;
				if (ent->m_iClassID == CL_CLASS(CWeapon_SLAM)) AddEntityString(ent, "SLAM");
				else if (ent->m_iClassID == CL_CLASS(CWeapon357)) AddEntityString(ent, ".357");
				else if (ent->m_iClassID == CL_CLASS(CWeaponAR2)) AddEntityString(ent, "AR2");
				else if (ent->m_iClassID == CL_CLASS(CWeaponAlyxGun)) AddEntityString(ent, "Alyx Gun");
				else if (ent->m_iClassID == CL_CLASS(CWeaponAnnabelle)) AddEntityString(ent, "Annabelle");
				else if (ent->m_iClassID == CL_CLASS(CWeaponBinoculars)) AddEntityString(ent, "Binoculars");
				else if (ent->m_iClassID == CL_CLASS(CWeaponBugBait)) AddEntityString(ent, "Bug Bait");
				else if (ent->m_iClassID == CL_CLASS(CWeaponCrossbow)) AddEntityString(ent, "Crossbow");
				else if (ent->m_iClassID == CL_CLASS(CWeaponShotgun)) AddEntityString(ent, "Shotgun");
				else if (ent->m_iClassID == CL_CLASS(CWeaponSMG1)) AddEntityString(ent, "SMG");
				else if (ent->m_iClassID == CL_CLASS(CWeaponRPG)) AddEntityString(ent, "RPG");
				if (string_count_backup != data[ent->m_IDX].string_count) {
					SetEntityColor(ent, colors::yellow);
				}
			}
		}
	}

	// Tank esp
	if (ent->m_iClassID == CL_CLASS(CTFTankBoss) && tank) {
		AddEntityString(ent, "Tank");
		
	// Dropped weapon esp
	} else if (ent->m_iClassID == CL_CLASS(CTFDroppedWeapon) && item_esp && item_dropped_weapons) {
		AddEntityString(ent, format("WEAPON ", RAW_ENT(ent)->GetClientClass()->GetName()));
		
	// MVM Money esp
	} else if (ent->m_iClassID == CL_CLASS(CCurrencyPack) && item_money) {
		if (CE_BYTE(ent, netvar.bDistributed)) {
			if (item_money_red) {
				AddEntityString(ent, "~$~");
			}
		} else {
			AddEntityString(ent, "$$$");
		}
		
	// Other item esp
	} else if (ent->m_ItemType != ITEM_NONE && item_esp) {
				
		// Health pack esp
		if (item_health_packs && (ent->m_ItemType >= ITEM_HEALTH_SMALL && ent->m_ItemType <= ITEM_HEALTH_LARGE || ent->m_ItemType == ITEM_HL_BATTERY)) {
			if (ent->m_ItemType == ITEM_HEALTH_SMALL) AddEntityString(ent, "[+]");
			if (ent->m_ItemType == ITEM_HEALTH_MEDIUM) AddEntityString(ent, "[++]");
			if (ent->m_ItemType == ITEM_HEALTH_LARGE) AddEntityString(ent, "[+++]");
			if (ent->m_ItemType == ITEM_HL_BATTERY) AddEntityString(ent, "[Z]");
			
		// TF2C Adrenaline esp
		} else if (item_adrenaline && ent->m_ItemType == ITEM_TF2C_PILL) {
			AddEntityString(ent, "[a]");
			
		// Ammo pack esp
		} else if (item_ammo_packs && ent->m_ItemType >= ITEM_AMMO_SMALL && ent->m_ItemType <= ITEM_AMMO_LARGE) {
			if (ent->m_ItemType == ITEM_AMMO_SMALL) AddEntityString(ent, "{i}");
			if (ent->m_ItemType == ITEM_AMMO_MEDIUM) AddEntityString(ent, "{ii}");
			if (ent->m_ItemType == ITEM_AMMO_LARGE) AddEntityString(ent, "{iii}");
			
		// Powerup esp
		} else if (item_powerups && ent->m_ItemType >= ITEM_POWERUP_FIRST && ent->m_ItemType <= ITEM_POWERUP_LAST) {
			AddEntityString(ent, format(powerups[ent->m_ItemType - ITEM_POWERUP_FIRST], " PICKUP"));
			
		// TF2C weapon spawner esp
		} else if (item_weapon_spawners && ent->m_ItemType >= ITEM_TF2C_W_FIRST && ent->m_ItemType <= ITEM_TF2C_W_LAST) {
			AddEntityString(ent, format(tf2c_weapon_names[ent->m_ItemType - ITEM_TF2C_W_FIRST], " SPAWNER"));
			if (CE_BYTE(ent, netvar.bRespawning)) AddEntityString(ent, "-- RESPAWNING --");
			
		// Halloween spell esp
		} else if (item_spellbooks && (ent->m_ItemType == ITEM_SPELL || ent->m_ItemType == ITEM_SPELL_RARE)) {
			if (ent->m_ItemType == ITEM_SPELL) {
				AddEntityString(ent, "Spell", colors::green);
			} else {
				AddEntityString(ent, "Rare Spell", colors::FromRGBA8(139, 31, 221, 255));
			}
		}
		
	// Building esp
	} else if (ent->m_Type == ENTITY_BUILDING && buildings) {
		
		// Check if enemy building 
		if (!ent->m_bEnemy && !teammates) return;
			
		// TODO maybe...
		/*if (legit && ent->m_iTeam != g_pLocalPlayer->team) {
			if (ent->m_lLastSeen > v_iLegitSeenTicks->GetInt()) {
				return;
			}
		}*/
		
		// Make a name for the building based on the building type and level
		if (show_name || show_class) {
			const std::string& name = (ent->m_iClassID == CL_CLASS(CObjectTeleporter) ? "Teleporter" : (ent->m_iClassID == CL_CLASS(CObjectSentrygun) ? "Sentry Gun" : "Dispenser"));
			int level = CE_INT(ent, netvar.iUpgradeLevel);
			AddEntityString(ent, format("LV ", level, ' ', name));
		}
		// If show health is true, then add a string with the health
		if (show_health) {
			AddEntityString(ent, format(ent->m_iHealth, '/', ent->m_iMaxHealth, " HP"), colors::Health(ent->m_iHealth, ent->m_iMaxHealth));
		}
		// Set the entity to repaint
		espdata.needs_paint = true;
		return;
		
	// Player esp
	} else if (ent->m_Type == ENTITY_PLAYER && ent->m_bAlivePlayer) {
		
		// Local player handling
		if (!(local_esp && g_IInput->CAM_IsThirdPerson()) &&
			ent->m_IDX == g_IEngine->GetLocalPlayer()) return;
		
		// Get player class
		int pclass = CE_INT(ent, netvar.iClass);
	
		// Attempt to get player info, and if cant, return
		player_info_s info;
		if (!g_IEngine->GetPlayerInfo(ent->m_IDX, &info)) return;
		
		// TODO, check if u can just use "ent->m_bEnemy" instead of m_iTeam
		// Legit mode handling
		if (legit && ent->m_iTeam != g_pLocalPlayer->team && playerlist::IsDefault(info.friendsID)) {
			if (IsPlayerInvisible(ent)) return; // Invis check
			if (vischeck && !ent->IsVisible()) return; // Vis check
			// TODO, maybe...
			//if (ent->m_lLastSeen > (unsigned)v_iLegitSeenTicks->GetInt()) return;
		}
		
		// Powerup handling
		if (powerup_esp) {
			powerup_type power = GetPowerupOnPlayer(ent);
			if (power != not_powerup)
				AddEntityString(ent, format("^ ", powerups[power], " ^"));
		}
		
		// Dont understand reasoning for this check
		if (ent->m_bEnemy || teammates || !playerlist::IsDefault(info.friendsID)) {
			
			// Playername 
			if (show_name)
				AddEntityString(ent, std::string(info.name));
			
			// Player class
			if (show_class) {
				if (pclass > 0 && pclass < 10)
					AddEntityString(ent, classes[pclass - 1]);
			}
			
#ifdef IPC_ENABLED
			// ipc bot esp
			if (show_bot_id && ipc::peer && ent != LOCAL_E) {
				for (unsigned i = 0; i < cat_ipc::max_peers; i++) {
					if (!ipc::peer->memory->peer_data[i].free && ipc::peer->memory->peer_user_data[i].friendid == info.friendsID) {
						AddEntityString(ent, format("BOT #", i));
						break;
					}
				}
			}
#endif
			// Health esp
			if (show_health) {
				AddEntityString(ent, format(ent->m_iHealth, '/', ent->m_iMaxHealth, " HP"), colors::Health(ent->m_iHealth, ent->m_iMaxHealth));
			}
			IF_GAME (IsTF()) {
				// Medigun Ubercharge esp
				if (show_ubercharge) {
					if (CE_INT(ent, netvar.iClass) == tf_medic) {
						int *weapon_list = (int*)((unsigned)(RAW_ENT(ent)) + netvar.hMyWeapons);
						for (int i = 0; weapon_list[i]; i++) {
							int handle = weapon_list[i];
							int eid = handle & 0xFFF;
							if (eid >= 32 && eid <= HIGHEST_ENTITY) {
								CachedEntity* weapon = ENTITY(eid);
								if (!CE_BAD(weapon) && weapon->m_iClassID == CL_CLASS(CWeaponMedigun) && weapon) {
									if (CE_INT(weapon, netvar.iItemDefinitionIndex) != 998) {
										AddEntityString(ent, format(floor( CE_FLOAT(weapon, netvar.m_flChargeLevel) * 100 ), '%', " Uber"), colors::Health(( CE_FLOAT(weapon, netvar.m_flChargeLevel) * 100 ), 100));
									} else AddEntityString(ent, format(floor( CE_FLOAT(weapon, netvar.m_flChargeLevel) * 100 ), '%', " Uber | Charges: ", floor( CE_FLOAT(weapon, netvar.m_flChargeLevel) / 0.25f )), colors::Health(( CE_FLOAT(weapon, netvar.m_flChargeLevel) * 100 ), 100));
									break;
								}
							}
						}
					}
				}
				// Conditions esp
				if (show_conditions) {
					const auto& clr = colors::EntityF(ent);
					// Invis
					if (IsPlayerInvisible(ent)) 
						AddEntityString(ent, "*CLOAKED*", colors::FromRGBA8(220.0f, 220.0f, 220.0f, 255.0f));
					// Uber/Bonk
					if (IsPlayerInvulnerable(ent))
						AddEntityString(ent, "*INVULNERABLE*");
					// Vaccinator
					if (HasCondition<TFCond_UberBulletResist>(ent)) {
						AddEntityString(ent, "*VACCINATOR*");
					} else if (HasCondition<TFCond_SmallBulletResist>(ent)) {
						AddEntityString(ent, "*PASSIVE RESIST*");
					}
					// Crit
					if (IsPlayerCritBoosted(ent))
						AddEntityString(ent, "*CRITS*", colors::orange);
					// Zoomed
					if (HasCondition<TFCond_Zoomed>(ent)) {
						AddEntityString(ent, "*ZOOMING*", colors::FromRGBA8(220.0f, 220.0f, 220.0f, 255.0f));
					// Slowed
					} else if (HasCondition<TFCond_Slowed>(ent)) {
						AddEntityString(ent, "*SLOWED*", colors::FromRGBA8(220.0f, 220.0f, 220.0f, 255.0f));
					}
					// Jarated
					if (HasCondition<TFCond_Jarated>(ent))
						AddEntityString(ent, "*JARATED*", colors::yellow);
				}
			}
			// Hoovy Esp
			if (IsHoovy(ent)) AddEntityString(ent, "Hoovy");
			
			// Active weapon esp
			int widx = CE_INT(ent, netvar.hActiveWeapon) & 0xFFF;
			if (IDX_GOOD(widx)) {
				CachedEntity* weapon = ENTITY(widx);
				if (CE_GOOD(weapon)) {
					if (show_weapon) {
						const char* weapon_name = vfunc<const char*(*)(IClientEntity*)>(RAW_ENT(weapon), 398, 0)(RAW_ENT(weapon));
						if (weapon_name) AddEntityString(ent, std::string(weapon_name));
					}
				}
			}
			
			// Notify esp to repaint
			espdata.needs_paint = true;
		}
		return;
	}
}

// Vars to store what bones connect to what
const std::string bonenames_leg_r[] = { "bip_foot_R", "bip_knee_R", "bip_hip_R" };
const std::string bonenames_leg_l[] = { "bip_foot_L", "bip_knee_L", "bip_hip_L" };
const std::string bonenames_bottom[] = { "bip_hip_R", "bip_pelvis", "bip_hip_L" };
const std::string bonenames_spine[] = { "bip_pelvis", "bip_spine_0", "bip_spine_1", "bip_spine_2", "bip_spine_3", "bip_neck", "bip_head" };
const std::string bonenames_arm_r[] = { "bip_upperArm_R", "bip_lowerArm_R", "bip_hand_R" };
const std::string bonenames_arm_l[] = { "bip_upperArm_L", "bip_lowerArm_L", "bip_hand_L" };
const std::string bonenames_up[] = { "bip_upperArm_R", "bip_spine_3", "bip_upperArm_L" };

// Dont fully understand struct but a guess is a group of something.
// I will return once I have enough knowlage to reverse this.
// NOTE: No idea on why we cant just use gethitbox and use the displacement on that insted of having all this extra code. Shouldnt gethitbox use cached hitboxes, if so it should be nicer on performance
struct bonelist_s {
	bool setup { false };
	bool success { false };
	int leg_r[3] { 0 };
	int leg_l[3] { 0 };
	int bottom[3] { 0 };
	int spine[7] { 0 };
	int arm_r[3] { 0 };
	int arm_l[3] { 0 };
	int up[3] { 0 };

	void Setup(const studiohdr_t* hdr) {
		if (!hdr) {
			setup = true;
			return;
		}
		std::unordered_map<std::string, int> bones {};
		for (int i = 0; i < hdr->numbones; i++) {
			bones[std::string(hdr->pBone(i)->pszName())] = i;
		}
		try {
			for (int i = 0; i < 3; i++) leg_r[i] = bones.at(bonenames_leg_r[i]);
			for (int i = 0; i < 3; i++) leg_l[i] = bones.at(bonenames_leg_l[i]);
			for (int i = 0; i < 3; i++) bottom[i] = bones.at(bonenames_bottom[i]);
			for (int i = 0; i < 7; i++) spine[i] = bones.at(bonenames_spine[i]);
			for (int i = 0; i < 3; i++) arm_r[i] = bones.at(bonenames_arm_r[i]);
			for (int i = 0; i < 3; i++) arm_l[i] = bones.at(bonenames_arm_l[i]);
			for (int i = 0; i < 3; i++) up[i] = bones.at(bonenames_up[i]);
			success = true;
		} catch (std::exception& ex) {
			logging::Info("Bone list exception: %s", ex.what());
		}
		setup = true;
	}

	void DrawBoneList(const matrix3x4_t* bones, int* in, int size, const rgba_t& color, const Vector& displacement) {
		Vector last_screen;
		Vector current_screen;
		for (int i = 0; i < size; i++) {
			Vector position(bones[in[i]][0][3], bones[in[i]][1][3], bones[in[i]][2][3]);
			position += displacement;
			if (!draw::WorldToScreen(position, current_screen)) {
				return;
			}
			if (i > 0) {
				drawgl::Line(last_screen.x, last_screen.y, current_screen.x - last_screen.x, current_screen.y - last_screen.y, color);
			}
			last_screen = current_screen;
		}
	}

	void Draw(CachedEntity* ent, const rgba_t& color) {
		const model_t* model = RAW_ENT(ent)->GetModel();
		if (not model) {
			return;
		}

		studiohdr_t* hdr =  g_IModelInfo->GetStudiomodel(model);

		if (!setup) {
			Setup(hdr);
		}
		if (!success) return;

		//ent->m_bBonesSetup = false;
		Vector displacement = RAW_ENT(ent)->GetAbsOrigin() - ent->m_vecOrigin;
		const auto& bones = ent->hitboxes.GetBones();
		DrawBoneList(bones, leg_r, 3, color, displacement);
		DrawBoneList(bones, leg_l, 3, color, displacement);
		DrawBoneList(bones, bottom, 3, color, displacement);
		DrawBoneList(bones, spine, 7, color, displacement);
		DrawBoneList(bones, arm_r, 3, color, displacement);
		DrawBoneList(bones, arm_l, 3, color, displacement);
		DrawBoneList(bones, up, 3, color, displacement);
		/*for (int i = 0; i < hdr->numbones; i++) {
			const auto& bone = ent->GetBones()[i];
			Vector pos(bone[0][3], bone[1][3], bone[2][3]);
			//pos += orig;
			Vector screen;
			if (draw::WorldToScreen(pos, screen)) {
				if (hdr->pBone(i)->pszName()) {
					draw::FString(fonts::ESP, screen.x, screen.y, fg, 2, "%s [%d]", hdr->pBone(i)->pszName(), i);
				} else
					draw::FString(fonts::ESP, screen.x, screen.y, fg, 2, "%d", i);
			}
		}*/
	}
};

std::unordered_map<studiohdr_t*, bonelist_s> bonelist_map {};

/*
 * According to profiler, this function is the most time-consuming (and gets called up to 200K times a second)
 */

CatEnum emoji_esp_enum({ "None", "Joy", "Thinking" });
CatVar emoji_esp(emoji_esp_enum, "esp_emoji", "0", "Emoji ESP", "Draw emoji on peopels head");
CatVar emoji_esp_size(CV_FLOAT, "esp_emoji_size", "32", "Emoji ESP Size");
CatVar emoji_esp_scaling(CV_SWITCH, "esp_emoji_scaling", "1", "Emoji ESP Scaling");
CatVar emoji_min_size(CV_INT, "esp_emoji_min_size", "20", "Emoji ESP min size", "Minimum size for an emoji when you use auto scaling");
textures::AtlasTexture joy_texture(64 * 4, textures::atlas_height - 64 * 4, 64, 64);
textures::AtlasTexture thinking_texture(64 * 5, textures::atlas_height - 64 * 4, 64, 64);
	
	
//CatVar draw_hitbox(CV_SWITCH, "esp_hitbox", "1", "Draw Hitbox");

	
// Used when processing entitys with cached data from createmove
void _FASTCALL ProcessEntityPT(CachedEntity* ent) {
	PROF_SECTION(PT_esp_process_entity);

	// Check to prevent crashes
	if (CE_BAD(ent)) return;
	
	// Grab esp data
	ESPData& ent_data = data[ent->m_IDX];

	// Get color of entity
	// TODO, check if we can move this after world to screen check
	rgba_t fg = ent_data.color;
	if (!fg || fg.a == 0.0f) fg = ent_data.color = colors::EntityF(ent);
	
	// Check if entity is on screen, then save screen position if true
	Vector screen, origin_screen;
	if (!draw::EntityCenterToScreen(ent, screen) && !draw::WorldToScreen(ent->m_vecOrigin, origin_screen)) return;

	// Unknown
	ent_data.esp_origin.Zero();

	// Get if ent should be transparent
	bool transparent = false;
	if (vischeck && !ent->IsVisible()) transparent = true;

	// Bone esp
	if (draw_bones && ent->m_Type == ENTITY_PLAYER) {
		const model_t* model = RAW_ENT(ent)->GetModel();
		if (model) {
			auto hdr = g_IModelInfo->GetStudiomodel(model);
			bonelist_map[hdr].Draw(ent, fg);
		}
	}

	// Tracers
	if (tracers && ent->m_Type == ENTITY_PLAYER) {
		
		// Grab the screen resolution and save to some vars
		int width, height;
		g_IEngine->GetScreenSize(width, height);
		
		// Center values on screen
		width = width / 2;
		// Only center height if we are using center mode
		if ((int)tracers == 1) height = height / 2;
		
		// Get world to screen
		Vector scn;
		draw::WorldToScreen(ent->m_vecOrigin, scn);
		
		// Draw a line
		drawgl::Line(scn.x, scn.y, width - scn.x, height - scn.y, fg);
	}
	
	// Emoji esp
	if (ent->m_Type == ENTITY_PLAYER) {
		if (emoji_esp) {
			auto hb = ent->hitboxes.GetHitbox(0);
			Vector hbm, hbx;
			if (draw::WorldToScreen(hb->min, hbm) && draw::WorldToScreen(hb->max, hbx)) {
				Vector head_scr;
				if (draw::WorldToScreen(hb->center, head_scr)) {
					float size = emoji_esp_scaling ? fabs(hbm.y - hbx.y) : float(emoji_esp_size);
					if (emoji_esp_scaling && (size < float(emoji_min_size))) {
						size = float(emoji_min_size);
					}
					textures::AtlasTexture* tx = nullptr;
					if (int(emoji_esp) == 1) tx = &joy_texture;
					if (int(emoji_esp) == 2) tx = &thinking_texture;
					if (tx)
						tx->Draw(head_scr.x - size / 2, head_scr.y - size / 2, size, size);
				}
			}
		}
	}
	
	// TODO Add Rotation matix
	// TODO Currently crashes, needs null check somewhere
	// Draw Hitboxes
	/*if (draw_hitbox && ent->m_Type == ENTITY_PLAYER) {
		
		// Loop through hitboxes
		for (int i = 0; i <= 17; i++) { // I should probs get how many hitboxes instead of using a fixed number...
			
			// Get a hitbox from the entity
			hitbox_cache::CachedHitbox* hb = ent->hitboxes.GetHitbox(i);
			
			// Create more points from min + max
			Vector box_points[8];
			Vector vec_tmp;
			for (int ii = 0; ii <= 8; ii++) { // 8 points to the box
				
				// logic le paste from sdk
				vec_tmp[0] = ( ii & 0x1 ) ? hb->max[0] : hb->min[0];
				vec_tmp[1] = ( ii & 0x2 ) ? hb->max[1] : hb->min[1];
				vec_tmp[2] = ( ii & 0x4 ) ? hb->max[2] : hb->min[2];
				
				// save to points array
				box_points[ii] = vec_tmp;
			}
			
			// Draw box from points
			// Draws a point to every other point. Ineffient, use now fix later...
			Vector scn1, scn2; // to screen
			for (int ii = 0; ii < 8; ii++) { 
				
				// Get first point
				if (!draw::WorldToScreen(box_points[ii], scn1)) continue;
				
				for (int iii = 0; iii < 8; iii++) {
					
					// Get second point
					if (!draw::WorldToScreen(box_points[iii], scn2)) continue;
					
					// Draw between points
					drawgl::Line(scn1.x, scn1.y, scn2.x - scn1.x, scn2.y - scn1.y, fg);
				}
			}
		}
	}*/

	// Box esp
	if (box_esp) {
		switch (ent->m_Type) {
		case ENTITY_PLAYER: 
			if (vischeck && !ent->IsVisible()) transparent = true;
			if (!fg) fg = colors::EntityF(ent);
			if (transparent) fg = colors::Transparent(fg);
			DrawBox(ent, fg, static_cast<bool>(box_healthbar), CE_INT(ent, netvar.iHealth), ent->m_iMaxHealth);
			break;
		case ENTITY_BUILDING: 
			if (CE_INT(ent, netvar.iTeamNum) == g_pLocalPlayer->team && !teammates) break;
			if (!transparent && vischeck && !ent->IsVisible()) transparent = true;
			if (!fg) fg = colors::EntityF(ent);
			if (transparent) fg = colors::Transparent(fg);
			DrawBox(ent, fg, static_cast<bool>(box_healthbar), CE_INT(ent, netvar.iBuildingHealth), CE_INT(ent, netvar.iBuildingMaxHealth));
			break;
		}
	}
	
	// Draw strings ???
	// TODO reverse this
	bool origin_is_zero = !box_esp || ent_data.esp_origin.IsZero(1.0f);
	if (origin_is_zero) ent_data.esp_origin = screen;
	if (ent_data.string_count) {
		PROF_SECTION(PT_esp_drawstrings);
		if (vischeck && !ent->IsVisible()) transparent = true;
		Vector draw_point = origin_is_zero ? screen : ent_data.esp_origin;
		for (int j = 0; j < ent_data.string_count; j++) {
			const ESPString& string = ent_data.strings[j];
			rgba_t color = string.color ? string.color : ent_data.color;
			if (transparent) color = colors::Transparent(color);
			if (!origin_is_zero) {
				FTGL_Draw(string.data, draw_point.x, draw_point.y, fonts::ftgl_ESP, color);
				draw_point.y += (int)fonts::ftgl_ESP->height - 1;
			} else {
				int size_x;
				FTGL_StringLength(string.data, fonts::ftgl_ESP, &size_x);
				FTGL_Draw(string.data, draw_point.x - size_x / 2, draw_point.y, fonts::ftgl_ESP, color);
				draw_point.y += (int)fonts::ftgl_ESP->height - 1;
			}
		}
	}
}

}}}
