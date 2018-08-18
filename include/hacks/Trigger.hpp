/*
 * HTrigger.h
 *
 *  Created on: Oct 5, 2016
 *      Author: nullifiedcat
 */

#pragma once

#include "common.hpp"

namespace hacks::shared::triggerbot
{

void CreateMove();
CachedEntity *FindEntInSight(float range);
bool ShouldShoot();
bool IsTargetStateGood(CachedEntity *entity);
CachedEntity *FindEntInSight(float range);
bool HeadPreferable(CachedEntity *target);
bool UpdateAimkey();
float EffectiveTargetingRange();
void Draw();
bool CheckLineBox(Vector B1, Vector B2, Vector L1, Vector L2, Vector &Hit);
} // namespace hacks::shared::triggerbot
