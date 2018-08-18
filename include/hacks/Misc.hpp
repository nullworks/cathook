/*
 * Misc.h
 *
 *  Created on: Nov 5, 2016
 *      Author: nullifiedcat
 */

#pragma once

#include "common.hpp"
#include "config.h"

namespace hacks::shared::misc
{

void CreateMove();
#if ENABLE_VISUALS
void DrawText();
#endif

extern int last_number;

extern float last_bucket;
} // namespace hacks::shared::misc
