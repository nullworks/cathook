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

// FIXME remove this temporary code already!
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
}

#include "../profiler.h"

static CatVar minigun_jump(CV_SWITCH, "minigun_jump", "0", "TF2C minigun jump", "Allows jumping while shooting with minigun");

CatVar jointeam(CV_SWITCH, "fb_autoteam", "1", "Joins player team automatically (NYI)");
CatVar joinclass(CV_STRING, "fb_autoclass", "spy", "Class that will be picked after joining a team (NYI)");

namespace engine_prediction {

float o_curtime;
float o_frametime;

void Start() {
	o_curtime = g_GlobalVars->curtime;
	o_frametime = g_GlobalVars->frametime;
	*g_PredictionRandomSeed = MD5_PseudoRandom(g_pUserCmd->command_number) & 0x7FFFFFFF;
	g_GlobalVars->curtime = CE_INT(LOCAL_E, netvar.nTickBase) * g_GlobalVars->interval_per_tick;
	g_GlobalVars->frametime = g_GlobalVars->interval_per_tick;
	g_IGameMovement->StartTrackPredictionErrors((CBasePlayer*)(RAW_ENT(LOCAL_E)));

	CMoveData data;

}

void End() {
	*g_PredictionRandomSeed = -1;
	g_GlobalVars->curtime = o_curtime;
	g_GlobalVars->frametime = o_frametime;
}

}

