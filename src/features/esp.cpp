
/*
 *
 *	This is the main esp for cathook.
 *
 */

#include <algorithm> // min()
#include <string.h>  // strcpy()

#include "../framework/gameticks.hpp"			// So we can run things in draw and world tick
#include "../framework/entitys.hpp"			// So we can use entitys
#include "../framework/input.hpp"		// So we can get screen size
#include "../framework/drawing.hpp"			// We do lots of drawing!
#include "../framework/game.hpp"			// So we can stop esp if not ingame

#include "esp.hpp"

// TODO, phase out esp cache

namespace features { namespace esp {

CatEnum esp_menu({ "Visuals", "Esp" }); // Menu locator for esp settings
CatVarBool esp_enabled(esp_menu, "esp", true, "ESP", "Master esp switch");
// Target selection
static CatVarBool esp_players(esp_menu, "esp_players", true, "ESP Players", "Whether to esp with players");
static CatVarBool esp_other_hostile(esp_menu, "esp_other_hostile", true, "ESP Other Hostile", "Whether to esp with other hostile entitys\nThis is anything not a player but still hostile to you");
static CatVarEnum esp_team(esp_menu, {"Enemy", "Friendly", "Both"}, "esp_team", 0, "Team", "Select which team to show esp on.");
// Box esp + Options
static CatVarEnum box_esp(esp_menu, {"None", "Normal", "Corners"}, "esp_box", 1, "Box", "Draw a 2D box");
static CatVarInt box_corner_size(esp_menu, "esp_box_corner_size", 10, "Corner Size", "Controls corner box size");
// Strings
static CatVarEnum esp_text_position(esp_menu, {"TOP RIGHT", "BOTTOM RIGHT", "ABOVE", "BELOW", "CENTER"}, "esp_text_position", 0, "Text position", "Defines text position");
// Health Esp
static CatVarEnum show_health(esp_menu, {"None", "Text", "Healthbar", "Both"}, "esp_health", 3, "Health ESP", "Show health");
// Other strings
static CatVarBool show_name(esp_menu, "esp_name", true, "Name ESP", "Shows the entity names of entitys");
static CatVarBool show_distance(esp_menu, "esp_distance", true, "Distance ESP", "Shows distance on entitys");
// Bone esp
static CatVarBool esp_bone(esp_menu, "esp_bones", true, "Bone ESP", "Shows cached bones");
static CatVarBool esp_bone_debug(esp_menu, "esp_bones_debug", false, "Bone ESP debug", "Shows debug info about bones");
// Tracers
static CatVarEnum tracers(esp_menu, {"OFF", "CENTER", "BOTTOM"}, "esp_tracers", 2, "Tracers", "Draws a line from the player to a position on your screen");

// Entity Box state enum
enum {EBOX_NOT_RAN, EBOX_FAILED, EBOX_SUCCESSFUL};
class ScreenBox {
public:
	ScreenBox(){}
	ScreenBox(std::pair<int, int> _min , std::pair<int, int> _max) : min(_min), max(_max) {}
	std::pair<int, int> min;
	std::pair<int, int> max;
	int state = EBOX_NOT_RAN;
};
static ScreenBox sbox; // For storing the world to screen box

// Sets the screenbox for an entity
static bool GetEntityBox(CatEntity* entity) {
	if (sbox.state != EBOX_NOT_RAN) return sbox.state == EBOX_SUCCESSFUL; // If we already have the screenbox.second, we return true
	sbox.state = EBOX_FAILED; // Pre-set this so we can return false at any time without worry
	sbox.min = std::make_pair(65536, 65536); sbox.max = std::make_pair(-65536, -65536); // Reset our cached screen box

	// Get our 8 points of our box
	CatVector points_w[8];
	GetCollision(entity).GetPoints(points_w);

	// Go through the points getting world to screen and create our screenbox with them
	for (auto& point : points_w) {
		if (!draw::WorldToScreen(point, point)) return false; // Get world to screen. Return false if we dont get our point

		// Create and expand the bounds based our our last point
		if (point.x > sbox.max.first) sbox.max.first = point.x;
		if (point.y > sbox.max.second) sbox.max.second = point.y;
		if (point.x < sbox.min.first) sbox.min.first = point.x;
		if (point.y < sbox.min.second) sbox.min.second = point.y;
	}

	// We now have our entity box, set the state and return true
	sbox.state = EBOX_SUCCESSFUL;
	return true;
}

// Esp draw func to be ran at draw
static void Draw() {

	// We dont want esp if its disabled, or while not ingame
	if (!esp_enabled || !g_GameInfo.in_game) return;

	// Loop through all entitys
	for (int i = 0; i < GetEntityCount(); i++) {
		CatEntity* entity = GetEntity(i);
		if (!entity || GetDormant(entity)) continue;

		// Target checking
		auto local_ent = GetLocalPlayer();
		if (local_ent && !InThirdperson(local_ent) && (CatEntity*)local_ent == entity) continue; // Determine whether to apply esp to local player
		auto type = GetType(entity);
		if ((type == ETYPE_PLAYER || type == ETYPE_OTHERHOSTILE) && !GetAlive(entity)) continue; // Dont esp dead players
		auto enemy = GetEnemy(entity);
		if (!(esp_team == 2 || (esp_team == 0) ? enemy : !enemy)) continue;

		// Reset the entity box state
		sbox.state = EBOX_NOT_RAN;
		// Get our color
		auto ent_color = colors::EntityColor(entity);
		// Check if main esp features should apply
		if ((esp_players && type == ETYPE_PLAYER) || (esp_other_hostile && type == ETYPE_OTHERHOSTILE)) {

			// Tracers
			if (tracers) {

				// Get world to screen
				CatVector scn;
				if (draw::WorldToScreen(GetOrigin(entity), scn)) {

					// Draw a line
					draw::Line(scn.x, scn.y, input::bounds.first / 2 - scn.x, ((tracers == 2) ? input::bounds.second : input::bounds.second / 2) - scn.y, ent_color);
				}
			}

			// Bone esp
			if (esp_bone) {

				// Loop through the bone sets
				for (const auto& current_set : bones::bonesets) {

					// Draw the bones in the bone set
					for (int ii = 0; ii < current_set.size() - 1; ii++) { // We do it like this so we can identify where we are in the loop

						// Get our 2 bones to connect
						CatVector bone1, bone2;
						if (GetBoneCenter(entity, current_set[ii], bone1) && GetBoneCenter(entity, current_set[ii + 1], bone2)) {

							// World to screen them
							if (draw::WorldToScreen(bone1, bone1) && draw::WorldToScreen(bone1, bone1)) {

								// Draw a line connecting the points
								draw::Line(bone1.x, bone1.y, bone1.x - bone1.x, bone1.y - bone1.y, ent_color);
							}
						}
					}
				}

				// Debug for bone esp, prints strings on screen of what bone is being represented
				if (esp_bone_debug) {
					for (int ii = 0; ii < EBone_count; ii++) {
						// Check if bone is good
						CatVector tmp;
						if (!GetBoneCenter(entity, ii, tmp)) continue;
						// Get wts
						if (!draw::WorldToScreen(tmp, tmp)) continue;
						// Draw
						char buf[8];
						sprintf(buf, "B:%i", ii);
						draw::String(buf, tmp.x, tmp.y, 0, 20, colors::white);
					}
				}
			}

			// Box esp
			if (box_esp) {

				// Attempt to get our entity box
				if (GetEntityBox(entity)) {

					// Simple box esp
					draw::Rect(sbox.min.first, sbox.min.second, sbox.max.first - sbox.min.first, sbox.max.second - sbox.min.second, colors::black);
					draw::Rect(sbox.min.first + 1, sbox.min.second + 1, sbox.max.first - sbox.min.first - 2, sbox.max.second - sbox.min.second - 2, ent_color);
					draw::Rect(sbox.min.first + 2, sbox.min.second + 2, sbox.max.first - sbox.min.first - 4, sbox.max.second - sbox.min.second - 4, colors::black);
				}
			}

			// Health bars
			if ((int)show_health >= 2) {

				// Attempt to get our entity box
				if (GetEntityBox(entity)) {

					// Get in bar height
					int hbh = (sbox.max.second - sbox.min.second - 2) * std::min((float)GetHealth(entity) / (float)GetMaxHealth(entity), 1.0f);

					// Draw
					draw::Rect(sbox.min.first - 7, sbox.min.second, 7, sbox.max.second - sbox.min.second, colors::black);
					draw::RectFilled(sbox.min.first - 6, sbox.max.second - hbh - 1, 5, hbh, colors::Health(entity));
				}
			}
		}

		// Strings

		// Get our strings
		std::vector<std::pair<const char*, CatVector4>> str_cache;

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
				static char buf[12];
				sprintf(buf, "%im", (int)GetDistance(entity));
				str_cache.push_back(std::make_pair(buf, colors::white));
			}
		}
		// Check if there is strings to draw
		if (!str_cache.empty()) {

			// Get world to screen
			CatVector draw_point;
			if (draw::WorldToScreen(GetOrigin(entity), draw_point)) {

				// Change draw point if needed & determine wheter we center the strings
				bool center_strings = true;
				if (sbox.state != EBOX_NOT_RAN && esp_text_position < 4) { // Check if we have an entity box
					center_strings = false; // Dont center strings

					switch(esp_text_position) {
					case 0: // TOP RIGHT
						draw_point = CatVector(sbox.max.first + 2, sbox.min.second, 0); break;
					case 3: // BELOW
						draw_point = CatVector(sbox.min.first, sbox.max.second, 0); break;
					case 1: // BOTTOM RIGHT
					case 2: { // ABOVE
						// Get our height
						int total_height = 0;
						for (const auto& str : str_cache) {
							auto size = draw::GetStringLength(str.first, 1, 28);
							total_height += size.second;
						}
						if (esp_text_position == 1) // BOTTOM RIGHT
							draw_point = CatVector(sbox.min.first, sbox.min.second - total_height);
						else // ABOVE
							draw_point = CatVector(sbox.max.first + 2, sbox.max.second - total_height);
						}
					}
				}

				// Draw our stringsDraw
				// Loop through strings
				for (const auto& str : str_cache) {

					// Get string sizes
					auto size = draw::GetStringLength(str.first, 0, 28);

					if (center_strings) { // Centered strings
						draw::String(str.first, draw_point.x - size.first / 2, draw_point.y, 0, 28, str.second);
					} else { // Not centered
						draw::String(str.first, draw_point.x, draw_point.y, 0, 28, str.second);
					}
					// Lower draw point for recursions
					draw_point.y += size.second;
				}
			}
		}
	}
}

void Init() {
	drawmgr_on(Draw);
}

}}
