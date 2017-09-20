#include <iostream>
#include <stdexcept>
#include <string>
#include "../util/mathlib.hpp"


#define MAX_STRINGS 32
#define MAX_ENTITIES 2048 // Increase as needed

// Just throw a bunch of possible teams in here. Increase as needed
enum ETeam {
	ETEAM_NONE,
	ETEAM_UNKNOWN,
	ETEAM_ENEMY,
	ETEAM_ALLY,
	ETEAM_RED,
	ETEAM_BLUE,
	ETEAM_YELLOW,
	ETEAM_GREEN
};

class CatEntity {
public:
	void Reset();
	int 		m_IDX(); 		// Returns entity number
	bool		exists = false; // Whether The entity exists in game
	bool 		dormant = true; // Exists but not active
	std::string entityName = "unknown";
	int  		team = ETEAM_NONE;
	bool 		alive = false;
	bool 		enemy = false;
	int 		maxHealth = 0;
	int 		health = 0;
	CatVector 	origin = CatVector(0, 0, 0);
	CatVector 	collision[8];
	//float m_flDistance { 0.0f }; // Use local player mgr
	
	// Use for projectile proj
	//Vector m_vecVelocity { 0 };
	//Vector m_vecAcceleration { 0 };
	
};

namespace entity_cache {

extern CatEntity array[MAX_ENTITIES]; // b1g fat array in
inline CatEntity& Get(int idx) {
	if (idx < 0 || idx >= MAX_ENTITIES) throw std::out_of_range("Entity index out of range!");
	return array[idx];
}
void Invalidate(); // Use in world tick
	
extern int HIGHEST_ENTITY;
}


