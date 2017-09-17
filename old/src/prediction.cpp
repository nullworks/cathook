/*
 * prediction.cpp
 *
 *  Created on: Dec 5, 2016
 *      Author: nullifiedcat
 */

#include "prediction.h"

#include <bspflags.h>
#include <cdll_int.h>
#include <cmodel.h>
#include <engine/IEngineTrace.h>
#include <icliententity.h>
#include <inetchannelinfo.h>
#include <mathlib/ssemath.h>
#include <mathlib/vector.h>

#include "conditions.h"
#include "entitycache.h"
#include "helpers.h"
#include "interfaces.h"
#include "localplayer.h"
#include "netvars.h"
#include "trace.h"

// TODO there is a Vector() object created each call.

Vector SimpleLatencyPrediction(CachedEntity* ent, int hb) {
	if (!ent) return Vector();
	Vector result;
	GetHitbox(ent, hb, result);
	float latency = g_IEngine->GetNetChannelInfo()->GetLatency(FLOW_OUTGOING) +
			g_IEngine->GetNetChannelInfo()->GetLatency(FLOW_INCOMING);
	result += CE_VECTOR(ent, netvar.vVelocity) * latency;
	return result;
}

float PlayerGravityMod(CachedEntity* player) {
//	int movetype = CE_INT(player, netvar.movetype);
//	if (movetype == MOVETYPE_FLY || movetype == MOVETYPE_NOCLIP) return 0.0f;
	if (HasCondition<TFCond_Parachute>(player)) return 0.448f;
	return 1.0f;
}

bool PerformProjectilePrediction(CachedEntity* target, int hitbox) {
	Vector src, vel, hit; ;
	//src = vfunc<Vector(*)(IClientEntity*)>(RAW_ENT(target), 299)(RAW_ENT(target));

	return true;
}


std::vector<std::vector<Vector>> predicted_players {};

int predicted_player_count = 0;

static CatVar debug_enginepred(CV_SWITCH, "debug_engine_pred_others", "0", "DO NOT USE - MOVEMENT");

