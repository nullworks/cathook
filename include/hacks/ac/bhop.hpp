/*
 * bhop.hpp
 *
 *  Created on: Jun 5, 2017
 *      Author: nullifiedcat
 */

#pragma once

class KeyValues;
class CachedEntity;

namespace ac { namespace bhop {

struct ac_data {
	int detections { 0 };
	bool was_on_ground { false };
	int ticks_on_ground { 0 };
	unsigned long last_accusation { 0 };
};

void ResetEverything();
void ResetPlayer(int idx);

void Init();
void Update(CachedEntity* player);
void Event(KeyValues* event);

}}



