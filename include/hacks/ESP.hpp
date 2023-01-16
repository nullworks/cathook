/*
 * HEsp.h
 *
 *  Created on: Oct 6, 2016
 *      Author: nullifiedcat
 */

#pragma once

#include "config.h"
#if ENABLE_VISUALS
#include "colors.hpp"

namespace hacks::shared::esp
{

// Init
void Shutdown();
// Strings
void SetEntityColor(CachedEntity *entity, const rgba_t &color);
} // namespace hacks::shared::esp
#endif
