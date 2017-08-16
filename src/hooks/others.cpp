/*
 * others.cpp
 *
 *  Created on: Jan 8, 2017
 *      Author: nullifiedcat
 */

#include "../common.h"
#include "../netmessage.h"
#include "../chatlog.hpp"
#include "../hack.h"
#include "ucccccp.hpp"
#include "../hitrate.hpp"
#include "hookedmethods.h"

#if ENABLE_VISUALS == 1

static CatVar no_invisibility(CV_SWITCH, "no_invis", "0", "Remove Invisibility", "Useful with chams!");

// This hook isn't used yet!
int C_TFPlayer__DrawModel_hook(IClientEntity* _this, int flags) {
	float old_invis = *(float*)((uintptr_t)_this + 79u);
	if (no_invisibility) {
		if (old_invis < 1.0f) {
			*(float*)((uintptr_t)_this + 79u) = 0.5f;
		}
	}

	*(float*)((uintptr_t)_this + 79u) = old_invis;
}

static CatVar no_arms(CV_SWITCH, "no_arms", "0", "No Arms", "Removes arms from first person");
static CatVar no_hats(CV_SWITCH, "no_hats", "0", "No Hats", "Removes non-stock hats");

void DrawModelExecute_hook(IVModelRender* _this, const DrawModelState_t& state, const ModelRenderInfo_t& info, matrix3x4_t* matrix) {
	static const DrawModelExecute_t original = (DrawModelExecute_t)hooks::modelrender.GetMethod(offsets::DrawModelExecute());
	static const char* name;
	static std::string sname;
	static IClientUnknown *unk;
	static IClientEntity *ent;

	if (!cathook || !(no_arms || no_hats || (clean_screenshots && g_IEngine->IsTakingScreenshot()))) {
		original(_this, state, info, matrix);
		return;
	}

	PROF_SECTION(DrawModelExecute);

	if (no_arms || no_hats) {
		if (info.pModel) {
			name = g_IModelInfo->GetModelName(info.pModel);
			if (name) {
				sname = name;
				if (no_arms && sname.find("arms") != std::string::npos) {
					return;
				} else if (no_hats && sname.find("player/items") != std::string::npos) {
					return;
				}
			}
		}
	}

	unk = info.pRenderable->GetIClientUnknown();
	if (unk) {
		ent = unk->GetIClientEntity();
		if (ent && !effect_chams::g_EffectChams.drawing && effect_chams::g_EffectChams.ShouldRenderChams(ent)) {
			return;
		}
	}

	original(_this, state, info, matrix);
}


int IN_KeyEvent_hook(void* _this, int eventcode, int keynum, const char* pszCurrentBinding) {
	static const IN_KeyEvent_t original = (IN_KeyEvent_t)hooks::client.GetMethod(offsets::IN_KeyEvent());
#if ENABLE_GUI
	SEGV_BEGIN;
	if (g_pGUI->ConsumesKey((ButtonCode_t)keynum) && g_pGUI->Visible()) {
		return 0;
	}
	SEGV_END;
#endif
	return original(_this, eventcode, keynum, pszCurrentBinding);
}

CatVar override_fov_zoomed(CV_FLOAT, "fov_zoomed", "0", "FOV override (zoomed)", "Overrides FOV with this value when zoomed in (default FOV when zoomed is 20)");
CatVar override_fov(CV_FLOAT, "fov", "0", "FOV override", "Overrides FOV with this value");

void OverrideView_hook(void* _this, CViewSetup* setup) {
	static const OverrideView_t original = (OverrideView_t)hooks::clientmode.GetMethod(offsets::OverrideView());
	static bool zoomed;
	SEGV_BEGIN;
	original(_this, setup);
	if (!cathook) return;
	if (g_pLocalPlayer->bZoomed && override_fov_zoomed) {
		setup->fov = override_fov_zoomed;
	} else {
		if (override_fov) {
			setup->fov = override_fov;
		}
	}
	draw::fov = setup->fov;
	SEGV_END;
}

#endif

bool CanPacket_hook(void* _this) {
	const CanPacket_t original = (CanPacket_t)hooks::netchannel.GetMethod(offsets::CanPacket());
	SEGV_BEGIN;
	return *bSendPackets && original(_this);
	SEGV_END;
	return false;
}

