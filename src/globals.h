/*
 * globals.h
 *
 *  Created on: Nov 16, 2016
 *      Author: nullifiedcat
 */

#ifndef GLOBALS_H_
#define GLOBALS_H_

#include <time.h>

class Vector;
class ConVar;
class CatVar;

extern int g_AppID;
extern unsigned long tickcount;

extern ConVar* sv_client_min_interp_ratio;
extern ConVar* cl_interp_ratio;
extern ConVar* cl_interp;
extern ConVar* cl_interpolate;

extern CatVar event_log;
extern CatVar cathook; // Master switch
extern CatVar ignore_taunting;
extern bool* bSendPackets;
extern CatVar show_antiaim;
extern CatVar force_thirdperson;
extern CatVar console_logging;
extern CatVar fast_outline;
extern CatVar roll_speedhack;
extern CatVar force_name;
extern char* force_name_newlined;
extern bool need_name_change;
extern int last_cmd_number;

extern char* disconnect_reason_newlined;
extern CatVar disconnect_reason;

extern time_t time_injected;

class GlobalSettings {
public:
	void Init();
	bool bInvalid { true };
	Vector last_angles;
};

class CUserCmd;
extern CUserCmd* g_pUserCmd;

extern GlobalSettings g_Settings;

#endif /* GLOBALS_H_ */
