
/*
 * entitycache.cpp
 *
 *  Created on: Nov 7, 2016
 *      Author: nullifiedcat
 */

#include "entitymgr.hpp"

// This method of const'ing the index is weird.

void CatEntity::Reset() {
	
	exists = false; // Whether The entity exists in game
	dormant = true; // Exists but not active
	entityName = "unknown";
	team = ETEAM_NONE; // Use enum
	alive = false;
	enemy = false;
	maxHealth = 0;
	health = 0;
	origin = CatVector(0, 0, 0);
}

int CatEntity::m_IDX() {
	return int(((unsigned)this - (unsigned)&entity_cache::array) / sizeof(CatEntity));
}

namespace entity_cache {

CatEntity array[MAX_ENTITIES] {};

void Invalidate() { // Please invalidate on world tick
	// Reset
	for (auto& ent : array) {
		ent.Reset();
	}
	// Updating is the job of the modules
}

// Stores number of highest entity we can use.
static int HIGHEST_ENTITY = 0;

}

