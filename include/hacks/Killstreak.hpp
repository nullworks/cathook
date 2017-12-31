/*
 * Killstreak.hpp
 *
 *  Created on: Nov 13, 2017
 *      Author: nullifiedcat
 */

#pragma once

#include "common.hpp"

namespace hacks
{
namespace tf2
{
namespace killstreak
{

int current_streak();
void init();
void shutdown();
void fire_event(IGameEvent *event);
void apply_killstreaks();
}
}
}
