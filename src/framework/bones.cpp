 
/*
 *
 *	This framework utility controls the usage of bones
 *
 *
 */

#include "bones.hpp"

namespace framework { namespace bones {
	
// This is how the bones are layed out
const int boneset_center[] = {EBone_head, EBone_top_spine, EBone_upper_spine, EBone_middle_spine, EBone_bottom_spine, EBone_pelvis}; // Size: 6
const int boneset_upper_limbs[] = {EBone_upper_arm_l, EBone_middle_arm_l, EBone_lower_arm_l, EBone_top_spine, EBone_lower_arm_r, EBone_middle_arm_r, EBone_upper_arm_r}; // Size: 7
const int boneset_lower_limbs[] = {EBone_upper_leg_l, EBone_middle_leg_l, EBone_lower_leg_l, EBone_pelvis,    EBone_lower_leg_r, EBone_middle_leg_r, EBone_upper_leg_r}; // Size: 7

// Contains all the sets
const std::vector<const int*> bonesets = {boneset_center, boneset_upper_limbs, boneset_lower_limbs};

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