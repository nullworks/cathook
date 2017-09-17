/*
 * localplayer.h
 *
 *  Created on: Oct 15, 2016
 *      Author: nullifiedcat
 */

#ifndef LOCALPLAYER_H_
#define LOCALPLAYER_H_

#include "fixsdk.h"
#include <mathlib/vector.h>

class CachedEntity;

class LocalPlayer {
public:
	void Update();
	int team;
	int health;
	int flags;
	char life_state;
	int clazz;
	bool bZoomed;
	float flZoomBegin;
	bool holding_sniper_rifle;
	weaponmode weapon_mode;
	bool using_action_slot_item { false };

	Vector v_ViewOffset;
	Vector v_Origin;
	Vector v_Eye;
	int entity_idx;
	CachedEntity* entity { 0 };
	CachedEntity* weapon();
	Vector v_OrigViewangles;
	Vector v_SilentAngles;
	bool bUseSilentAngles;
	bool bAttackLastTick;
};

#define LOCAL_E g_pLocalPlayer->entity
#define LOCAL_W g_pLocalPlayer->weapon()

extern LocalPlayer* g_pLocalPlayer;

#endif /* LOCALPLAYER_H_ */
