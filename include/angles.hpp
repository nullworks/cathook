/*
 * angles.hpp
 *
 *  Created on: Jun 5, 2017
 *      Author: nullifiedcat
 */

#pragma once

#include "mathlib/vector.h"
#include "entitycache.hpp"
namespace angles
{

struct angle_data_s
{
    static constexpr size_t count = 16;
    void push(const Vector &angle);
    float deviation(int steps) const;

    Vector angles[count]{};
    bool good{ false };
    int angle_index{ 0 };
    int angle_count{ 0 };
};

extern angle_data_s data_[PLAYER_ARRAY_SIZE];

void Update();
} // namespace angles
