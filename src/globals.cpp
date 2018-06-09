/*
 * globals.cpp
 *
 *  Created on: Nov 25, 2016
 *      Author: nullifiedcat
 */

#include "common.hpp"

time_t time_injected{ 0 };

int g_AppID = 0;

void ThirdpersonCallback(IConVar *var, const char *pOldValue, float flOldValue)
{
    if (force_thirdperson.convar_parent && !force_thirdperson)
    {
        if (g_pLocalPlayer && CE_GOOD(g_pLocalPlayer->entity))
            CE_INT(g_pLocalPlayer->entity, netvar.nForceTauntCam) = 0;
    }
}

ConVar *sv_client_min_interp_ratio;
ConVar *cl_interp_ratio;
ConVar *cl_interp;
ConVar *cl_interpolate;

unsigned long tickcount   = 0;
char *force_name_newlined = new char[32]{ 0 };
bool need_name_change     = true;
int last_cmd_number       = 0;
CatVar force_name(CV_STRING, XORSTR("name"), XORSTR(""), XORSTR("Force name"));
CatVar
    cathook(CV_SWITCH, XORSTR("enabled"), XORSTR("1"), XORSTR("CatHook enabled"),
            XORSTR("Disabling this completely disables cathook (can be re-enabled)"));
CatVar ignore_taunting(CV_SWITCH, XORSTR("ignore_taunting"), XORSTR("1"), XORSTR("Ignore taunting"),
                       XORSTR("Aimbot/Triggerbot won't attack taunting enemies"));
// CatVar send_packets(CV_SWITCH, XORSTR("sendpackets"), XORSTR("1"), XORSTR("Send packets"), XORSTR("Internal
// useXORSTR(");
CatVar show_antiaim(CV_SWITCH, XORSTR("thirdperson_angles"), XORSTR("1"), XORSTR("Real TP angles"),
                    XORSTR("You can see your own AA/Aimbot angles in thirdperson"));
CatVar force_thirdperson(CV_SWITCH, XORSTR("thirdperson"), XORSTR("0"), XORSTR("Thirdperson"),
                         XORSTR("Enable thirdperson view"));
CatVar console_logging(
    CV_SWITCH, XORSTR("log"), XORSTR("0"), XORSTR("Debug Log"),
    XORSTR("Disable this if you don't need cathook messages in your console"));
// CatVar fast_outline(CV_SWITCH, XORSTR("fastoutline"), XORSTR("0"), XORSTR("Low quality outline"),
// XORSTR("Might increase performance when there is a lot of ESP text to draw"));
CatVar roll_speedhack(CV_KEY, XORSTR("rollspeedhack"), XORSTR("0"), XORSTR("Roll Speedhack"),
                      XORSTR("Roll speedhack key"));
char *disconnect_reason_newlined = new char[256]{ 0 };
CatVar disconnect_reason(CV_STRING, XORSTR("disconnect_reason"), XORSTR(""),
                         XORSTR("Disconnect reason"), XORSTR("A custom disconnect reason"));

CatVar event_log(CV_SWITCH, XORSTR("events"), XORSTR("0"), XORSTR("Advanced Events"));
void GlobalSettings::Init()
{
    sv_client_min_interp_ratio = g_ICvar->FindVar(XORSTR("sv_client_min_interp_ratio"));
    cl_interp_ratio            = g_ICvar->FindVar(XORSTR("cl_interp_ratio"));
    cl_interp                  = g_ICvar->FindVar(XORSTR("cl_interp"));
    cl_interpolate             = g_ICvar->FindVar(XORSTR("cl_interpolate"));

    force_thirdperson.OnRegister([](CatVar *var) {
        var->convar_parent->InstallChangeCallback(ThirdpersonCallback);
    });
    force_name.InstallChangeCallback(
        [](IConVar *var, const char *old, float oldfl) {
            std::string nl(force_name.GetString());
            ReplaceString(nl, XORSTR("\\n"), XORSTR("\n"));
            strncpy(force_name_newlined, nl.c_str(), 31);
            (void) oldfl;
        });
    disconnect_reason.InstallChangeCallback(
        [](IConVar *var, const char *old, float oldfl) {
            std::string nl(disconnect_reason.GetString());
            ReplaceString(nl, XORSTR("\\n"), XORSTR("\n"));
            strncpy(disconnect_reason_newlined, nl.c_str(), 255);
            (void) oldfl;
        });
    bInvalid = true;
}

CUserCmd *g_pUserCmd = nullptr;

GlobalSettings g_Settings;
