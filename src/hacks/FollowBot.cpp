/*
 * FollowBot.cpp
 *
 *  Created on: Mar 20, 2017
 *      Author: nullifiedcat
 */

#include "FollowBot.h"

#include "../common.h"

#ifdef IPC_ENABLED

namespace hacks { namespace shared { namespace followbot {

unsigned follow_steamid { 0 };
Vector last_direction;
float lost_time { 0 };
float idle_time { 0 };
int following_idx { 0 };

std::set<int> selection {};
std::set<int> selection_secondary {};

//Cats vars for old followbot
float  destination_point_time { 0.0f };
Vector destination_point {};
bool   destination_reached { false };
bool allow_moving { true };

//Initalize vars for BreadCrumb followbot
//An array for storing the breadcrumbs
static CatVar crumbFollowDistance(CV_FLOAT, "fb_follow_distance", "150", "Followbot Distance", "How close the bots should stay to you");
Vector breadcrumbs [55];
float crumbWaitPrune = 0;
float crumbWaitVis = 0;
int crumbBottom = 0;
int crumbTop = 0;
int crumbArrayLength = 0;
bool crumbAbleToMove = false;
bool crumbStopped = true;
bool crumbFindNew = false;
bool crumbForceMove = false;

//A function to start the crumb followbot up
void CrumbStart() {
    //A check to make sure using the fb tool repeatedly doesnt clear the cache of crumbs
    if (crumbStopped || !crumbAbleToMove) {
        crumbTop = 0;
        crumbBottom = 0;
        crumbArrayLength = 0;
        crumbWaitPrune = g_GlobalVars->curtime;
        crumbWaitVis = g_GlobalVars->curtime;
        crumbFindNew = true;
        crumbStopped = false;
        crumbAbleToMove = true;
        logging::Info("Crumb Start");
    }
}

//A function to place a crumb into the array 
void CrumbTopAdd(Vector crumbToAdd) {
    //Once the crumbs have hit the limit of the array, loop around and over write unused spots
    if (crumbTop == 55) {
        crumbTop = 0;
    } else { 
        //Else, bump the top number market of the array
        crumbTop = crumbTop + 1;
    }
    
    //Put the newly determined crumb into the array and add to the length
    crumbArrayLength = crumbArrayLength + 1;
    breadcrumbs[crumbTop] = crumbToAdd; 
    logging::Info("Crumb Top add");
    
    //The array can only hold so many crumbs, once it goes over its cap, stop the bot to prevent un-needed movement
    if (crumbArrayLength > 55) {
        crumbStopped = true;
        logging::Info("Crumb Overload!\nDumping array");
    }
}

//A function to free a crumb from the array
void CrumbBottomAdd() {
    //Once the crumbs have hit the limit of the array, loop around and over write unused spots
    if (crumbBottom == 55) {
        crumbBottom = 0;
    } else {
        //Else, bump the top number market of the array
        crumbBottom = crumbBottom + 1;
    }
    
    //Subtract from the length to make room for more crumbs 
    crumbArrayLength = crumbArrayLength - 1;
    logging::Info("Crumb Bottom add");
    
    //A check to detect if too many crumbs have been removed. Without crumbs the bot will just use random variables in the array.
    //To prevent un-nessasary movement, just clear the array and wait for player
    if (crumbArrayLength < 0) {
        crumbStopped = true;
        logging::Info("Crumb Over-Prune!\nDumping array");
    }
}

//A function to calculate distance from a entity, to the floor
float crumbDistanceToFloor(CachedEntity* ent) {
	if (ent->m_Type == ENTITY_PLAYER) {
		if (CE_INT(ent, netvar.iFlags) & FL_ONGROUND) return 0;
	}
	Vector& origin = ent->m_vecOrigin;
	float v1 = DistanceToGround(origin + Vector(10.0f, 10.0f, 0.0f));
	float v2 = DistanceToGround(origin + Vector(-10.0f, 10.0f, 0.0f));
	float v3 = DistanceToGround(origin + Vector(10.0f, -10.0f, 0.0f));
	float v4 = DistanceToGround(origin + Vector(-10.0f, -10.0f, 0.0f));
	return MIN(v1, MIN(v2, MIN(v3, v4)));
}

bool IsBot(CachedEntity* entity) {
	if (!ipc::peer) return false;
	if (entity->m_Type == ENTITY_PLAYER) {
		if (ipc::peer) {
			for (unsigned i = 1; i < cat_ipc::max_peers; i++) {
				if (!ipc::peer->memory->peer_data[i].free && ipc::peer->memory->peer_user_data[i].friendid == entity->player_info.friendsID) {
					return true;
				}
			}
		}
	}
	return false;
}

void SelectEntity(int idx) {
	logging::Info("Selecting entity %i", idx);
	CachedEntity* entity = ENTITY(idx);
	if (CE_BAD(entity)) return;
	std::set<int>& current_selection = IsBot(entity) ? selection : selection_secondary;
	if (current_selection.find(idx) != current_selection.end()) {
		current_selection.erase(current_selection.find(idx));
		logging::Info("Deselected!");
	} else {
		current_selection.insert(idx);
		logging::Info("Selected!");
	}
}

void AddMessageHandlers(ipc::peer_t* peer) {
	peer->SetCommandHandler(ipc::commands::set_follow_steamid, [](cat_ipc::command_s& command, void* payload) {
		logging::Info("IPC Message: now following %ld", *(unsigned*)&command.cmd_data);
		hacks::shared::followbot::follow_steamid = *(unsigned*)&command.cmd_data;
        //Tell followbot to follow
        crumbForceMove = true;
        CrumbStart();
	});
	peer->SetCommandHandler(ipc::commands::move_to_vector, [](cat_ipc::command_s& command, void* payload) {
		float* data = (float*)&command.cmd_data;
		logging::Info("IPC Message: moving to %.2f %.2f %.2f", data[0], data[1], data[2]);
		destination_point = Vector(data[0], data[1], data[2]);
		destination_point_time = g_GlobalVars->curtime;
		destination_reached = false;
        //prevent followbot from following
        crumbAbleToMove = false;
        crumbStopped = true;
	});
	peer->SetCommandHandler(ipc::commands::start_moving, [](cat_ipc::command_s& command, void* payload) {
		allow_moving = true;
        //Tell followbot to follow
        crumbAbleToMove = true;
	});
	peer->SetCommandHandler(ipc::commands::stop_moving, [](cat_ipc::command_s& command, void* payload) {
		allow_moving = false;
        crumbStopped = true;
	});
}

CatCommand follow_me("fb_follow_me", "Makes all bots follow you", []() {
	if (ipc::peer) {
		unsigned id = g_ISteamUser->GetSteamID().GetAccountID();
		ipc::peer->SendMessage((const char*)&id, 0, ipc::commands::set_follow_steamid, 0, 0);
	}
});

unsigned MakeMask() {
	unsigned result = 0;
	if (!ipc::peer) return 0;
	// O(n^2) ik
	for (const auto& idx : selection) {
		CachedEntity* ent = ENTITY(idx);
		if (CE_BAD(ent)) continue;
		for (unsigned i = 1; i < cat_ipc::max_peers; i++) {
			if (!ipc::peer->memory->peer_data[i].free && ipc::peer->memory->peer_user_data[i].friendid == ent->player_info.friendsID) {
				result |= (1 << i);
			}
		}
	}
	return result;
}

CatCommand tool("fb_tool", "Followbot multitool", [](const CCommand& args) {
	if (!ipc::peer) return;
	if (args.ArgC() == 1) {
		// TODO open a gui or something...
	} else {
		if (!strcmp(args.Arg(1), "select")) {
			logging::Info("FB TOOL -> SELECT");
			if (g_IInputSystem->IsButtonDown(ButtonCode_t::KEY_LSHIFT)) {
				// Shift cleans selection..
				selection.clear();
				selection_secondary.clear();
				logging::Info("Selection cleared!");
			} else {
				logging::Info("Selecting entity...");
				int eindex = 0;
				WhatIAmLookingAt(&eindex, nullptr);
				if (eindex) {
					SelectEntity(eindex);
				}
			}
		} else if (!strcmp(args.Arg(1), "move")) {
			logging::Info("FB TOOL -> MOVE");
			Vector destination;
			WhatIAmLookingAt(nullptr, &destination);
			float array[3] = { destination.x, destination.y, destination.z };
			ipc::peer->SendMessage((const char*)array, MakeMask(), ipc::commands::move_to_vector, nullptr, 0);
		} else if (!strcmp(args.Arg(1), "stay")) {
			logging::Info("FB TOOL -> STAY");
			ipc::peer->SendMessage(nullptr, MakeMask(), ipc::commands::stop_moving, nullptr, 0);
		} else if (!strcmp(args.Arg(1), "follow")) {
			logging::Info("FB TOOL -> FOLLOW");
			ipc::peer->SendMessage(nullptr, MakeMask(), ipc::commands::start_moving, nullptr, 0);
		}
	}
});

CatCommand follow("fb_follow", "Follows you (or player with SteamID specified)", [](const CCommand& args) {
	follow_steamid = strtol(args.Arg(1), nullptr, 10);
});
CatVar bot(CV_SWITCH, "fb_bot", "0", "This player is a bot", "Set to 1 in followbots' configs");
CatVar mimic_slot(CV_SWITCH, "fb_mimic_slot", "1", "Mimic selected weapon", "If enabled, this bot will select same weapon slot as the owner");
CatVar always_medigun(CV_SWITCH, "fb_always_medigun", "1", "Always use Medigun", "Medics will always use Medigun");
//CatVar sync_taunt(CV_SWITCH, "fb_sync_taunt", "1", "Synchronize taunts", "Bots will taunt if owner is taunting");

// I've spent 2 days on writing this method.
// I couldn't succeed.
// Just removed everything and put movement fix code from createmove here.
std::pair<float, float> ComputeMove(const Vector& a, const Vector& b) {
	Vector diff = (b - a);
	if (diff.Length() == 0) return { 0, 0 };
	const float x = diff.x;
	const float y = diff.y;
	Vector vsilent(x, y, 0);
	float speed = sqrt(vsilent.x * vsilent.x + vsilent.y * vsilent.y);
	Vector ang;
	VectorAngles(vsilent, ang);
	float yaw = DEG2RAD(ang.y - g_pUserCmd->viewangles.y);
	return { cos(yaw) * 450, -sin(yaw) * 450 };
}

// I've removed that too early.
void PrintDebug() {
/*
	const Vector& a = LOCAL_E->m_vecOrigin;
	const Vector& b = last_direction;

	Vector diff = (b - a);
	if (diff.Length() == 0) return;
	AddSideString(format("dx: ", diff.x));
	AddSideString(format("dy: ", diff.y));
	//float v_cos = diff.x / diff.Length();
	//float rad = acos(v_cos);
	//if (diff.y < 0) rad = 2 * PI - rad;
	//AddSideString(format("angle: ", rad / PI, " PI"));
	float yan = g_Settings.last_angles.y;
	float yaw = DEG2RAD(yan);
	AddSideString(format("yaw:  ", yaw / PI, " PI"));
	//float rad_diff = yaw - rad;
	//AddSideString(format("diff: ", rad_diff / PI, " PI"));

	auto move = ComputeMove(a, b);
	AddSideString(format("forward: ", move.first));
	AddSideString(format("side:    ", move.second));*/
}

void WalkTo(const Vector& vector) {
	if (CE_VECTOR(LOCAL_E, netvar.vVelocity).IsZero(1.0f)) {
		if (!idle_time) idle_time = g_GlobalVars->curtime;
		if (LOCAL_E->m_vecOrigin.DistTo(vector) > 150.0f) {
			if (g_GlobalVars->curtime - idle_time > 2.0f) {
				if (!g_pLocalPlayer->bZoomed)
					g_pUserCmd->buttons |= IN_JUMP;
			}
		} else {
			idle_time = 0;
		}
	}
	auto result = ComputeMove(LOCAL_E->m_vecOrigin, vector);

	g_pUserCmd->forwardmove = result.first;
	g_pUserCmd->sidemove = result.second;
}

void AfterCreateMove() {
	auto it = selection.begin();
	while (it != selection.end()) {
		int idx = *it;
		CachedEntity* entity = ENTITY(idx);
		if (CE_BAD(entity)) {
			selection.erase(it++);
		} else {
			hacks::shared::esp::AddEntityString(entity, "[SELECTED]", colors::orange);
			if (fmod(g_GlobalVars->curtime, 2.0f) < 1.0f) {
				hacks::shared::esp::SetEntityColor(entity, colors::yellow);
			}
			++it;
		}
	}
	it = selection_secondary.begin();
	while (it != selection_secondary.end()) {
		int idx = *it;
		CachedEntity* entity = ENTITY(idx);
		if (CE_BAD(entity)) {
			selection_secondary.erase(it++);
		} else {
			hacks::shared::esp::AddEntityString(entity, "[SELECTED (SECONDARY)]", colors::orange);
			if (fmod(g_GlobalVars->curtime, 2.0f) < 1.0f) {
				hacks::shared::esp::SetEntityColor(entity, colors::yellow);
			}
			++it;
		}
	}
	DoWalking();
}

void DoWalking() {
	if (!bot) return;
	following_idx = 0;
	for (int i = 1; i < 32 && i < HIGHEST_ENTITY; i++) {
		CachedEntity* ent = ENTITY(i);
		if (CE_BAD(ent)) continue;
		if (ent->player_info.friendsID == follow_steamid) {
			following_idx = i;
			break;
		}
	}
	CachedEntity* found_entity = ENTITY(following_idx);

	static float last_slot_check = 0.0f;
	if (g_GlobalVars->curtime < last_slot_check) last_slot_check = 0.0f;

	if (following_idx && (always_medigun || mimic_slot) && (g_GlobalVars->curtime - last_slot_check > 1.0f) && !g_pLocalPlayer->life_state && !CE_BYTE(found_entity, netvar.iLifeState)) {
		int owner_weapon_eid = (CE_INT(found_entity, netvar.hActiveWeapon) & 0xFFF);
		IClientEntity* owner_weapon = g_IEntityList->GetClientEntity(owner_weapon_eid);
		if (owner_weapon && CE_GOOD(g_pLocalPlayer->weapon())) {
			// IsBaseCombatWeapon()
			if (vfunc<bool(*)(IClientEntity*)>(RAW_ENT(g_pLocalPlayer->weapon()), 190, 0)(RAW_ENT(g_pLocalPlayer->weapon())) &&
			    vfunc<bool(*)(IClientEntity*)>(owner_weapon, 190, 0)(owner_weapon)) {
				int my_slot = vfunc<int(*)(IClientEntity*)>(RAW_ENT(g_pLocalPlayer->weapon()), 395, 0)(RAW_ENT(g_pLocalPlayer->weapon()));
				int owner_slot = vfunc<int(*)(IClientEntity*)>(owner_weapon, 395, 0)(owner_weapon);
				if (g_pLocalPlayer->clazz == tf_medic && always_medigun) {
					if (my_slot != 1) {
						g_IEngine->ExecuteClientCmd("slot2");
					}
				} else {
					if (my_slot != owner_slot) {
						g_IEngine->ExecuteClientCmd(format("slot", owner_slot + 1).c_str());
					}
				}
			}
			// FIXME proper classes
			// FIXME IsBaseCombatWeapon
		}
		last_slot_check = g_GlobalVars->curtime;
	}
	
    //Nullified Cats Followbot code used for followbot tool
    //Keeping this for the followbot move and stay commands
    //TODO remove player follow from this or change it to be a backup for the crumb follow bot
    if (!crumbAbleToMove) {
        if (destination_point_time > g_GlobalVars->curtime) destination_point_time = 0.0f;

        if (!destination_reached && (g_GlobalVars->curtime - destination_point_time < 5.0f)) {
            WalkTo(destination_point);
            last_direction = destination_point;
            if (g_pLocalPlayer->v_Origin.DistTo(destination_point) < 50.0f) destination_reached = true;
        } else if (following_idx) {
            if (allow_moving) {
                if (!found_entity->IsVisible()) {
                    if (!lost_time) {
                        lost_time = g_GlobalVars->curtime;
                    }
                    if (g_GlobalVars->curtime - lost_time < 2.0f) {
                        WalkTo(last_direction);
                    }
                } else {
                    lost_time = 0;
                    if (found_entity->m_vecOrigin.DistTo(LOCAL_E->m_vecOrigin) > 150.0f) {
                        WalkTo(found_entity->m_vecOrigin);
                    }
                    last_direction = found_entity->m_vecOrigin;
                }
            }
        }
    }
    
    //Breadcrumb followbot
    if (allow_moving && crumbAbleToMove && !crumbStopped) {
        //Generate new breadcrumbs made by the player only if they are close to the ground. If the bot is told to generate a starting point, it does that as well.
        if ((found_entity->m_vecOrigin.DistTo(breadcrumbs[crumbTop]) > 40.0F || crumbFindNew) && crumbDistanceToFloor(found_entity) < 25) {
            //Add to the crumb.
            CrumbTopAdd(found_entity->m_vecOrigin);
            
            //If the bot was forced to select a point, add a buffer crumb and bump to the newly selected point and use it
            if (crumbFindNew) {
                crumbFindNew = false;
                CrumbTopAdd(found_entity->m_vecOrigin);
                CrumbBottomAdd();
            }
        }
        
        //Prune used crumbs from the stack to make way for new ones when you get close to them.
        if (g_pLocalPlayer->v_Origin.DistTo(breadcrumbs[crumbBottom]) < 40.0F) {
            //Debug Logging
            logging::Info("Pruning");
            
            //When the bot is forced to move to the player, since they have reached their destination we reset the var
            crumbForceMove = false;
            
            //Check 15 times for close crumbs to prune, this allows simple miss steps to be smoothed out as well as make room for new crumbs
            for (int i = 0; i < 15; i++) {
                //When one is close or too high, just bump the array and reset the stuck timer
                if (g_pLocalPlayer->v_Origin.DistTo(breadcrumbs[crumbBottom]) < 60.0F) {
                    CrumbBottomAdd();
                    
                //When pruning is finished. Break the loop
                } else {
                	crumbWaitPrune = g_GlobalVars->curtime;
                    logging::Info("Finish Prune");
                    break;
                }
            }
            //Reset stuck timer
            crumbWaitPrune = g_GlobalVars->curtime;
        }
        
        //When the player is not visible to the bot, within 5 seconds of it not being visible we stop the bot and wait for the player to collect it
        if (!found_entity->IsVisible()) {
            if (g_GlobalVars->curtime - 5.0F > crumbWaitVis && g_GlobalVars->curtime - 2.5F > crumbWaitPrune) {
                //crumbStopped = true;
                logging::Info("Crumb Lost!");
            }

        //If the player is visible, then reset our lost timer.
        } else crumbWaitVis = g_GlobalVars->curtime;
        

        //When player to follow is too far away. the bot cant see the player or the bot is forced to the player, then follow breadcrumbs
        if ((g_pLocalPlayer->v_Origin.DistTo(found_entity->m_vecOrigin) > (float)crumbFollowDistance || crumbForceMove) && crumbArrayLength >= 1 ) {
            WalkTo(breadcrumbs[crumbBottom]);

            //If a crumb hasnt been pruned in a while, it probably cant travel to it so reset and wait for the player to collect it.
            if (g_GlobalVars->curtime - 2.5F > crumbWaitPrune) {
            	crumbStopped = true;
            	logging::Info("Cannot goto next crumb!\nCrumb Lost!");
            }
            
        //If the bot is next to the player then we clear our crumbs as theres no need to follow previously generated ones.
        } else if (g_pLocalPlayer->v_Origin.DistTo(found_entity->m_vecOrigin) < 100.0F && found_entity->IsVisible()) {
            crumbForceMove = false;
        	crumbStopped = true;
        	CrumbStart();
        }
        
    //If the bot is lost but it finds the player again, start the followbot again.
    } else if (crumbStopped && allow_moving && crumbAbleToMove && found_entity->IsVisible() && (g_pLocalPlayer->v_Origin.DistTo(found_entity->m_vecOrigin) < 3000.0F)) {
        crumbForceMove = true;
        CrumbStart();
        logging::Info("Crumb found player!");
    }
    
	if (following_idx) {
		if (found_entity->m_vecOrigin.DistTo(LOCAL_E->m_vecOrigin) > 150.0f) {
			if (LOCAL_E->m_vecOrigin.DistTo(found_entity->m_vecOrigin) > 350.0f) {
				if (g_pLocalPlayer->bZoomed) g_pUserCmd->buttons |= IN_ATTACK2;
			}
		}
		if (CE_INT(found_entity, netvar.iClass) == tf_heavy && g_pLocalPlayer->clazz == tf_heavy) {
			if (HasCondition(found_entity, TFCond_Slowed)) {
				g_pUserCmd->buttons |= IN_ATTACK2;
			}
		}
		if (HasCondition(found_entity, TFCond_Zoomed)) {
			if (!g_pLocalPlayer->bZoomed && g_pLocalPlayer->clazz == tf_sniper) {
				g_pUserCmd->buttons |= IN_ATTACK2;
			}
		}
	}
}

}}}

#endif
