/*
 * HTrigger.h
 *
 *  Created on: Oct 5, 2016
 *      Author: nullifiedcat
 */

#pragma once

#include "common.hpp"
#include "Backtrack.hpp"

namespace hacks::shared::triggerbot
{
bool ShouldShoot();
bool IsTargetStateGood(CachedEntity *entity, std::optional<tf2::backtrack::BacktrackData> bt_data = std::nullopt);
CachedEntity *FindEntInSight(float range, bool no_players = false);
bool HeadPreferable(CachedEntity *target);
bool UpdateAimkey();
void Draw();
bool CheckLineBox(Vector B1, Vector B2, Vector L1, Vector L2, Vector &Hit);
} // namespace hacks::shared::triggerbot
