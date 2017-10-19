 
/*
 *
 *	This framework utility controls the usage of bones
 *
 *
 */

#include "bones.hpp"

namespace framework { namespace bones {
	
// This is how the bones are layed out
// Use bonesets[0] for center line
const std::vector<std::vector<int>> bonesets = {{EBone_head, EBone_top_spine, EBone_upper_spine, EBone_middle_spine, EBone_bottom_spine, EBone_pelvis}, // Center
												{EBone_lower_arm_l, EBone_middle_arm_l, EBone_upper_arm_l, EBone_top_spine, EBone_upper_arm_r, EBone_middle_arm_r, EBone_lower_arm_r}, // Upper limbs
												{EBone_lower_leg_l, EBone_middle_leg_l, EBone_upper_leg_l, EBone_pelvis,    EBone_upper_leg_r, EBone_middle_leg_r, EBone_lower_leg_r}}; // Lower limbs
	
// Returns a catvector from an input bone if avaiable
bool GetCatBoneFromEnt(CatEntity* entity, int bone, CatVector& vector) {
	
	auto find = entity->bones.find(bone);
	if(find != entity->bones.end()) { // We have a bone, return the center of the hitbox
		vector = find->second.center();	
		return true;
	}
	vector = entity->origin;
	return false; // No name, return origin
}

}}