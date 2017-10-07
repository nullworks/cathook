 
/*
 *
 *	The esp features header file!
 *
 */

#include "../util/colors.hpp" // For colors and colored strings

#define MAX_ESP_STRINGS 32	  // Strings allowed for esp

namespace esp {

// Entitys strings
struct ESPData {
	int string_count = 0;
	colors::ColoredString strings[MAX_ESP_STRINGS];
	rgba_t color = colors::white;
};

// Entity Box state enum
enum {
	EBOX_NOT_RAN,
	EBOX_FAILED,
	EBOX_SUCCESSFUL
};
	
// Main esp func to be called on draw
void Draw();

void WorldTick();

// Use in world tick to add a string to an entity
void AddEspString(CatEntity* entity, std::string input_string, rgba_t color);
void AddEspString(CatEntity* entity, std::string input_string);

}