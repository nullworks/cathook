
/*
 *
 *	Header for the entity mgr
 *
 */

#pragma once

#include <memory> // std::unique_ptr

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

class CatEntity;

extern CMFunction<bool(CatEntity*)> GetDormant;
extern CMFunction<bool(CatEntity*)> GetAlive;
extern CMFunction<int(CatEntity*)> GetHealth;
extern CMFunction<int(CatEntity*)> GetMaxHealth;
extern CMFunction<int(CatEntity*)> GetTeam;
extern CMFunction<int(CatEntity*)> GetType;
extern CMFunction<const char*(CatEntity*)> GetName;
extern CMFunction<CatVector(CatEntity*)> GetOrigin;
extern CMFunction<CatBox(CatEntity*)> GetCollision;
extern CMFunction<int(CatEntity*)> GetSteamId;
extern CMFunction<bool(CatEntity*, int, CatBox&)> GetBone;
inline bool GetBoneCenter(CatEntity* this_ptr, int hb, CatVector& bone) {
	CatBox tmp;
	auto ret = GetBone(this_ptr, hb, tmp);
	bone = tmp.GetCenter();
	return ret;
}
float GetDistance(CatEntity*);
bool GetEnemy(CatEntity*);

class CatLocalPlayer;

extern CMFunction<bool(CatLocalPlayer*)> InThirdperson;
extern CMFunction<void(CatLocalPlayer*)> Attack;
extern CMFunction<CatVector(CatLocalPlayer*)> GetCamera;
extern CMFunction<CatVector(CatLocalPlayer*)> GetCameraAngle;
extern CMFunction<void(CatLocalPlayer*, CatVector)> SetCameraAngle;

extern CMFunction<int()> GetEntityCount;
extern CMFunction<CatEntity*(int)> GetEntity;
extern CMFunction<CatLocalPlayer*()> GetLocalPlayer;

// Bone stuff
namespace bones {

// Sets of bones that are connected
extern const std::vector<int> bonesets[3];

}
