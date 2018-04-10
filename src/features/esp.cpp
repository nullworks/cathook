
/*
 *
 *	This is the main esp for cathook.
 *
 */

#include <algorithm> // min()
#include <string.h>  // strcpy()

#include "../framework/gameticks.hpp"			// So we can run things in draw
#include "../framework/entitys.hpp"			// So we can use entitys
#include "../framework/input.hpp"		// So we can get screen size
#include "../framework/drawing.hpp"			// We do lots of drawing!
#include "../framework/game.hpp"			// So we can stop esp if not ingame

#include "esp.hpp"

namespace features::esp {

CatEnum esp_menu({ "Visuals", "Esp" }); // Menu locator for esp settings
static CatVarBool esp_enabled(esp_menu, "esp", true, "ESP", "Master esp switch");
// Target selection
static CatVarBool esp_players(esp_menu, "esp_players", true, "ESP Players", "Whether to esp with players");
static CatVarBool esp_other_hostile(esp_menu, "esp_other_hostile", true, "ESP Other Hostile", "Whether to esp with other hostile entitys\nThis is anything not a player but still hostile to you");
static CatEnum esp_team_enum({"Enemy", "Friendly", "Both"});
static CatVarEnum esp_team(esp_menu, esp_team_enum, "esp_team", 2, "Team", "Select which team to show esp on.");
// Box esp + Options
static CatEnum box_esp_enum({"None", "Normal", "Corners"});
static CatVarEnum box_esp(esp_menu, box_esp_enum, "esp_box", 1, "Box", "Draw a 2D box");
static CatVarInt box_corner_size(esp_menu, "esp_box_corner_size", 10, "Corner Size", "Controls corner box size");
// Strings
static CatEnum esp_text_position_enum({"TOP RIGHT", "BOTTOM RIGHT", "ABOVE", "BELOW", "CENTER"});
static CatVarEnum esp_text_position(esp_menu, esp_text_position_enum, "esp_text_position", 0, "Text position", "Defines text position");
// Health Esp
static CatEnum show_health_enum({"None", "Text", "Healthbar", "Both"});
static CatVarEnum show_health(esp_menu, show_health_enum, "esp_health", 3, "Health ESP", "Show health");
static CatEnum health_bar_pos_enum({"Left", "Right", "Top", "Bottom"});
static CatVarEnum health_bar_pos(esp_menu, health_bar_pos_enum, "esp_health_bar_pos", 0, "Health Bar Position", "Where to put the health bar");

// Other strings
static CatVarBool show_name(esp_menu, "esp_name", true, "Name ESP", "Shows the entity names of entitys");
static CatVarBool show_distance(esp_menu, "esp_distance", true, "Distance ESP", "Shows distance on entitys");
// Bone esp
static CatVarBool esp_bone(esp_menu, "esp_bones", true, "Bone ESP", "Shows cached bones");
static CatVarBool esp_bone_debug(esp_menu, "esp_bones_debug", false, "Bone ESP debug", "Shows debug info about bones");
// Tracers
static CatEnum tracers_enum({"OFF", "CENTER", "BOTTOM"});
static CatVarEnum tracers(esp_menu, tracers_enum, "esp_tracers", 2, "Tracers", "Draws a line from the player to a position on your screen");
// Other
static CatEnum box_mode_enum({"Collision", "Bone", "Hitbox"});
static CatVarEnum box_mode(esp_menu, box_mode_enum, "esp_box_mode", 0, "Box mode", "What method to use to get the esp box");

// Externed, add you functions to get strings onto entities
std::vector<std::pair<const char*, CatColor>(*)(CatEntity*)> GetEntityStrings;

// Esp draw func to be ran at draw
static void Draw() {

	// We dont want esp if its disabled, or while not ingame
	if (!esp_enabled || !game::GetInGame()) return;

	// Loop through all entitys
	auto ent_count = GetEntityCount();
	for (int i = 0; i < ent_count; i++) {
		CatEntity* entity = GetEntity(i);
		if (!entity || GetDormant(entity)) continue;

		// Target checking
		auto local_ent = GetLocalPlayer();
		if (local_ent && !InThirdperson(local_ent) && (CatEntity*)local_ent == entity) continue; // Determine whether to apply esp to local player
		auto type = GetType(entity);
		if ((type == ETYPE_PLAYER || type == ETYPE_OTHERHOSTILE) && !GetAlive(entity)) continue; // Dont esp dead players
		auto enemy = GetEnemy(entity);
		if (esp_team != 2 && ((esp_team == 0) ? !enemy : enemy)) continue;

		// Cant use namespaces here
		enum {SBOX_NOT_RAN, SBOX_FAILED, SBOX_SUCCESSFUL};
		std::pair<int, int> sbox_min;
		std::pair<int, int> sbox_max;
		unsigned short sbox_state = SBOX_NOT_RAN;
		// Call this to tell if you're allowed to use the entity box
		auto GetEntityBox = [&]() -> bool {
		 	if (sbox_state != SBOX_NOT_RAN)
		 		return sbox_state == SBOX_SUCCESSFUL; // If we already have the screenbox.second, we return true
			sbox_state = SBOX_FAILED; // Pre-set this so we can return false at any time without worry
			sbox_min = {65536, 65536}; sbox_max = {-65536, -65536}; // Reset our cached screen box
			auto ExpandWithPoint = [&](const std::pair<int, int>& pnt) {
				// Create and expand the bounds based our our last point
				if (pnt.first > sbox_max.first)
					sbox_max.first = pnt.first;
				if (pnt.second > sbox_max.second)
					sbox_max.second = pnt.second;
				if (pnt.first < sbox_min.first)
					sbox_min.first = pnt.first;
				if (pnt.second < sbox_min.second)
					sbox_min.second = pnt.second;
			};
			// Convinient for everything to use
			std::pair<int, int> wts;
		 	// If we use bones, try to do so here, we still want collision as a fallback
		 	if (box_mode >= 1) {
				// get bones
			 	std::vector<CatBox> cur_bones;
			 	for (int ii = 0; ii < EBone_count; ii++) {
					CatBox tmp;
				 	if (GetBone(entity, ii, tmp))
				 		cur_bones.push_back(tmp);
			 	}
			 	if (!cur_bones.empty()) {
					for (auto& tmp : cur_bones) {
						if (box_mode == 1) { // Bone
							if (!draw::WorldToScreen(tmp.GetCenter(), wts))
								return false;
							ExpandWithPoint(wts);
						} else { // Hitbox
							for(auto pt : tmp.GetPoints()) {
								if (!draw::WorldToScreen(pt, wts))
									return false;
								ExpandWithPoint(wts);
							}
						}
					}
					sbox_state = SBOX_SUCCESSFUL;
					return true;
			 	}
	 	 	}
			// Get our 8 points of our box
			auto points = GetCollision(entity).GetPoints();

			// Go through the points getting world to screen and create our screenbox with them
			for (auto& pt : points) {
  			if (!draw::WorldToScreen(pt, wts))
					return false;
				ExpandWithPoint(wts);
			}

			// We now have our entity box, set the state and return true
			sbox_state = SBOX_SUCCESSFUL;
		 	return true;
	 	};

		// Get our color
		auto ent_color = colors::EntityColor(entity);

		// Check if main esp features should apply
		if ((esp_players && type == ETYPE_PLAYER) || (esp_other_hostile && type == ETYPE_OTHERHOSTILE)) {

			// Tracers
			if (tracers) {

				// Get world to screen
				std::pair<int, int> scn;
				if (draw::WorldToScreen(GetOrigin(entity), scn)) {

					// Draw a line
					auto bounds = input::GetBounds();
					draw::Line(scn.first, scn.second, bounds.first / 2 - scn.first, ((tracers == 2) ? bounds.second : bounds.second / 2) - scn.second, ent_color);
				}
			}

			// Bone esp
			if (esp_bone) {
				// This is for how to draw the bones
				const std::vector<int> bonesets[] = {
					{EBone_head, EBone_top_spine, EBone_upper_spine, EBone_middle_spine, EBone_bottom_spine, EBone_pelvis}, // Center
					{EBone_lower_arm_l, EBone_middle_arm_l, EBone_upper_arm_l, EBone_top_spine, EBone_upper_arm_r, EBone_middle_arm_r, EBone_lower_arm_r}, // Upper limbs
					{EBone_lower_leg_l, EBone_middle_leg_l, EBone_upper_leg_l, EBone_pelvis,    EBone_upper_leg_r, EBone_middle_leg_r, EBone_lower_leg_r}  // Lower limbs
				};
				// Loop through the bone sets
				for (const auto& current_set : bonesets) {
					static CatVector last_bone = CatVector(); // Our last bone location as we might not get every bone
					// Draw the bones in the bone set
					for (size_t ii = 0; ii < current_set.size() - 1; ii++) { // We do it like this so we can identify where we are in the loop
						// Get our 2 bones to connect
						CatVector bone1, bone2;
						if (last_bone != CatVector() || GetBoneCenter(entity, current_set[ii], bone1)) {
							if (GetBoneCenter(entity, current_set[ii + 1], bone2)) {
								// World to screen them
								std::pair<int, int> scn1, scn2;
								if (draw::WorldToScreen((last_bone != CatVector()) ? last_bone : bone1, scn1) && draw::WorldToScreen(bone2, scn2)) {
									// Draw a line connecting the points
									draw::Line(scn1.first, scn1.second, scn2.first - scn1.first, scn2.second - scn1.second, ent_color);
								}
							last_bone = bone2; // save the bone so we dont need it again
							}
						}
					}
					// Clear it out as we dont want a different set connecting to it
					last_bone = CatVector();
				}

				// Debug for bone esp, prints strings on screen of what bone is being represented
				if (esp_bone_debug) {
					for (int ii = 0; ii < EBone_count; ii++) {
						// Check if bone is good
						CatVector tmp;
						if (!GetBoneCenter(entity, ii, tmp)) continue;
						// Get wts
						std::pair<int, int> wts;
						if (!draw::WorldToScreen(tmp, wts)) continue;
						// Draw
						char buf[8];
						sprintf(buf, "B:%i", ii);
						draw::String(buf, wts.first, wts.second, draw::default_font, draw::default_font_size, colors::white);
					}
				}
			}

			// Box esp
			if (box_esp) {

				// Attempt to get our entity box
				if (GetEntityBox()) {

					// Simple box esp
					draw::Rect(sbox_min.first, sbox_min.second, sbox_max.first - sbox_min.first, sbox_max.second - sbox_min.second, colors::black);
					draw::Rect(sbox_min.first + 1, sbox_min.second + 1, sbox_max.first - sbox_min.first - 2, sbox_max.second - sbox_min.second - 2, ent_color);
					draw::Rect(sbox_min.first + 2, sbox_min.second + 2, sbox_max.first - sbox_min.first - 4, sbox_max.second - sbox_min.second - 4, colors::black);
				}
			}

			// Health bars
			if ((int)show_health >= 2) {

				// Attempt to get our entity box
				if (GetEntityBox()) {

					// TODO, add right, top, and bottom health bars

					// Get in bar height
					int hbh = (sbox_max.second - sbox_min.second - 2) * std::min((float)GetHealth(entity) / (float)GetMaxHealth(entity), 1.0f);

					// Draw
					draw::Rect(sbox_min.first - 7, sbox_min.second, 7, sbox_max.second - sbox_min.second, colors::black);
					draw::RectFilled(sbox_min.first - 6, sbox_max.second - hbh - 1, 5, hbh, colors::Health(entity));
				}
			}
		}

		// Strings

		// Get our strings
		std::vector<std::pair<const char*, CatColor>> str_cache;

		if ((esp_players && type == ETYPE_PLAYER) || (esp_other_hostile && type == ETYPE_OTHERHOSTILE)) {
			// Name esp
			if (show_name) {
				str_cache.push_back(std::make_pair(GetName(entity), ent_color));
			}

			// Health esp
			if (show_health == 1 || show_health == 3) {
				static char buf[12];
				sprintf(buf, "%i/%iHP", GetHealth(entity), GetMaxHealth(entity));
				str_cache.push_back(std::make_pair(buf, colors::Health(entity)));
			}

			// Distance esp
			if (show_distance) {
				static char buf[8];
				sprintf(buf, "%im", (int)GetDistance(entity));
				str_cache.push_back(std::make_pair(buf, colors::white));
			}

			// Write module strings
			for (auto i : GetEntityStrings) {
				auto tmp = i(entity);
				if (!tmp.first) continue;
				str_cache.push_back(tmp);
			}
		}
		// Check if there is strings to draw
		if (!str_cache.empty()) {

			// Get world to screen
			std::pair<int, int> draw_point;
			if (draw::WorldToScreen(GetOrigin(entity), draw_point)) {

				// Change draw point if needed & determine wheter we center the strings
				bool center_strings = true;
				if (sbox_state == SBOX_SUCCESSFUL && esp_text_position < 4) { // Check if we have an entity box
					center_strings = false; // Dont center strings

					switch(esp_text_position) {
					case 0: // TOP RIGHT
						draw_point = {sbox_max.first + 2, sbox_min.second}; break;
					case 3: // BELOW
						center_strings = true;
						draw_point = {sbox_min.first, sbox_max.second}; break;
					case 2: // ABOVE
						center_strings = true;
					case 1: { // BOTTOM RIGHT
						// Get our height
						int total_height = 0;
						for (const auto& str : str_cache) {
							auto size = draw::GetStringLength(str.first, draw::default_font, draw::default_font_size);
							total_height += size.second;
						}
						if (esp_text_position == 1) // BOTTOM RIGHT
							draw_point = {sbox_min.first, sbox_min.second - total_height};
						else // ABOVE
							draw_point = {sbox_max.first + 2, sbox_max.second - total_height};
						}
					}
				}

				// Draw our stringsDraw
				// Loop through strings
				for (const auto& str : str_cache) {

					// Get string sizes
					auto size = draw::GetStringLength(str.first, draw::default_font, draw::default_font_size);

					if (center_strings) // Centered strings
						draw::String(str.first, draw_point.first - size.first / 2, draw_point.second, draw::default_font, draw::default_font_size, str.second);
					else // Not centered
						draw::String(str.first, draw_point.first, draw_point.second, draw::default_font, draw::default_font_size, str.second);

					// Lower draw point for recursions
					draw_point.second += size.second;
				}
			}
		}
	}
}

void Init() {
	drawmgr.REventDuring(Draw);
}

}
