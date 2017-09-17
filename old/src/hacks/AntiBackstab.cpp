/*
 * AntiBackstab.cpp
 *
 *  Created on: Apr 14, 2017
 *      Author: nullifiedcat
 */

#include "../common.h"
#include "../hack.h"

namespace hacks { namespace tf2 { namespace antibackstab {

static CatVar enabled(CV_SWITCH, "antibackstab", "0", "Enable", "Main anti-backstab switch");
static CatVar distance(CV_FLOAT, "antibackstab_distance", "200", "Distance", "Distance Until anti-backstab reacts");
static CatVar silent(CV_SWITCH, "antibackstab_silent", "1", "Silent", "Works silently without moving your view");
static CatVar angle(CV_FLOAT, "antibackstab_angle", "150", "Detection Angle");
static CatVar sayno(CV_SWITCH, "antibackstab_nope", "0", "Nope!", "Memes");

void SayNope() {
	static float last_say = 0.0f;
	if (g_GlobalVars->curtime < last_say) last_say = 0.0f;
	if (g_GlobalVars->curtime - last_say < 1.5f) return;
	hack::ExecuteCommand("voicemenu 0 7");
	last_say = g_GlobalVars->curtime;
}

float GetAngle(CachedEntity* spy) {
	float yaw, yaw2, anglediff;
	Vector diff;
	yaw = g_pLocalPlayer->v_OrigViewangles.y;
	const Vector& A = LOCAL_E->m_vecOrigin;
	const Vector& B = spy->m_vecOrigin;
	diff = (A - B);
	yaw2 = acos(diff.x / diff.Length()) * 180.0f / PI;
	if (diff.y < 0) yaw2 = -yaw2;
	anglediff = yaw - yaw2;
	if (anglediff > 180) anglediff -= 360;
	if (anglediff < -180) anglediff += 360;
	//logging::Info("Angle: %.2f | %.2f | %.2f | %.2f", yaw, yaw2, anglediff, yaw - yaw2);
	return anglediff;
}

CachedEntity* ClosestSpy() {
	CachedEntity *closest, *ent;
	float closest_dist, dist;

	closest = nullptr;
	closest_dist = 0.0f;

	for (int i = 1; i < 32 && i < g_IEntityList->GetHighestEntityIndex(); i++) {
		ent = ENTITY(i);
		if (CE_BAD(ent)) continue;
		if (CE_BYTE(ent, netvar.iLifeState)) continue;
		if (CE_INT(ent, netvar.iClass) != tf_class::tf_spy) continue;
		if (CE_INT(ent, netvar.iTeamNum) == g_pLocalPlayer->team) continue;
		if (IsPlayerInvisible(ent)) continue;
		dist = ent->m_flDistance;
		if (fabs(GetAngle(ent)) > (float)angle) {
			break;
			//logging::Info("Backstab???");
		}
		if (dist < (float)distance && (dist < closest_dist || !closest_dist)) {
			closest_dist = dist;
			closest = ent;
		}
	}
	return closest;
}

void CreateMove() {
	CachedEntity *spy;
	Vector diff;
	float yaw2, resultangle;

	if (!enabled) return;
	spy = ClosestSpy();
	if (spy) {
		const Vector& A = LOCAL_E->m_vecOrigin;
		const Vector& B = spy->m_vecOrigin;
		diff = (A - B);
		yaw2 = acos(diff.x / diff.Length()) * 180.0f / PI;
		if (diff.y < 0) yaw2 = -yaw2;
		if (yaw2 < -180) yaw2 += 360;
		if (yaw2 > 180) yaw2 -= 360;
		resultangle = -180 + yaw2;
		if (resultangle < -180) resultangle += 360;
		g_pUserCmd->viewangles.y = resultangle;
		if (silent) {
			// This isn't a spy aimbot.
			if (!(g_pUserCmd->buttons & IN_ATTACK))
				g_pLocalPlayer->bUseSilentAngles = true;
		}
		if (sayno) SayNope();
	}
}

void PaintTraverse() {
	if (!enabled) return;
	/*CachedEntity* spy = ClosestSpy();
	if (!spy) return;
	const Vector& A = LOCAL_E->m_vecOrigin;
	const Vector& B = spy->m_vecOrigin;
	const float yaw = g_pLocalPlayer->v_OrigViewangles.y;
	const Vector diff = (A - B);
	float yaw2 = acos(diff.x / diff.Length()) * 180.0f / PI;
	if (diff.y < 0) yaw2 = -yaw2;
	float anglediff = yaw - yaw2;
	if (anglediff > 180) anglediff -= 360;
	if (anglediff < -180) anglediff += 360;
	AddSideString(format("closest: ", B.x, ' ', B.y, ' ', B.z));
	AddSideString(format("yaw: ", yaw));
	AddSideString(format("diff: ", diff.x, ' ', diff.y, ' ', diff.z));
	AddSideString(format("yaw2: ", yaw2));
	AddSideString(format("anglediff: ", anglediff));*/
}

}}}

