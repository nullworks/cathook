/*
 * HAimbot.h
 *
 *  Created on: Oct 8, 2016
 *      Author: nullifiedcat
 */

#pragma once

#include "common.hpp"

class ConVar;
class IClientEntity;

namespace hacks::shared::aimbot
{
extern settings::Boolean ignore_cloak;
extern unsigned last_target_ignore_timer;
// Used to store aimbot data to prevent calculating it again
struct AimbotCalculatedData_s
{
    unsigned long predict_tick{ 0 };
    bool predict_type{ 0 };
    Vector aim_position{ 0 };
    unsigned long vcheck_tick{ 0 };
    bool visible{ false };
    float fov{ 0 };
    int hitbox{ 0 };
};

// Functions used to calculate aimbot data, and if already calculated use it
Vector PredictEntity(CachedEntity *entity, bool vischeck);
bool VischeckPredictedEntity(CachedEntity *entity);
bool BacktrackVisCheck(CachedEntity *entity);

// Functions called by other functions for when certian game calls are run
void Reset();

// Stuff to make storing functions easy
bool isAiming();
CachedEntity *CurrentTarget();
bool ShouldAim();
CachedEntity *RetrieveBestTarget(bool aimkey_state);
bool IsTargetStateGood(CachedEntity *entity);
void Aim(CachedEntity *entity);
void DoAutoshoot(CachedEntity *target = nullptr);
int BestHitbox(CachedEntity *target);
int ClosestHitbox(CachedEntity *target);
void DoSlowAim(Vector &inputAngle);
bool UpdateAimkey();
float EffectiveTargetingRange();
} // namespace hacks::shared::aimbot
