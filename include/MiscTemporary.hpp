/*
  Created by Jenny White on 29.04.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#pragma once

#include <settings/Bool.hpp>
#include "common.hpp"
#include "DetourHook.hpp"

#define MENU_COLOR (menu_color)

// This is a temporary file to put code that needs moving/refactoring in.
extern bool *bSendPackets;
extern bool ignoreKeys;
extern std::array<int, 32> bruteint;
extern std::array<Timer, 32> timers;

extern Timer DelayTimer;
extern bool firstcm;
extern bool ignoredc;
extern bool user_sensitivity_ratio_set;

extern bool calculated_can_shoot;
extern float prevflow;
extern int prevflowticks;
#if ENABLE_VISUALS
extern int spectator_target;
extern CLC_VoiceData *voicecrash;
extern bool freecam_is_toggled;
#endif
extern settings::Boolean clean_chat;

extern settings::Boolean clean_screenshots;
extern settings::Boolean crypt_chat;
extern settings::Boolean nolerp;
extern float backup_lerp;
extern settings::Int fakelag_amount;
extern settings::Boolean fakelag_midair;
extern settings::Boolean no_zoom;
extern settings::Boolean no_scope;
extern settings::Boolean disable_visuals;
extern settings::Int print_r;
extern settings::Int print_g;
extern settings::Int print_b;
extern Color menu_color;
extern int stored_buttons;
typedef void (*CL_SendMove_t)();
extern DetourHook cl_warp_sendmovedetour;
extern DetourHook cl_nospread_sendmovedetour;
namespace hooked_methods
{
void sendIdentifyMessage(bool reply);
extern settings::Boolean identify;
} // namespace hooked_methods
