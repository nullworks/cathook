/*
  Created on 29.07.18.
*/

#include <optional>
#include "mathlib/vector.h"
#include "cdll_int.h"

#pragma once
class IClientEntity;

struct brutedata
{
    int brutenum{ 0 };
    int hits_in_a_row{ 0 };
    Vector original_angle{};
    Vector new_angle{};
};

namespace hacks::shared::anti_anti_aim
{
extern std::unordered_map<unsigned, brutedata> resolver_map;
void increaseBruteNum(int idx);
void frameStageNotify(ClientFrameStage_t stage);
// void resolveEnt(int IDX, IClientEntity *entity = nullptr);
} // namespace hacks::shared::anti_anti_aim
