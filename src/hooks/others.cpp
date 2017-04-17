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

int C_TFPlayer__DrawModel_hook(IClientEntity* _this, int flags) {
	float old_invis = *(float*)((uintptr_t)_this + 79u);
	if (no_invisibility) {
		if (old_invis < 1.0f) {
			*(float*)((uintptr_t)_this + 79u) = 0.5f;
		}
	}

	*(float*)((uintptr_t)_this + 79u) = old_invis;
}

static CatVar no_arms(CV_SWITCH, "no_arms", "0", "No Arms");
static CatVar no_hats(CV_SWITCH, "no_hats", "0", "No Hats");

void DrawModelExecute_hook(IVModelRender* _this, const DrawModelState_t& state, const ModelRenderInfo_t& info, matrix3x4_t* matrix) {
	/*IClientUnknown* unknown = info.pRenderable->GetIClientUnknown();
	if (unknown) {
		IClientEntity* entity = unknown->GetIClientEntity();
		if (entity && entity->entindex() != -1) {
			if (entity->GetClientClass()->m_ClassID == g_pClassID->C_Player) {
				//CMatRenderContextPtr ptr();
			}
		}
	}*/
	if (!(no_arms || no_hats || (clean_screenshots && g_IEngine->IsTakingScreenshot()))) {
		((DrawModelExecute_t)(hooks::hkIVModelRender->GetMethod(hooks::offDrawModelExecute)))(_this, state, info, matrix);
		return;
	}

	if (no_arms || no_hats) {
		if (info.pModel) {
			const char* name = g_IModelInfo->GetModelName(info.pModel);
			if (name) {
				std::string sname(name);
				if (no_arms && sname.find("arms") != std::string::npos) {
					return;
				} else if (no_hats && sname.find("player/items") != std::string::npos) {
					return;
				}
			}
		}
	}

	IClientUnknown* unk = info.pRenderable->GetIClientUnknown();
	if (unk) {
		IClientEntity* ent = unk->GetIClientEntity();
		if (ent && !g_EffectChams.drawing && g_EffectChams.ShouldRenderChams(ent)) {
			return;
		}
	}

	((DrawModelExecute_t)(hooks::hkIVModelRender->GetMethod(hooks::offDrawModelExecute)))(_this, state, info, matrix);
}

bool CanPacket_hook(void* thisptr) {
	SEGV_BEGIN;
	return send_packets && ((CanPacket_t*)hooks::hkNetChannel->GetMethod(hooks::offCanPacket))(thisptr);
	SEGV_END;
	return false;
}

CUserCmd* GetUserCmd_hook(IInput* thisptr, int sequence_number) {
	CUserCmd* def = ((GetUserCmd_t*)(hooks::hkInput->GetMethod(hooks::offGetUserCmd)))(thisptr, sequence_number);
	if (def && command_number_mod.find(def->command_number) != command_number_mod.end()) {
		logging::Info("Replacing command %i with %i", def->command_number, command_number_mod[def->command_number]);
		int oldcmd = def->command_number;
		def->command_number = command_number_mod[def->command_number];
		def->random_seed = MD5_PseudoRandom(def->command_number) & 0x7fffffff;
		command_number_mod.erase(command_number_mod.find(oldcmd));
	}
	return def;
}

int IN_KeyEvent_hook(void* thisptr, int eventcode, int keynum, const char* pszCurrentBinding) {
	SEGV_BEGIN;
	if (g_pGUI->ConsumesKey((ButtonCode_t)keynum)) {
		return 0;
	}
	return ((IN_KeyEvent_t*)hooks::hkClient->GetMethod(hooks::offKeyEvent))(thisptr, eventcode, keynum, pszCurrentBinding);
	SEGV_END;
	return 0;
}

static CatVar log_sent(CV_SWITCH, "debug_log_sent_messages", "0", "Log sent messages");

static CatCommand plus_use_action_slot_item_server("+cat_use_action_slot_item_server", "use_action_slot_item_server", []() {
	KeyValues* kv = new KeyValues("+use_action_slot_item_server");
	g_IEngine->ServerCmdKeyValues(kv);
});

static CatCommand minus_use_action_slot_item_server("-cat_use_action_slot_item_server", "use_action_slot_item_server", []() {
	KeyValues* kv = new KeyValues("-use_action_slot_item_server");
	g_IEngine->ServerCmdKeyValues(kv);
});

bool SendNetMsg_hook(void* thisptr, INetMessage& msg, bool bForceReliable = false, bool bVoice = false) {
	SEGV_BEGIN;
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
	//logging::Info("Sending NetMsg! %i", msg.GetType());
	if (hacks::shared::airstuck::IsStuck() && cathook && !g_Settings.bInvalid) {
		switch (msg.GetType()) {
		case net_NOP:
		case net_SignonState:
		case net_StringCmd:
		case 16: // cmdKeyValues
			break;
		default:
			//logging::Info("Blocked net message %i %s", msg.GetType(), msg.GetName());
			return false;
		}
	}
	return ((SendNetMsg_t*)hooks::hkNetChannel->GetMethod(hooks::offSendNetMsg))(thisptr, msg, bForceReliable, bVoice);
	SEGV_END;
	return false;
}

CatVar disconnect_reason(CV_STRING, "disconnect_reason", "", "Disconnect reason", "A custom disconnect reason");

void Shutdown_hook(void* thisptr, const char* reason) {
	SEGV_BEGIN;
	if (cathook && (disconnect_reason.convar_parent->m_StringLength > 3) && strstr(reason, "user")) {
		((Shutdown_t*)hooks::hkNetChannel->GetMethod(hooks::offShutdown))(thisptr, disconnect_reason.GetString());
	} else {
		((Shutdown_t*)hooks::hkNetChannel->GetMethod(hooks::offShutdown))(thisptr, reason);
	}
	SEGV_END;
}

