/*
 * HTrigger.h
 *
 *  Created on: Oct 5, 2016
 *      Author: nullifiedcat
 */

#pragma once

#include "common.hpp"

namespace hacks::tf2::backtrack
{
class BacktrackData;
}
namespace hacks::shared::triggerbot
{

void CreateMove();
bool ShouldShoot();
bool IsTargetStateGood(CachedEntity *entity, hacks::tf2::backtrack::BacktrackData *bt_data = nullptr);
CachedEntity *FindEntInSight(float range, bool no_players = false);
bool HeadPreferable(CachedEntity *target);
bool UpdateAimkey();
float EffectiveTargetingRange();
void Draw();
bool CheckLineBox(Vector B1, Vector B2, Vector L1, Vector L2, Vector &Hit);
} // namespace hacks::shared::triggerbot
