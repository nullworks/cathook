/*
 * localplayer.h
 *
 *  Created on: Oct 15, 2016
 *      Author: nullifiedcat
 */

#pragma once

#include <mathlib/vector.h>
#include <enums.hpp>

class CachedEntity;

class LocalPlayer
{
public:
    // Start of CM
    void Update();
    // End of CM
    void UpdateEnd();
    int team;
    int health;
    int flags;
    char life_state;
    int clazz;
    bool bZoomed;
    float flZoomBegin;
    bool holding_sniper_rifle;
    bool holding_sapper;
    weaponmode weapon_mode;
    bool using_action_slot_item{ false };

    Vector v_ViewOffset;
    Vector v_Origin;
    Vector v_Eye;
    int entity_idx;
    CachedEntity *entity{ 0 };
    CachedEntity *weapon();
    Vector v_OrigViewangles;
    Vector v_SilentAngles;
    bool bUseSilentAngles;
    bool bAttackLastTick;

    bool isFakeAngleCM = false;
    Vector realAngles{ 0.0f, 0.0f, 0.0f };
};

#define LOCAL_E g_pLocalPlayer->entity
#define LOCAL_W g_pLocalPlayer->weapon()

extern LocalPlayer *g_pLocalPlayer;