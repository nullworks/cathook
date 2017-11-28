
/*
 *
 *	Header for the entity mgr
 *
 */

#pragma once

#include <utility> // Pair
#include <vector> // For bone stuff

#include "../util/mathlib.hpp"	// CatVectors and CatBoxes

#define MAX_ENTITIES 2048 // Increase as needed

// Nice macros
#define CE_GOOD(a) (a.exists && !a.dormant)
#define CE_BAD(a) (!a.exists || a.dormant)

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

// Bone position enum, Add bones as needed!
enum {
	EBone_head,			// Middle section
	EBone_top_spine,
	EBone_upper_spine,
	EBone_middle_spine,
	EBone_bottom_spine,
	EBone_pelvis,
	EBone_upper_arm_l,	// Upper limbs
	EBone_upper_arm_r,
	EBone_middle_arm_l,
	EBone_middle_arm_r,
	EBone_lower_arm_l,
	EBone_lower_arm_r,
	EBone_upper_leg_l,	// Lower limbs
	EBone_upper_leg_r,
	EBone_middle_leg_l,
	EBone_middle_leg_r,
	EBone_lower_leg_l,
	EBone_lower_leg_r,
	EBone_count
};

// Class to contain entity information
class CatEntity {
public:
	CatEntity();
	inline bool operator==(const CatEntity& in_ent) const { return IDX == in_ent.IDX; }

	// Generic entity functions
	inline void Reset() {
		exists = false;
		dormant = true;
		alive = false;
		type = ETYPE_NONE;
		health = 100;
		max_health = 100;
	}
	// Cant inline these for... uhh... reasons...
	const int IDX; // Returns entity number in the array
	float Distance() const;
	bool Enemy() const;
	// Generic entity states
	bool exists = false; 	// Whether The entity exists in game
	bool dormant = true; 	// Exists but not active
	bool alive = false;
	int health = 0;
	int max_health = 100;
	int team = ETEAM_NONE;
	int	type = ETYPE_NONE;

	// Other usefull info
	char entity_name[50] = "unknown";// Name of the entity
	CatVector origin = CatVector();
	CatBox collision = CatBox();
	int steam32 = 0;

	// To store hitbox/bone location, This is for the bone manager and related to handle!
	std::pair<bool, CatBox> bones[EBone_count];
};

class CatLocalPlayer {
public:
	inline void Reset() {
		// Reset some stuff to defaults
		camera_position = CatVector();
		InThirdperson = false;
		// Our player commands
		attack         = false;
		attack_prevent = false;
		camera_angles  = CatVector();
		real_angles    = CatVector();
	}

	CatEntity* entity = nullptr;		// Contains the cat entity of our local player
	CatVector camera_position = CatVector(); 	// Point where the users camera is
	bool InThirdperson = false;			// Set to true if your camera is in thirdperson

	// Our player commands
	bool attack 		   			= false;		// Used to control if attacking should happen
	bool attack_prevent   	= false;		// Used for when you wish to prevent attacking
	CatVector camera_angles = CatVector();	// Angles of what the player sees
	CatVector real_angles   = CatVector();	// The real angles that the game should be at and keep sync with
};

// Global entity info
extern CatEntity g_CatEntitys[];
extern CatLocalPlayer g_LocalPlayer;
inline CatEntity& GetEntity(const int& IDX) { return g_CatEntitys[IDX]; }

// Bone stuff
namespace bones {

// Sets of bones that are connected
extern const std::vector<int> bonesets[3];

// Returns a reference to a catbone if it exists
inline bool GetBone(const CatEntity& entity, const int& bone, CatVector& input) {
	// Ensure we have a bone
	if (bone < 0 || bone >= EBone_count) return false;
	if (!entity.bones[bone].first) return false;
	// Change input and return
	input = entity.bones[bone].second.center();
	return true;
}

inline bool GetBone(const CatEntity& entity, const int& bone, CatBox& input) {
	// Ensure we have a bone
	if (bone < 0 || bone >= EBone_count) return false;
	if (!entity.bones[bone].first) return false;
	// Change input and return
	input = entity.bones[bone].second;
	return true;
}

// Use to reset bones
void ResetBones(CatEntity& entity) { for (auto& box : entity.bones) box.first = false; }

}
