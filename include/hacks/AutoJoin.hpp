/*
 * AutoJoin.hpp
 *
 *  Created on: Jul 28, 2017
 *      Author: nullifiedcat
 */

#pragma once
#include "common.hpp"
namespace hacks::shared::autojoin
{

void resetQueueTimer();
void update();
void updateSearch();
void onShutdown();
} // namespace hacks::shared::autojoin