void Prediction_CreateMove() {
	static bool setup = false;
	if (!setup) {
		setup = true;
		predicted_players.resize(32);
	}
	if (!debug_enginepred) return;
	for (int i = 1; i < g_GlobalVars->maxClients; i++) {
		CachedEntity* ent = ENTITY(i);
		if (CE_GOOD(ent)) {
			Vector o = ent->m_vecOrigin;
			predicted_players[i].clear();
			for (int j = 0; j < 20; j++) {
				Vector r = EnginePrediction(ent, 0.05f);
				ent->m_vecOrigin = r;
				predicted_players[i].push_back(std::move(r));
			}
			ent->m_vecOrigin = o;
			CE_VECTOR(ent, 0x354) = o;
			//logging::Info("Predicted %d to be at [%.2f, %.2f, %.2f] vs [%.2f, %.2f, %.2f]", i, r.x,r.y,r.z, o.x, o.y, o.z);
			predicted_player_count = i;
		}
	}
}
#if ENABLE_VISUALS == 1
void Prediction_PaintTraverse() {
	if (!debug_enginepred) return;
	for (int i = 1; i < predicted_player_count; i++) {
		CachedEntity* ent = ENTITY(i);
		if (CE_GOOD(ent)) {
			Vector previous_screen;
			if (!draw::WorldToScreen(ent->m_vecOrigin, previous_screen)) continue;
			rgba_t color = colors::FromRGBA8(255, 0, 0, 255);
			for (int j = 0; j < predicted_players[i].size(); j++) {
				Vector screen;
				if (draw::WorldToScreen(predicted_players[i][j], screen)) {
					drawgl::Line(screen.x, screen.y, previous_screen.x - screen.x, previous_screen.y - screen.y, color);
					previous_screen = screen;
				} else {
					break;
				}
				color.r -= 1.0f / 20.0f;
			}

		}
	}
}
#endif
Vector EnginePrediction(CachedEntity* entity, float time) {
	Vector result = entity->m_vecOrigin;
	IClientEntity* ent = RAW_ENT(entity);

	typedef void(*SetupMoveFn)(IPrediction*, IClientEntity *, CUserCmd *, class IMoveHelper *, CMoveData *);
	typedef void(*FinishMoveFn)(IPrediction*, IClientEntity *, CUserCmd*, CMoveData*);

	void **predictionVtable = *((void ***)g_IPrediction);
	SetupMoveFn oSetupMove = (SetupMoveFn)(*(unsigned*)(predictionVtable + 19));
	FinishMoveFn oFinishMove = (FinishMoveFn)(*(unsigned*)(predictionVtable + 20));

	//CMoveData *pMoveData = (CMoveData*)(sharedobj::client->lmap->l_addr + 0x1F69C0C);
	//CMoveData movedata {};
	char* object = new char[165];
	CMoveData *pMoveData = (CMoveData*)object;

	float frameTime = g_GlobalVars->frametime;
	float curTime = g_GlobalVars->curtime;

	CUserCmd fakecmd {};

	memset(&fakecmd, 0, sizeof(CUserCmd));

	Vector vel;
	if (velocity::EstimateAbsVelocity)
		velocity::EstimateAbsVelocity(RAW_ENT(entity), vel);
	else
		vel = CE_VECTOR(entity, netvar.vVelocity);
	fakecmd.command_number = last_cmd_number;
	fakecmd.forwardmove = vel.x;
	fakecmd.sidemove = -vel.y;
	Vector oldangles = CE_VECTOR(entity, netvar.m_angEyeAngles);
	static Vector zerov {0,0,0};
	CE_VECTOR(entity, netvar.m_angEyeAngles) = zerov;

	CUserCmd* original_cmd = NET_VAR(ent, 4188, CUserCmd*);

	NET_VAR(ent, 4188, CUserCmd*) = &fakecmd;

	g_GlobalVars->curtime =  g_GlobalVars->interval_per_tick * NET_INT(ent, netvar.nTickBase);
	g_GlobalVars->frametime = time;

	Vector old_origin = entity->m_vecOrigin;
	NET_VECTOR(ent, 0x354) = entity->m_vecOrigin;

	//*g_PredictionRandomSeed = MD5_PseudoRandom(g_pUserCmd->command_number) & 0x7FFFFFFF;
	g_IGameMovement->StartTrackPredictionErrors(reinterpret_cast<CBasePlayer*>(ent));
	oSetupMove(g_IPrediction, ent, &fakecmd, NULL, pMoveData);
	g_IGameMovement->ProcessMovement(reinterpret_cast<CBasePlayer*>(ent), pMoveData);
	oFinishMove(g_IPrediction, ent, &fakecmd, pMoveData);
	g_IGameMovement->FinishTrackPredictionErrors(reinterpret_cast<CBasePlayer*>(ent));

	delete[] object;

	NET_VAR(entity, 4188, CUserCmd*) = original_cmd;

	g_GlobalVars->frametime = frameTime;
	g_GlobalVars->curtime = curTime;

	result = ent->GetAbsOrigin();
	NET_VECTOR(ent, 0x354) = old_origin;
	CE_VECTOR(entity, netvar.m_angEyeAngles) = oldangles;
	return result;
}

