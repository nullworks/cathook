/*
 * prediction.h
 *
 *  Created on: Dec 5, 2016
 *      Author: nullifiedcat
 */

#ifndef PREDICTION_H_
#define PREDICTION_H_

#include "enums.h"

class CachedEntity;
class Vector;

Vector SimpleLatencyPrediction(CachedEntity* ent, int hb);

bool PerformProjectilePrediction(CachedEntity* target, int hitbox);

Vector ProjectilePrediction(CachedEntity* ent, int hb, float speed, float gravitymod, float entgmod);
Vector ProjectilePrediction_Engine(CachedEntity* ent, int hb, float speed, float gravitymod, float entgmod /* ignored */);

float PlayerGravityMod(CachedEntity* player);

Vector EnginePrediction(CachedEntity* player, float time);
void Prediction_CreateMove();
#if ENABLE_VISUALS == 1
void Prediction_PaintTraverse();
#endif

float DistanceToGround(CachedEntity* ent);
float DistanceToGround(Vector origin);

#endif /* PREDICTION_H_ */
