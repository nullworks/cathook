
/*
 *
 *	Header for the entity mgr
 *
 */

#pragma once

#include <utility> // Pair
#include <vector> // For bone stuff

#include "../util/mathlib.hpp"	// CatVectors and CatBoxes
#include "../util/functions.hpp" // CMFunction()

// Entity Team
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

// Entity Type
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

class CatEntity {
public:
	virtual bool GetDormant() = 0;
	virtual bool GetAlive() = 0;
	virtual int GetHealth();
	virtual int GetMaxHealth();
	virtual int GetTeam() = 0;
	virtual int GetType() = 0;
	virtual const char* GetName();
	virtual CatVector GetOrigin() = 0;
	virtual CatBox GetCollision();
	virtual int GetSteamId();
	virtual bool GetBone(int bone, CatBox& input);
	virtual bool GetBone(int bone, CatVector& input);
};
class CatLocalPlayer : public CatEntity {
public:
	virtual bool InThirdperson() {}
	virtual void Attack() {}
	virtual CatVector GetCamera() { return CatVector(); } 	// Get point where the users camera is
	virtual CatVector GetCameraAngle() { return CatVector(); }
	virtual void SetCameraAngle(CatVector) {}
};

CMFunction<int()> GetEntityCount {[](){ return 0; }};
CMFunction<CatEntity*(int)> GetEntity {[](int) -> CatEntity* { return nullptr; }};
CMFunction<CatLocalPlayer*()> GetLocalPlayer {[]() -> CatLocalPlayer* { return nullptr; }};

// Bone stuff
namespace bones {

// Sets of bones that are connected
extern const std::vector<int> bonesets[3];

}
