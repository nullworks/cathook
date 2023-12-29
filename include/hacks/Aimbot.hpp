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
struct AimbotTarget_t
{
    unsigned long predict_tick{ 0 };
    bool predict_type{ 0 };
    Vector aim_position{ 0 };
    unsigned long vcheck_tick{ 0 };
    CachedEntity* ent { nullptr };
    bool visible{ false };
    float fov{ 0 };
    int hitbox{ 0 };
    bool valid { false };
};
extern settings::Boolean ignore_cloak;
extern unsigned last_target_ignore_timer;
// Used to store aimbot data to prevent calculating it again
// Functions used to calculate aimbot data, and if already calculated use it
Vector PredictEntity(AimbotTarget_t& entity);

// Functions called by other functions for when certian game calls are run
void Reset();

// Stuff to make storing functions easy
bool isAiming();
CachedEntity *CurrentTarget();
bool ShouldAim();
AimbotTarget_t RetrieveBestTarget(bool aimkey_state);
AimbotTarget_t GetTarget(CachedEntity *entity);
bool Aim(AimbotTarget_t entity);
void DoAutoshoot(AimbotTarget_t target = {});
int notVisibleHitbox(CachedEntity *target, int preferred);
std::vector<Vector> getHitpointsVischeck(CachedEntity *ent, int hitbox);
float projectileHitboxSize(int projectile_size);
int autoHitbox(CachedEntity *target);
bool hitscanSpecialCases(AimbotTarget_t target_entity, int weapon_case);
bool projectileSpecialCases(AimbotTarget_t target_entity, int weapon_case);
int BestHitbox(CachedEntity *target);
bool isHitboxMedium(int hitbox);
int ClosestHitbox(CachedEntity *target);
void DoSlowAim(Vector &inputAngle);
bool UpdateAimkey();
} // namespace hacks::shared::aimbot
