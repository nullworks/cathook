/*
 * others.cpp
 *
 *  Created on: Jan 8, 2017
 *      Author: nullifiedcat
 */

#include "../common.h"
#include "../netmessage.h"
#include "../hack.h"
#include "hookedmethods.h"

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
	hacks::shared::lagexploit::GetUserCmd(def, sequence_number);
	return def;
}

int IN_KeyEvent_hook(void* _this, int eventcode, int keynum, const char* pszCurrentBinding) {
	static const IN_KeyEvent_t original = (IN_KeyEvent_t)hooks::client.GetMethod(offsets::IN_KeyEvent());
	SEGV_BEGIN;
	if (g_pGUI->ConsumesKey((ButtonCode_t)keynum) && g_pGUI->Visible()) {
		return 0;
	}
	return original(_this, eventcode, keynum, pszCurrentBinding);
	SEGV_END;
	return 0;
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

bool SendNetMsg_hook(void* _this, INetMessage& msg, bool bForceReliable = false, bool bVoice = false) {
	static size_t say_idx, say_team_idx;
	static int offset;
	static std::string newlines;
	static NET_StringCmd stringcmd;

	// This is a INetChannel hook - it SHOULDN'T be static because netchannel changes.
	const SendNetMsg_t original = (SendNetMsg_t)hooks::netchannel.GetMethod(offsets::SendNetMsg());
	SEGV_BEGIN;
	// net_StringCmd
	if (msg.GetType() == 4 && (newlines_msg)) {
		std::string str(msg.ToString());
		say_idx = str.find("net_StringCmd: \"say \"");
		say_team_idx = str.find("net_StringCmd: \"say_team \"");
		if (!say_idx || !say_team_idx) {
			offset = say_idx ? 26 : 21;
			if (newlines_msg) {
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

CatVar disconnect_reason(CV_STRING, "disconnect_reason", "", "Disconnect reason", "A custom disconnect reason");

void Shutdown_hook(void* _this, const char* reason) {
	// This is a INetChannel hook - it SHOULDN'T be static because netchannel changes.
	const Shutdown_t original = (Shutdown_t)hooks::netchannel.GetMethod(offsets::Shutdown());
	SEGV_BEGIN;
	if (cathook && (disconnect_reason.convar_parent->m_StringLength > 3) && strstr(reason, "user")) {
		original(_this, disconnect_reason.GetString());
	} else {
		original(_this, reason);
	}
	SEGV_END;
}

static CatVar glow_enabled(CV_SWITCH, "glow_old_enabled", "0", "Enable", "Make sure to enable glow_outline_effect_enable in tf2 settings");
static CatVar glow_alpha(CV_FLOAT, "glow_old_alpha", "1", "Alpha", "Glow Transparency", 0.0f, 1.0f);
static CatVar resolver(CV_SWITCH, "resolver", "0", "Resolve angles");

const char* GetFriendPersonaName_hook(ISteamFriends* _this, CSteamID steamID) {
	static const GetFriendPersonaName_t original = (GetFriendPersonaName_t)hooks::steamfriends.GetMethod(offsets::GetFriendPersonaName());
	if ((force_name.convar->m_StringLength > 2) && steamID == g_ISteamUser->GetSteamID()) {
		return force_name.GetString();
	}
	return original(_this, steamID);
}

static CatVar cursor_fix_experimental(CV_SWITCH, "experimental_cursor_fix", "0", "Cursor fix");

void FrameStageNotify_hook(void* _this, int stage) {
	static IClientEntity *ent;
	static ConVar* glow_outline_effect = g_ICvar->FindVar("glow_outline_effect_enable");

	static const FrameStageNotify_t original = (FrameStageNotify_t)hooks::client.GetMethod(offsets::FrameStageNotify());
	SEGV_BEGIN;
	if (!g_IEngine->IsInGame()) g_Settings.bInvalid = true;
	// TODO hack FSN hook
	hacks::tf2::skinchanger::FrameStageNotify(stage);
	if (resolver && cathook && !g_Settings.bInvalid && stage == FRAME_NET_UPDATE_POSTDATAUPDATE_START) {
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
	if (TF && cathook && !g_Settings.bInvalid && stage == FRAME_RENDER_START) {
		if (cursor_fix_experimental) {
			if (gui_visible) {
				g_ISurface->SetCursorAlwaysVisible(true);
			} else {
				g_ISurface->SetCursorAlwaysVisible(false);
			}
		}
		if (CE_GOOD(LOCAL_E) && no_zoom) RemoveCondition(LOCAL_E, TFCond_Zoomed);
		if (glow_outline_effect->GetBool()) {
			if (glow_enabled) {
				for (int i = 0; i < g_GlowObjectManager->m_GlowObjectDefinitions.Size(); i++) {
					GlowObjectDefinition_t& glowobject = g_GlowObjectManager->m_GlowObjectDefinitions[i];
					if (glowobject.m_nNextFreeSlot != ENTRY_IN_USE)
						continue;
					int color = GetEntityGlowColor(glowobject.m_hEntity.m_Index & 0xFFF);
					if (color == 0) {
						glowobject.m_flGlowAlpha = 0.0f;
					} else {
						glowobject.m_flGlowAlpha = (float)glow_alpha;
					}
					unsigned char _b = (color >> 16) & 0xFF;
					unsigned char _g = (color >> 8)  & 0xFF;
					unsigned char _r = (color) & 0xFF;
					glowobject.m_vGlowColor.x = (float)_r / 255.0f;
					glowobject.m_vGlowColor.y = (float)_g / 255.0f;
					glowobject.m_vGlowColor.z = (float)_b / 255.0f;
				}
			}
			// Remove glow from dead entities
			for (int i = 0; i < g_GlowObjectManager->m_GlowObjectDefinitions.Count(); i++) {
				if (g_GlowObjectManager->m_GlowObjectDefinitions[i].m_nNextFreeSlot != ENTRY_IN_USE) continue;
				IClientEntity* ent = (IClientEntity*)g_IEntityList->GetClientEntityFromHandle(g_GlowObjectManager->m_GlowObjectDefinitions[i].m_hEntity);
				if (ent && ent->IsDormant()) {
					g_GlowObjectManager->DisableGlow(i);
				} else if (ent && ent->GetClientClass()->m_ClassID == g_pClassID->C_Player) {
					if (NET_BYTE(ent, netvar.iLifeState) != LIFE_ALIVE) {
						g_GlowObjectManager->DisableGlow(i);
					}
				}
			}
			if (glow_enabled) {
				for (int i = 1; i < g_IEntityList->GetHighestEntityIndex(); i++) {
					IClientEntity* entity = g_IEntityList->GetClientEntity(i);
					if (!entity || i == g_IEngine->GetLocalPlayer() || entity->IsDormant())
						continue;
					if (!CanEntityEvenGlow(i)) continue;
					int clazz = entity->GetClientClass()->m_ClassID;
					int current_handle = g_GlowObjectManager->GlowHandle(entity);
					bool shouldglow = ShouldEntityGlow(i);
					if (current_handle != -1) {
						if (!shouldglow) {
							g_GlowObjectManager->DisableGlow(current_handle);
						}
					} else {
						if (shouldglow) {
							g_GlowObjectManager->EnableGlow(entity, colors::white);
						}
					}
				}
			}
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
	SAFE_CALL(original(_this, stage));
	SEGV_END;
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
	SEGV_END;
}

static CatVar clean_chat(CV_SWITCH, "clean_chat", "0", "Clean chat", "Removes newlines from chat");
static CatVar dispatch_log(CV_SWITCH, "debug_log_usermessages", "0", "Log dispatched user messages");

bool DispatchUserMessage_hook(void* _this, int type, bf_read& buf) {
	static const DispatchUserMessage_t original = (DispatchUserMessage_t)hooks::client.GetMethod(offsets::DispatchUserMessage());
	SEGV_BEGIN;
	if (clean_chat) {
		if (type == 4) {
			int loop_index = 0;

			int s = buf.GetNumBytesLeft();
			char* data = new char[s];
			for (int i = 0; i < s; i++)
				data[i] = buf.ReadByte();
			int j = 0;
			char c;
			for (int i = 0; i < 3; i++) {
				while ((c = data[j++]) && (loop_index < 128)) {
					loop_index++;
					if ((c == '\n' || c == '\r') && (i == 1 || i == 2)) data[j - 1] = '*';
				}
			}
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
	hacks::shared::spam::Reset();
	original(_this, newmap);
}

void LevelShutdown_hook(void* _this) {
	static const LevelShutdown_t original = (LevelShutdown_t)hooks::clientmode.GetMethod(offsets::LevelShutdown());
	need_name_change = true;
	playerlist::Save();
	g_Settings.bInvalid = true;
	hacks::shared::aimbot::Reset();
	chat_stack::Reset();
	hacks::shared::spam::Reset();
	original(_this);
}

