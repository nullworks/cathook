/*
 * globals.cpp
 *
 *  Created on: Nov 25, 2016
 *      Author: nullifiedcat
 */

#include "common.h"
#include "sdk.h"
#include "copypasted/CSignature.h"

time_t time_injected { 0 };

int g_AppID = 0;

void ThirdpersonCallback(IConVar* var, const char* pOldValue, float flOldValue) {
	if (force_thirdperson.convar_parent && !force_thirdperson) {
		if (g_pLocalPlayer && CE_GOOD(g_pLocalPlayer->entity))
			CE_INT(g_pLocalPlayer->entity, netvar.nForceTauntCam) = 0;
	}
}

ConVar* sv_client_min_interp_ratio;
ConVar* cl_interp_ratio;
ConVar* cl_interp;
ConVar* cl_interpolate;

unsigned long tickcount = 0;
char* force_name_newlined = new char[32] { 0 };
bool need_name_change = true;
int last_cmd_number = 0;
CatVar force_name(CV_STRING, "name", "", "Force name");
CatVar cathook(CV_SWITCH, "enabled", "1", "CatHook enabled", "Disabling this completely disables cathook (can be re-enabled)");
CatVar ignore_taunting(CV_SWITCH, "ignore_taunting", "1", "Ignore taunting", "Aimbot/Triggerbot won't attack taunting enemies");
bool* bSendPackets; // i'd probably want to hook it, idk.
//CatVar send_packets(CV_SWITCH, "sendpackets", "1", "Send packets", "Internal use");
CatVar show_antiaim(CV_SWITCH, "thirdperson_angles", "1", "Real TP angles", "You can see your own AA/Aimbot angles in thirdperson");
CatVar force_thirdperson(CV_SWITCH, "thirdperson", "0", "Thirdperson", "Enable thirdperson view");
CatVar console_logging(CV_SWITCH, "log", "1", "Debug Log", "Disable this if you don't need cathook messages in your console");
//CatVar fast_outline(CV_SWITCH, "fastoutline", "0", "Low quality outline", "Might increase performance when there is a lot of ESP text to draw");
CatVar roll_speedhack(CV_KEY, "rollspeedhack", "0", "Roll Speedhack", "Roll speedhack key");
char* disconnect_reason_newlined = new char[256] { 0 };
CatVar disconnect_reason(CV_STRING, "disconnect_reason", "", "Disconnect reason", "A custom disconnect reason");

CatVar event_log(CV_SWITCH, "events", "1", "Advanced Events");
void GlobalSettings::Init() {
	sv_client_min_interp_ratio = g_ICvar->FindVar("sv_client_min_interp_ratio");
	cl_interp_ratio = g_ICvar->FindVar("cl_interp_ratio");
	cl_interp = g_ICvar->FindVar("cl_interp");
	cl_interpolate = g_ICvar->FindVar("cl_interpolate");

	bSendPackets = new bool;
	*bSendPackets = true;
	force_thirdperson.OnRegister([](CatVar* var) {
		var->convar_parent->InstallChangeCallback(ThirdpersonCallback);
	});
	force_name.InstallChangeCallback([](IConVar* var, const char* old, float oldfl) {
		std::string nl(force_name.GetString());
		ReplaceString(nl, "\\n", "\n");
		strncpy(force_name_newlined, nl.c_str(), 31);
		(void)oldfl;
	});
	disconnect_reason.InstallChangeCallback([](IConVar* var, const char* old, float oldfl) {
		std::string nl(disconnect_reason.GetString());
		ReplaceString(nl, "\\n", "\n");
		strncpy(disconnect_reason_newlined, nl.c_str(), 255);
		(void)oldfl;
	});
	bInvalid = true;
}

CUserCmd* g_pUserCmd = nullptr;

GlobalSettings g_Settings;
