/*
 * backpacktf.hpp
 *
 *  Created on: Jul 23, 2017
 *      Author: nullifiedcat
 */

#pragma once

class CatVar;

namespace backpacktf {

constexpr float REFINED_METAL_PRICE = 0.075f; // $
constexpr unsigned REQUEST_INTERVAL = 10; // Make a backpack.tf request every 30 seconds
constexpr unsigned MAX_CACHE_AGE = 60 * 30;
constexpr unsigned OUTDATED_AGE = 60 * 60 * 24 * 3; // After how many seconds backpack is marked "outdated" (possibly private)

extern CatVar enable_bptf;

struct backpack_data_s {
	bool pending { false };
	bool bad { true };
	bool no_value { false }; // No recorded value
	bool outdated_value { false }; // Outdated value. Private inventory?
	unsigned last_request { 0 };
	float value { 0 };
	unsigned id { 0 };
};

const backpack_data_s& get_data(unsigned id);
void init();
bool enabled();

}
