 
/*
 *
 *	The esp features header file!
 *
 */

#include "../util/colors.hpp" 	// For colors and colored strings
#include "../util/catvars.hpp"	// I would like to use catvars

#define MAX_ESP_STRINGS 32	  // Strings allowed for esp

namespace features { namespace esp {

extern CatEnum esp_menu;
	
// Entitys strings
struct ESPData {
	int string_count = 0;
	colors::ColoredString strings[MAX_ESP_STRINGS];
	rgba_t color = colors::white;
};
	
// Main esp func to be called on draw
void Draw();

void WorldTick();

// Use in world tick to add a string to an entity
void AddEspString(CatEntity* entity, std::string input_string, const rgba_t& color);
void AddEspString(CatEntity* entity, std::string input_string);
// Use in world tick to set esp color
void SetEspColor(CatEntity* entity, const rgba_t& color);

}}