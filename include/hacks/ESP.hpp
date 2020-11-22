/*
 * HEsp.h
 *
 *  Created on: Oct 6, 2016
 *      Author: nullifiedcat
 */

#pragma once

#include "config.h"
#include "colors.hpp"
#if ENABLE_VISUALS
namespace hacks::shared::esp
{

// Init
void Init();

// Strings
void SetEntityColor(CachedEntity *entity, const rgba_t &color);
} // namespace hacks::shared::esp
#endif
