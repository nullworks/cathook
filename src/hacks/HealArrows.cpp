/*
 * HealArrows.cpp
 *
 *  Created on: Jun 1, 2017
 *      Author: nullifiedcat
 */

#include "../common.h"

namespace hacks { namespace tf2 { namespace healarrow {

static CatVar healarrow_charge(CV_FLOAT, "healarrow_charge", "0.99", "Healarrow Charge","How long should cathook charge the bow");
static CatVar healarrow_timeout(CV_FLOAT, "healarrow_timeout", "7", "Healarrow Timeout","How long shoudl cathook wait between healarrows\nSetting this value too low or too high will break healarrows);
static CatVar healarrow(CV_SWITCH, "healarrow", "0", "Heal Arrow", "Super l33t healing arrows. Use huntsman and let Cathook setup an arrow shot that will heal enemies.\nRequires you to actually hit something, so it's recommended to have a friend on the enemy team.\nThis has no end to the health it can provide, but a max health of 1000 health per shot.\nPersonally, I once got a friend up to 50K health. Literally immortal butterknife spy of death\nUse huntsman, obviously.");

float healarrow_time = 0.0f;

void CreateMove() {
	if (CE_BAD(LOCAL_W)) return;
	if (healarrow) {
		if (g_pLocalPlayer->weapon()->m_iClassID == CL_CLASS(CTFCompoundBow)) {
			if (healarrow_time > g_GlobalVars->curtime) healarrow_time = 0.0f;
			float begincharge = CE_FLOAT(g_pLocalPlayer->weapon(), netvar.flChargeBeginTime);
			float charge = 0;
			if (begincharge != 0) {
				charge = g_GlobalVars->curtime - begincharge;
				if (charge > 1.0f) charge = 1.0f;
			}
			if (g_pUserCmd->command_number && ((g_GlobalVars->curtime - healarrow_time) > float(healarrow_timeout)) && (charge > (float)healarrow_charge) && (g_pUserCmd->buttons & IN_ATTACK)) {
				command_number_mod[g_pUserCmd->command_number] = g_pUserCmd->command_number + 450;
				g_pUserCmd->buttons &= ~IN_ATTACK;
				healarrow_time = g_GlobalVars->curtime;
				logging::Info("healarrow");
			}
		}
	}
}

void Draw() {
	if (healarrow) {
		if ((g_GlobalVars->curtime - healarrow_time) < float(healarrow_timeout)) {
			AddCenterString(format("Heal arrow charge: ", int(min(100.0f, (g_GlobalVars->curtime - healarrow_time) / float(healarrow_timeout)) * 100.0f), '%'), colors::yellow);
//			AddCenterString(format("Heal arrow time: ", healarrow_time));
		} else {
			AddCenterString("Heal arrow ready", colors::green);
		}
	}
}

}}}