Vector ProjectilePrediction_Engine(CachedEntity* ent, int hb, float speed, float gravitymod, float entgmod /* ignored */) {
	Vector origin = ent->m_vecOrigin;
	Vector hitbox;
	GetHitbox(ent, hb, hitbox);
	Vector hitbox_offset = hitbox - origin;

	if (speed == 0.0f) return Vector();
	Vector velocity;
	if (velocity::EstimateAbsVelocity)
		velocity::EstimateAbsVelocity(RAW_ENT(ent), velocity);
	else
		velocity = CE_VECTOR(ent, netvar.vVelocity);
	// TODO ProjAim
	float medianTime = g_pLocalPlayer->v_Eye.DistTo(hitbox) / speed;
	float range = 1.5f;
	float currenttime = medianTime - range;
	if (currenttime <= 0.0f) currenttime = 0.01f;
	float besttime = currenttime;
	float mindelta = 65536.0f;
	Vector bestpos = origin;
	Vector current = origin;
	int maxsteps = 40;
	bool onground = false;
	if (ent->m_Type == ENTITY_PLAYER) {
		if (CE_INT(ent, netvar.iFlags) & FL_ONGROUND) onground = true;
	}
	float steplength = ((float)(2 * range) / (float)maxsteps);
	for (int steps = 0; steps < maxsteps; steps++, currenttime += steplength) {
		ent->m_vecOrigin = current;
		current = EnginePrediction(ent, steplength);

		if (onground) {
			float toground = DistanceToGround(current);
			current.z -= toground;
		}

		float rockettime = g_pLocalPlayer->v_Eye.DistTo(current) / speed;
		if (fabs(rockettime - currenttime) < mindelta) {
			besttime = currenttime;
			bestpos = current;
			mindelta = fabs(rockettime - currenttime);
		}
	}
	ent->m_vecOrigin = origin;
	CE_VECTOR(ent, 0x354) = origin;
	bestpos.z += (400 * besttime * besttime * gravitymod);
	// S = at^2/2 ; t = sqrt(2S/a)*/
	Vector result = bestpos + hitbox_offset;
	logging::Info("[Pred][%d] delta: %.2f   %.2f   %.2f", result.x - origin.x, result.y - origin.y, result.z - origin.z	);
	return result;
}

CatVar debug_pp_extrapolate(CV_SWITCH, "debug_pp_extrapolate", "0", "Extrapolate entity position when predicting projectiles");
CatVar debug_pp_rockettimeping(CV_SWITCH, "debug_pp_rocket_time_ping", "0", "Compensate for ping in pp");

Vector ProjectilePrediction(CachedEntity* ent, int hb, float speed, float gravitymod, float entgmod) {
	if (!ent) return Vector();
	Vector result;
	if (not debug_pp_extrapolate) {
		GetHitbox(ent, hb, result);
	} else {
		result = SimpleLatencyPrediction(ent, hb);
	}
	if (speed == 0.0f) return Vector();
	float dtg = DistanceToGround(ent);
	Vector vel;
	if (velocity::EstimateAbsVelocity)
		velocity::EstimateAbsVelocity(RAW_ENT(ent), vel);
	else
		vel = CE_VECTOR(ent, netvar.vVelocity);
	// TODO ProjAim
	float medianTime = g_pLocalPlayer->v_Eye.DistTo(result) / speed;
	float range = 1.5f;
	float currenttime = medianTime - range;
	if (currenttime <= 0.0f) currenttime = 0.01f;
	float besttime = currenttime;
	float mindelta = 65536.0f;
	Vector bestpos = result;
	int maxsteps = 300;
	for (int steps = 0; steps < maxsteps; steps++, currenttime += ((float)(2 * range) / (float)maxsteps)) {
		Vector curpos = result;
		curpos += vel * currenttime;
		if (dtg > 0.0f) {
			curpos.z -= currenttime * currenttime * 400 * entgmod;
			if (curpos.z < result.z - dtg) curpos.z = result.z - dtg;
		}
		float rockettime = g_pLocalPlayer->v_Eye.DistTo(curpos) / speed;
		if (debug_pp_rockettimeping) rockettime += g_IEngine->GetNetChannelInfo()->GetLatency(FLOW_OUTGOING);
		if (fabs(rockettime - currenttime) < mindelta) {
			besttime = currenttime;
			bestpos = curpos;
			mindelta = fabs(rockettime - currenttime);
		}
	}
	bestpos.z += (400 * besttime * besttime * gravitymod);
	// S = at^2/2 ; t = sqrt(2S/a)*/
	return bestpos;
}

float DistanceToGround(CachedEntity* ent) {
	if (ent->m_Type == ENTITY_PLAYER) {
		if (CE_INT(ent, netvar.iFlags) & FL_ONGROUND) return 0;
	}
	Vector& origin = ent->m_vecOrigin;
	float v1 = DistanceToGround(origin + Vector(10.0f, 10.0f, 0.0f));
	float v2 = DistanceToGround(origin + Vector(-10.0f, 10.0f, 0.0f));
	float v3 = DistanceToGround(origin + Vector(10.0f, -10.0f, 0.0f));
	float v4 = DistanceToGround(origin + Vector(-10.0f, -10.0f, 0.0f));
	return MIN(v1, MIN(v2, MIN(v3, v4)));
}

