
/*
 *
 *	This is the main esp for cathook.
 *
 */

#include <algorithm> // min()

#include "../util/logging.h"				// For debugging purposes
#include "../framework/entitys.hpp"			// So we can use entitys
#include "../framework/inputmgr.hpp"		// So we can get screen size
#include "../framework/drawing.hpp"			// We do lots of drawing!
#include "../framework/game.hpp"			// So we can stop esp if not ingame
#include "../gui/gui.hpp"					// For fonts

#include "esp.hpp"

// TODO, make set entity color change strings that are using the current entity color to another

namespace features { namespace esp {

// Currently, the gui doesnt reconize these. Just set to the options you wish to use
CatEnum esp_menu({ "Visuals", "Esp" }); // Menu locator for esp settings
CatVarBool esp_enabled(esp_menu, "esp", true, "ESP", "Master esp switch");
// Target selection
CatVarBool esp_players(esp_menu, "esp_players", true, "ESP Players", "Whether to esp with players");
CatVarBool esp_other_hostile(esp_menu, "esp_other_hostile", true, "ESP Other Hostile", "Whether to esp with other hostile entitys\nThis is anything not a player but still hostile to you");
// Box esp + Options
CatEnum box_esp_enum({ "None", "Normal", "Corners" });
CatVarInt box_esp(esp_menu, box_esp_enum, "esp_box", 1, "Box", "Draw a 2D box");
CatVarInt box_corner_size(esp_menu, "esp_box_corner_size", 10, "Corner Size", "Controls corner box size");
// Strings
CatEnum esp_text_position_enum({"TOP RIGHT", "CENTER", "BELOW", "ABOVE", "BOTTOM RIGHT"}); // Aranged in a way to make things easier below
CatVarInt esp_text_position(esp_menu, esp_text_position_enum, "esp_text_position", 0, "Text position", "Defines text position");
// Health Esp
CatEnum show_health_enum({ "None", "Text", "Healthbar", "Both" });
CatVarInt show_health(esp_menu, show_health_enum, "esp_health", 3, "Health ESP", "Show health");
// Other strings
CatVarBool show_name(esp_menu, "esp_name", true, "Name ESP", "Shows the entity names of entitys");
CatVarBool show_distance(esp_menu, "esp_distance", true, "Distance ESP", "Shows distance on entitys");
// Bone esp
CatVarBool bone_esp(esp_menu, "esp_bones", true, "Bone ESP", "Shows cached bones");
// Tracers
CatEnum tracers_enum({ "OFF", "CENTER", "BOTTOM" });
CatVarInt tracers(esp_menu, tracers_enum, "esp_tracers", 0, "Tracers", "Draws a line from the player to a position on your screen");

// Entitys strings
struct ESPData {
	int string_count = 0;
	std::pair<char[40], CatVector4> strings[MAX_ESP_STRINGS];
	CatVector4 color = colors::white;
};

// An array to store our cached esp strings
ESPData esp_cache[MAX_ENTITIES];

// Entity Box state enum
enum {EBOX_NOT_RAN, EBOX_FAILED, EBOX_SUCCESSFUL};
int ebox_state = EBOX_NOT_RAN; 	// To store the state of our box cache
CatBox ebox;					// To store the cached entity box

// Sets the screenbox for an entity
bool GetEntityBox(CatEntity* entity) {
	if (ebox_state != EBOX_NOT_RAN) return ebox_state == EBOX_SUCCESSFUL; // If we already have the ebox, we return true
	ebox_state = EBOX_FAILED; // Pre-set this so we can return false at any time without worry
	ebox = CatBox(CatVector(65536, 65536), CatVector(-1, -1)); // Reset our cached screen box

	// Create a array for storing box points
	static CatVector points_w[8]; // World vectors

	// Get our 8 points of our box
	entity->collision.GetPoints(points_w);

	// Go through the points getting world to screen and create our screenbox with them
	for (const CatVector& point : points_w) {
		static CatVector tmp_screen;
		if (!draw::WorldToScreen(point, tmp_screen)) return false; // Get world to screen. Return false if we dont get our point

		// Create and expand the bounds based our our last point
		if (tmp_screen.x > ebox.max.x) ebox.max.x = tmp_screen.x;
		if (tmp_screen.y > ebox.max.y) ebox.max.y = tmp_screen.y;
		if (tmp_screen.x < ebox.min.x) ebox.min.x = tmp_screen.x;
		if (tmp_screen.y < ebox.min.y) ebox.min.y = tmp_screen.y;
	}

	// We now have our entity box, set the state and return true
	ebox_state = EBOX_SUCCESSFUL;
	return true;
}

// Esp draw func to be ran at draw
void Draw() {

	// We dont want esp if its disabled, or while not ingame
	if (!esp_enabled || !g_GameInfo.in_game) return;

	// Loop through all entitys
	for (const CatEntity& entity : g_CatEntitys) {
		if (CE_BAD(entity)) continue;

		// Target checking
		if (!g_LocalPlayer.cam_in_thirdperson && g_LocalPlayer.entity == entity) continue;// Determine whether to apply esp to local player
		if (entity->type == ETYPE_PLAYER && !entity->alive) continue; // Dont esp dead players

		// Reset the entity box state
		ebox_state = EBOX_NOT_RAN;

		// Check if main esp features should apply
		if ((esp_players && entity->type == ETYPE_PLAYER) || (esp_other_hostile && entity->type == ETYPE_OTHERHOSTILE)) {

			// Tracers
			if (tracers) {

				// Get world to screen
				static CatVector scn;
				if (draw::WorldToScreen(entity->origin, scn)) {

					// Draw a line
					draw::Line(scn.x, scn.y, input::bounds.first / 2 - scn.x, ((int)tracers == 2) ? input::bounds.second : input::bounds.second / 2 - scn.y, esp_cache[i].color);
				}
			}

			// Bone esp
			if (bone_esp) {

				// Loop through the bone sets
				for (const std::vector<int> current_set : framework::bones::bonesets) {

					// Draw the bones in the bone set
					for (int i = 0; i < current_set.size() - 1; i++) { // We do it like this so we can identify where we are in the loop
						static CatVector bone1, bone2; // Storage vector we use elsewhere

						// Get our 2 bones to connect
						if (bones::GetCatBone(entity, current_set[i], bone1) && bones::GetCatBone(entity, current_set[i + 1], bone2)) {

							// World to screen them
							static CatVector scn1, scn2;
							if (draw::WorldToScreen(bone1, scn1) && draw::WorldToScreen(bone2, scn2))) {

								// Draw a line connecting the points
								draw::Line(scn1.x, scn1.y, scn2.x - scn1.x, scn2.y - scn1.y, esp_cache[i].color);
							}
						}
					}
				}
			}

			// Box esp
			if (box_esp) {

				// Attempt to get our entity box
				if (GetEntityBox(entity)) {

					// Simple box esp
					draw::Rect(ebox.min.x, ebox.min.y, ebox.max.x - ebox.min.x, ebox.max.y - ebox.min.y, colors::black);
					draw::Rect(ebox.min.x + 1, ebox.min.y + 1, ebox.max.x - ebox.min.x - 2, ebox.max.y - ebox.min.y - 2, esp_cache[i].color);
					draw::Rect(ebox.min.x + 2, ebox.min.y + 2, ebox.max.x - ebox.min.x - 4, ebox.max.y - ebox.min.y - 4, colors::black);
				}
			}

			// Health bars
			if ((int)show_health >= 2) {

				// Attempt to get our entity box
				if (GetEntityBox(entity)) {

					// Get in bar height
					int hbh = (ebox.max.y - ebox.min.y - 2) * std::min((float)entity->health / (float)entity->max_health, 1.0f);

					// Draw
					draw::Rect(ebox.min.x - 7, ebox.min.y, 7, ebox.max.y - ebox.min.y, colors::black);
					draw::RectFilled(ebox.min.x - 6, ebox.max.y - hbh - 1, 5, hbh, colors::Health(entity));
				}
			}
		}

		// Strings
		// Check if there is strings to draw
		if (esp_cache[i].string_count) {

			// Get world to screen
			static CatVector draw_point;
			if (draw::WorldToScreen(entity->origin, draw_point)) {

				// Change draw point if needed & determine wheter we center the strings
				bool center_strings = true;
				if (ebox_state != EBOX_NOT_RAN && (int)esp_text_position != 1) { // Check if we have an entity box
					center_strings = false; // Dont center strings

					// Change the position of the draw point depending on the user settings
					if ((int)esp_text_position > 3) { // For when we need total height of strings
						// Get total height
						int total_height = 0;
						static int height, width;
						for (int ii = 0; ii < esp_cache[i].string_count; ii++) {
							draw::GetStringLength(esp_cache[i].strings[ii].string.c_str(), (int)gui::gui_font, (int)gui::gui_font_size, width, height);
							total_height += height;
						}
						switch ((int)esp_text_position) { // Add more as needed
						case 4: // ABOVE
							draw_point = CatVector(ebox.max.x + 2, ebox.max.y - total_height); break;
						case 5: // BOTTOM RIGHT
							draw_point = CatVector(ebox.min.x, ebox.min.y - total_height);
						}
					} else {
						switch ((int)esp_text_position) { // Same as above
						case 0: // TOP RIGHT
							draw_point = CatVector(ebox.max.x + 2, ebox.min.y, 0); break;
						case 2: // BELOW
							draw_point = CatVector(ebox.min.x, ebox.max.y, 0);
						}
					}
				}

				// Draw our strings
				// Loop through strings
				for (int ii = 0; ii < esp_cache[i].string_count; ii++) {

					// Get string sizes
					static int width, height;
					draw::GetStringLength(esp_cache[i].strings[ii].string.c_str(), gui::gui_font, gui::gui_font_size, width, height);

					// Centered strings
					if (center_strings) {
						draw::String(esp_cache[i].strings[ii].string.c_str(), draw_point.x - width / 2, draw_point.y, (int)gui::gui_font, (int)gui::gui_font_size, esp_cache[i].strings[ii].color);

					// Not centered
					} else {
						draw::String(esp_cache[i].strings[ii].string.c_str(), draw_point.x, draw_point.y, (int)gui::gui_font, (int)gui::gui_font_size, esp_cache[i].strings[ii].color);
					}

					// Lower draw point for recursions
					draw_point.y += height;
				}
			}
		}
	}
}

// Esp Function to
void WorldTick() {
	// We dont want esp if its disabled, or while not ingame
	if (!esp_enabled || !g_GameInfo.in_game) return;

	// Loop through all entitys
	for (const CatEntity& entity : g_CatEntitys) {
		if (CE_BAD(entity)) continue;
		int i = entity.IDX();

		// Update the entitys color
		esp_cache[i].color = colors::Entity(entity);
		// Reset strings
		esp_cache[i].string_count = 0;

		// Add default strings
		if ((esp_players && entity->type == ETYPE_PLAYER) || (esp_other_hostile && entity->type == ETYPE_OTHERHOSTILE)) {

			// Name esp
			if (show_name)
				AddEspString(entity, entity->entity_name, esp_cache[i].color);

			// Health esp
			if ((int)show_health == 1 || (int)show_health == 3)
				AddEspString(entity, format(entity->health, '/', entity->max_health, " HP"), colors::Health(entity));

			// Distance esp
			if (show_distance)
				AddEspString(entity, format((int)entity->Distance(), "m"));
		}
	}
}

// Please add your esp strings during world tick as it is less intensive than doing this at draw
// Use to add a string to esp for an entity with a color
void AddEspString(CatEntity* entity,  std::string input_string, const rgba_t& color) {
	int idx = entity->IDX();
	if (esp_cache[idx].string_count >= MAX_ESP_STRINGS) return; // Prevent overflow
	esp_cache[idx].strings[esp_cache[idx].string_count] = std::make_pair(input_string, color);
	esp_cache[idx].string_count++;
}

void SetEspColor(CatEntity* entity, const rgba_t& color) {
	esp_cache[entity->IDX()].color = color;
}

}}
