/*
 * Announcer.hpp
 *
 *  Created on: Nov 13, 2017
 *      Author: nullifiedcat
 */

#pragma once

#include "common.hpp"

namespace hacks::shared::announcer
{

void playsound(const std::string &);
void init();
void shutdown();
} // namespace hacks::shared::announcer
