/*
 * AntiAim.cpp
 *
 *  Created on: Oct 26, 2016
 *      Author: nullifiedcat
 */

#include "AntiAim.h"

#include "../common.h"
#include "../sdk.h"

namespace hacks { namespace shared { namespace antiaim {

CatVar enabled(CV_SWITCH, "aa_enabled", "0", "Anti-Aim", "Master AntiAim switch");
CatVar yaw(CV_FLOAT, "aa_yaw", "0.0", "Yaw", "Static yaw (left/right)", 360.0);
CatVar pitch(CV_FLOAT, "aa_pitch", "-89.0", "Pitch", "Static pitch (up/down)", -89.0, 89.0);
CatEnum yaw_mode_enum({ "KEEP", "STATIC", "JITTER", "BIGRANDOM", "RANDOM", "SPIN", "OFFSETKEEP" });
CatEnum pitch_mode_enum({ "KEEP", "STATIC", "JITTER", "RANDOM", "FLIP", "FAKEFLIP", "FAKEUP", "FAKEDOWN", "UP", "DOWN" });
CatVar yaw_mode(yaw_mode_enum, "aa_yaw_mode", "0", "Yaw mode", "Yaw mode");
CatVar pitch_mode(pitch_mode_enum, "aa_pitch_mode", "0", "Pitch mode", "Pitch mode");
CatVar roll(CV_FLOAT, "aa_roll", "0", "Roll", "Roll angle (viewangles.z)", -180, 180);
CatVar no_clamping(CV_SWITCH, "aa_no_clamp", "0", "Don't clamp angles", "Use this with STATIC mode for unclamped manual angles");
CatVar spin(CV_FLOAT, "aa_spin", "10.0", "Spin speed", "Spin speed (degrees/second)");

CatVar aaaa_enabled(CV_SWITCH, "aa_aaaa_enabled", "0", "Enable AAAA", "Enable Anti-Anti-Anti-Aim (Overrides AA Pitch)");
CatVar aaaa_interval(CV_FLOAT, "aa_aaaa_interval", "0", "Interval", "Interval in seconds, 0 = random");
CatVar aaaa_interval_random_high(CV_FLOAT, "aa_aaaa_interval_high", "15", "Interval Ceiling", "Upper bound for random AAAA interval");
CatVar aaaa_interval_random_low(CV_FLOAT, "aa_aaaa_interval_low", "3", "Interval Floor", "Lower bound for random AAAA interval");
CatEnum aaaa_modes_enum({"(FAKE)UP", "(FAKE)DOWN"});
CatVar aaaa_mode(aaaa_modes_enum, "aa_aaaa_mode", "0", "Mode", "Anti-Anti-Anti-Aim Mode");
CatVar aaaa_flip_key(CV_KEY, "aa_aaaa_flip_key", "0", "Flip key", "If you press that key, current AA will change");

float cur_yaw = 0.0f;
int safe_space = 0;

float aaaa_timer_start = 0.0f;
float aaaa_timer = 0.0f;
int aaaa_stage = 0;
bool aaaa_key_pressed = false;

float GetAAAAPitch() {
	switch ((int)aaaa_mode) {
	case 0:
		return aaaa_stage ? -271 : -89;
	case 1:
		return aaaa_stage ? 271 : 89;
	}
	return 0;
}

float GetAAAATimerLength() {
	if (aaaa_interval) {
		return (float)aaaa_interval;
	} else {
		return RandFloatRange((float)aaaa_interval_random_low, (float)aaaa_interval_random_high);
	}
}

void NextAAAA() {
	aaaa_stage++;
	// TODO temporary..
	if (aaaa_stage > 1) aaaa_stage = 0;
}

void UpdateAAAAKey() {
	if (g_IInputSystem->IsButtonDown((ButtonCode_t)(int)aaaa_flip_key)) {
		if (!aaaa_key_pressed) {
			aaaa_key_pressed = true;
			NextAAAA();
		}
	} else aaaa_key_pressed = false;
}

void UpdateAAAATimer() {
	const float& curtime = g_GlobalVars->curtime;
	if (aaaa_timer_start > curtime) aaaa_timer_start = 0.0f;
	if (!aaaa_timer || !aaaa_timer_start) {
		aaaa_timer = GetAAAATimerLength();
		aaaa_timer_start = curtime;
	} else {
		if (curtime - aaaa_timer_start > aaaa_timer) {
			NextAAAA();
			aaaa_timer_start = curtime;
			aaaa_timer = GetAAAATimerLength();
		}
	}
}

void SetSafeSpace(int safespace) {
	if (safespace > safe_space) safe_space = safespace;
}

bool ShouldAA(CUserCmd* cmd) {
	if (!enabled) return false;
	if (cmd->buttons & IN_USE) return false;
	if (cmd->buttons & IN_ATTACK) {
		if (!(TF2 && g_pLocalPlayer->weapon()->m_iClassID == g_pClassID->CTFCompoundBow)) {
			if (CanShoot()) return false;
		}
	}
	if ((cmd->buttons & IN_ATTACK2) && g_pLocalPlayer->weapon()->m_iClassID == g_pClassID->CTFLunchBox) return false;
	switch (GetWeaponMode(g_pLocalPlayer->entity)) {
	case weapon_projectile:
		if (g_pLocalPlayer->weapon()->m_iClassID == g_pClassID->CTFCompoundBow) {
			if (!(cmd->buttons & IN_ATTACK)) {
				if (g_pLocalPlayer->bAttackLastTick) SetSafeSpace(4);
			}
			break;
		}
		/* no break */
	case weapon_melee:
	case weapon_throwable:
		if ((cmd->buttons & (IN_ATTACK | IN_ATTACK2)) || g_pLocalPlayer->bAttackLastTick) {
			SetSafeSpace(8);
			return false;
		}
	}
	if (safe_space) {
		safe_space--;
		if (safe_space < 0) safe_space = 0;
		return false;
	}
	return true;
}

void ProcessUserCmd(CUserCmd* cmd) {
	if (!ShouldAA(cmd)) return;
	float& p = cmd->viewangles.x;
	float& y = cmd->viewangles.y;
	static bool flip = false;
	bool clamp = !no_clamping;
	switch ((int)yaw_mode) {
	case 1: // FIXED
		y = (float)yaw;
		break;
	case 2: // JITTER
		if (flip) y += 90;
		else y -= 90;
		break;
	case 3: // BIGRANDOM
		y = RandFloatRange(-65536.0f, 65536.0f);
		clamp = false;
		break;
	case 4: // RANDOM
		y = RandFloatRange(-180.0f, 180.0f);
		break;
	case 5: // SPIN
		cur_yaw += (float)spin;
		if (cur_yaw > 180) cur_yaw = -180;
		if (cur_yaw < -180) cur_yaw = 180;
		y = cur_yaw;
		break;
	case 6: // OFFSETKEEP
		y += (float)yaw;
		break;
	}
	switch ((int)pitch_mode) {
	case 1:
		p = (float)pitch;
		break;
	case 2:
		if (flip) p += 30.0f;
		else p -= 30.0f;
		break;
	case 3:
		p = RandFloatRange(-89.0f, 89.0f);
		break;
	case 4:
		p = flip ? 89.0f : -89.0f;
		break;
	case 5:
		p = flip ? 271.0f : -271.0f;
		clamp = false;
		break;
	case 6:
		p = -271.0f;
		clamp = false;
		break;
	case 7:
		p = 271.0f;
		clamp = false;
		break;
	case 8:
		p = -89.0f;
		break;
	case 9:
		p = 89.0f;
		break;
	}
	flip = !flip;
	if (clamp) fClampAngle(cmd->viewangles);
	if (roll) cmd->viewangles.z = (float)roll;
	if (aaaa_enabled) {
		UpdateAAAAKey();
		UpdateAAAATimer();
		p = GetAAAAPitch();
	}
	g_pLocalPlayer->bUseSilentAngles = true;
}

}}}