CUserCmd* GetUserCmd_hook(IInput* _this, int sequence_number) {
	static const GetUserCmd_t original = (GetUserCmd_t)hooks::input.GetMethod(offsets::GetUserCmd());
	static CUserCmd* def;
	static int oldcmd;
	static INetChannel* ch;

	def = original(_this, sequence_number);
	if (def && command_number_mod.find(def->command_number) != command_number_mod.end()) {
		//logging::Info("Replacing command %i with %i", def->command_number, command_number_mod[def->command_number]);
		oldcmd = def->command_number;
		def->command_number = command_number_mod[def->command_number];
		def->random_seed = MD5_PseudoRandom(def->command_number) & 0x7fffffff;
		command_number_mod.erase(command_number_mod.find(oldcmd));
		*(int*)((unsigned)g_IBaseClientState + offsets::lastoutgoingcommand()) = def->command_number - 1;
		ch = (INetChannel*)g_IEngine->GetNetChannelInfo();//*(INetChannel**)((unsigned)g_IBaseClientState + offsets::m_NetChannel());
		*(int*)((unsigned)ch + offsets::m_nOutSequenceNr()) = def->command_number - 1;
	}
	return def;
}

static CatVar log_sent(CV_SWITCH, "debug_log_sent_messages", "0", "Log sent messages");

static CatCommand plus_use_action_slot_item_server("+cat_use_action_slot_item_server", "use_action_slot_item_server", []() {
	KeyValues* kv = new KeyValues("+use_action_slot_item_server");
	g_pLocalPlayer->using_action_slot_item = true;
	g_IEngine->ServerCmdKeyValues(kv);
});

static CatCommand minus_use_action_slot_item_server("-cat_use_action_slot_item_server", "use_action_slot_item_server", []() {
	KeyValues* kv = new KeyValues("-use_action_slot_item_server");
	g_pLocalPlayer->using_action_slot_item = false;
	g_IEngine->ServerCmdKeyValues(kv);
});

static CatVar newlines_msg(CV_INT, "chat_newlines", "0", "Prefix newlines", "Add # newlines before each your message", 0, 24);
// TODO replace \\n with \n
// TODO name \\n = \n
//static CatVar queue_messages(CV_SWITCH, "chat_queue", "0", "Queue messages", "Use this if you want to use spam/killsay and still be able to chat normally (without having your msgs eaten by valve cooldown)");

static CatVar airstuck(CV_KEY, "airstuck", "0", "Airstuck");
static CatVar crypt_chat(CV_SWITCH, "chat_crypto", "0", "Crypto chat", "Start message with !! and it will be only visible to cathook users");

bool SendNetMsg_hook(void* _this, INetMessage& msg, bool bForceReliable = false, bool bVoice = false) {
	static size_t say_idx, say_team_idx;
	static int offset;
	static std::string newlines;
	static NET_StringCmd stringcmd;

	// This is a INetChannel hook - it SHOULDN'T be static because netchannel changes.
	const SendNetMsg_t original = (SendNetMsg_t)hooks::netchannel.GetMethod(offsets::SendNetMsg());
	SEGV_BEGIN;
	// net_StringCmd
	if (msg.GetType() == 4 && (newlines_msg || crypt_chat)) {
		std::string str(msg.ToString());
		say_idx = str.find("net_StringCmd: \"say \"");
		say_team_idx = str.find("net_StringCmd: \"say_team \"");
		if (!say_idx || !say_team_idx) {
			offset = say_idx ? 26 : 21;
			bool crpt = false;
			if (crypt_chat) {
				std::string msg(str.substr(offset));
				msg = msg.substr(0, msg.length() - 2);
				if (msg.find("!!") == 0) {
					msg = ucccccp::encrypt(msg.substr(2));
					str = str.substr(0, offset) + msg + "\"\"";
					crpt = true;
				}
			}
			if (!crpt && newlines_msg) {
				// TODO move out? update in a value change callback?
				newlines = std::string((int)newlines_msg, '\n');
				str.insert(offset, newlines);
			}
			str = str.substr(16, str.length() - 17);
			//if (queue_messages && !chat_stack::CanSend()) {
				stringcmd.m_szCommand = str.c_str();
				return original(_this, stringcmd, bForceReliable, bVoice);
			//}
		}
	}
	static ConVar* sv_player_usercommand_timeout = g_ICvar->FindVar("sv_player_usercommand_timeout");
	static float lastcmd = 0.0f;
	if (lastcmd > g_GlobalVars->absoluteframetime) {
		lastcmd = g_GlobalVars->absoluteframetime;
	}
	if (airstuck.KeyDown() && !g_Settings.bInvalid) {
		if (CE_GOOD(LOCAL_E)) {
			if (lastcmd + sv_player_usercommand_timeout->GetFloat() - 0.1f < g_GlobalVars->curtime) {
				if (msg.GetType() == clc_Move) return false;
			} else {
				lastcmd = g_GlobalVars->absoluteframetime;
			}
		}
	}
	if (log_sent && msg.GetType() != 3 && msg.GetType() != 9) {
		logging::Info("=> %s [%i] %s", msg.GetName(), msg.GetType(), msg.ToString());
		unsigned char buf[4096];
		bf_write buffer("cathook_debug_buffer", buf, 4096);
		logging::Info("Writing %i", msg.WriteToBuffer(buffer));
		std::string bytes = "";
		constexpr char h2c[] = "0123456789abcdef";
		for (int i = 0; i <  buffer.GetNumBytesWritten(); i++) {
			//bytes += format(h2c[(buf[i] & 0xF0) >> 4], h2c[(buf[i] & 0xF)], ' ');
			bytes += format((unsigned short)buf[i], ' ');
		}
		logging::Info("%i bytes => %s", buffer.GetNumBytesWritten(), bytes.c_str());
	}
	return original(_this, msg, bForceReliable, bVoice);
	SEGV_END;
	return false;
}