bool CreateMove_hook(void* thisptr, float inputSample, CUserCmd* cmd) {
	static CreateMove_t original_method = (CreateMove_t)hooks::clientmode.GetMethod(offsets::CreateMove());
	static bool time_replaced, ret, speedapplied;
	static float curtime_old, servertime, speed, yaw;
	static Vector vsilent, ang;
	static INetChannel* ch;

	SEGV_BEGIN;
	tickcount++;
	g_pUserCmd = cmd;

	if (TF2C && CE_GOOD(LOCAL_W) && minigun_jump && LOCAL_W->m_iClassID == g_pClassID->CTFMinigun) {
		CE_INT(LOCAL_W, netvar.iWeaponState) = 0;
	}

	ret = original_method(thisptr, inputSample, cmd);

	PROF_SECTION(CreateMove);

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

	if (g_pUserCmd && g_pUserCmd->command_number) last_cmd_number = g_pUserCmd->command_number;

	ch = (INetChannel*)g_IEngine->GetNetChannelInfo();
	if (ch && !hooks::IsHooked((void*)ch)) {
		hooks::netchannel.Set(ch);
		//hooks::netchannel.HookMethod((void*)CanPacket_hook, offsets::CanPacket());
		hooks::netchannel.HookMethod((void*)SendNetMsg_hook, offsets::SendNetMsg());
		hooks::netchannel.HookMethod((void*)Shutdown_hook, offsets::Shutdown());
		hooks::netchannel.Apply();
	}

	/**bSendPackets = true;
	if (hacks::shared::lagexploit::ExploitActive()) {
		*bSendPackets = ((g_pUserCmd->command_number % 4) == 0);
		//logging::Info("%d", *bSendPackets);
	}*/

	//logging::Info("canpacket: %i", ch->CanPacket());
	//if (!cmd) return ret;


	time_replaced = false;
	curtime_old = g_GlobalVars->curtime;

	if (!g_Settings.bInvalid && CE_GOOD(g_pLocalPlayer->entity)) {
		servertime = (float)CE_INT(g_pLocalPlayer->entity, netvar.nTickBase) * g_GlobalVars->interval_per_tick;
		g_GlobalVars->curtime = servertime;
		time_replaced = true;
	}
	if (g_Settings.bInvalid) {
		entity_cache::Invalidate();
	}
//	PROF_BEGIN();
	{
		PROF_SECTION(EntityCache);
		SAFE_CALL(entity_cache::Update());
	}
//	PROF_END("Entity Cache updating");
	SAFE_CALL(g_pPlayerResource->Update());
	SAFE_CALL(g_pLocalPlayer->Update());
	g_Settings.bInvalid = false;
	// Disabled because this causes EXTREME aimbot inaccuracy
	//if (!cmd->command_number) return ret;
#ifdef IPC_ENABLED
	static int team_joining_state = 0;
	static float last_jointeam_try = 0;
	static CachedEntity *found_entity, *ent;

	if (hacks::shared::followbot::bot) {

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
			found_entity = nullptr;
			for (int i = 1; i < 32 && i < HIGHEST_ENTITY; i++) {
				ent = ENTITY(i);
				if (CE_BAD(ent)) continue;
				if (ent->player_info.friendsID == hacks::shared::followbot::follow_steamid) {
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
#endif
	if (CE_GOOD(g_pLocalPlayer->entity)) {
		ResetCritHack();
		if (TF2) SAFE_CALL(UpdateHoovyList());
			g_pLocalPlayer->v_OrigViewangles = cmd->viewangles;
//		PROF_BEGIN();
		//RunEnginePrediction(g_pLocalPlayer->entity, cmd);
		SAFE_CALL(hacks::shared::esp::CreateMove());
		if (!g_pLocalPlayer->life_state && CE_GOOD(g_pLocalPlayer->weapon())) {
			if (TF) SAFE_CALL(hacks::tf::uberspam::CreateMove());
			if (TF2) SAFE_CALL(hacks::tf2::antibackstab::CreateMove());
			if (TF2) SAFE_CALL(hacks::tf2::noisemaker::CreateMove());
			SAFE_CALL(hacks::shared::bunnyhop::CreateMove());
			SAFE_CALL(hacks::shared::aimbot::CreateMove());
			SAFE_CALL(hacks::shared::antiaim::ProcessUserCmd(cmd));
			if (TF) SAFE_CALL(hacks::tf::autosticky::CreateMove());
			if (TF) SAFE_CALL(hacks::tf::autoreflect::CreateMove());
			SAFE_CALL(hacks::shared::triggerbot::CreateMove());
			if (TF) SAFE_CALL(hacks::tf::autoheal::CreateMove());
			if (TF2) SAFE_CALL(hacks::tf2::autobackstab::CreateMove());
		}
		//SAFE_CALL(CREATE_MOVE(FollowBot));
		SAFE_CALL(hacks::shared::misc::CreateMove());
		SAFE_CALL(hacks::shared::spam::CreateMove());
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
	hacks::shared::lagexploit::CreateMove();

	// TODO Auto Steam Friend
	if (g_GlobalVars->framecount % 1000 == 0) {
		playerlist::DoNotKillMe();
#ifdef IPC_ENABLED
		ipc::UpdatePlayerlist();
#endif
	}

	if (CE_GOOD(g_pLocalPlayer->entity)) {
		speedapplied = false;
		if (roll_speedhack && g_pGUI->m_bPressedState[(int)roll_speedhack] && !(cmd->buttons & IN_ATTACK)) { // FIXME OOB
			speed = cmd->forwardmove;
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
				vsilent.x = cmd->forwardmove;
				vsilent.y = cmd->sidemove;
				vsilent.z = cmd->upmove;
				speed = sqrt(vsilent.x * vsilent.x + vsilent.y * vsilent.y);
				VectorAngles(vsilent, ang);
				yaw = DEG2RAD(ang.y - g_pLocalPlayer->v_OrigViewangles.y + cmd->viewangles.y);
				cmd->forwardmove = cos(yaw) * speed;
				cmd->sidemove = sin(yaw) * speed;
			}

			ret = false;
		}
#ifdef IPC_ENABLED
		if (CE_GOOD(g_pLocalPlayer->entity) && !g_pLocalPlayer->life_state) {
			SAFE_CALL(hacks::shared::followbot::AfterCreateMove());
		}
#endif
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
