/*
 * Misc.cpp
 *
 *  Created on: Nov 5, 2016
 *      Author: nullifiedcat
 */

#include "Misc.h"

#include <unistd.h>

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "../beforecheaders.h"
#include <fstream>
#include "../aftercheaders.h"

#include <link.h>
#include "../sharedobj.h"

#include "../hack.h"
#include "../common.h"
#include "../sdk.h"
#include "../hooks/hookedmethods.h"
#include "../netmessage.h"
#include "../copypasted/CSignature.h"

namespace hacks { namespace shared { namespace misc {

//static CatVar remove_conditions(CV_SWITCH, "remove_conditions", "0", "Remove conditions");

static CatVar render_zoomed(CV_SWITCH, "render_zoomed", "0", "Render model when zoomed-in", "Renders player model while being zoomed in as Sniper");

void* C_TFPlayer__ShouldDraw_original = nullptr;

bool C_TFPlayer__ShouldDraw_hook(IClientEntity* thisptr) {
	if (thisptr == g_IEntityList->GetClientEntity(g_IEngine->GetLocalPlayer()) && g_pLocalPlayer->bZoomed && thisptr) {
		//NET_INT(thisptr, netvar.iCond) &= ~(1 << TFCond_Zoomed);
		//bool result = ((bool(*)(IClientEntity*))C_TFPlayer__ShouldDraw_original)(thisptr);
		//NET_INT(thisptr, netvar.iCond) |=  (1 << TFCond_Zoomed);
		return true;
	} else {
		return ((bool(*)(IClientEntity*))C_TFPlayer__ShouldDraw_original)(thisptr);
	}
}

CatVar crit_hack_next(CV_SWITCH, "crit_hack_next", "0", "Next crit info");

void DumpRecvTable(CachedEntity* ent, RecvTable* table, int depth, const char* ft, unsigned acc_offset) {
	bool forcetable = ft && strlen(ft);
	if (!forcetable || !strcmp(ft, table->GetName()))
		logging::Info("==== TABLE: %s", table->GetName());
	for (int i = 0; i < table->GetNumProps(); i++) {
		RecvProp* prop = table->GetProp(i);
		if (!prop) continue;
		if (prop->GetDataTable()) {
			DumpRecvTable(ent, prop->GetDataTable(), depth + 1, ft, acc_offset + prop->GetOffset());
		}
		if (forcetable && strcmp(ft, table->GetName())) continue;
		switch (prop->GetType()) {
		case SendPropType::DPT_Float:
			logging::Info("%s [0x%04x] = %f", prop->GetName(), prop->GetOffset(), CE_FLOAT(ent, acc_offset + prop->GetOffset()));
		break;
		case SendPropType::DPT_Int:
			logging::Info("%s [0x%04x] = %i | %u | %hd | %hu", prop->GetName(), prop->GetOffset(), CE_INT(ent, acc_offset + prop->GetOffset()), CE_VAR(ent, acc_offset +  prop->GetOffset(), unsigned int), CE_VAR(ent, acc_offset + prop->GetOffset(), short), CE_VAR(ent, acc_offset + prop->GetOffset(), unsigned short));
		break;
		case SendPropType::DPT_String:
			logging::Info("%s [0x%04x] = %s", prop->GetName(), prop->GetOffset(), CE_VAR(ent, prop->GetOffset(), char*));
		break;
		case SendPropType::DPT_Vector:
			logging::Info("%s [0x%04x] = (%f, %f, %f)", prop->GetName(), prop->GetOffset(), CE_FLOAT(ent, acc_offset + prop->GetOffset()), CE_FLOAT(ent, acc_offset + prop->GetOffset() + 4), CE_FLOAT(ent, acc_offset + prop->GetOffset() + 8));
		break;
		case SendPropType::DPT_VectorXY:
			logging::Info("%s [0x%04x] = (%f, %f)", prop->GetName(), prop->GetOffset(), CE_FLOAT(ent, acc_offset + prop->GetOffset()), CE_FLOAT(ent,acc_offset +  prop->GetOffset() + 4));
		break;
		}

	}
	if (!ft || !strcmp(ft, table->GetName()))
		logging::Info("==== END OF TABLE: %s", table->GetName());
}

static CatCommand dump_vars("debug_dump_netvars", "Dump netvars of entity", [](const CCommand& args) {
	if (args.ArgC() < 1) return;
	if (!atoi(args[1])) return;
	int idx = atoi(args[1]);
	CachedEntity* ent = ENTITY(idx);
	if (CE_BAD(ent)) return;
	ClientClass* clz = RAW_ENT(ent)->GetClientClass();
	logging::Info("Entity %i: %s", ent->m_IDX, clz->GetName());
	const char* ft = (args.ArgC() > 1 ? args[2] : 0);
	DumpRecvTable(ent, clz->m_pRecvTable, 0, ft, 0);
});

CatVar nopush_enabled(CV_SWITCH, "nopush_enabled", "0", "No Push", "Prevents other players from pushing you around.");

IClientEntity* found_crit_weapon = nullptr;
int found_crit_number = 0;
int last_number = 0;

// SUPER SECRET CODE DONOT STEEL

static CatEnum spycrab_mode_enum({"DISABLED", "FORCE CRAB", "FORCE NON-CRAB"});
static CatVar spycrab_mode(spycrab_mode_enum, "spycrab", "0", "Spycrab", "Defines spycrab taunting mode");

int no_taunt_ticks = 0;

typedef int(*StartSceneEvent_t)(IClientEntity* _this, int, int, void*, void*, IClientEntity*);
StartSceneEvent_t StartSceneEvent_original = nullptr;
int StartSceneEvent_hooked(IClientEntity* _this, int sceneInfo, int choreoScene, void* choreoEvent, void* choreoActor, IClientEntity* unknown) {
	const char* str = (const char*)((unsigned)choreoScene + 396);
	if (_this == g_IEntityList->GetClientEntity(g_IEngine->GetLocalPlayer()) && spycrab_mode && CE_GOOD(LOCAL_W) && LOCAL_W->m_iClassID == CL_CLASS(CTFWeaponPDA_Spy)) {
		if (!strcmp(str, "scenes/player/spy/low/taunt05.vcd")) {
			if ((int)spycrab_mode == 2) {
				RemoveCondition<TFCond_Taunting>(LOCAL_E);
				no_taunt_ticks = 6;
				hacks::shared::lagexploit::AddExploitTicks(15);
			}
		} else if (strstr(str, "scenes/player/spy/low/taunt")) {
			if ((int)spycrab_mode == 1) {
				RemoveCondition<TFCond_Taunting>(LOCAL_E);
				no_taunt_ticks = 6;
				hacks::shared::lagexploit::AddExploitTicks(15);
			}
		}
	}
	return StartSceneEvent_original(_this, sceneInfo, choreoScene, choreoEvent, choreoActor, unknown);
}

float last_bucket = 0;

static CatCommand test_chat_print("debug_print_chat", "machine broke", [](const CCommand& args) {
	CHudBaseChat* chat = (CHudBaseChat*)g_CHUD->FindElement("CHudChat");
	if (chat) {
		std::unique_ptr<char> str(strfmt("\x07%06X[CAT]\x01 %s", 0x4D7942, args.ArgS()));
		chat->Printf(str.get());
	} else {
		logging::Info("Chat is null!");
	}
});


CatVar tauntslide_tf2(CV_SWITCH, "tauntslide_tf2", "0", "Tauntslide", "Allows free movement while taunting with movable taunts\nOnly works in tf2");
CatVar auto_balance_spam(CV_SWITCH, "request_balance_spam", "0", "Inf Auto Balance Spam", "Use to send a autobalance request to the server that doesnt prevent you from using it again\nCredits to Blackfire");
	
// Use to send a autobalance request to the server that doesnt prevent you from using it again
// Allowing infinite use of it.
// Credits to blackfire 
void SendAutoBalanceRequest() {
	if (!g_IEngine->IsInGame()) return;
	KeyValues* kv = new KeyValues("AutoBalanceVolunteerReply");
	kv->SetInt("response", 1);
	g_IEngine->ServerCmdKeyValues(kv);
}
// Catcommand for above
CatCommand SendAutoBlRqCatCom("request_balance", "Request Infinite Auto-Balance", [](const CCommand& args) {
	SendAutoBalanceRequest();
});
	
void CreateMove() {
	static bool flswitch = false;
	static IClientEntity *localplayer, *weapon, *last_weapon = nullptr;
	static int critWarmup = 0;
	static int tries, cmdn, md5seed, rseed, c, b;
	static crithack_saved_state state;
	static bool chc;
	static bool changed = false;
	static ConVar *pNoPush = g_ICvar->FindVar("tf_avoidteammates_pushaway");

	//Tauntslide needs improvement for movement but it mostly works
	IF_GAME (IsTF2()) {
		//Only work if the catvar enables it
		if (tauntslide_tf2) {
			//Check to prevent crashing
			if (CE_GOOD(LOCAL_E)) {
				//If the local player is taunting
				if (HasCondition<TFCond_Taunting>(LOCAL_E)) {
					float forward = 0;
					float side = 0;
					
					//get directions
					if (g_pUserCmd->buttons & IN_FORWARD) forward += 450;
					if (g_pUserCmd->buttons & IN_BACK) forward -= 450;
					if (g_pUserCmd->buttons & IN_MOVELEFT) side -= 450;
					if (g_pUserCmd->buttons & IN_MOVERIGHT) side += 450;
					
					//Push them to userCmd
					g_pUserCmd->forwardmove = forward;
					g_pUserCmd->sidemove = side;
					
					//Grab Camera angle
					static QAngle cameraAngle;
					g_IEngine->GetViewAngles(cameraAngle);
					
					//Set userAngle = camera angles
					g_pUserCmd->viewangles.y = cameraAngle[1];
					g_pLocalPlayer->v_OrigViewangles.y = cameraAngle[1];
					
					//Use silent since we dont want to prevent the player from looking around
					g_pLocalPlayer->bUseSilentAngles = true;
				}
			}
		}
	}

	if (no_taunt_ticks && CE_GOOD(LOCAL_E)) {
		RemoveCondition<TFCond_Taunting>(LOCAL_E);
		no_taunt_ticks--;
	}
	// TODO FIXME this should be moved out of here
	IF_GAME (IsTF2()) {
		PROF_SECTION(CM_misc_hook_checks);
		localplayer = g_IEntityList->GetClientEntity(g_IEngine->GetLocalPlayer());
		if (localplayer && spycrab_mode) {
			void** vtable = *(void***)(localplayer);
			if (vtable[0x111] != StartSceneEvent_hooked) {
				StartSceneEvent_original = (StartSceneEvent_t)vtable[0x111];
				void* page = (void*)((uintptr_t)vtable &~ 0xFFF);
				mprotect(page, 0xFFF, PROT_READ | PROT_WRITE | PROT_EXEC);
				vtable[0x111] = (void*)StartSceneEvent_hooked;
				mprotect(page, 0xFFF, PROT_READ | PROT_EXEC);
			}
		}
		if (render_zoomed && localplayer) {
			// Patchking local player
			void** vtable = *(void***)(localplayer);
			if (vtable[offsets::ShouldDraw()] != C_TFPlayer__ShouldDraw_hook) {
				C_TFPlayer__ShouldDraw_original = vtable[offsets::ShouldDraw()];
				void* page = (void*)((uintptr_t)vtable &~ 0xFFF);
				mprotect(page, 0xFFF, PROT_READ | PROT_WRITE | PROT_EXEC);
				vtable[offsets::ShouldDraw()] = (void*)C_TFPlayer__ShouldDraw_hook;
				mprotect(page, 0xFFF, PROT_READ | PROT_EXEC);
			}
		}
	}

	IF_GAME (IsTF2C()) {
		if (tauntslide) RemoveCondition<TFCond_Taunting>(LOCAL_E);
	}


	if (g_pUserCmd->command_number && found_crit_number > g_pUserCmd->command_number + 66 * 20) found_crit_number = 0;
	if (g_pUserCmd->command_number) last_number = g_pUserCmd->command_number;

	static int last_checked_command_number = 0;
	static IClientEntity* last_checked_weapon = nullptr;

	IF_GAME (IsTF2()) {
		if (crit_hack_next && CE_GOOD(LOCAL_E) && CE_GOOD(LOCAL_W) && WeaponCanCrit() && RandomCrits()) {
			PROF_SECTION(CM_misc_crit_hack_prediction);
			weapon = RAW_ENT(LOCAL_W);
			// IsBaseCombatWeapon
			if (weapon &&
				vfunc<bool(*)(IClientEntity*)>(weapon, 1944 / 4, 0)(weapon)) {
				/*if (experimental_crit_hack.KeyDown()) {
					if (!g_pUserCmd->command_number || critWarmup < 8) {
						if (g_pUserCmd->buttons & IN_ATTACK) {
							critWarmup++;
						} else {
							critWarmup = 0;
						}
						g_pUserCmd->buttons &= ~(IN_ATTACK);
					}
				}*/
				if (g_pUserCmd->command_number && (last_checked_weapon != weapon || last_checked_command_number < g_pUserCmd->command_number)) {
					tries = 0;
					cmdn = g_pUserCmd->command_number;
					chc = false;
					state.Save(weapon);
					while (!chc && tries < 4096) {
						md5seed = MD5_PseudoRandom(cmdn) & 0x7fffffff;
						rseed = md5seed;
						//float bucket = *(float*)((uintptr_t)RAW_ENT(LOCAL_W) + 2612u);
						*g_PredictionRandomSeed = md5seed;
						c = LOCAL_W->m_IDX << 8;
						b = LOCAL_E->m_IDX;
						rseed = rseed ^ (b | c);
						*(float*)(weapon + 2872ul) = 0.0f;
						RandomSeed(rseed);
						chc = vfunc<bool(*)(IClientEntity*)>(weapon, 1836 / 4, 0)(weapon);
						if (!chc) {
							tries++;
							cmdn++;
						}
					}
					last_checked_command_number = cmdn;
					last_checked_weapon = weapon;
					state.Load(weapon);
					last_bucket = state.bucket;
					if (chc) {
						found_crit_weapon = weapon;
						found_crit_number = cmdn;
					}
				}
				if (g_pUserCmd->buttons & (IN_ATTACK)) {
					if (found_crit_weapon == weapon && g_pUserCmd->command_number < found_crit_number) {
						if (g_IInputSystem->IsButtonDown((ButtonCode_t)((int)experimental_crit_hack))) {
							command_number_mod[g_pUserCmd->command_number] = cmdn;
						}
					}
				}
			}
		}
	}

	{
		PROF_SECTION(CM_misc_crit_hack_apply);
		if (!AllowAttacking()) g_pUserCmd->buttons &= ~IN_ATTACK;
	}

	if (WeaponCanCrit()) {
		PROF_SECTION(CM_misc_crit_hack_bucket_fixing);
		weapon = RAW_ENT(LOCAL_W);
		float& bucket = *(float*)((uintptr_t)(weapon) + 2612);
		if (g_pUserCmd->command_number) {
			changed = false;
		}
		if (bucket != last_bucket) {
			if (changed && weapon == last_weapon) {
				bucket = last_bucket;
			} else {
				//logging::Info("db: %.2f", g_pUserCmd->command_number, bucket - last_bucket);
			}
			changed = true;
		}
		last_weapon = weapon;
		last_bucket = bucket;
	}

	if (flashlight_spam) {
		if (flswitch && !g_pUserCmd->impulse) g_pUserCmd->impulse = 100;
		flswitch = !flswitch;
	}
	
	
	// AntiAfk That after a certian time without movement keys depressed, causes random keys to be spammed for 1 second
	if (anti_afk) {
		
		// Time last idle
		static float afk_time_idle = 0;
		
		// If the timer exceeds 1 minute, jump and reset the timer
		if (g_GlobalVars->curtime - 60 > afk_time_idle) {
		
			// Send random commands
			g_pUserCmd->sidemove = RandFloatRange(-450.0, 450.0);
			g_pUserCmd->forwardmove  = RandFloatRange(-450.0, 450.0);
			g_pUserCmd->buttons = rand();
			// Prevent attack command
			g_pUserCmd->buttons &= ~IN_ATTACK; 
			
			// After 1 second we reset the idletime
			if (g_GlobalVars->curtime - 61 > afk_time_idle) {
				logging::Info("Finish anti-idle");
				afk_time_idle = g_GlobalVars->curtime;
			}
		} else {
			// If the player uses a button, reset the timer
			if (g_pUserCmd->buttons & IN_FORWARD || g_pUserCmd->buttons & IN_BACK || g_pUserCmd->buttons & IN_MOVELEFT || g_pUserCmd->buttons & IN_MOVERIGHT || g_pUserCmd->buttons & IN_JUMP || !LOCAL_E->m_bAlivePlayer)
				afk_time_idle = g_GlobalVars->curtime;
		}
	}
	
	IF_GAME (IsTF2()) {
		
		// Spams infinite autobalance spam function 
		if (auto_balance_spam) {

			// Time Last used
			static float auto_balance_time = 0;

			// If the timer exceeds 1 minute, jump and reset the timer
			if (g_GlobalVars->curtime - 0.15 > auto_balance_time) {
				
				// Use the Inf Request func
				SendAutoBalanceRequest();
				
				// Reset timer
				auto_balance_time = g_GlobalVars->curtime;
			}
		}
		
		// Simple No-Push through cvars
    	if (nopush_enabled == pNoPush-> GetBool()) pNoPush->SetValue (!nopush_enabled);
    }
}

#if ENABLE_VISUALS == 1

void DrawText() {
	if (crit_info && CE_GOOD(LOCAL_W)) {
		if (CritKeyDown() || experimental_crit_hack.KeyDown()) {
			AddCenterString("FORCED CRITS!", colors::red);
		}
		IF_GAME (IsTF2()) {
			if (!vfunc<bool(*)(IClientEntity*)>(RAW_ENT(LOCAL_W), 465, 0)(RAW_ENT(LOCAL_W)))
				AddCenterString("Random crits are disabled", colors::yellow);
			else {
				if (!WeaponCanCrit())
					AddCenterString("Weapon can't randomly crit", colors::yellow);
				else
					AddCenterString("Weapon can randomly crit");
			}
			AddCenterString(format("Bucket: ", *(float*)((uintptr_t)RAW_ENT(LOCAL_W) + 2612u)));
			if (crit_hack_next && found_crit_number > last_number && found_crit_weapon == RAW_ENT(LOCAL_W)) {
				AddCenterString(format("Next crit in: ", roundf(((found_crit_number - last_number) / 66.0f) * 10.0f) / 10.0f, 's'));
			}
			//AddCenterString(format("Time: ", *(float*)((uintptr_t)RAW_ENT(LOCAL_W) + 2872u)));
		}
	}


	if (!debug_info) return;
		if (CE_GOOD(g_pLocalPlayer->weapon())) {
			AddSideString(format("Slot: ", vfunc<int(*)(IClientEntity*)>(RAW_ENT(g_pLocalPlayer->weapon()), 395, 0)(RAW_ENT(g_pLocalPlayer->weapon()))));
			AddSideString(format("Taunt Concept: ", CE_INT(LOCAL_E, netvar.m_iTauntConcept)));
			AddSideString(format("Taunt Index: ", CE_INT(LOCAL_E, netvar.m_iTauntIndex)));
			AddSideString(format("Sequence: ", CE_INT(LOCAL_E, netvar.m_nSequence)));
			AddSideString(format("Velocity: ", LOCAL_E->m_vecVelocity.x, ' ', LOCAL_E->m_vecVelocity.y, ' ', LOCAL_E->m_vecVelocity.z));
			AddSideString(format("Velocity3: ", LOCAL_E->m_vecVelocity.Length()));
			AddSideString(format("Velocity2: ", LOCAL_E->m_vecVelocity.Length2D()));
			AddSideString("NetVar Velocity");
			Vector vel = CE_VECTOR(LOCAL_E, netvar.vVelocity);
			AddSideString(format("Velocity: ", vel.x, ' ', vel.y, ' ', vel.z));
			AddSideString(format("Velocity3: ", vel.Length()));
			AddSideString(format("Velocity2: ", vel.Length2D()));
			AddSideString(format("flSimTime: ", LOCAL_E->var<float>(netvar.m_flSimulationTime)));
			if (g_pUserCmd) AddSideString(format("command_number: ", last_cmd_number));
			/*AddSideString(colors::white, "Weapon: %s [%i]", RAW_ENT(g_pLocalPlayer->weapon())->GetClientClass()->GetName(), g_pLocalPlayer->weapon()->m_iClassID);
			//AddSideString(colors::white, "flNextPrimaryAttack: %f", CE_FLOAT(g_pLocalPlayer->weapon(), netvar.flNextPrimaryAttack));
			//AddSideString(colors::white, "nTickBase: %f", (float)(CE_INT(g_pLocalPlayer->entity, netvar.nTickBase)) * gvars->interval_per_tick);
			AddSideString(colors::white, "CanShoot: %i", CanShoot());
			//AddSideString(colors::white, "Damage: %f", CE_FLOAT(g_pLocalPlayer->weapon(), netvar.flChargedDamage));
			if (TF2) AddSideString(colors::white, "DefIndex: %i", CE_INT(g_pLocalPlayer->weapon(), netvar.iItemDefinitionIndex));
			//AddSideString(colors::white, "GlobalVars: 0x%08x", gvars);
			//AddSideString(colors::white, "realtime: %f", gvars->realtime);
			//AddSideString(colors::white, "interval_per_tick: %f", gvars->interval_per_tick);
			//if (TF2) AddSideString(colors::white, "ambassador_can_headshot: %i", (gvars->curtime - CE_FLOAT(g_pLocalPlayer->weapon(), netvar.flLastFireTime)) > 0.95);
			AddSideString(colors::white, "WeaponMode: %i", GetWeaponMode(g_pLocalPlayer->entity));
			AddSideString(colors::white, "ToGround: %f", DistanceToGround(g_pLocalPlayer->v_Origin));
			AddSideString(colors::white, "ServerTime: %f", CE_FLOAT(g_pLocalPlayer->entity, netvar.nTickBase) * g_GlobalVars->interval_per_tick);
			AddSideString(colors::white, "CurTime: %f", g_GlobalVars->curtime);
			AddSideString(colors::white, "FrameCount: %i", g_GlobalVars->framecount);
			float speed, gravity;
			GetProjectileData(g_pLocalPlayer->weapon(), speed, gravity);
			AddSideString(colors::white, "ALT: %i", g_pLocalPlayer->bAttackLastTick);
			AddSideString(colors::white, "Speed: %f", speed);
			AddSideString(colors::white, "Gravity: %f", gravity);
			AddSideString(colors::white, "CIAC: %i", *(bool*)(RAW_ENT(LOCAL_W) + 2380));
			if (TF2) AddSideString(colors::white, "Melee: %i", vfunc<bool(*)(IClientEntity*)>(RAW_ENT(LOCAL_W), 1860 / 4, 0)(RAW_ENT(LOCAL_W)));
			if (TF2) AddSideString(colors::white, "Bucket: %.2f", *(float*)((uintptr_t)RAW_ENT(LOCAL_W) + 2612u));
			//if (TF2C) AddSideString(colors::white, "Seed: %i", *(int*)(sharedobj::client->lmap->l_addr + 0x00D53F68ul));
			//AddSideString(colors::white, "IsZoomed: %i", g_pLocalPlayer->bZoomed);
			//AddSideString(colors::white, "CanHeadshot: %i", CanHeadshot());
			//AddSideString(colors::white, "IsThirdPerson: %i", iinput->CAM_IsThirdPerson());
			//if (TF2C) AddSideString(colors::white, "Crits: %i", s_bCrits);
			//if (TF2C) AddSideString(colors::white, "CritMult: %i", RemapValClampedNC( CE_INT(LOCAL_E, netvar.iCritMult), 0, 255, 1.0, 6 ));
			for (int i = 0; i < HIGHEST_ENTITY; i++) {
				CachedEntity* e = ENTITY(i);
				if (CE_GOOD(e)) {
					if (e->m_Type == EntityType::ENTITY_PROJECTILE) {
						//logging::Info("Entity %i [%s]: V %.2f (X: %.2f, Y: %.2f, Z: %.2f) ACC %.2f (X: %.2f, Y: %.2f, Z: %.2f)", i, RAW_ENT(e)->GetClientClass()->GetName(), e->m_vecVelocity.Length(), e->m_vecVelocity.x, e->m_vecVelocity.y, e->m_vecVelocity.z, e->m_vecAcceleration.Length(), e->m_vecAcceleration.x, e->m_vecAcceleration.y, e->m_vecAcceleration.z);
						AddSideString(colors::white, "Entity %i [%s]: V %.2f (X: %.2f, Y: %.2f, Z: %.2f) ACC %.2f (X: %.2f, Y: %.2f, Z: %.2f)", i, RAW_ENT(e)->GetClientClass()->GetName(), e->m_vecVelocity.Length(), e->m_vecVelocity.x, e->m_vecVelocity.y, e->m_vecVelocity.z, e->m_vecAcceleration.Length(), e->m_vecAcceleration.x, e->m_vecAcceleration.y, e->m_vecAcceleration.z);
					}
				}
			}//AddSideString(draw::white, draw::black, "???: %f", NET_FLOAT(g_pLocalPlayer->entity, netvar.test));
			//AddSideString(draw::white, draw::black, "VecPunchAngle: %f %f %f", pa.x, pa.y, pa.z);
			//draw::DrawString(10, y, draw::white, draw::black, false, "VecPunchAngleVel: %f %f %f", pav.x, pav.y, pav.z);
			//y += 14;
			//AddCenterString(draw::font_handle, input->GetAnalogValue(AnalogCode_t::MOUSE_X), input->GetAnalogValue(AnalogCode_t::MOUSE_Y), draw::white, L"S\u0FD5");*/
		}
}

#endif

void Schema_Reload() {
	logging::Info("Custom schema loading is not supported right now.");
	/*
	static uintptr_t InitSchema_s = gSignatures.GetClientSignature("55 89 E5 57 56 53 83 EC 4C 0F B6 7D 14 C7 04 ? ? ? ? 01 8B 5D 18 8B 75 0C 89 5C 24 04 E8 ? ? ? ? 89 F8 C7 45 C8 00 00 00 00 8D 7D C8 84 C0 8B 45 10 C7 45 CC");
	typedef void(*InitSchema_t)(void*, void*, CUtlBuffer& buffer, bool byte, unsigned version);
	static InitSchema_t InitSchema = (InitSchema_t)InitSchema_s;
	static uintptr_t GetItemSchema_s = gSignatures.GetClientSignature("55 89 E5 83 EC 18 89 5D F8 8B 1D ? ? ? ? 89 7D FC 85 DB 74 12 89 D8 8B 7D FC 8B 5D F8 89 EC 5D C3 8D B6 00 00 00 00 C7 04 24 A8 06 00 00 E8 ? ? ? ? B9 AA 01 00 00 89 C3 31 C0 89 DF");
	typedef void*(*GetItemSchema_t)(void);
	static GetItemSchema_t GetItemSchema = (GetItemSchema_t)GetItemSchema_s;//(*(uintptr_t*)GetItemSchema_s + GetItemSchema_s + 4);

	logging::Info("0x%08x 0x%08x", InitSchema, GetItemSchema);
	void* itemschema = (void*)((unsigned)GetItemSchema() + 4);
	void* data;
	passwd* pwd = getpwuid(getuid());
	char* user = pwd->pw_name;
	char* path = strfmt("/home/%s/.cathook/items_game.txt", user);
	FILE* file = fopen(path, "r");
	delete [] path;
	fseek(file, 0L, SEEK_END);
	char buffer[4 * 1000 * 1000];
	size_t len = ftell(file);
	rewind(file);
	buffer[len + 1] = 0;
	fread(&buffer, sizeof(char), len, file);
	fclose(file);
	CUtlBuffer buf(&buffer, 4 * 1000 * 1000, 9);
	logging::Info("0x%08x 0x%08x", InitSchema, GetItemSchema);
	InitSchema(0, itemschema, buf, false, 0xDEADCA7);
	*/
}

CatVar debug_info(CV_SWITCH, "debug_info", "0", "Debug info", "Shows some debug info in-game");
CatVar flashlight_spam(CV_SWITCH, "flashlight", "0", "Flashlight spam", "HL2DM flashlight spam");
CatVar crit_info(CV_SWITCH, "crit_info", "0", "Show crit info"); // TODO separate
CatVar crit_hack(CV_KEY, "crit_hack", "0", "Crit Key");
CatVar crit_melee(CV_SWITCH, "crit_melee", "0", "Melee crits");
CatVar crit_suppress(CV_SWITCH, "crit_suppress", "0", "Disable random crits", "Can help saving crit bucket for forced crits");
CatVar anti_afk(CV_SWITCH, "anti_afk", "0", "Anti-AFK", "Sends random commands to prevent being kicked from server");
CatVar tauntslide(CV_SWITCH, "tauntslide", "0", "TF2C tauntslide", "Allows moving and shooting while taunting");

CatCommand name("name_set", "Immediate name change", [](const CCommand& args) {
	if (args.ArgC() < 2) {
		logging::Info("Set a name, silly");
		return;
	}
	if (g_Settings.bInvalid) {
		logging::Info("Only works ingame!");
		return;
	}
	std::string new_name(args.ArgS());
	ReplaceString(new_name, "\\n", "\n");
	NET_SetConVar setname("name", new_name.c_str());
	INetChannel* ch = (INetChannel*)g_IEngine->GetNetChannelInfo();
	if (ch) {
		setname.SetNetChannel(ch);
		setname.SetReliable(false);
		ch->SendNetMsg(setname, false);
	}
});
CatCommand say_lines("say_lines", "Say with newlines (\\n)", [](const CCommand& args) {
	std::string message(args.ArgS());
	ReplaceString(message, "\\n", "\n");
	std::string cmd = format("say ", message);
	g_IEngine->ServerCmd(cmd.c_str());
});
CatCommand disconnect("disconnect", "Disconnect with custom reason", [](const CCommand& args) {
	INetChannel* ch = (INetChannel*)g_IEngine->GetNetChannelInfo();
	if (!ch) return;
	ch->Shutdown(args.ArgS());
});
CatCommand schema("schema", "Load custom schema", Schema_Reload);
CatCommand disconnect_vac("disconnect_vac", "Disconnect (fake VAC)", []() {
	INetChannel* ch = (INetChannel*)g_IEngine->GetNetChannelInfo();
	if (!ch) return;
	ch->Shutdown("VAC banned from secure server\n");
});
CatCommand set_value("set", "Set value", [](const CCommand& args) {
	if (args.ArgC() < 2) return;
	ConVar* var = g_ICvar->FindVar(args.Arg(1));
	if (!var) return;
	std::string value(args.Arg(2));
	ReplaceString(value, "\\n", "\n");
	var->SetValue(value.c_str());
	logging::Info("Set '%s' to '%s'", args.Arg(1), value.c_str());
});

}}}