static CatVar die_if_vac(CV_SWITCH, "die_if_vac", "0", "Die if VAC banned");

void Shutdown_hook(void* _this, const char* reason) {
	// This is a INetChannel hook - it SHOULDN'T be static because netchannel changes.
	const Shutdown_t original = (Shutdown_t)hooks::netchannel.GetMethod(offsets::Shutdown());
	logging::Info("Disconnect: %s", reason);
	if (strstr(reason, "VAC banned")) {
		if (die_if_vac) {
			logging::Info("VAC banned");
			*(int*)0 = 0;
			exit(1);
		}
	} else if (strstr(reason, "VAC")) {
		logging::Info("VAC error?");
	}
#if IPC_ENABLED
	ipc::UpdateServerAddress(true);
#endif
	SEGV_BEGIN;
	if (cathook && (disconnect_reason.convar_parent->m_StringLength > 3) && strstr(reason, "user")) {
		original(_this, disconnect_reason_newlined);
	} else {
		original(_this, reason);
	}
	SEGV_END;
}

static CatVar resolver(CV_SWITCH, "resolver", "0", "Resolve angles");

CatEnum namesteal_enum({ "OFF", "PASSIVE", "ACTIVE" });
CatVar namesteal(namesteal_enum, "name_stealer", "0", "Name Stealer", "Attemt to steal your teammates names. Usefull for avoiding kicks\nPassive only changes when the name stolen is no longer the best name to use\nActive Attemps to change the name whenever possible");

static std::string stolen_name;

// Func to get a new entity to steal name from
bool StolenName(){

	// Array to store potential namestealer targets with a bookkeeper to tell how full it is
	int potential_targets[32];
	int potential_targets_length = 0;
	
	// Go through entities looking for potential targets
	for (int i = 1; i < HIGHEST_ENTITY; i++) {
		CachedEntity* ent = ENTITY(i);
		
		// Check if ent is a good target
		if (!ent) continue;
		if (ent == LOCAL_E) continue;
		if (!ent->m_Type == ENTITY_PLAYER) continue;
		if (ent->m_bEnemy) continue;
		
		// Check if name is current one
		player_info_s info;
		if (g_IEngine->GetPlayerInfo(ent->m_IDX, &info)) {
					
			// If our name is the same as current, than change it
			if (std::string(info.name) == stolen_name) {
				// Since we found the ent we stole our name from and it is still good, if user settings are passive, then we return true and dont alter our name
				if ((int)namesteal == 1) {
					return true;
				// Otherwise we continue to change our name to something else
				} else continue;
			}
			
		// a ent without a name is no ent we need, contine for a different one
		} else continue;
				
		// Save the ent to our array
		potential_targets[potential_targets_length] = i;
		potential_targets_length++;
		
		// With our maximum amount of players reached, dont search for anymore
		if (potential_targets_length >= 32) break;
	}
	
	// Checks to prevent crashes
	if (potential_targets_length == 0) return false;
	
	// Get random number that we can use with our array
	int target_random_num = floor(RandFloatRange(0, potential_targets_length - 0.1F));
	
	// Get a idx from our random array position
	int new_target = potential_targets[target_random_num];
	
	// Grab username of user
	player_info_s info;
	if (g_IEngine->GetPlayerInfo(new_target, &info)) {
				
		// If our name is the same as current, than change it and return true
		stolen_name = std::string(info.name);
		return true;
	}
	
	// Didnt get playerinfo
	return false;											
}

static CatVar ipc_name(CV_STRING, "name_ipc", "", "IPC Name");

