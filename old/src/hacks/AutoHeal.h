/*
 * AutoHeal.h
 *
 *  Created on: Dec 3, 2016
 *      Author: nullifiedcat
 */

#ifndef HACKS_AUTOHEAL_H_
#define HACKS_AUTOHEAL_H_

#include "../common.h"

namespace hacks { namespace tf { namespace autoheal {

extern CatVar enabled;
extern CatVar silent;
// TODO extern CatVar target_only;
void CreateMove();

struct patient_data_s {
	float last_damage { 0.0f };
	int last_health { 0 };
	int accum_damage { 0 }; // accumulated damage over X seconds (data stored for AT least 5 seconds)
	float accum_damage_start { 0.0f };
};

extern std::vector<patient_data_s> data;

void UpdateData();
int BestTarget();
int HealingPriority(int idx);
bool CanHeal(int idx);

}}}

#endif /* HACKS_AUTOHEAL_H_ */
