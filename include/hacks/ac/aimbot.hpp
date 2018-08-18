/*
 * aimbot.hpp
 *
 *  Created on: Jun 5, 2017
 *      Author: nullifiedcat
 */

#pragma once

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
extern int amount[32];

void ResetEverything();
void ResetPlayer(int idx);

void Init();
void Update(CachedEntity *player);
void Event(KeyValues *event);
} // namespace ac::aimbot
