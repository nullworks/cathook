/*
 *prediction.h
 *
 * Created on: Dec 5, 2016
 *     Author: nullifiedcat
 */

#pragma once

#include <enums.hpp>
#include "config.h"
#include "vector"
#include <optional>
#include "interfaces.hpp"
#include "sdk.hpp"

#pragma once

class CachedEntity;
class Vector;

Vector SimpleLatencyPrediction(CachedEntity *ent, int hb);

bool PerformProjectilePrediction(CachedEntity *target, int hitbox);

Vector BuildingPrediction(CachedEntity *building, Vector vec, float speed, float gravity, float proj_startvelocity = 0.0f);
Vector ProjectilePrediction(CachedEntity *ent, int hb, float speed, float gravitymod, float entgmod, float proj_startvelocity = 0.0f);
Vector ProjectilePrediction_Engine(CachedEntity *ent, int hb, float speed, float gravitymod, float entgmod /* ignored */, float proj_startvelocity = 0.0f);

std::vector<Vector> Predict(Vector pos, float offset, Vector vel, Vector acceleration, std::pair<Vector, Vector> minmax, float time, int count, bool vischeck = true);
Vector PredictStep(Vector pos, Vector &vel, Vector acceleration, std::pair<Vector, Vector> &minmax, float time, float steplength = g_GlobalVars->interval_per_tick, bool vischeck = true, std::optional<float> grounddistance = std::nullopt);
float PlayerGravityMod(CachedEntity *player);

Vector EnginePrediction(CachedEntity *player, float time);
#if ENABLE_VISUALS
void Prediction_PaintTraverse();
#endif

float DistanceToGround(CachedEntity *ent);
float DistanceToGround(Vector origin);
float DistanceToGround(Vector origin, Vector mins, Vector maxs);
