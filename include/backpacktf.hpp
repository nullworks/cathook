/*
 * backpacktf.hpp
 *
 *  Created on: Jul 23, 2017
 *      Author: nullifiedcat
 */

#pragma once

class CatVar;

namespace backpacktf
{

constexpr float REFINED_METAL_PRICE = 0.075f; // $
constexpr unsigned REQUEST_INTERVAL =
    10; // Make a backpack.tf request every 30 seconds
constexpr unsigned MAX_CACHE_AGE = 60 * 30;
constexpr unsigned OUTDATED_AGE =
    60 * 60 * 24 * 3; // After how many seconds backpack is marked "outdated"

const backpack_data_s &get_data(unsigned id);
void init();
bool enabled();
}
