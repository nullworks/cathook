 
/*
 *
 *	This is the main esp for cathook.
 *
 */

#include "../logging.h"						// For debugging purposes
#include "../framework/entitys.hpp"			// So we can use entitys
#include "../framework/localplayers.hpp"	// So we can determine whether to apply esp to local player
#include "../framework/worldtoscreen.hpp"	// ofc we need world to screen
#include "../framework/inputmgr.hpp"		// So we can get screen size
#include "../framework/drawing.hpp"			// We do lots of drawing!
#include "../framework/game.hpp"			// So we can stop esp if not ingame
#include "../util/catvars.hpp"				// I would like to use catvars

#include "esp.hpp"

// Currently, the gui doesnt reconize these. Just set to the options you wish to use
CatEnum esp_menu({ "Visuals", "Esp" }); // Menu locator for esp settings
CatVarBool esp_enabled(esp_menu, "esp", true, "ESP", "Master esp switch");
// Box esp + Options
CatEnum box_esp_enum({ "None", "Normal", "Corners" });
CatVarInt box_esp(esp_menu, box_esp_enum, "esp_box", 1, "Box", "Draw a 2D box");
CatVarInt box_corner_size(esp_menu, "esp_box_corner_size", 10, "Corner Size", "Controls corner box size");
// Tracers
CatEnum tracers_enum({ "OFF", "CENTER", "BOTTOM" });
CatVarInt tracers(esp_menu, tracers_enum, "esp_tracers", 2, "Tracers", "Draws a line from the player to a position on your screen");
	
namespace esp {

// Entity Box state enum
enum {
	EBOX_NOT_RAN,
	EBOX_FAILED,
	EBOX_SUCCESSFUL
};
static int ebox_state = EBOX_NOT_RAN; 	// To store the state of our box cache
static CatBox ebox;						// To store the cached entity box

// Sets the screenbox for an entity
bool GetEntityBox(CatEntity* entity) {
	if (ebox_state != EBOX_NOT_RAN) return ebox_state == EBOX_SUCCESSFUL; // If we already have the ebox, we return true
	ebox_state = EBOX_FAILED; // Pre-set this so we can return false at any time without worry
	ebox = CatBox(CatVector(65536, 65536), CatVector(-1, -1)); // Reset our cached screen box
	
	// Create a array for storing box points
	static CatVector points_w[8]; // World vectors
	static CatVector tmp_screen;  // Tmp vec for world to screen
	// Get our 8 points of our box
	entity->collision.GetPoints(points_w);

	// Go through the points getting world to screen and create our screenbox with them
	for (int i = 0; i < 8; i++) {
		if (!draw::WorldToScreen(points_w[i], tmp_screen)) return false; // Get world to screen. Return false if we dont get our point
		
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
	for (int i = 0; i < entity_cache::HIGHEST_ENTITY; i++) {
		// Get our ent and ensure its okay to use
		CatEntity* entity = entity_cache::Get(i);
		if (CE_BAD(entity)) continue;

		// Ensure we have an entity we use in esp
		if (entity->type != ETYPE_PLAYER) continue;
		
		// Determine whether to apply esp to local player
		if (!g_LocalPlayer.cam_in_thirdperson && g_LocalPlayer.CatEntity == entity) continue;
		
		// Dont esp dead players
		if (entity->type == ETYPE_PLAYER && !entity->alive) continue;
		
		rgba_t ent_color = colors::Entity(entity); // Get our entitys color
		ebox_state = EBOX_NOT_RAN;	// Reset the entity box state
		
		// Tracers
		if (tracers && entity->type == ETYPE_PLAYER) {
				
			// Get world to screen
			CatVector scn;
			CatVector origin = entity->origin;
			if (draw::WorldToScreen(origin, scn)) {

				// Center values on screen
				int width = CatUserInp.boundsx / 2;
				int height = ((int)tracers == 2) ? CatUserInp.boundsy : CatUserInp.boundsy / 2; // Only center height if we are using center mode

				// Draw a line
				draw::Line(scn.x, scn.y, width - scn.x, height - scn.y, ent_color);
			}
		}
		
		// Box esp
		if (box_esp && entity->type == ETYPE_PLAYER) {
			
			// Attempt to get our entity box
			if (GetEntityBox(entity)) {
				
				// Simple box esp
				if ((int)box_esp == 1) {
					draw::Rect(ebox.min.x, ebox.min.y, ebox.max.x - ebox.min.x, ebox.max.y - ebox.min.y, colors::black);
					draw::Rect(ebox.min.x + 1, ebox.min.y + 1, ebox.max.x - ebox.min.x - 2, ebox.max.y - ebox.min.y - 2, ent_color);
					draw::Rect(ebox.min.x + 2, ebox.min.y + 2, ebox.max.x - ebox.min.x - 4, ebox.max.y - ebox.min.y - 4, colors::black);
			
				// With box corners, we do some different technique
				} else {
					// Black corners
					draw::RectFilled(ebox.min.x, ebox.min.y, (int)box_corner_size, 3, colors::black);								// Top Left
					draw::RectFilled(ebox.min.x, ebox.min.y + 3, 3, (int)box_corner_size - 3, colors::black);
					draw::RectFilled(ebox.max.x - (int)box_corner_size + 1, ebox.min.y, (int)box_corner_size, 3, colors::black);		// Top Right
					draw::RectFilled(ebox.max.x - 3 + 1, ebox.min.y + 3, 3, (int)box_corner_size - 3, colors::black);
					draw::RectFilled(ebox.min.x, ebox.max.y - 3, (int)box_corner_size, 3, colors::black);							// Bottom Left
					draw::RectFilled(ebox.min.x, ebox.max.y - (int)box_corner_size, 3, (int)box_corner_size - 3, colors::black);
					draw::RectFilled(ebox.max.x - (int)box_corner_size + 1, ebox.max.y - 3, (int)box_corner_size, 3, colors::black); // Bottom Right
					draw::RectFilled(ebox.max.x - 2, ebox.max.y - (int)box_corner_size, 3, (int)box_corner_size - 3, colors::black);
					// Colored corners
					draw::Line(ebox.min.x + 1, ebox.min.y + 1, (int)box_corner_size - 2, 0, ent_color); 		// Top Left
					draw::Line(ebox.min.x + 1, ebox.min.y + 1, 0, (int)box_corner_size - 2, ent_color);
					draw::Line(ebox.max.x - 1, ebox.min.y + 1, -((int)box_corner_size - 2), 0, ent_color); 	// Top Right
					draw::Line(ebox.max.x - 1, ebox.min.y + 1, 0, (int)box_corner_size - 2, ent_color);
					draw::Line(ebox.min.x + 1, ebox.max.y - 2, (int)box_corner_size - 2, 0, ent_color);		// Bottom Left
					draw::Line(ebox.min.x + 1, ebox.max.y - 2, 0, -((int)box_corner_size - 2), ent_color);
					draw::Line(ebox.max.x - 1, ebox.max.y - 2, -((int)box_corner_size - 2), 0, ent_color);	// Bottom Right
					draw::Line(ebox.max.x - 1, ebox.max.y - 2, 0, -((int)box_corner_size - 2), ent_color);
				}
			}
		}
	}
}




}