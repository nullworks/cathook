
/*
 *
 *	Header for the entity mgr
 *
 */

#pragma once

#include <iostream>
#include <stdexcept>
#include <string>

#include <unordered_map> 		// Used to store our bones
#include "../util/mathlib.hpp"	// CatVectors and CatBoxes

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
	ETYPE_PLAYER,		// Hey, a player!
	ETYPE_OTHERHOSTILE, // Not player but still hostile
	ETYPE_PROJECTILE,	// Bullet projectiles count 
	ETYPE_PICKUP_HEALTH,// Self explanitory
	ETYPE_PICKUP_SHEILD,	
	ETYPE_PICKUP_AMMO,
	ETYPE_GENERIC		// Some generic crap
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
	
	// To store hitbox/bone locations
	// This is for the bone manager to handle!
	// Int is the bone from the enum, catbox is the hitbox
	std::unordered_map<int, CatBox> bones;
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

class CLocalPlayer {
public:
	void Reset();
	
	CatEntity* entity = nullptr;	// Contains the cat entity of our local player
	CatVector camera_position = CatVector(); 	// Point where the users camera is 
	bool cam_in_thirdperson = false;	// Set to true if your camera is in thirdperson
	
	// Our player commands
	bool attack 		   = false;					// Used to control if attacking should happen
	bool attack_prevent    = false;			// Used for when you wish to prevent attacking
	CatVector camera_angles = CatVector();	// Angles of what the player sees
	CatVector real_angles   = CatVector();	// The real angles that the game should be at and keep sync with
};

extern CLocalPlayer g_LocalPlayer;

