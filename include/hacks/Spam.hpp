/*
 * Spam.h
 *
 *  Created on: Jan 21, 2017
 *      Author: nullifiedcat
 */

#pragma once
#include <string>
#include <vector>

class CatCommand;

namespace hacks::shared::spam
{

extern const std::vector<std::string> builtin_default;
extern const std::vector<std::string> builtin_lennyfaces;
extern const std::vector<std::string> builtin_blanks;
extern const std::vector<std::string> builtin_nonecore;
extern const std::vector<std::string> builtin_lmaobox;
extern const std::vector<std::string> builtin_lithium;

bool isActive();
void init();
} // namespace hacks::shared::spam
