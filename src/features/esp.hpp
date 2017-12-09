
/*
 *
 *	The esp features header file!
 *
 */

#pragma once 
#include "../util/colors.hpp" 	// For colors and colored strings
#include "../framework/catvars.hpp"	// I would like to use catvars

#define MAX_ESP_STRINGS 32	  // Strings allowed for esp

namespace features { namespace esp {

extern CatEnum esp_menu;
extern CatVarBool esp_enabled;

void Init(); // Call to init esp

// Use in world tick to add a string to an entity
void AddEspString(const CatEntity& entity, const char* input_string, const CatVector4& color = colors::white);
// Use in world tick to set esp color
void SetEspColor(const CatEntity& entity, const CatVector4& color);

}}
