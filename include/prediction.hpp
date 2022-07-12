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
struct StrafePredictionData;

Vector SimpleLatencyPrediction(CachedEntity *ent, int hb);

// The first entry ignores initial velocity, the second one does not.
// Used for vischeck things.
std::pair<Vector, Vector> ProjectilePrediction(CachedEntity *ent, int hb, float speed, float gravitymod, float entgmod, float proj_startvelocity = 0.0f);
std::pair<Vector, Vector> ProjectilePrediction_Engine(CachedEntity *ent, int hb, float speed, float gravitymod, float entgmod, float proj_startvelocity = 0.0f);
std::pair<Vector, Vector> BuildingPrediction(CachedEntity *building, Vector vec, float speed, float gravity, float proj_startvelocity);
std::vector<Vector> Predict(CachedEntity *player, Vector pos, float offset, Vector vel, Vector acceleration, std::pair<Vector, Vector> minmax, int count, bool vischeck = true);
Vector PredictStep(Vector pos, Vector &vel, const Vector &acceleration, std::pair<Vector, Vector> *minmax, float steplength = g_GlobalVars->interval_per_tick, StrafePredictionData *strafepred = nullptr, bool vischeck = true, std::optional<float> grounddistance = std::nullopt);
float PlayerGravityMod(CachedEntity *player);

Vector EnginePrediction(CachedEntity *player, float time, Vector *VecVelocity = nullptr);
#if ENABLE_VISUALS
void Prediction_PaintTraverse();
#endif

float DistanceToGround(CachedEntity *ent);
float DistanceToGround(Vector origin);
float DistanceToGround(Vector origin, Vector mins, Vector maxs);
