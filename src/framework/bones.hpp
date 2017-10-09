 
/*
 *
 *	Bone frameworks header file
 *
 *
 */

#pragma once

#include <vector>	// This makes arrays easier

#include "../util/mathlib.hpp"	// We need catboxes
#include "entitys.hpp"			// We use CatEntitys

namespace framework { namespace bones {

// Sets of bones that are connected
extern const int boneset_center[];
extern const int boneset_upper_limbs[];
extern const int boneset_lower_limbs[];

// Contains the bone sets
extern const std::vector<const int*> bonesets;

bool GetCatBoneFromEnt(CatEntity* entity, int bone, CatVector& vector);

}}

// Bone position enum
// Add bones as needed!
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
	EBone_lower_leg_r
};