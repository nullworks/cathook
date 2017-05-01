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
	int movetype = CE_INT(player, netvar.movetype);
	if (movetype == MOVETYPE_FLY || movetype == MOVETYPE_NOCLIP) return 0.0f;
	if (HasCondition(player, TFCond_Parachute)) return 0.448f;
	return 1.0f;
}

bool PerformProjectilePrediction(CachedEntity* target, int hitbox) {
	Vector src, vel, hit; ;
	//src = vfunc<Vector(*)(IClientEntity*)>(RAW_ENT(target), 299)(RAW_ENT(target));

	return true;
}

Vector ProjectilePrediction(CachedEntity* ent, int hb, float speed, float gravitymod, float entgmod) {
	if (!ent) return Vector();
	Vector result = SimpleLatencyPrediction(ent, hb);
	if (speed == 0.0f) return Vector();
	float dtg = DistanceToGround(ent);
	Vector vel = CE_VECTOR(ent, netvar.vVelocity);
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
	static trace_t* ground_trace = new trace_t();
	Ray_t ray;
	Vector endpos = origin;
	endpos.z -= 8192;
	ray.Init(origin, endpos);
	g_ITrace->TraceRay(ray, MASK_PLAYERSOLID, trace::g_pFilterNoPlayer, ground_trace);
	return 8192.0f * ground_trace->fraction;
}
