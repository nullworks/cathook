
/*
 *
 *	The cathook framework uses its own "entitys" to do its actions.
 *  This allows it to work with any module that can fill the entity manager with entitys
 *
 */

// Stuff to init with

#include "entitys.hpp"

// Globals to control how entitys work
CatEntity g_CatEntitys[MAX_ENTITIES];
CatLocalPlayer g_LocalPlayer;

// We need a constructor to get the idx
CatEntity::CatEntity() IDX(int(((unsigned)this - (unsigned)&g_CatEntitys) / sizeof(CatEntity)) {

}

bool CatEntity::Enemy() {
	if (team == ETEAM_ALLY) return false;
	if (team == ETEAM_ENEMY) return true;
	if (g_LocalPlayer.entity == this) return false; // Local ents are friendly, duh
	if (g_LocalPlayer.entity) return g_LocalPlayer.entity->team != team;
	return true;
}

float CatEntity::Distance() {
	return (g_LocalPlayer.entity) ? origin.DistTo(g_LocalPlayer.entity->origin) : 0;
}

// Bone stuff
// This is how the bones are layed out
// Use bonesets[0] for center line
const std::vector<int> bonesets[] = {
	{EBone_head, EBone_top_spine, EBone_upper_spine, EBone_middle_spine, EBone_bottom_spine, EBone_pelvis}, // Center
	{EBone_lower_arm_l, EBone_middle_arm_l, EBone_upper_arm_l, EBone_top_spine, EBone_upper_arm_r, EBone_middle_arm_r, EBone_lower_arm_r}, // Upper limbs
	{EBone_lower_leg_l, EBone_middle_leg_l, EBone_upper_leg_l, EBone_pelvis,    EBone_upper_leg_r, EBone_middle_leg_r, EBone_lower_leg_r}  // Lower limbs
};
