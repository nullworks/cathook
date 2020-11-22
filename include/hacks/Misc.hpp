/*
 * Misc.h
 *
 *  Created on: Nov 5, 2016
 *      Author: nullifiedcat
 */

#pragma once

#include "config.h"

namespace hacks::shared::misc
{
void generate_schema();
void Schema_Reload();
void CreateMove();
#if ENABLE_VISUALS
void DrawText();
#endif
int getCarriedBuilding();
extern int last_number;

extern float last_bucket;
} // namespace hacks::shared::misc
