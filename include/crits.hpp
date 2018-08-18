/*
 * crits.h
 *
 *  Created on: Feb 25, 2017
 *      Author: nullifiedcat
 */

#pragma once

class CUserCmd;
class IClientEntity;

// BUGBUG TODO this struct is outdated
struct crithack_saved_state
{
    float unknown2868;
    float unknown2864;
    int unknown2620;
    float unknown2880;
    char unknown2839;
    float bucket2616;
    int seed2876;

    void Save(IClientEntity *entity);
    void Load(IClientEntity *entity);
};

namespace criticals
{

void create_move();
void draw();
bool random_crits_enabled();
} // namespace criticals

#include <unordered_map>

extern int *g_PredictionRandomSeed;
extern std::unordered_map<int, int> command_number_mod;