static CatVar glow_enabled(CV_SWITCH, "glow_enabled", "0", "Enable", "Make sure to enable glow_outline_effect_enable in tf2 settings");
static CatVar glow_alpha(CV_FLOAT, "glow_alpha", "1", "Alpha", "Glow Transparency", 0.0f, 1.0f);

void FrameStageNotify_hook(void* thisptr, int stage) {
	SEGV_BEGIN;
	//logging::Info("FrameStageNotify %i", stage);
	// Ambassador to festive ambassador changer. simple.
	if (!g_IEngine->IsInGame()) g_Settings.bInvalid = true;
	//logging::Info("fsi begin");// TODO dbg
	SVDBG("FSN %i", __LINE__);
	// TODO hack FSN hook
	if (TF && cathook && !g_Settings.bInvalid && stage == FRAME_RENDER_START) {
		if (glow_enabled) {
			for (int i = 0; i < g_GlowObjectManager->m_GlowObjectDefinitions.m_Size; i++) {
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
				/*bool old_byte = NET_BYTE(entity, 0xDA1);
				bool new_byte = (glow_enabled && ShouldEntityGlow(i));
				if (CanEntityEvenGlow(i)) NET_BYTE(entity, 0xDA1) = new_byte;
				if (old_byte != new_byte && clazz == g_pClassID->CTFPlayer) {
					if (new_byte) {
						vfunc<void(*)(IClientEntity*)>(entity, 290, 0)(entity);
					} else {
						vfunc<void(*)(IClientEntity*)>(entity, 291, 0)(entity);
					}
				}*/

			}
		}
		SVDBG("FSN %i", __LINE__);
		if (force_thirdperson && !g_pLocalPlayer->life_state && CE_GOOD(g_pLocalPlayer->entity)) {
			SVDBG("FSN %i", __LINE__);
			CE_INT(g_pLocalPlayer->entity, netvar.nForceTauntCam) = 1;
		}
		SVDBG("FSN %i", __LINE__);
		if (stage == 5 && show_antiaim && g_IInput->CAM_IsThirdPerson()) {
			SVDBG("FSN %i", __LINE__);
			if (CE_GOOD(g_pLocalPlayer->entity)) {
				CE_FLOAT(g_pLocalPlayer->entity, netvar.deadflag + 4) = g_Settings.last_angles.x;
				CE_FLOAT(g_pLocalPlayer->entity, netvar.deadflag + 8) = g_Settings.last_angles.y;
			}
			SVDBG("FSN %i", __LINE__);
		}
	}
	SVDBG("FSN %i", __LINE__);
	SAFE_CALL(((FrameStageNotify_t*)hooks::hkClient->GetMethod(hooks::offFrameStageNotify))(thisptr, stage));
	SEGV_END;
}

CatVar override_fov_zoomed(CV_FLOAT, "fov_zoomed", "0", "FOV override (zoomed)", "Overrides FOV with this value when zoomed in (default FOV when zoomed is 20)");
CatVar override_fov(CV_FLOAT, "fov", "0", "FOV override", "Overrides FOV with this value");

void OverrideView_hook(void* thisptr, CViewSetup* setup) {
	SEGV_BEGIN;
	((OverrideView_t*)hooks::hkClientMode->GetMethod(hooks::offOverrideView))(thisptr, setup);
	if (!cathook) return;
	bool zoomed = g_pLocalPlayer->bZoomed;
	if (zoomed && override_fov_zoomed) {
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

bool DispatchUserMessage_hook(void* thisptr, int type, bf_read& buf) {
	SEGV_BEGIN;
	if (clean_chat) {
		if (type == 4) {
			int s = buf.GetNumBytesLeft();
			char* data = new char[s];
			for (int i = 0; i < s; i++)
				data[i] = buf.ReadByte();
			int j = 0;
			for (int i = 0; i < 3; i++) {
				while (char c = data[j++]) {
					if ((c == '\n' || c == '\r') && (i == 1 || i == 2)) data[j - 1] = '?';
				}
			}
			buf = bf_read(data, s);
			buf.Seek(0);
		}
	}
	if (dispatch_log) {
		logging::Info("D> %i", type);
	}
	//if (type != net_Tick) logging::Info("Got message: %s", type);
	return ((DispatchUserMessage_t*)hooks::hkClient->GetMethod(hooks::offFrameStageNotify + 1))(thisptr, type, buf);
	SEGV_END; return false;
}

void LevelInit_hook(void* thisptr, const char* newmap) {
	playerlist::Save();
	((LevelInit_t*) hooks::hkClientMode->GetMethod(hooks::offLevelInit))(thisptr, newmap);
	g_IEngine->ClientCmd_Unrestricted("exec cat_matchexec");
	hacks::shared::aimbot::Reset();
	hacks::shared::airstuck::Reset();
//	LEVEL_SHUTDOWN(FollowBot);
	//if (TF) LEVEL_INIT(SpyAlert);
	chat_stack::Reset();
	hacks::shared::spam::Reset();
}

bool CanInspect_hook(IClientEntity*) { return true; }

void LevelShutdown_hook(void* thisptr) {
	((LevelShutdown_t*) hooks::hkClientMode->GetMethod(hooks::offLevelShutdown))(thisptr);
	g_Settings.bInvalid = true;
	hacks::shared::aimbot::Reset();
	hacks::shared::airstuck::Reset();
	chat_stack::Reset();
	hacks::shared::spam::Reset();
}

