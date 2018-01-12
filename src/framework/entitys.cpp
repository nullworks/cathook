
/*
 *
 *	The cathook framework uses its own "entitys" to do its actions.
 *  This allows it to work with any module that can fill the entity manager with entitys
 *
 */

#include "entitys.hpp"

CMFunction<bool(CatEntity*)> GetDormant {[](auto) -> auto { return true; }};
CMFunction<bool(CatEntity*)> GetAlive {[](auto) -> auto { return true; }};
CMFunction<int(CatEntity*)> GetHealth {[](auto) -> auto { return 100; }};
CMFunction<int(CatEntity*)> GetMaxHealth {[](auto) -> auto { return 100; }};
CMFunction<int(CatEntity*)> GetTeam {[](auto) -> int { return ETEAM_NONE; }};
CMFunction<int(CatEntity*)> GetType {[](auto) -> int { return ETYPE_NONE; }};
CMFunction<const char*(CatEntity*)> GetName {[](auto) -> auto { return "unknown"; }};
CMFunction<CatVector(CatEntity*)> GetOrigin {[](auto) -> auto { return CatVector(); }};
CMFunction<CatBox(CatEntity*)> GetCollision {[](auto) -> auto { return CatBox(); }};
CMFunction<int(CatEntity*)> GetSteamId {[](auto) -> auto { return -1; }};
CMFunction<bool(CatEntity*, int, CatBox&)> GetBone {[](CatEntity*, int, CatBox&) -> auto { return false; }};

CMFunction<bool(CatLocalPlayer*)> InThirdperson {[](auto) -> auto { return false; }};
CMFunction<void(CatLocalPlayer*)> Attack {[](auto){}};
CMFunction<CatVector(CatLocalPlayer*)> GetCamera {[](auto) -> auto { return CatVector(); }};
CMFunction<CatVector(CatLocalPlayer*)> GetCameraAngle {[](auto) -> auto { return CatVector(); }};
CMFunction<void(CatLocalPlayer*, CatVector)> SetCameraAngle {[](CatLocalPlayer*, CatVector){}};

CMFunction<int()> GetEntityCount {[]() -> auto { return 0; }};
CMFunction<CatEntity*(int)> GetEntity {[](auto) -> CatEntity* { return nullptr; }};
CMFunction<CatLocalPlayer*()> GetLocalPlayer {[]() -> CatLocalPlayer* { return nullptr; }};

float GetDistance(CatEntity* this_ptr) {
	auto local_ent = GetLocalPlayer();
	return (local_ent) ? GetOrigin(this_ptr).DistTo(GetOrigin((CatEntity*)local_ent)) : 0;
};
bool GetEnemy(CatEntity* this_ptr) {
	auto team = GetTeam(this_ptr);
	if (team == ETEAM_ALLY) return false;
	if (team == ETEAM_ENEMY) return true;
	auto local_ent = (CatEntity*)GetLocalPlayer();
	if (local_ent == this_ptr) return false; // Local ents are friendly, duh
	if (local_ent) return GetTeam(local_ent) != team;
	return true;
}

namespace bones {
// Bone stuff
// This is how the bones are layed out
// Use bonesets[0] for center line
const std::vector<int> bonesets[] = {
	{EBone_head, EBone_top_spine, EBone_upper_spine, EBone_middle_spine, EBone_bottom_spine, EBone_pelvis}, // Center
	{EBone_lower_arm_l, EBone_middle_arm_l, EBone_upper_arm_l, EBone_top_spine, EBone_upper_arm_r, EBone_middle_arm_r, EBone_lower_arm_r}, // Upper limbs
	{EBone_lower_leg_l, EBone_middle_leg_l, EBone_upper_leg_l, EBone_pelvis,    EBone_upper_leg_r, EBone_middle_leg_r, EBone_lower_leg_r}  // Lower limbs
};

}