float DistanceToGround(Vector origin) {
	trace_t ground_trace;
	Ray_t ray;
	Vector endpos = origin;
	endpos.z -= 8192;
	ray.Init(origin, endpos);
	g_ITrace->TraceRay(ray, MASK_PLAYERSOLID, &trace::filter_no_player, &ground_trace);
	return 8192.0f * ground_trace.fraction;
}

/*
// Set of to be fuctions for preciting players, similear to ncc prediction.

// The way air prediction works is that we use getabsvel to get a baseline position of where the player could be
// next tick. Then we save that into the array for our math next tick.
// After the first tick passed, we check to see how the GetAbsVel function actually performed and we can correct for its 
// mistakes by comparing the result from GetAbsVel last tick and where the player currently is this tick and applying an 
// offset for predictions. 
// With the new offset, you can use GetAbsVel and apply the offset to get 1 tick but for every other time you would need 
// to apply the offset due to the way airstrafing works.
// GetAbsVel only works in a strait fassion of what the players current velocity and doesnt factor in what the players 
// next velocity could be due to the player having the ability to airstrafe and change that by quite a bit. 

// Ground prediction works in the way of it using GetAbsVel to get a baseline direction of where the player is going and
// attempting to predict the players movement from that. The prediction should check its surroundings for corners, walls, 
// and the like to determine a path of where the player could potentially go. We would also want to check if players 
// collision box would intercept a wall or floor and interpolate even more with that in mind. 
// If a player is moving too steeply onto a wall, the prediction should stop there and count that as a place for where the 
// player would be for any time after it.

// we can change between the two prediction types based on the ground flag netvar. 

// For using the predictions to work as projectile aimbot, we can determine the distance from the player kind of like how 
// the current projectile aimbot currently does but we will follow the predicted path instead of just placing a vector in
// a really simple fassion.

// This is based on the estimation that GetAbsVelocity predicts players next position for the next createmove tick

// A set of vectors for every potential player
static Vector last_predicted_vector[32];
// An array to store predictions
static Vector last_predictions[32];
// Vectors to determine whether the player was in the air last tick
static bool last_predicted_inair[32];

// Should be run every createmove to predict playermovement
void RunPredictPlayers() {
	
	// Create a cached ent for use in the for loop
	CachedEntity* ent;
	
	// Loop through players
	for (int i = 0; i < 32; i++) {
		
		// Get an ent from current loop and check for dormancy/null
		ent = ENTITY(i);
		if (CE_BAD(ent)) continue;
		
		// Grab netvar for ground to control type of prediction
		int flags = CE_INT(g_pLocalPlayer->entity, netvar.iFlags);
		bool ground = (flags & (1 << 0));
		
		// For ground prediction, we would just use the old method for now
		if (ground) {
			
			// Set our last "in air" state to false
			last_predicted_vector_inair[i] = false;
			
			
		// For air prediction, attempts to exerpolate strafing speed
		} else {
			
			// If we were not in the air last tick, we need to create our first prediction 
			if (!last_predicted_inair[i]) {
				
				// Set "in air" to true to allow air prediction to work next tick
				last_predicted_inair[i] = true;
				// Get our abs velocity and set it into the array
				velocity::EstimateAbsVelocity(RAW_ENT(ent), last_predicted_vector[i]);

				
			// Since we have been in the air last tick, we can create an offset off of prediction errors
			} else {
				
				// Create a storage vector and get abs velocity of 
				Vector current_prediction;
				velocity::EstimateAbsVelocity(RAW_ENT(ent), current_prediction);
				last_predictions[32];
			}
		}
	}
}



// Draws our predicted player pathing for debug or visual use
void DrawPredictPlayers() {
	// TODO	
}
*/
