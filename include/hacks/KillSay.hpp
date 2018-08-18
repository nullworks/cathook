/*
 * KillSay.h
 *
 *  Created on: Jan 19, 2017
 *      Author: nullifiedcat
 */

#pragma once

#include "common.hpp"

namespace hacks::shared::killsay
{

void init();
void shutdown();
void reload();

extern const std::vector<std::string> builtin_default;
extern const std::vector<std::string> builtin_nonecore_offensive;
extern const std::vector<std::string> builtin_nonecore_mlg;
} // namespace hacks::shared::killsay
