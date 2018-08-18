/*
 *prediction.h
 *
 * Created on: Dec 5, 2016
 *     Author: nullifiedcat
 */

#pragma once

#include <enums.hpp>
#include "config.h"

class CachedEntity;
class Vector;

Vector SimpleLatencyPrediction(CachedEntity *ent, int hb);

bool PerformProjectilePrediction(CachedEntity *target, int hitbox);

Vector ProjectilePrediction(CachedEntity *ent, int hb, float speed,
                            float gravitymod, float entgmod);
Vector ProjectilePrediction_Engine(CachedEntity *ent, int hb, float speed,
                                   float gravitymod,
                                   float entgmod /* ignored */);

float PlayerGravityMod(CachedEntity *player);

Vector EnginePrediction(CachedEntity *player, float time);
void Prediction_CreateMove();
#if ENABLE_VISUALS
void Prediction_PaintTraverse();
#endif

float DistanceToGround(CachedEntity *ent);
float DistanceToGround(Vector origin);
