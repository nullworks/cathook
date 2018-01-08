
/*
 *
 *	The cathook framework uses its own "entitys" to do its actions.
 *  This allows it to work with any module that can fill the entity manager with entitys
 *
 */

#include "entitys.hpp"

bool CatEntity::GetDormant() { return true; }
bool CatEntity::GetAlive() { return false; }
int CatEntity::GetHealth() { return 100; }
int CatEntity::GetMaxHealth() { return 100; }
int CatEntity::GetTeam() { return ETEAM_NONE; }
int CatEntity::GetType() { return ETYPE_NONE; }
const char* CatEntity::GetName() { return "unknown"; }
CatVector CatEntity::GetOrigin() { return CatVector(); }
CatBox CatEntity::GetCollision() { return CatBox(); }
int CatEntity::GetSteamId() { return -1; }
bool CatEntity::GetBone(int bone, CatBox& input) { return false; }
bool CatEntity::GetBone(int bone, CatVector& input) {
	CatBox tmp_box;
	auto ret = GetBone(bone, tmp_box);
	if (ret) input = tmp_box.center();
	return ret;
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
