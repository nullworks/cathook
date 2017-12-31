/*
 * AntiCheat.hpp
 *
 *  Created on: Jun 5, 2017
 *      Author: nullifiedcat
 */

#pragma once

#include "common.hpp"

#include "ac/aimbot.hpp"
#include "ac/antiaim.hpp"
#include "ac/bhop.hpp"

namespace hacks
{
namespace shared
{
namespace anticheat
{

void Accuse(int eid, const std::string &hack, const std::string &details);

void Init();
void CreateMove();

void ResetPlayer(int index);
void ResetEverything();
}
}
}
