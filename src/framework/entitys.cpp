
/*
 *
 *	The cathook framework uses its own "entitys" to do its actions.
 *  This allows it to work with any module that can fill the entity manager with entitys
 *
 */

#include "entitys.hpp"

namespace entity_cache {

// The main storage for our entity cache
CatEntity array[MAX_ENTITIES];
	
// Stores number of highest entity we can use. Please set in your module
int HIGHEST_ENTITY = 0;

// Used to set highest entity usable.
void SetHighest(int inp) {
	if (inp >= MAX_ENTITIES) inp = MAX_ENTITIES - 1;
	HIGHEST_ENTITY = inp;
}
	
// Invalidates cache
void Invalidate() { // Please invalidate on world tick
	// Reset all
	for (CatEntity& ent : array) {
		ent.Reset();
	}
	HIGHEST_ENTITY = 0;
	// Reset our local player
	g_LocalPlayer.Reset();
}
	
}

// Reset this entity
void CatEntity::Reset() {
	exists = false;
	dormant = true;
	type = ETYPE_NONE;
	alive = false;
	enemy = false;
	max_health = 100;	// Good base health so we dont need to change if there isnt variable health
	health = 0;
}

// Returns the entity number from the entity array
int CatEntity::IDX() {
	return int(((unsigned)this - (unsigned)&entity_cache::array) / sizeof(CatEntity));
}

float CatEntity::Distance() {
	if (g_LocalPlayer.entity)
		return origin.DistTo(g_LocalPlayer.entity->origin);
	return 0;	
}

// Var for our local player
CLocalPlayer g_LocalPlayer;

void CLocalPlayer::Reset() {
	// Reset some stuff to defaults
	camera_position = CatVector();
	cam_in_thirdperson = false;
	// Our player commands
	attack         = false;
	attack_prevent = false;
	camera_angles  = CatVector();
	real_angles    = CatVector();
}



