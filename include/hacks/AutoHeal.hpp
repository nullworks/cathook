/*
 * AutoHeal.h
 *
 *  Created on: Dec 3, 2016
 *      Author: nullifiedcat
 */

#pragma once

#include "common.hpp"

namespace hacks::tf::autoheal
{

void CreateMove();

struct patient_data_s
{
    float last_damage{ 0.0f };
    int last_health{ 0 };
    int accum_damage{
        0
    }; // accumulated damage over X seconds (data stored for AT least 5 seconds)
    float accum_damage_start{ 0.0f };
};

extern std::vector<patient_data_s> data;

void UpdateData();
int BestTarget();
int HealingPriority(int idx);
bool CanHeal(int idx);
} // namespace hacks::tf::autoheal
