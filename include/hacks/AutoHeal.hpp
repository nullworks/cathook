/*
 * AutoHeal.h
 *
 *  Created on: Dec 3, 2016
 *      Author: nullifiedcat
 */

#pragma once

#include "common.hpp"

namespace hacks
{
namespace tf
{
namespace autoheal
{
// TODO extern CatVar target_only;
void CreateMove();

struct patient_data_s
{
    float last_damage{ 0.0f };
    int last_health{ 0 };
    int accum_damage{
        0
    }; // accumulated damage over X seconds
};

extern std::vector<patient_data_s> data;

void UpdateData();
int BestTarget();
int HealingPriority(int idx);
bool CanHeal(int idx);
}
}
}
