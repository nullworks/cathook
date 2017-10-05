
/*
 *
 *	The cathook framework uses its own "entitys" to do its actions.
 *  This allows it to work with any module that can fill the entity manager with entitys
 *
 */

#include "localplayers.hpp" // Used for distance to local player

#include "entitys.hpp"

namespace entity_cache {

// The main storage for our entity cache
static CatEntity array[MAX_ENTITIES];

// Stores number of highest entity we can use. Please set in your module
static int HIGHEST_ENTITY = 0;

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
}
	
}

// Reset this entity
void CatEntity::Reset() {
	exists = false;
	dormant = true;
	//entityName = "unknown"; // Useless and wasteful to set?
	team = ETEAM_NONE;
	type = ETYPE_NONE;
	alive = false;
	enemy = false;
	maxHealth = 0;
	health = 0;
	origin = CatVector(0, 0, 0);
}

// Returns the entity number from the entity array
int CatEntity::IDX() {
	return int(((unsigned)this - (unsigned)&entity_cache::array) / sizeof(CatEntity));
}

float CatEntity::Distance() {
	if (g_LocalPlayer.CatEntity)
		return origin.DistTo(g_LocalPlayer.CatEntity->origin);
	return 0;	
}

