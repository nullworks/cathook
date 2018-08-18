/*
 * globals.cpp
 *
 *  Created on: Nov 25, 2016
 *      Author: nullifiedcat
 */

#include <globals.h>
#include <settings/Bool.hpp>

#include "common.hpp"

static settings::Bool global_enable{ "hack.enable", "true" };

time_t time_injected{ 0 };

int g_AppID = 0;

ConVar *sv_client_min_interp_ratio;
ConVar *cl_interp_ratio;
ConVar *cl_interp;
ConVar *cl_interpolate;

unsigned long tickcount   = 0;
char *force_name_newlined = new char[32]{ 0 };
bool need_name_change     = true;
int last_cmd_number       = 0;

void GlobalSettings::Init()
{
    sv_client_min_interp_ratio = g_ICvar->FindVar("sv_client_min_interp_ratio");
    cl_interp_ratio            = g_ICvar->FindVar("cl_interp_ratio");
    cl_interp                  = g_ICvar->FindVar("cl_interp");
    cl_interpolate             = g_ICvar->FindVar("cl_interpolate");

    bInvalid = true;
}

CUserCmd *current_user_cmd{ nullptr };

bool isHackActive()
{
    return *global_enable;
}

GlobalSettings g_Settings{};
