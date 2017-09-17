/*
 * SpyAlert.cpp
 *
 *  Created on: Dec 5, 2016
 *      Author: nullifiedcat
 */

#include "SpyAlert.h"

#include "../common.h"
#include "../sdk.h"

namespace hacks { namespace tf { namespace spyalert {

CatVar enabled(CV_SWITCH, "spyalert_enabled", "0", "Enable", "Master SpyAlert switch");
CatVar distance_warning(CV_FLOAT, "spyalert_warning", "500.0", "Warning distance", "Distance where yellow warning shows");
CatVar distance_backstab(CV_FLOAT, "spyalert_backstab", "200.0", "Backstab distance", "Distance where red warning shows");
CatVar sound_alerts(CV_SWITCH, "spyalert_sound", "1", "Sound Alerts", "Demoman yells spy when a spy is within distance");
CatVar sound_alert_interval(CV_FLOAT, "spyalert_interval", "3", "Alert Interval", "Sound alert interval");

bool warning_triggered = false;
bool backstab_triggered = false;
float last_say = 0.0f;



void Draw() {
	CachedEntity *closest_spy, *ent;
	float closest_spy_distance, distance;
	int spy_count;

	if (!enabled) return;
	if (g_pLocalPlayer->life_state) return;	
	closest_spy = nullptr;
	closest_spy_distance = 0.0f;
	spy_count = 0;
	if (last_say > g_GlobalVars->curtime) last_say = 0;
	for (int i = 0; i < HIGHEST_ENTITY && i < 32; i++) {
		ent = ENTITY(i);
		if (CE_BAD(ent)) continue;
		if (CE_BYTE(ent, netvar.iLifeState)) continue;
		if (CE_INT(ent, netvar.iClass) != tf_class::tf_spy) continue;
		if (CE_INT(ent, netvar.iTeamNum) == g_pLocalPlayer->team) continue;
		if (IsPlayerInvisible(ent)) continue;
		distance = ent->m_flDistance;
		if (distance < closest_spy_distance || !closest_spy_distance) {
			closest_spy_distance = distance;
			closest_spy = ent;
		}
		if (distance < (float)distance_warning) {
			spy_count++;
		}
	}
	if (closest_spy && closest_spy_distance < (float)distance_warning) {
		if (closest_spy_distance < (float)distance_backstab) {
			if (!backstab_triggered) {
				if (sound_alerts && (g_GlobalVars->curtime - last_say) > (float)sound_alert_interval) {
					g_ISurface->PlaySound("vo/demoman_cloakedspy03.mp3");
					last_say = g_GlobalVars->curtime;
				}
				backstab_triggered = true;
			}
			AddCenterString(format("BACKSTAB WARNING! ", (int)(closest_spy_distance / 64 * 1.22f), "m (", spy_count, ")"), colors::red);
		} else if (closest_spy_distance < (float)distance_warning) {
			backstab_triggered = false;
			if (!warning_triggered) {
				if (sound_alerts && (g_GlobalVars->curtime - last_say) > (float)sound_alert_interval) {
					g_ISurface->PlaySound("vo/demoman_cloakedspy01.mp3");
					last_say = g_GlobalVars->curtime;
				}
				warning_triggered = true;
			}
			AddCenterString(format("Incoming spy! ", (int)(closest_spy_distance / 64 * 1.22f), "m (", spy_count, ")"), colors::yellow);
		}
	} else {
		warning_triggered = false;
		backstab_triggered = false;
	}
}

}}}