const char* GetFriendPersonaName_hook(ISteamFriends* _this, CSteamID steamID) {
	static const GetFriendPersonaName_t original = (GetFriendPersonaName_t)hooks::steamfriends.GetMethod(offsets::GetFriendPersonaName());
  
#if IPC_ENABLED
	if (ipc::peer) {
		static std::string namestr(ipc_name.GetString());
		namestr.assign(ipc_name.GetString());
		if (namestr.length() > 3) {
			ReplaceString(namestr, "%%", std::to_string(ipc::peer->client_id));
			return namestr.c_str();
		}
	}
#endif

	// Check User settings if namesteal is allowed
	if (namesteal && steamID == g_ISteamUser->GetSteamID()) {
		
		// We dont want to steal names while not in-game as there are no targets to steal from
		if (g_IEngine->IsInGame()) {

			// Check if we have a username to steal, func automaticly steals a name in it. 
			if (StolenName()) {
				
				// Return the name that has changed from the func above
				return format(stolen_name, "\x0F").c_str();
			}
		}
	}
	
	if ((strlen(force_name.GetString()) > 1) && steamID == g_ISteamUser->GetSteamID()) {

		return force_name_newlined;
	}
	return original(_this, steamID);
}

static CatVar cursor_fix_experimental(CV_SWITCH, "experimental_cursor_fix", "1", "Cursor fix");

void FireGameEvent_hook(void* _this, IGameEvent* event) {
	static const FireGameEvent_t original = (FireGameEvent_t)hooks::clientmode4.GetMethod(offsets::FireGameEvent());
	const char* name = event->GetName();
	if (name) {
		if (event_log) {
			if (!strcmp(name, "player_connect_client") ||
				!strcmp(name, "player_disconnect") ||
				!strcmp(name, "player_team")) {
				return;
			}
		}
	}
	original(_this, event);
}

static CatVar hitrate_check(CV_SWITCH, "hitrate", "0", "Monitor hitrate");

void FrameStageNotify_hook(void* _this, int stage) {
	static IClientEntity *ent;

	PROF_SECTION(FrameStageNotify_TOTAL);

	static const FrameStageNotify_t original = (FrameStageNotify_t)hooks::client.GetMethod(offsets::FrameStageNotify());
	SEGV_BEGIN;
	if (!g_IEngine->IsInGame()) g_Settings.bInvalid = true;
#if ENABLE_VISUALS == 1
	{
		PROF_SECTION(FSN_skinchanger);
		hacks::tf2::skinchanger::FrameStageNotify(stage);
	}
#endif
	if (stage == FRAME_NET_UPDATE_POSTDATAUPDATE_START) {
		angles::Update();
		hacks::shared::anticheat::CreateMove();
		if (hitrate_check) {
			hitrate::Update();
		}
	}
	if (resolver && cathook && !g_Settings.bInvalid && stage == FRAME_NET_UPDATE_POSTDATAUPDATE_START) {
		PROF_SECTION(FSN_resolver);
		for (int i = 1; i < 32 && i < HIGHEST_ENTITY; i++) {
			if (i == g_IEngine->GetLocalPlayer()) continue;
			ent = g_IEntityList->GetClientEntity(i);
			if (ent && !ent->IsDormant() && !NET_BYTE(ent, netvar.iLifeState)) {
				Vector& angles = NET_VECTOR(ent, netvar.m_angEyeAngles);
				if (angles.x >= 90) angles.x = -89;
				if (angles.x <= -90) angles.x = 89;
				angles.y = fmod(angles.y + 180.0f, 360.0f);
				if (angles.y < 0) angles.y += 360.0f;
				angles.y -= 180.0f;
			}
		}
	}
	if (stage == FRAME_START) {
#if IPC_ENABLED
		static Timer nametimer {};
		if (nametimer.test_and_set(1000 * 10)) {
			if (ipc::peer) {
				ipc::StoreClientData();
			}
		}
		static Timer ipc_timer {};
		if (ipc_timer.test_and_set(1000)) {
			if (ipc::peer) {
				ipc::Heartbeat();
				ipc::UpdateTemporaryData();
			}
		}
#endif
		hacks::shared::autojoin::UpdateSearch();
		if (!hack::command_stack().empty()) {
			PROF_SECTION(PT_command_stack);
			std::lock_guard<std::mutex> guard(hack::command_stack_mutex);
			while (!hack::command_stack().empty()) {
				logging::Info("executing %s", hack::command_stack().top().c_str());
				g_IEngine->ClientCmd_Unrestricted(hack::command_stack().top().c_str());
				hack::command_stack().pop();
			}
		}
#if TEXTMODE_STDIN == 1
		static auto last_stdin = std::chrono::system_clock::from_time_t(0);
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - last_stdin).count();
		if (ms > 500) {
			UpdateInput();
			last_stdin = std::chrono::system_clock::now();
		}
#endif
	}
