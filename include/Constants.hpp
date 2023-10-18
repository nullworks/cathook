/*
 * Constants.hpp
 *
 *  Created on: October 16, 2019
 *      Author: LightCat
 */

// This file will include global constant expressions
#pragma once

constexpr const char *CON_PREFIX = "cat_";

constexpr int MAX_ENTITIES = 2048;
constexpr int MAX_PLAYERS  = 100;
// 0 is "World" but we still can have MAX_PLAYERS players, so consider that
constexpr int PLAYER_ARRAY_SIZE = 1 + MAX_PLAYERS;
