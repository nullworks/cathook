/*
 * aimbot.hpp
 *
 *  Created on: Jun 5, 2017
 *      Author: nullifiedcat
 */

#pragma once

#include "config.h"
#include "common.hpp"
#include <cstddef>

class KeyValues;
class CachedEntity;

namespace ac::aimbot
{

struct ac_data
{
    size_t detections;
    int check_timer;
    int last_weapon;
};
extern int amount[MAX_PLAYERS];

void ResetEverything();
boost::unordered_flat_map<int, Vector> &player_orgs();
void ResetPlayer(int idx);

void Init();
void Update(CachedEntity *player);
void Event(KeyValues *event);
} // namespace ac::aimbot
