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
#include <link.h>

#include "../profiler.h"

static CatVar minigun_jump(CV_SWITCH, "minigun_jump", "0", "TF2C minigun jump", "Allows jumping while shooting with minigun");

CatVar jointeam(CV_SWITCH, "fb_autoteam", "1", "Joins player team automatically (NYI)");
CatVar joinclass(CV_STRING, "fb_autoclass", "spy", "Class that will be picked after joining a team (NYI)");

CatVar nolerp(CV_SWITCH, "nolerp", "0", "NoLerp mode (experimental)");

class CMoveData;
namespace engine_prediction {


void RunEnginePrediction(IClientEntity* ent, CUserCmd *ucmd) {
	if (!ent) return;

	typedef void(*SetupMoveFn)(IPrediction*, IClientEntity *, CUserCmd *, class IMoveHelper *, CMoveData *);
	typedef void(*FinishMoveFn)(IPrediction*, IClientEntity *, CUserCmd*, CMoveData*);

	void **predictionVtable = *((void ***)g_IPrediction);
	SetupMoveFn oSetupMove = (SetupMoveFn)(*(unsigned*)(predictionVtable + 19));
	FinishMoveFn oFinishMove = (FinishMoveFn)(*(unsigned*)(predictionVtable + 20));

	//CMoveData *pMoveData = (CMoveData*)(sharedobj::client->lmap->l_addr + 0x1F69C0C);
	//CMoveData movedata {};
	char* object = new char[165];
	CMoveData *pMoveData = (CMoveData*)object;

	float frameTime = g_GlobalVars->frametime;
	float curTime = g_GlobalVars->curtime;

	CUserCmd defaultCmd;
	if(ucmd == NULL) {
		ucmd = &defaultCmd;
	}

	NET_VAR(ent, 4188, CUserCmd*) = ucmd;

	g_GlobalVars->curtime =  g_GlobalVars->interval_per_tick * NET_INT(ent, netvar.nTickBase);
	g_GlobalVars->frametime = g_GlobalVars->interval_per_tick;

	*g_PredictionRandomSeed = MD5_PseudoRandom(g_pUserCmd->command_number) & 0x7FFFFFFF;
	g_IGameMovement->StartTrackPredictionErrors(reinterpret_cast<CBasePlayer*>(ent));
	oSetupMove(g_IPrediction, ent, ucmd, NULL, pMoveData);
	g_IGameMovement->ProcessMovement(reinterpret_cast<CBasePlayer*>(ent), pMoveData);
	oFinishMove(g_IPrediction, ent, ucmd, pMoveData);
	g_IGameMovement->FinishTrackPredictionErrors(reinterpret_cast<CBasePlayer*>(ent));

	delete[] object;

	NET_VAR(ent, 4188, CUserCmd*) = nullptr;

	g_GlobalVars->frametime = frameTime;
	g_GlobalVars->curtime = curTime;

	return;
}

/*float o_curtime;
float o_frametime;

void Start() {
	g_IGameMovement->StartTrackPredictionErrors((CBasePlayer*)(RAW_ENT(LOCAL_E)));

	IClientEntity* player = RAW_ENT(LOCAL_E);
	// CPredictableId::ResetInstanceCounters();
	*(reinterpret_cast<CUserCmd*>(reinterpret_cast<uintptr_t>(player) + 1047)) = g_pUserCmd;
	o_curtime = g_GlobalVars->curtime;
	o_frametime = g_GlobalVars->frametime;
	*g_PredictionRandomSeed = MD5_PseudoRandom(g_pUserCmd->command_number) & 0x7FFFFFFF;
	g_GlobalVars->curtime = CE_INT(LOCAL_E, netvar.nTickBase) * g_GlobalVars->interval_per_tick;
	g_GlobalVars->frametime = g_GlobalVars->interval_per_tick;

	CMoveData data;

}

void End() {
	*g_PredictionRandomSeed = -1;
	g_GlobalVars->curtime = o_curtime;
	g_GlobalVars->frametime = o_frametime;
}*/

}