#if ENABLE_VISUALS == 1
	if (cathook && !g_Settings.bInvalid && stage == FRAME_RENDER_START) {
#if ENABLE_GUI
		if (cursor_fix_experimental) {
			if (gui_visible) {
				g_ISurface->SetCursorAlwaysVisible(true);
			} else {
				g_ISurface->SetCursorAlwaysVisible(false);
			}
		}
#endif
		IF_GAME(IsTF()) {
			if (CE_GOOD(LOCAL_E) && no_zoom) RemoveCondition<TFCond_Zoomed>(LOCAL_E);
		}
		if (force_thirdperson && !g_pLocalPlayer->life_state && CE_GOOD(g_pLocalPlayer->entity)) {
			CE_INT(g_pLocalPlayer->entity, netvar.nForceTauntCam) = 1;
		}
		if (stage == 5 && show_antiaim && g_IInput->CAM_IsThirdPerson()) {
			if (CE_GOOD(g_pLocalPlayer->entity)) {
				CE_FLOAT(g_pLocalPlayer->entity, netvar.deadflag + 4) = g_Settings.last_angles.x;
				CE_FLOAT(g_pLocalPlayer->entity, netvar.deadflag + 8) = g_Settings.last_angles.y;
			}
		}
	}
#endif /* TEXTMODE */
	SAFE_CALL(original(_this, stage));
	SEGV_END;
}

static CatVar clean_chat(CV_SWITCH, "clean_chat", "0", "Clean chat", "Removes newlines from chat");
static CatVar dispatch_log(CV_SWITCH, "debug_log_usermessages", "0", "Log dispatched user messages");

bool DispatchUserMessage_hook(void* _this, int type, bf_read& buf) {
	int loop_index, s, i, j;
	char *data, c;

	static const DispatchUserMessage_t original = (DispatchUserMessage_t)hooks::client.GetMethod(offsets::DispatchUserMessage());
	SEGV_BEGIN;
	if (type == 4) {
		loop_index = 0;
		s = buf.GetNumBytesLeft();
		if (s < 256) {
			data = (char*)alloca(s);
			for (i = 0; i < s; i++)
				data[i] = buf.ReadByte();
			j = 0;
			std::string name;
			std::string message;
			for (i = 0; i < 3; i++) {
				while ((c = data[j++]) && (loop_index < 128)) {
					loop_index++;
					if (clean_chat)
						if ((c == '\n' || c == '\r') && (i == 1 || i == 2)) data[j - 1] = '*';
					if (i == 1) name.push_back(c);
					if (i == 2) message.push_back(c);
				}
			}
			if (crypt_chat) {
				if (message.find("!!") == 0) {
					if (ucccccp::validate(message)) {
						PrintChat("\x07%06X%s\x01: %s", 0xe05938, name.c_str(), ucccccp::decrypt(message).c_str());
					}
				}
			}
			chatlog::LogMessage(data[0], message);
			buf = bf_read(data, s);
			buf.Seek(0);
		}
	}
	if (dispatch_log) {
		logging::Info("D> %i", type);
	}
	return original(_this, type, buf);
	SEGV_END;
	return false;
}

void LevelInit_hook(void* _this, const char* newmap) {
	static const LevelInit_t original = (LevelInit_t)hooks::clientmode.GetMethod(offsets::LevelInit());
	playerlist::Save();
	g_IEngine->ClientCmd_Unrestricted("exec cat_matchexec");
	hacks::shared::aimbot::Reset();
	chat_stack::Reset();
	hacks::shared::anticheat::ResetEverything();
	original(_this, newmap);
	hacks::shared::walkbot::OnLevelInit();
#if IPC_ENABLED
	if (ipc::peer) {
		ipc::peer->memory->peer_user_data[ipc::peer->client_id].ts_connected = time(nullptr);
	}
#endif
}

void LevelShutdown_hook(void* _this) {
	static const LevelShutdown_t original = (LevelShutdown_t)hooks::clientmode.GetMethod(offsets::LevelShutdown());
	need_name_change = true;
	playerlist::Save();
	g_Settings.bInvalid = true;
	hacks::shared::aimbot::Reset();
	chat_stack::Reset();
	hacks::shared::anticheat::ResetEverything();
	original(_this);
#if IPC_ENABLED
	if (ipc::peer) {
		ipc::peer->memory->peer_user_data[ipc::peer->client_id].ts_disconnected = time(nullptr);
	}
#endif
}

