/*
 * aimbot.hpp
 *
 *  Created on: Jun 5, 2017
 *      Author: nullifiedcat
 */

#pragma once

class KeyValues;
class CachedEntity;

#include <stddef.h>

namespace ac { namespace aimbot {

struct ac_data {
	size_t  detections;
	int		check_timer;
	int		last_weapon;
};

void ResetEverything();
void ResetPlayer(int idx);

void Init();
void Update(CachedEntity* player);
void Event(KeyValues* event);

}}
