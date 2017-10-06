
/*
 *
 *	Header for the entity mgr
 *
 */

#ifndef ENTITYMGR
#define ENTITYMGR

#include <iostream>
#include <stdexcept>
#include <string>

#include "../util/mathlib.hpp"

#define MAX_ENTITIES 2048 // Increase as needed

// Nice macros
#define CE_GOOD(a) (a && a->exists && !a->dormant)
#define CE_BAD(a) (!a || !a->exists || a->dormant)

// Just throw a bunch of possible teams in here. Increase as needed
enum {
	ETEAM_NONE,
	ETEAM_UNKNOWN,
	ETEAM_ENEMY,
	ETEAM_ALLY,
	ETEAM_RED,
	ETEAM_BLUE,
	ETEAM_YELLOW,
	ETEAM_GREEN
};

enum {
	ETYPE_NONE,			// Idk wut
	ETYPE_GENERIC,		// Some generic crap
	ETYPE_PLAYER,		// Hey, a player!
	ETYPE_OTHER,		// Something not known
	ETYPE_OTHERHOSTILE, // Not player but still hostile
	ETYPE_PROJECTILE,	// Bullet projectiles count 
	ETYPE_PICKUP_HEALTH,// Self explanitory
	ETYPE_PICKUP_SHEILD,	
	ETYPE_PICKUP_AMMO
};

class CatEntity {
public:
	void 		Reset();				// Resets this entity
	int 		IDX(); 					// Returns entity number
	bool		exists = false; 		// Whether The entity exists in game
	bool 		dormant = true; 		// Exists but not active
	
	std::string entity_name = "unknown";	// Name of the entity
	int  		team = ETEAM_NONE;
	bool 		alive = false;
	bool 		enemy = true;
	int 		max_health = 100;
	int 		health = 0;
	
	int			type = ETYPE_NONE;
	
	CatVector 	origin = CatVector(0, 0, 0);

	CatBox 		collision;
	float 		Distance(); // Distance from origin to local player
	
	// TODO, use for player list + player state
	unsigned int steam32 = 0;
			
};

namespace entity_cache {

extern CatEntity array[MAX_ENTITIES]; // b1g fat array in
// Used for getting our cached entity from an idx
inline CatEntity* Get(int IDX) {
	return &array[IDX];
}
void Invalidate(); // Use in world tick
void SetHighest(int inp); // Use to set highest entity
//CachedEntity* GetEntity(int IDX); // Returns ent from array
extern int HIGHEST_ENTITY;
}

#endif