static CatVar engine_pred(CV_SWITCH, "engine_prediction", "0", "Engine Prediction");
static CatVar debug_projectiles(CV_SWITCH, "debug_projectiles", "0", "Debug Projectiles");

static CatVar fakelag_amount(CV_INT, "fakelag", "0", "Bad Fakelag");

bool CreateMove_hook(void* thisptr, float inputSample, CUserCmd* cmd) {
	static CreateMove_t original_method = (CreateMove_t)hooks::clientmode.GetMethod(offsets::CreateMove());
	bool time_replaced, ret, speedapplied;
	float curtime_old, servertime, speed, yaw;
	Vector vsilent, ang;
	INetChannel* ch;

	SEGV_BEGIN;
	tickcount++;
	g_pUserCmd = cmd;

	IF_GAME (IsTF2C()) {
		if (CE_GOOD(LOCAL_W) && minigun_jump && LOCAL_W->m_iClassID == CL_CLASS(CTFMinigun)) {
			CE_INT(LOCAL_W, netvar.iWeaponState) = 0;
		}
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
		hooks::netchannel.HookMethod((void*)CanPacket_hook, offsets::CanPacket());
		hooks::netchannel.HookMethod((void*)SendNetMsg_hook, offsets::SendNetMsg());
		hooks::netchannel.HookMethod((void*)Shutdown_hook, offsets::Shutdown());
		hooks::netchannel.Apply();
#if IPC_ENABLED
		ipc::UpdateServerAddress();
#endif
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

	if (nolerp) {
		g_pUserCmd->tick_count += 1;
		if (sv_client_min_interp_ratio->GetInt() != -1) {
			//sv_client_min_interp_ratio->m_nFlags = 0;
			sv_client_min_interp_ratio->SetValue(-1);
		}
		if (cl_interp->m_fValue != 0) {
			cl_interp->SetValue(0);
			cl_interp->m_fValue = 0.0f;
			cl_interp->m_nValue = 0;
		}
		if (cl_interp_ratio->GetInt() != 0) cl_interp_ratio->SetValue(0);
		if (cl_interpolate->GetInt() != 0) cl_interpolate->SetValue(0);
	}

	if (!g_Settings.bInvalid && CE_GOOD(g_pLocalPlayer->entity)) {
		servertime = (float)CE_INT(g_pLocalPlayer->entity, netvar.nTickBase) * g_GlobalVars->interval_per_tick;
		g_GlobalVars->curtime = servertime;
		time_replaced = true;
	}
	if (g_Settings.bInvalid) {
		entity_cache::Invalidate();
	}

	// Disabled because this causes EXTREME aimbot inaccuracy
	//if (!cmd->command_number) return ret;
//	PROF_BEGIN();
	{
		PROF_SECTION(EntityCache);
		SAFE_CALL(entity_cache::Update());
	}
//	PROF_END("Entity Cache updating");
	{
		PROF_SECTION(CM_PlayerResource);
		SAFE_CALL(g_pPlayerResource->Update());
	}
	{
		PROF_SECTION(CM_LocalPlayer);
		SAFE_CALL(g_pLocalPlayer->Update());
	}
	g_Settings.bInvalid = false;

	hacks::shared::autojoin::Update();

#ifdef IPC_ENABLED
	static int team_joining_state = 0;
	static float last_jointeam_try = 0;
	CachedEntity *found_entity, *ent;

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
		IF_GAME (IsTF2()) {
			SAFE_CALL(UpdateHoovyList());
		}
		g_pLocalPlayer->v_OrigViewangles = cmd->viewangles;
#if ENABLE_VISUALS == 1
		{
			PROF_SECTION(CM_esp);
			SAFE_CALL(hacks::shared::esp::CreateMove());
		}
#endif
		if (!g_pLocalPlayer->life_state && CE_GOOD(g_pLocalPlayer->weapon())) {
			{
				PROF_SECTION(CM_walkbot);
				SAFE_CALL(hacks::shared::walkbot::Move());
			}
			// Walkbot can leave game.
			if (!g_IEngine->IsInGame()) {
				return ret;
			}
			IF_GAME (IsTF()) {
				PROF_SECTION(CM_uberspam);
				SAFE_CALL(hacks::tf::uberspam::CreateMove());
			}
			IF_GAME (IsTF2()) {
				PROF_SECTION(CM_antibackstab);
				SAFE_CALL(hacks::tf2::antibackstab::CreateMove());
			}
			IF_GAME (IsTF2()) {
				PROF_SECTION(CM_noisemaker);
				SAFE_CALL(hacks::tf2::noisemaker::CreateMove());
			}
			{
				PROF_SECTION(CM_bunnyhop);
				SAFE_CALL(hacks::shared::bunnyhop::CreateMove());
			}
			if (engine_pred) engine_prediction::RunEnginePrediction(RAW_ENT(LOCAL_E), g_pUserCmd);
			{
				PROF_SECTION(CM_aimbot);
				SAFE_CALL(hacks::shared::aimbot::CreateMove());
			}
			{
				PROF_SECTION(CM_antiaim);
				SAFE_CALL(hacks::shared::antiaim::ProcessUserCmd(cmd));
			}
			IF_GAME (IsTF()) {
				PROF_SECTION(CM_autosticky);
				SAFE_CALL(hacks::tf::autosticky::CreateMove());
			}
			IF_GAME (IsTF()) {
				PROF_SECTION(CM_autoreflect);
				SAFE_CALL(hacks::tf::autoreflect::CreateMove());
			}
			{
				PROF_SECTION(CM_triggerbot);
				SAFE_CALL(hacks::shared::triggerbot::CreateMove());
			}
			IF_GAME (IsTF()) {
				PROF_SECTION(CM_autoheal);
				SAFE_CALL(hacks::tf::autoheal::CreateMove());
			}
			IF_GAME (IsTF2()) {
				PROF_SECTION(CM_autobackstab);
				SAFE_CALL(hacks::tf2::autobackstab::CreateMove());
			}
			if (debug_projectiles)
				projectile_logging::Update();
			Prediction_CreateMove();
		}
		{
			PROF_SECTION(CM_misc);
			SAFE_CALL(hacks::shared::misc::CreateMove());
		}
		{
			PROF_SECTION(CM_spam);
			SAFE_CALL(hacks::shared::spam::CreateMove());
		}
	}
	if (time_replaced) g_GlobalVars->curtime = curtime_old;
	g_Settings.bInvalid = false;
	{
		PROF_SECTION(CM_chat_stack);
		chat_stack::OnCreateMove();
	}
	{
		PROF_SECTION(CM_healarrow);
		hacks::tf2::healarrow::CreateMove();
	}
	{
		PROF_SECTION(CM_lagexploit);
		hacks::shared::lagexploit::CreateMove();
	}

	// TODO Auto Steam Friend

#if IPC_ENABLED
	{
		PROF_SECTION(CM_playerlist);
		static Timer ipc_update_timer {};
	//	playerlist::DoNotKillMe();
		if (ipc_update_timer.test_and_set(1000 * 10)) {
			ipc::UpdatePlayerlist();
		}
	}
#endif

	*bSendPackets = true;

	if (CE_GOOD(g_pLocalPlayer->entity)) {
		static int fakelag_queue = 0;
		if (fakelag_amount) {
			if (fakelag_queue == int(fakelag_amount) || (g_pUserCmd->buttons & IN_ATTACK)) {
				*bSendPackets = true;
			} else if (fakelag_queue < int(fakelag_amount)) {
				*bSendPackets = false;
			} else {
				fakelag_queue = 0;
			}
			fakelag_queue++;
		}
		speedapplied = false;
		if (roll_speedhack && g_IInputSystem->IsButtonDown((ButtonCode_t)((int)roll_speedhack)) && !(cmd->buttons & IN_ATTACK)) {
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
			PROF_SECTION(CM_followbot);
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
