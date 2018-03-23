
/*
 *
 *	Header for the entity mgr
 *
 */

#pragma once

#include "../util/mathlib.hpp"	// CatVectors and CatBoxes
#include "../util/functions.hpp" // CMFunction()

// Enums, yay!
enum { // Entity Team
	ETEAM_NONE,
	ETEAM_UNKNOWN,
	ETEAM_ENEMY,
	ETEAM_ALLY,
	ETEAM_RED,
	ETEAM_BLUE,
	ETEAM_YELLOW,
	ETEAM_GREEN
};
enum { // Entity Type
	ETYPE_NONE,			// Idk wut
	ETYPE_PLAYER,		// Hey, a player!
	ETYPE_OTHERHOSTILE, // Not player but still hostile
	ETYPE_PROJECTILE,	// Bullet projectiles count
	ETYPE_PICKUP_HEALTH,// Self explanitory
	ETYPE_PICKUP_SHEILD,
	ETYPE_PICKUP_AMMO,
	ETYPE_GENERIC		// Some generic crap
};
enum { // Bone position enum, Add bones as needed, position shouldnt matter if everything is made right...
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

class CatEntity; // incomplete class, basicly void*, cast to and from your games entity pointers
// main functions to get info out of entities
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

class CatLocalPlayer; // same as above
// Local player specific
extern CMFunction<void(CatLocalPlayer*)> Attack;
extern CMFunction<void(CatLocalPlayer*)> Jump;
extern CMFunction<void(CatLocalPlayer*, CatVector)> WalkTo;
extern CMFunction<bool(CatLocalPlayer*)> InThirdperson;
extern CMFunction<CatVector(CatLocalPlayer*)> GetCamera;
extern CMFunction<CatVector(CatLocalPlayer*)> GetCameraAngle;
extern CMFunction<void(CatLocalPlayer*, CatVector)> SetCameraAngle;
extern CMFunction<void(CatLocalPlayer*, CatVector)> SetSilentCameraAngle;
// Entity manager stuff
extern CMFunction<int()> GetEntityCount;
extern CMFunction<CatEntity*(int)> GetEntity;
extern CMFunction<CatLocalPlayer*()> GetLocalPlayer;

// "Make life easy"
inline bool GetBoneCenter(CatEntity* this_ptr, int hb, CatVector& bone) {
	CatBox tmp;
	auto ret = GetBone(this_ptr, hb, tmp);
	if (ret) bone = tmp.GetCenter(); return ret;
}
inline float GetDistance(CatEntity* this_ptr) {
	auto local_ent = GetLocalPlayer();
	return (local_ent) ? GetOrigin(this_ptr).DistTo(GetOrigin((CatEntity*)local_ent)) : 0;
};
inline bool GetEnemy(CatEntity* this_ptr) {
	auto team = GetTeam(this_ptr);
	if (team == ETEAM_ALLY) return false;
	if (team == ETEAM_ENEMY) return true;
	auto local_ent = (CatEntity*)GetLocalPlayer();
	if (local_ent == this_ptr) return false; // Local ents are friendly, duh
	if (local_ent) return GetTeam(local_ent) != team;
	return true;
}
