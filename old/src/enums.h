/*
 * enums.h
 *
 *  Created on: Oct 7, 2016
 *      Author: nullifiedcat
 */

#ifndef ENUMS_H_
#define ENUMS_H_

enum EntityType {
	ENTITY_GENERIC,
	ENTITY_PLAYER,
	ENTITY_BUILDING,
	ENTITY_PROJECTILE
};

enum powerup_type {
	not_powerup = -1,
	strength,
	resistance,
	vampire,
	reflect,
	haste,
	regeneration,
	precision,
	agility,
	knockout,
	king,
	plague,
	supernova,
	crits,
	POWERUP_COUNT
};

enum powerup_owner {
	neutral = 0,
	red = 1,
	blue = 2
};

enum tf_team {
	TEAM_UNK = 0,
	TEAM_SPEC,
	TEAM_RED,
	TEAM_BLU
};

enum tf_class {
	tf_scout = 1,
	tf_sniper,
	tf_soldier,
	tf_demoman,
	tf_medic,
	tf_heavy,
	tf_pyro,
	tf_spy,
	tf_engineer
};

enum MinigunState_t {
	AC_STATE_IDLE = 0,
    AC_STATE_STARTFIRING,
    AC_STATE_FIRING,
    AC_STATE_SPINNING
};

enum weaponmode {
	weapon_invalid = -1,
	weapon_hitscan = 0,
	weapon_projectile,
	weapon_melee,
	weapon_pda,
	weapon_medigun,
	weapon_consumable,
	weapon_throwable
};

enum hitbox_t {
	head = 0,
	pelvis = 1,
	spine_0 = 2,
	spine_1 = 3,
	spine_2 = 4,
	spine_3 = 5,
	upperArm_L = 6,
	lowerArm_L = 7,
	hand_L = 8,
	upperArm_R = 9,
	lowerArm_R = 10,
	hand_R = 11,
	hip_L = 12,
	knee_L = 13,
	foot_L = 14,
	hip_R = 15,
	knee_R = 16,
	foot_R = 17
};

#endif /* ENUMS_H_ */