/*void DumpRecvTable(CachedEntity* ent, RecvTable* table, int depth, const char* ft, unsigned acc_offset) {
	bool forcetable = ft && strlen(ft);
	if (!forcetable || !strcmp(ft, table->GetName()))
		logging::Info("==== TABLE: %s", table->GetName());
	for (int i = 0; i < table->GetNumProps(); i++) {
		RecvProp* prop = table->GetProp(i);
		if (!prop) continue;
		if (prop->GetDataTable()) {
			DumpRecvTable(ent, prop->GetDataTable(), depth + 1, ft, acc_offset + prop->GetOffset());
		}
		if (forcetable && strcmp(ft, table->GetName())) continue;
		switch (prop->GetType()) {
		case SendPropType::DPT_Float:
			logging::Info("%s [0x%04x] = %f", prop->GetName(), prop->GetOffset(), CE_FLOAT(ent, acc_offset + prop->GetOffset()));
		break;
		case SendPropType::DPT_Int:
			logging::Info("%s [0x%04x] = %i | %u | %hd | %hu", prop->GetName(), prop->GetOffset(), CE_INT(ent, acc_offset + prop->GetOffset()), CE_VAR(ent, acc_offset +  prop->GetOffset(), unsigned int), CE_VAR(ent, acc_offset + prop->GetOffset(), short), CE_VAR(ent, acc_offset + prop->GetOffset(), unsigned short));
		break;
		case SendPropType::DPT_String:
			logging::Info("%s [0x%04x] = %s", prop->GetName(), prop->GetOffset(), CE_VAR(ent, prop->GetOffset(), char*));
		break;
		case SendPropType::DPT_Vector:
			logging::Info("%s [0x%04x] = (%f, %f, %f)", prop->GetName(), prop->GetOffset(), CE_FLOAT(ent, acc_offset + prop->GetOffset()), CE_FLOAT(ent, acc_offset + prop->GetOffset() + 4), CE_FLOAT(ent, acc_offset + prop->GetOffset() + 8));
		break;
		case SendPropType::DPT_VectorXY:
			logging::Info("%s [0x%04x] = (%f, %f)", prop->GetName(), prop->GetOffset(), CE_FLOAT(ent, acc_offset + prop->GetOffset()), CE_FLOAT(ent,acc_offset +  prop->GetOffset() + 4));
		break;
		}

	}
	if (!ft || !strcmp(ft, table->GetName()))
		logging::Info("==== END OF TABLE: %s", table->GetName());
}

void CC_DumpVars(const CCommand& args) {
	if (args.ArgC() < 1) return;
	if (!atoi(args[1])) return;
	int idx = atoi(args[1]);
	CachedEntity* ent = ENTITY(idx);
	if (CE_BAD(ent)) return;
	ClientClass* clz = RAW_ENT(ent)->GetClientClass();
	logging::Info("Entity %i: %s", ent->m_IDX, clz->GetName());
	const char* ft = (args.ArgC() > 1 ? args[2] : 0);
	DumpRecvTable(ent, clz->m_pRecvTable, 0, ft, 0);
}*/

