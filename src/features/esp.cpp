
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

namespace features { namespace esp {

CatEnum esp_menu({ "Visuals", "Esp" }); // Menu locator for esp settings
CatVarBool esp_enabled(esp_menu, "esp", true, "ESP", "Master esp switch");
// Target selection
static CatVarBool esp_players(esp_menu, "esp_players", true, "ESP Players", "Whether to esp with players");
static CatVarBool esp_other_hostile(esp_menu, "esp_other_hostile", true, "ESP Other Hostile", "Whether to esp with other hostile entitys\nThis is anything not a player but still hostile to you");
static CatEnum esp_team_enum({"Enemy", "Friendly", "Both"});
static CatVarEnum esp_team(esp_menu, esp_team_enum, "esp_team", 0, "Team", "Select which team to show esp on.");
// Box esp + Options
static CatEnum box_esp_enum({ "None", "Normal", "Corners" });
static CatVarEnum box_esp(esp_menu, box_esp_enum, "esp_box", 1, "Box", "Draw a 2D box");
static CatVarInt box_corner_size(esp_menu, "esp_box_corner_size", 10, "Corner Size", "Controls corner box size");
// Strings
static CatEnum esp_text_position_enum({"TOP RIGHT", "CENTER", "BELOW", "ABOVE", "BOTTOM RIGHT"}); // Aranged in a way to make things easier below
static CatVarEnum esp_text_position(esp_menu, esp_text_position_enum, "esp_text_position", 0, "Text position", "Defines text position");
// Health Esp
static CatEnum show_health_enum({ "None", "Text", "Healthbar", "Both" });
static CatVarEnum show_health(esp_menu, show_health_enum, "esp_health", 3, "Health ESP", "Show health");
// Other strings
static CatVarBool show_name(esp_menu, "esp_name", true, "Name ESP", "Shows the entity names of entitys");
static CatVarBool show_distance(esp_menu, "esp_distance", true, "Distance ESP", "Shows distance on entitys");
// Bone esp
static CatVarBool esp_bone(esp_menu, "esp_bones", true, "Bone ESP", "Shows cached bones");
static CatVarBool esp_bone_debug(esp_menu, "esp_bones_debug", false, "Bone ESP debug", "Shows debug info about bones");
// Tracers
static CatEnum tracers_enum({ "OFF", "CENTER", "BOTTOM" });
static CatVarEnum tracers(esp_menu, tracers_enum, "esp_tracers", 2, "Tracers", "Draws a line from the player to a position on your screen");

// Entitys strings
struct ESPData {
	int string_count = 0;
	std::pair<char[64], CatVector4> strings[MAX_ESP_STRINGS];
	CatVector4 color = colors::white;
};

// An array to store our cached esp strings
static ESPData esp_cache[MAX_ENTITIES];

// Entity Box state enum
enum {EBOX_NOT_RAN, EBOX_FAILED, EBOX_SUCCESSFUL};
// TODO, replace catbox with something that uses int instead of floats to save cycles
static std::pair<int, CatBox> screenbox; // For storing the world to screen box

// Sets the screenbox for an entity
static bool GetEntityBox(const CatEntity& entity) {
	if (screenbox.first != EBOX_NOT_RAN) return screenbox.first == EBOX_SUCCESSFUL; // If we already have the screenbox.second, we return true
	screenbox.first = EBOX_FAILED; // Pre-set this so we can return false at any time without worry
	screenbox.second = CatBox(CatVector(65536, 65536), CatVector(-65536, -65536)); // Reset our cached screen box

	// Get our 8 points of our box
	CatVector points_w[8];
	entity.collision.GetPoints(points_w);

	// Go through the points getting world to screen and create our screenbox with them
	for (const auto& point : points_w) {
		CatVector tmp;
		if (!draw::WorldToScreen(point, tmp)) return false; // Get world to screen. Return false if we dont get our point

		// Create and expand the bounds based our our last point
		if (tmp.x > screenbox.second.max.x) screenbox.second.max.x = tmp.x;
		if (tmp.y > screenbox.second.max.y) screenbox.second.max.y = tmp.y;
		if (tmp.x < screenbox.second.min.x) screenbox.second.min.x = tmp.x;
		if (tmp.y < screenbox.second.min.y) screenbox.second.min.y = tmp.y;
	}

	// We now have our entity box, set the state and return true
	screenbox.first = EBOX_SUCCESSFUL;
	return true;
}

// Esp draw func to be ran at draw
static void Draw() {

	// We dont want esp if its disabled, or while not ingame
	if (!esp_enabled || !g_GameInfo.in_game) return;

	// Loop through all entitys
	for (const auto& entity: g_CatEntitys) {
		if (CE_BAD(entity)) continue;

		// Target checking
		if (!g_LocalPlayer.InThirdperson() && g_LocalPlayer.entity == &entity) continue; // Determine whether to apply esp to local player
		if ((entity.type == ETYPE_PLAYER || entity.type == ETYPE_OTHERHOSTILE) && !entity.alive) continue; // Dont esp dead players
		if (!(esp_team == 2 || (esp_team == 0) ? entity.Enemy() : !entity.Enemy())) continue;

		// Reset the entity box state
		screenbox.first = EBOX_NOT_RAN;

		// Check if main esp features should apply
		if ((esp_players && entity.type == ETYPE_PLAYER) || (esp_other_hostile && entity.type == ETYPE_OTHERHOSTILE)) {

			// Tracers
			if (tracers) {

				// Get world to screen
				CatVector scn;
				if (draw::WorldToScreen(entity.origin, scn)) {

					// Draw a line
					draw::Line(scn.x, scn.y, input::bounds.first / 2 - scn.x, ((tracers == 2) ? input::bounds.second : input::bounds.second / 2) - scn.y, esp_cache[entity.IDX].color);
				}
			}

			// Bone esp
			if (esp_bone) {

				// Loop through the bone sets
				for (const auto& current_set : bones::bonesets) {

					// Draw the bones in the bone set
					for (int i = 0; i < current_set.size() - 1; i++) { // We do it like this so we can identify where we are in the loop

						// Get our 2 bones to connect
						CatVector bone1, bone2;
						if (bones::GetBone(entity, current_set[i], bone1) && bones::GetBone(entity, current_set[i + 1], bone2)) {

							// World to screen them
							CatVector scn1, scn2;
							if (draw::WorldToScreen(bone1, scn1) && draw::WorldToScreen(bone2, scn2)) {

								// Draw a line connecting the points
								draw::Line(scn1.x, scn1.y, scn2.x - scn1.x, scn2.y - scn1.y, esp_cache[entity.IDX].color);
							}
						}
					}
				}

				// Debug for bone esp, prints strings on screen of what bone is being represented
				if (esp_bone_debug) {
					int tmp = -1;
					for (const auto& bone : entity.bones) {
						tmp++; // Increment bone
						// Check if bone is good
						if (!bone.first) continue;
						// Get wts
						CatVector scn;
						if (!draw::WorldToScreen(bone.second.center(), scn)) continue;
						// Draw
						char buf[8];
						sprintf(buf, "B:%i", tmp);
						draw::String(buf, scn.x, scn.y, 0, 20, colors::white);
					}
				}
			}

			// Box esp
			if (box_esp) {

				// Attempt to get our entity box
				if (GetEntityBox(entity)) {

					// Simple box esp
					draw::Rect(screenbox.second.min.x, screenbox.second.min.y, screenbox.second.max.x - screenbox.second.min.x, screenbox.second.max.y - screenbox.second.min.y, colors::black);
					draw::Rect(screenbox.second.min.x + 1, screenbox.second.min.y + 1, screenbox.second.max.x - screenbox.second.min.x - 2, screenbox.second.max.y - screenbox.second.min.y - 2, esp_cache[entity.IDX].color);
					draw::Rect(screenbox.second.min.x + 2, screenbox.second.min.y + 2, screenbox.second.max.x - screenbox.second.min.x - 4, screenbox.second.max.y - screenbox.second.min.y - 4, colors::black);
				}
			}

			// Health bars
			if ((int)show_health >= 2) {

				// Attempt to get our entity box
				if (GetEntityBox(entity)) {

					// Get in bar height
					int hbh = (screenbox.second.max.y - screenbox.second.min.y - 2) * std::min((float)entity.health / (float)entity.max_health, 1.0f);

					// Draw
					draw::Rect(screenbox.second.min.x - 7, screenbox.second.min.y, 7, screenbox.second.max.y - screenbox.second.min.y, colors::black);
					draw::RectFilled(screenbox.second.min.x - 6, screenbox.second.max.y - hbh - 1, 5, hbh, colors::Health(entity));
				}
			}
		}

		// Strings
		// Check if there is strings to draw
		if (esp_cache[entity.IDX].string_count) {

			// Get world to screen
			CatVector draw_point;
			if (draw::WorldToScreen(entity.origin, draw_point)) {

				// Change draw point if needed & determine wheter we center the strings
				bool center_strings = true;
				if (screenbox.first != EBOX_NOT_RAN && (int)esp_text_position != 1) { // Check if we have an entity box
					center_strings = false; // Dont center strings

					// Change the position of the draw point depending on the user settings
					if ((int)esp_text_position > 3) { // For when we need total height of strings
						// Get total height
						int total_height = 0;
						for (int ii = 0; ii < esp_cache[entity.IDX].string_count; ii++) {
							auto size = draw::GetStringLength(esp_cache[entity.IDX].strings[ii].first, 1, 28);
							total_height += size.second;
						}
						switch ((int)esp_text_position) { // Add more as needed
						case 4: // ABOVE
							draw_point = CatVector(screenbox.second.max.x + 2, screenbox.second.max.y - total_height); break;
						case 5: // BOTTOM RIGHT
							draw_point = CatVector(screenbox.second.min.x, screenbox.second.min.y - total_height);
						}
					} else {
						switch ((int)esp_text_position) { // Same as above
						case 0: // TOP RIGHT
							draw_point = CatVector(screenbox.second.max.x + 2, screenbox.second.min.y, 0); break;
						case 2: // BELOW
							draw_point = CatVector(screenbox.second.min.x, screenbox.second.max.y, 0);
						}
					}
				}

				// Draw our stringsDraw
				// Loop through strings
				for (int i = 0; i < esp_cache[entity.IDX].string_count; i++) {

					// Get string sizes
					auto size = draw::GetStringLength(esp_cache[entity.IDX].strings[i].first, 1, 28);

					if (center_strings) // Centered strings
						draw::String(esp_cache[entity.IDX].strings[i].first, draw_point.x - size.first / 2, draw_point.y, 1, 28, esp_cache[entity.IDX].strings[i].second);
					else // Not centered
						draw::String(esp_cache[entity.IDX].strings[i].first, draw_point.x, draw_point.y, 1, 28, esp_cache[entity.IDX].strings[i].second);

					// Lower draw point for recursions
					draw_point.y += size.second;
				}
			}
		}
	}
}

// Tick to be run at world tick for cache purposes
static void WorldTick() {
	// We dont want esp if its disabled, or while not ingame
	if (!esp_enabled || !g_GameInfo.in_game) return;

	// Loop through all entitys
	for (const auto& entity : g_CatEntitys) {
		if (CE_BAD(entity)) continue;

		// Update the entitys color
		esp_cache[entity.IDX].color = colors::EntityColor(entity);
		// Reset strings
		esp_cache[entity.IDX].string_count = 0;

		// Add default strings
		if ((esp_players && entity.type == ETYPE_PLAYER) || (esp_other_hostile && entity.type == ETYPE_OTHERHOSTILE)) {
			char buf[64]; // for sprintf

			// Name esp
			if (show_name)
				AddEspString(entity, entity.entity_name, esp_cache[entity.IDX].color);

			// Health esp
			if ((int)show_health == 1 || (int)show_health == 3) {
				sprintf(buf, "%i/%iHP", entity.health, entity.max_health);
				AddEspString(entity, buf, colors::Health(entity));
			}

			// Distance esp
			if (show_distance) {
				sprintf(buf, "%im", (int)entity.Distance());
				AddEspString(entity, buf);
			}
		}
	}
}

// Please add your esp strings during world tick as it is less intensive than doing this at draw
// Use to add a string to esp for an entity with a color
void AddEspString(const CatEntity& entity, const char* input_string, CatVector4 color) {
	// Copy our values
	strcpy(esp_cache[entity.IDX].strings[esp_cache[entity.IDX].string_count].first, input_string);
	esp_cache[entity.IDX].strings[esp_cache[entity.IDX].string_count].second = color;
	// Incriment count
	esp_cache[entity.IDX].string_count++;
}

void SetEspColor(const CatEntity& entity, CatVector4 color) {
	// change any strings with entity color to the new one
	for (int i = 0; i < esp_cache[entity.IDX].string_count; i++) {
		if (esp_cache[entity.IDX].strings[i].second == esp_cache[entity.IDX].color)
			esp_cache[entity.IDX].strings[i].second = color;
	}
	// Change the main entity color
	esp_cache[entity.IDX].color = color;
}

void Init() {
	drawmgr_on(Draw);
	wtickmgr_before(WorldTick);
}

}}
