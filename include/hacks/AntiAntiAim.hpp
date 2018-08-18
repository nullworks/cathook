/*
  Created on 29.07.18.
*/

#pragma once
#include <core/interfaces.hpp>
#include <core/sdk.hpp>
#include <globals.h>
#include <core/netvars.hpp>
#include <settings/Bool.hpp>
#include <localplayer.hpp>
#include <entitycache.hpp>

namespace hacks::shared::anti_anti_aim
{

void createMove();
void resolveEnt(int IDX, IClientEntity *entity = nullptr);
} // namespace hacks::shared::anti_anti_aim
