/*
 * CreateMove.cpp
 *
 *  Created on: Jan 8, 2017
 *      Author: nullifiedcat
 */

#include "CreateMove.h"

#include "../hooks.h"
#include "../hack.h"
#include "../common.h"
#include "hookedmethods.h"

float AngleDiff( float destAngle, float srcAngle )
{
	float delta;

	delta = fmodf(destAngle - srcAngle, 360.0f);
	if ( destAngle > srcAngle )
	{
		if ( delta >= 180 )
			delta -= 360;
	}
	else
	{
		if ( delta <= -180 )
			delta += 360;
	}
	return delta;
}//TODO temporary

#include "../profiler.h"

static CatVar minigun_jump(CV_SWITCH, "minigun_jump", "0", "TF2C minigun jump", "Allows jumping while shooting with minigun");

CatVar jointeam(CV_SWITCH, "fb_autoteam", "1", "Joins player team automatically (NYI)");
CatVar joinclass(CV_STRING, "fb_autoclass", "spy", "Class that will be picked after joining a team (NYI)");

bool CreateMove_hook(void* thisptr, float inputSample, CUserCmd* cmd) {
	SEGV_BEGIN;

	g_pUserCmd = cmd;

	if (TF2C && CE_GOOD(LOCAL_W) && minigun_jump && LOCAL_W->m_iClassID == g_pClassID->CTFMinigun) {
		CE_INT(LOCAL_W, netvar.iWeaponState) = 0;
	}
	bool ret;
	{
		PROF_SECTION(VALVE_CreateMove);
		ret = ((CreateMove_t*)hooks::hkClientMode->GetMethod(hooks::offCreateMove))(thisptr, inputSample, cmd);
	};

	PROF_SECTION(CATHOOK_CreateMove);

	if (!cmd) {
		return ret;
	}

	if (!cathook) {
		return ret;
	}

	if (!g_IEngine->IsInGame()) {
		g_Settings.bInvalid = true;
		return true;
	}

//	PROF_BEGIN();

	INetChannel* ch = (INetChannel*)g_IEngine->GetNetChannelInfo();
	if (ch && !hooks::IsHooked((void*)((uintptr_t)ch))) {
		hooks::hkNetChannel = new hooks::VMTHook();
		hooks::hkNetChannel->Init(ch, 0);
		hooks::hkNetChannel->HookMethod((void*)CanPacket_hook, hooks::offCanPacket);
		//hooks::hkNetChannel->HookMethod((void*)SendNetMsg_hook, hooks::offSendNetMsg);
		hooks::hkNetChannel->HookMethod((void*)Shutdown_hook, hooks::offShutdown);
		hooks::hkNetChannel->Apply();
	}
	//logging::Info("canpacket: %i", ch->CanPacket());
	//if (!cmd) return ret;

	bool time_replaced = false;
	float curtime_old = g_GlobalVars->curtime;

	if (!g_Settings.bInvalid && CE_GOOD(g_pLocalPlayer->entity)) {
		float servertime = (float)CE_INT(g_pLocalPlayer->entity, netvar.nTickBase) * g_GlobalVars->interval_per_tick;
		g_GlobalVars->curtime = servertime;
		time_replaced = true;
	}
	if (g_Settings.bInvalid) {
		gEntityCache.Invalidate();
	}
//	PROF_BEGIN();
	{
		PROF_SECTION(EntityCache);
		SAFE_CALL(gEntityCache.Update());
	}
//	PROF_END("Entity Cache updating");

	{
		PROF_SECTION(OtherCache);
		SAFE_CALL(g_pPlayerResource->Update());
		SAFE_CALL(g_pLocalPlayer->Update());
	}
	g_Settings.bInvalid = false;
	// Disabled because this causes EXTREME aimbot inaccuracy
	//if (!cmd->command_number) return ret;

	if (hacks::shared::followbot::bot) {
		PROF_SECTION(CM_FollowBot);
		static int team_joining_state = 0;
		static float last_jointeam_try = 0;

		if (g_GlobalVars->curtime < last_jointeam_try) {
			team_joining_state = 0;
			last_jointeam_try = 0.0f;
		}

		if (!g_pLocalPlayer->team || (g_pLocalPlayer->team == TEAM_SPEC)) {
			//if (!team_joining_state) logging::Info("Bad team, trying to join...");
			team_joining_state = 1;
		}
		else {
			if (team_joining_state) {
				logging::Info("Trying to change CLASS");
				g_IEngine->ExecuteClientCmd(format("join_class ", joinclass.GetString()).c_str());
			}
			team_joining_state = 0;
		}

		if (team_joining_state) {
			CachedEntity* found_entity = nullptr;
			for (int i = 1; i < 32 && i < HIGHEST_ENTITY; i++) {
				CachedEntity* ent = ENTITY(i);
				if (CE_BAD(ent)) continue;
				if (!ent->m_pPlayerInfo) continue;
				if (ent->m_pPlayerInfo->friendsID == hacks::shared::followbot::follow_steamid) {
					found_entity = ent;
					break;
				}
			}

			if (found_entity && CE_GOOD(found_entity)) {
				if (jointeam && (g_GlobalVars->curtime - last_jointeam_try) > 1.0f) {
					last_jointeam_try = g_GlobalVars->curtime;
					switch (CE_INT(found_entity, netvar.iTeamNum)) {
					case TEAM_RED:
						logging::Info("Trying to join team RED");
						g_IEngine->ExecuteClientCmd("jointeam red"); break;
					case TEAM_BLU:
						logging::Info("Trying to join team BLUE");
						g_IEngine->ExecuteClientCmd("jointeam blue"); break;
					}
				}
			}
		}

	}
	if (CE_GOOD(g_pLocalPlayer->entity)) {
		ResetCritHack();
		if (TF2) SAFE_CALL(UpdateHoovyList());
		g_pLocalPlayer->v_OrigViewangles = cmd->viewangles;
		{
			PROF_SECTION(CM_ESP);
			SAFE_CALL(hacks::shared::esp::CreateMove());
		}
		if (!g_pLocalPlayer->life_state && CE_GOOD(g_pLocalPlayer->weapon())) {
			if (TF2) {
				PROF_SECTION(CM_AntiBackstab);
				SAFE_CALL(hacks::tf2::antibackstab::CreateMove());
			}
			if (TF2) {
				PROF_SECTION(CM_Noisemaker);
				SAFE_CALL(hacks::tf2::noisemaker::CreateMove());
			}
			{
				PROF_SECTION(CM_Bunnyhop);
				SAFE_CALL(hacks::shared::bunnyhop::CreateMove());
			}
			{
				PROF_SECTION(CM_Aimbot);
				SAFE_CALL(hacks::shared::aimbot::CreateMove());
			}
			{
				PROF_SECTION(CM_AntiAim);
				SAFE_CALL(hacks::shared::antiaim::ProcessUserCmd(cmd));
			}
			if (TF) {
				PROF_SECTION(CM_AutoSticky);
				SAFE_CALL(hacks::tf::autosticky::CreateMove());
			}
			if (TF) {
				PROF_SECTION(CM_AutoReflect);
				SAFE_CALL(hacks::tf::autoreflect::CreateMove());
			}
			{
				PROF_SECTION(CM_Triggerbot);
				SAFE_CALL(hacks::shared::triggerbot::CreateMove());
			}
			if (TF) {
				PROF_SECTION(CM_AutoHeal);
				SAFE_CALL(hacks::tf::autoheal::CreateMove());
			}
			if (TF2) {
				PROF_SECTION(CM_AutoBackstab);
				SAFE_CALL(hacks::tf2::autobackstab::CreateMove());
			}
		}
		//SAFE_CALL(CREATE_MOVE(FollowBot));
		{
			PROF_SECTION(CM_Misc);
			SAFE_CALL(hacks::shared::misc::CreateMove());
		}
		{
			PROF_SECTION(CM_Spam);
			SAFE_CALL(hacks::shared::spam::CreateMove());
		}
//		PROF_END("Hacks processing");
		if (time_replaced) g_GlobalVars->curtime = curtime_old;
	}
	/*for (IHack* i_hack : hack::hacks) {
		if (!i_hack->CreateMove(thisptr, inputSample, cmd)) {
			ret = false;
		}
	}*/
	g_Settings.bInvalid = false;
	chat_stack::OnCreateMove();

	// TODO Auto Steam Friend
	if (g_GlobalVars->framecount % 1000 == 0) {
		PROF_SECTION(CM_Playerlist);
		playerlist::DoNotKillMe();
		ipc::UpdatePlayerlist();
	}

	if (CE_GOOD(g_pLocalPlayer->entity)) {
		PROF_SECTION(CM_FinalizingAngles);
		bool speedapplied = false;
		if ((int)roll_speedhack > 0 && (int)roll_speedhack <= BUTTON_CODE_LAST && g_pGUI->m_bPressedState[(int)roll_speedhack] && !(cmd->buttons & IN_ATTACK)) {
			float speed = cmd->forwardmove;
			if (fabs(speed) > 0.0f) {
				cmd->forwardmove = -speed;
				cmd->sidemove = 0.0f;
				cmd->viewangles.y = g_pLocalPlayer->v_OrigViewangles.y;
				cmd->viewangles.y -= 180.0f;
				if (cmd->viewangles.y < -180.0f) cmd->viewangles.y += 360.0f;
				cmd->viewangles.z = 90.0f;
				g_pLocalPlayer->bUseSilentAngles = true;
				speedapplied = true;
			}
		}

		if (g_pLocalPlayer->bUseSilentAngles) {
			if (!speedapplied) {
				Vector vsilent(cmd->forwardmove, cmd->sidemove, cmd->upmove);
				float speed = sqrt(vsilent.x * vsilent.x + vsilent.y * vsilent.y);
				Vector ang;
				VectorAngles(vsilent, ang);
				float yaw = DEG2RAD(ang.y - g_pLocalPlayer->v_OrigViewangles.y + cmd->viewangles.y);
				cmd->forwardmove = cos(yaw) * speed;
				cmd->sidemove = sin(yaw) * speed;
			}

			ret = false;
		}

		if (CE_GOOD(g_pLocalPlayer->entity) && !g_pLocalPlayer->life_state) {
			SAFE_CALL(hacks::shared::followbot::AfterCreateMove());
		}
		if (cmd)
			g_Settings.last_angles = cmd->viewangles;
	}

//	PROF_END("CreateMove");
	if (!(cmd->buttons & IN_ATTACK)) {
		//LoadSavedState();
	}
	g_pLocalPlayer->bAttackLastTick = (cmd->buttons & IN_ATTACK);
	return ret;

	SEGV_END;
	return true;
}
