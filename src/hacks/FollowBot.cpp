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
	
	
/* Big Followbot TODO list
1. Fix crash when setting followbot_idx var and remove the fix var in its place
*/

	
// User settings
CatVar bot(CV_SWITCH, "fb_bot", "0", "Master Followbot Switch", "Set to 1 in followbots' configs");
CatVar follow_distance(CV_FLOAT, "fb_distance", "175", "Follow Distance", "How close the bots should stay to the target");
CatVar follow_activation(CV_FLOAT, "fb_activation", "175", "Activation Distance", "How close a player should be until the followbot will pick them as a target");
CatVar mimic_slot(CV_SWITCH, "fb_mimic_slot", "0", "Mimic selected weapon", "If enabled, this bot will select same weapon slot as the owner");
CatVar always_medigun(CV_SWITCH, "fb_always_medigun", "0", "Always use Medigun", "Medics will always use Medigun");
CatVar crumb_draw(CV_SWITCH, "fb_crumb_draw", "1", "Draw Crumbs", "Draws the path made for the followbot");
CatVar roaming(CV_SWITCH, "fb_roaming", "0", "Roaming", "Allows the bot to find a different target if it cant find one using the steam id");
CatVar sync_taunt(CV_SWITCH, "fb_sync_taunt", "0", "Mimic taunts", "Bots will taunt if target is taunting");

// Var to store the current steamid to follow
unsigned follow_steamid { 1 };

// Vars that decides how the followbot will act and move
EFollowType current_follow_type = EFollowType::ENTITY;
CachedEntity* target_last = 0;
int following_idx = 0;	
bool allow_moving = true; 

// Arrays to store selected ents
std::set<int> selection {};
std::set<int> selection_secondary {};

// Vars for vector followbot
float  destination_point_time { 0.0f };
Vector destination_point {};
	
// Var that control jumping and the followbot
float idle_time = 0;
	
// Vars for breadcrumb followbot
	
// An array for storing the breadcrumbs
static Vector breadcrumbs [64];
// Int for storing length of array
constexpr int MAX_CRUMBS = 64;
// Array Bookkeeping vars
int crumbBottom = 0;
int crumbTop = 0;
int crumbArrayLength = 0;
// Used for states for the followbot
float crumb_prune_timeout = 0;
bool crumbStopped = true;
bool crumbFindNew = false;
bool crumbForceMove = false;

// Function called after create move
void AfterCreateMove() {
	
	// Primary Selection handling and selection clearing
	auto it = selection.begin();
	while (it != selection.end()) {
		int idx = *it;
		CachedEntity* entity = ENTITY(idx);
		if (CE_BAD(entity)) {
			selection.erase(it++);
		} else {
#if ENABLE_VISUALS == 1
			hacks::shared::esp::AddEntityString(entity, "[SELECTED]", colors::orange);
			if (fmod(g_GlobalVars->curtime, 2.0f) < 1.0f) {
				hacks::shared::esp::SetEntityColor(entity, colors::yellow);
			}
#endif
			++it;
		}
	}
	
	// Secondary Selection handling
	it = selection_secondary.begin();
	while (it != selection_secondary.end()) {
		int idx = *it;
		CachedEntity* entity = ENTITY(idx);
		if (CE_BAD(entity)) {
			selection_secondary.erase(it++);
		} else {
#if ENABLE_VISUALS == 1
			hacks::shared::esp::AddEntityString(entity, "[SELECTED (SECONDARY)]", colors::orange);
			if (fmod(g_GlobalVars->curtime, 2.0f) < 1.0f) {
				hacks::shared::esp::SetEntityColor(entity, colors::yellow);
			}
#endif
			++it;
		}
	}
	
	// Processing Selection is done, attemt to walk
	DoWalking();
}

// Function for followbots to use for following, weapon selection, etc...
void DoWalking() {
	
	if (!bot) return;
	
	
	
	// Get our best target, preferably from a steamid

	//following_idx = 0;
	CachedEntity* best_target = nullptr;
	CachedEntity* target_priority = nullptr;

	// Get ent from steamid
	for (int i = 1; i < HIGHEST_ENTITY; i++) {

		CachedEntity* ent = ENTITY(i);

		if (CE_BAD(ent)) continue;
		if (ent == LOCAL_E) continue;
		if (ent->m_Type != ENTITY_PLAYER) continue;
		if (!ent->m_bAlivePlayer) continue;
		if (ent->m_bEnemy) continue;
		if (g_pLocalPlayer->v_Origin.DistTo(ent->m_vecOrigin) > 3400.0F) continue;
		
		if (ent->player_info.friendsID == follow_steamid) {
			target_priority = ent;
			break;
		}
	}
	
	// Check Steam id target
	if (CE_GOOD(target_priority)) {
		// If last target isnt the priority, vis check it
		if (target_last != target_priority)  {
			if (target_priority->IsVisible()) {
				target_last = target_priority;
				best_target = target_priority;
				crumbStopped = true;
			}
		} else {
			best_target = target_priority;
		}
	} 
	
	// If we cant use steam id target, try someone else
	if (roaming && CE_BAD(best_target)) {
	
		// Check last target if good
		if (CE_GOOD(target_last)) {
			if (g_pLocalPlayer->v_Origin.DistTo(target_last->m_vecOrigin) < 3400.0F && target_last->m_bAlivePlayer && !crumbStopped) {
				best_target = target_last;	
			}
		}
		
		// Without good target we must find a new one 
		if (CE_BAD(best_target) || g_GlobalVars->curtime - 2.5F > crumb_prune_timeout) {	
			
			float target_highest_score = -256;
			CachedEntity* ent;
			target_last = nullptr;
			crumbStopped = true;
			
			for (int i = 0; i < HIGHEST_ENTITY; i++) {
				ent = ENTITY(i);

				if (CE_BAD(ent)) continue;
				if (ent == LOCAL_E) continue;
				if (!ent->m_bAlivePlayer) continue;
				if (IsBot(ent)) continue;
				
				if (ent->m_bEnemy) continue;
				if (g_pLocalPlayer->v_Origin.DistTo(ent->m_vecOrigin) > 3400.0F) continue;
				// Check activation distance
				if (g_pLocalPlayer->v_Origin.DistTo(ent->m_vecOrigin) > (float)follow_activation) continue;
				if (!ent->IsVisible()) continue;
					
				// Distance Priority works in our case
				float scr = 4096.0f - g_pLocalPlayer->v_Origin.DistTo(ent->m_vecOrigin);
				if (scr > target_highest_score) {
					target_highest_score = scr;
					target_last = ent;
					best_target = ent;
				}
			}
		}
	}

	CachedEntity* found_entity = best_target;
	// TODO, setting following_idx causes a crash for an unknown reason, probs to do with autoheal. 
	// I created a different var to take its place and prevent the crash but i need to fix the crash with the externed var.
	// For now this works and it will stay like this untill I find a way to fix it
	int following_idx2 = 0;
	if (CE_GOOD(found_entity)) {
		following_idx2 = found_entity->m_IDX;
#if ENABLE_VISUALS == 1
		hacks::shared::esp::AddEntityString(found_entity, "[FOLLOWING]", colors::green);
		hacks::shared::esp::SetEntityColor(found_entity, colors::green);
#endif
	} else {
		crumbStopped = true;
		return;
	}
		
			
	
	
	
	// Slot Changer/Mimicer
	
	// Set a static var for last slot check
	static float last_slot_check = 0.0f;
			
	// If curtime is less than the last time we checked the slot, then reset our check timer 
	if (g_GlobalVars->curtime < last_slot_check) last_slot_check = 0.0f;
			
	// If we have a follow target, user settings tell us that we should mess with the slot, the local player is alive, and our target is alive
	if (following_idx2 && (always_medigun || mimic_slot) && (g_GlobalVars->curtime - last_slot_check > 1.0f) && !g_pLocalPlayer->life_state && !CE_BYTE(found_entity, netvar.iLifeState)) {
		
		// We are checking our slot so reset the timer
		last_slot_check = g_GlobalVars->curtime;
		
		// Get the follow targets active weapon
		int owner_weapon_eid = (CE_INT(found_entity, netvar.hActiveWeapon) & 0xFFF);
		IClientEntity* owner_weapon = g_IEntityList->GetClientEntity(owner_weapon_eid);
		
		// If both the follow targets and the local players weapons arnt null or dormant
		if (owner_weapon && CE_GOOD(g_pLocalPlayer->weapon())) {
			
			// IsBaseCombatWeapon()
			if (vfunc<bool(*)(IClientEntity*)>(RAW_ENT(g_pLocalPlayer->weapon()), 190, 0)(RAW_ENT(g_pLocalPlayer->weapon())) &&
			    vfunc<bool(*)(IClientEntity*)>(owner_weapon, 190, 0)(owner_weapon)) {
				
				// Get the players slot numbers and store in some vars
				int my_slot = vfunc<int(*)(IClientEntity*)>(RAW_ENT(g_pLocalPlayer->weapon()), 395, 0)(RAW_ENT(g_pLocalPlayer->weapon()));
				int owner_slot = vfunc<int(*)(IClientEntity*)>(owner_weapon, 395, 0)(owner_weapon);
				
				// If the local player is a medic and user settings allow, then keep the medigun out
				if (g_pLocalPlayer->clazz == tf_medic && always_medigun) {
					if (my_slot != 1) {
						g_IEngine->ExecuteClientCmd("slot2");
					}
					
				// Else we attemt to keep our weapon mimiced with our follow target
				} else {
					if (my_slot != owner_slot) {
						g_IEngine->ExecuteClientCmd(format("slot", owner_slot + 1).c_str());
					}
				}
			}
		}
	}
	
	
	
	
	
	// Main followbot code
	

	// Switch to different types of following mechanisms depending on the type we need to go to
	switch (current_follow_type) {
	case EFollowType::VECTOR: // If were using a vector to follow, we just go directly to it

		// If destination_point_time is more than curtime than we reset it to zero
		if (destination_point_time > g_GlobalVars->curtime) destination_point_time = 0.0f;
			
		// If we havent reached our destination and the destination point timeout isnt more than 5 seconds, then we continue to walk to the destination point 
		if (g_GlobalVars->curtime - destination_point_time < 5.0f) {
			
			// Walk to the point
			followbot::WalkTo(destination_point);
			
			// If we have reached the destination point then we want to disable the vector followbot
			if (g_pLocalPlayer->v_Origin.DistTo(destination_point) < 50.0f) {
				current_follow_type = EFollowType::ENTITY;
			}
		}
		// Break from the switch
		break;

	case EFollowType::ENTITY: // If were using a player to follow, we use the breadcrumb followbot

		if (CE_GOOD(found_entity)) {
			// If the bot is lost but it finds the player again, start the followbot again.
			if (crumbStopped) {
				crumbForceMove = true;
				CrumbReset();
			}
		} else {
		// If the entity we have isnt good, we stop the crumb followbot
			crumbStopped = true;
		} 
			
		// Breadcrumb followbot
		if (!crumbStopped) {
				
			// Generate new breadcrumbs made by the player only if they are close to the ground. If the bot is told to generate a starting point, it does that as well.
			if ((found_entity->m_vecOrigin.DistTo(breadcrumbs[crumbTop]) > 40.0F || crumbFindNew) && DistanceToGround(found_entity) < 40) {
					
				// Add to the crumb.
				CrumbTopAdd(found_entity->m_vecOrigin);

				// If the bot was forced to select a point, we tell it that we no longer need a new one and clear the bottom crumb to use the newest one
				if (crumbFindNew) {
					crumbFindNew = false;
					CrumbBottomAdd();
				}

			}

			// Prune used crumbs from the stack to make way for new ones when you get close to them.
			if (g_pLocalPlayer->v_Origin.DistTo(breadcrumbs[crumbBottom]) < 40.0F ) {
				
				// Debug Logging
				logging::Info("Pruning");

				// When the bot is forced to move to the player, since they have reached their destination we reset the var
				crumbForceMove = false;

				// Check 15 times for close crumbs to prune, this allows simple miss steps to be smoothed out as well as make room for new crumbs
				for (int i = 0; i < 15; i++) {

					// When one is close or too high, just bump the array and reset the stuck timer
					if (g_pLocalPlayer->v_Origin.DistTo(breadcrumbs[crumbBottom]) < 60.0F && crumbArrayLength > 1) {
						CrumbBottomAdd();

					// When pruning is finished. Break the loop
					} else {
						crumb_prune_timeout = g_GlobalVars->curtime;
						logging::Info("Finish Prune");
						break;
					}
				}
				// Reset stuck timer
				crumb_prune_timeout = g_GlobalVars->curtime;
			}

			// When player to follow is too far away. the bot cant see the player or the bot is forced to the player, then follow breadcrumbs if movement is allowed
			if ((g_pLocalPlayer->v_Origin.DistTo(found_entity->m_vecOrigin) > (float)follow_distance || crumbForceMove) && crumbArrayLength >= 1 && allow_moving) {
				followbot::WalkTo(breadcrumbs[crumbBottom]);

				// If a crumb hasnt been pruned in a while, it probably cant travel to it so reset and wait for the player to collect it.
				if (g_GlobalVars->curtime - 2.5F > crumb_prune_timeout) {
					crumbStopped = true;
					logging::Info("Cannot goto next crumb!\nCrumb Lost!");
				}

			// If the bot is next to the player then we clear our crumbs as theres no need to follow previously generated ones.
			} else if (g_pLocalPlayer->v_Origin.DistTo(found_entity->m_vecOrigin) < 100.0F && found_entity->IsVisible()) {
				CrumbReset();
				crumbForceMove = false;
			}
		}
	}

    
			
			
			
	// If we have a follow target, then we attempt to mimic their conditions
	if (following_idx2) {
		
		// If follow target is too far away and the local player is zoomed, then unzoom
		if (found_entity->m_vecOrigin.DistTo(LOCAL_E->m_vecOrigin) > 325.0f) {
			if (g_pLocalPlayer->bZoomed) g_pUserCmd->buttons |= IN_ATTACK2;
		}
		
		// If found target is heavy and the local player is too,
		if (CE_INT(found_entity, netvar.iClass) == tf_heavy && g_pLocalPlayer->clazz == tf_heavy) {
			
			// If found target is spun up, then spin up too
			if (HasCondition<TFCond_Slowed>(found_entity)) {
				g_pUserCmd->buttons |= IN_ATTACK2;
			}
		}
		
		// If found target is zoomed
		if (HasCondition<TFCond_Zoomed>(found_entity)) {
			
			// If the local player isnt zoomed and its class is sniper, then zoom in to mimic the followed target
			if (!g_pLocalPlayer->bZoomed && g_pLocalPlayer->clazz == tf_sniper) {
				g_pUserCmd->buttons |= IN_ATTACK2;
			}
		}
	
		// If user settings allow, we attemt to mimic taunting player
		if (sync_taunt) {
			
			// Check if target is taunting
			if (HasCondition<TFCond_Taunting>(found_entity)) {
				
				// Check if local player isnt taunting
				if (!HasCondition<TFCond_Taunting>(LOCAL_E)) {
					g_IEngine->ExecuteClientCmd("taunt");
				}
			}
		}
		
		//Check if target is crouching
		if (CE_INT(found_entity, netvar.iFlags) & FL_DUCKING) {
				
			// Check if local player isnt taunting
			if (!g_pUserCmd->buttons & IN_DUCK) {
				g_pUserCmd->buttons |= IN_DUCK;
			}
		}
	}
}

// Used on a bot to select a target to follow
CatCommand follow("fb_follow", "Follows you (or player with SteamID specified)", [](const CCommand& args) {
	
	// Set our target steam id to the argument put into the command
	follow_steamid = strtol(args.Arg(1), nullptr, 10);
	
	// Tell the followbot to follow entities
	current_follow_type = EFollowType::ENTITY;
	// Allow the followbot to move
	allow_moving = true;
});
	
// Used by the main player to send an ipc request to the bots to follow
CatCommand follow_me("fb_follow_me", "Makes all bots follow you", []() {
	
	// Check if the ipc server is connected
	if (ipc::peer) {
		// Get our steam id
		unsigned id = g_ISteamUser->GetSteamID().GetAccountID();
		// Send our id through the ipc server
		ipc::peer->SendMessage((const char*)&id, 0, ipc::commands::set_follow_steamid, 0, 0);
	}
});

// User command for cat_fb_tool
CatCommand tool("fb_tool", "Followbot multitool", [](const CCommand& args) {
	
	// Check if ipc server is connected
	if (!ipc::peer) return;
	
	// If argument is "select"
	if (!strcmp(args.Arg(1), "select")) {
		logging::Info("FB TOOL -> SELECT");
		
		// Check if the shift key is depressed
		if (g_IInputSystem->IsButtonDown(ButtonCode_t::KEY_LSHIFT)) {
			
			// Shift cleans selection..
			selection.clear();
			selection_secondary.clear();
			logging::Info("Selection cleared!");
			
		// If the shift key isnt depressed
		} else {
						
			// Make a var and try to get an ent of what we are looking at
			int eindex = 0;
			WhatIAmLookingAt(&eindex, nullptr);
			logging::Info("Selecting entity...");
			
			// If the entity isnt null, Add the entity to the selection
			if (eindex) {
				SelectEntity(eindex);
			}
		}
		
	// If argument is "move"
	} else if (!strcmp(args.Arg(1), "move")) {
		logging::Info("FB TOOL -> MOVE");
		
		// Create a vector and attemt to get an end vector from it
		Vector destination;
		WhatIAmLookingAt(nullptr, &destination);
		
		// Send the vector through the ipc server and tell the bots to move to it
		float array[3] = { destination.x, destination.y, destination.z };
		ipc::peer->SendMessage((const char*)array, MakeMask(), ipc::commands::move_to_vector, nullptr, 0);
		
	// If argument is "stay"
	} else if (!strcmp(args.Arg(1), "stay")) {
		logging::Info("FB TOOL -> STAY");
		
		// Send a message through the ipc server to notify the bots to stop moving
		ipc::peer->SendMessage(nullptr, MakeMask(), ipc::commands::stop_moving, nullptr, 0);
		
	// If argument is "follow"
	} else if (!strcmp(args.Arg(1), "follow")) {
		logging::Info("FB TOOL -> FOLLOW");
		
		// Send a message through the ipc server to notify the bots to start moving
		ipc::peer->SendMessage(nullptr, MakeMask(), ipc::commands::start_moving, nullptr, 0);
	} 
});
	
// Helper Functions for bot selection

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
	
// A Function to tell whether an entity input into it is a bot
bool IsBot(CachedEntity* entity) {
	if (!ipc::peer) return false;
	if (entity->m_Type == ENTITY_PLAYER) {
		if (ipc::peer) {
			for (unsigned i = 0; i < cat_ipc::max_peers; i++) {
				if (!ipc::peer->memory->peer_data[i].free && ipc::peer->memory->peer_user_data[i].friendid == entity->player_info.friendsID) {
					return true;
				}
			}
		}
	}
	return false;
}
	
// :thinking:
unsigned MakeMask() {
	unsigned result = 0;
	if (!ipc::peer) return 0;
	// O(n^2) ik
	for (const auto& idx : selection) {
		CachedEntity* ent = ENTITY(idx);
		if (CE_BAD(ent)) continue;
		for (unsigned i = 0; i < cat_ipc::max_peers; i++) {
			if (!ipc::peer->memory->peer_data[i].free && ipc::peer->memory->peer_user_data[i].friendid == ent->player_info.friendsID) {
				result |= (1 << i);
			}
		}
	}
	return result;
}
			
// Ipc message handlers. When an ipc event comes from an ipc server, its called below 
void AddMessageHandlers(ipc::peer_t* peer) {
	
	// When the ipc server gives us a steam id to follow, we run the following
	peer->SetCommandHandler(ipc::commands::set_follow_steamid, [](cat_ipc::command_s& command, void* payload) {
		// Log in console the steam id that were following
		logging::Info("IPC Message: now following %ld", *(unsigned*)&command.cmd_data);
		// Set our steam id that we want to follow
		hacks::shared::followbot::follow_steamid = *(unsigned*)&command.cmd_data;
		// Tell the followbot to follow entities
		current_follow_type = EFollowType::ENTITY;
	});
	
	// When the ipc server gives us a vector to follow, we run the following
	peer->SetCommandHandler(ipc::commands::move_to_vector, [](cat_ipc::command_s& command, void* payload) {
		// Create a var to recieve the payload with
		float* data = (float*)&command.cmd_data;
		// Log in console the vector that we will attemt to goto
		logging::Info("IPC Message: moving to %.2f %.2f %.2f", data[0], data[1], data[2]);
		// Set our dest info with the payload data
		destination_point = Vector(data[0], data[1], data[2]);
		destination_point_time = g_GlobalVars->curtime;
		// Notify the followbot to follow vectors
		current_follow_type = EFollowType::VECTOR;
	});
	
	// When the ipc server tells us to start moving, we run the following
	peer->SetCommandHandler(ipc::commands::start_moving, [](cat_ipc::command_s& command, void* payload) {
		allow_moving = true;
	});
	
	// When the ipc server tells us to stop moving, we run the following
	peer->SetCommandHandler(ipc::commands::stop_moving, [](cat_ipc::command_s& command, void* payload) {
		allow_moving = false;
	});
}

// Function for when you want to goto a vector
void WalkTo(const Vector& vector) {
	
	// Check if the local player is not moving
	if (CE_VECTOR(LOCAL_E, netvar.vVelocity).IsZero(1.0f)) {
		// Set idle time if we havent already
		if (!idle_time) idle_time = g_GlobalVars->curtime;

		// If the time idle is over 2 seconds
		if (g_GlobalVars->curtime - idle_time > 2.0f) {
			// If the player isnt zoomed, then jump
			if (!g_pLocalPlayer->bZoomed)
				g_pUserCmd->buttons |= IN_JUMP;
			
		// Since the vector is close enough we reset our idle timer
		} else {
			idle_time = 0;
		}
	}
	
	// Calculate how to get to a vector
	auto result = ::ComputeMove(LOCAL_E->m_vecOrigin, vector);
	// Push our move to usercmd
	g_pUserCmd->forwardmove = result.first;
	g_pUserCmd->sidemove = result.second;
}

// I've spent 2 days on writing this method.
// I couldn't succeed.
// Just removed everything and put movement fix code from createmove here.
// Helper function for WalkTo
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
			
	
// Crumb Followbot Helper functions
	
// A function to reset the crumb followbot
void CrumbReset() {
	
	// We just reset the bookkeeping vars for the array, no need to clear the array as everything will be re-written anyways
    crumbTop = 0;
    crumbBottom = 0;
   	crumbArrayLength = 0;
    crumb_prune_timeout = g_GlobalVars->curtime;
    crumbFindNew = true;
    crumbStopped = false;
    logging::Info("Crumb Reset");

}

// A function to place a crumb into the array 
void CrumbTopAdd(Vector crumbToAdd) {
	
    // Once the crumbs have hit the limit of the array, loop around and over write unused spots
    if (crumbTop == MAX_CRUMBS) {
        crumbTop = 0;
    } else { 
        // Else, bump the top number market of the array
        crumbTop++;
    }
    
    // Put the newly determined crumb into the array and add to the length
    crumbArrayLength++;
    breadcrumbs[crumbTop] = crumbToAdd; 
    logging::Info("Crumb Top add");
    
    // The array can only hold so many crumbs, once it goes over its cap, stop the bot to prevent un-needed movement
    if (crumbArrayLength > MAX_CRUMBS) {
		CrumbReset();
        crumbStopped = true;
        logging::Info("Crumb Overload!\nDumping array");
    }
}

// A function to free a crumb from the array
void CrumbBottomAdd() {
	
    // Once the crumbs have hit the limit of the array, loop around and over write unused spots
    if (crumbBottom == MAX_CRUMBS) {
        crumbBottom = 0;
    } else {
        // Else, bump the top number market of the array
        crumbBottom++;
    }
    
    // Subtract from the length to make room for more crumbs 
    crumbArrayLength--;
    logging::Info("Crumb Bottom add");
    
    // A check to detect if too many crumbs have been removed. Without crumbs the bot will just use random variables in the array.
    // To prevent un-nessasary movement, just clear the array and wait for player
    if (crumbArrayLength < 0) {
		CrumbReset();
        crumbStopped = true;
        logging::Info("Crumb Over-Prune!\nDumping array");
    }
}
#if ENABLE_VISUALS == 1
// Function called when we need to draw onto the screen
void Draw() {
	
	// Dont draw if bot isnt true
	if (!bot) return;
	
	// If user settings allow, draw our breadcrumbs
	if (crumb_draw)
		DrawFollowbot();
}
	
// A Function for when we want to draw out the crumbs in the array onto the screen
void DrawFollowbot() {
	
	// Usefull debug info to know
	AddSideString(format("Array Length: ", crumbArrayLength));
	AddSideString(format("Top Crumb: ", crumbTop));
	AddSideString(format("Bottom Crumb: ", crumbBottom));
	AddSideString(format("Crumb Stopped: ", crumbStopped));
	AddSideString(format("Curtime: ", g_GlobalVars->curtime));
	AddSideString(format("Timeout: ", crumb_prune_timeout));
	
	
	// Disabled as the enum was misbehaving for an unknown reason
	
	/*switch (current_follow_type) {
	case EFollowType::VECTOR: // If our follow type is a vector, then we just draw a rect on the vector
			
		// Create a vector for the screen, run world to screen to bring the world vector into the screen vector, then draw a rect at the point
		Vector scn;	
		draw::WorldToScreen(destination_point, scn);
		drawgl::FilledRect(scn.x - 3, scn.y - 3, 6, 6);	
		// Return as we are finished
		break;
			
	case EFollowType::ENTITY: // If our follow type is entity, then we draw out the crumbs here
		logging::Info("Drawcrumb1");
		// Check if we have enough crumbs to draw a line between
		if (crumbArrayLength < 2) {
			
			// If not, we check if we have 1 point and draw on it
			if (crumbArrayLength == 1) {
				
				// Get the entity vector to screen and draw a rect on it
				Vector scn;	
				draw::WorldToScreen(breadcrumbs[crumbBottom], scn);
				drawgl::FilledRect(scn.x - 3, scn.y - 3, 6, 6);	
			}
			
			// Return as we have nothing else to do
			break;
		}

		// Create book keeping vars to draw with
		Vector scnSrt, scnEnd;
		int tmpCrumb1, tmpCrumb2;

		// For loop to draw through the crumbs
		for (int i = 0; i < crumbArrayLength; i++) {

			// We need 2 crumbs to draw a line, so we get the number in the array for 2 crumbs
			tmpCrumb1 = crumbBottom + i;
			tmpCrumb2 = crumbBottom + i + 1;

			// Correction for array numbers when one goes over our limit
			if (tmpCrumb1 >= MAX_CRUMBS)
				tmpCrumb1 - MAX_CRUMBS;
			if (tmpCrumb2 >= MAX_CRUMBS)
				tmpCrumb1 - MAX_CRUMBS;

			// Take our 2 crumbs and get a position on the screen
			draw::WorldToScreen(breadcrumbs[tmpCrumb1], scnSrt);
			draw::WorldToScreen(breadcrumbs[tmpCrumb2], scnEnd);

			// Draw a line from one crumb to the other
			drawgl::Line(scnSrt.x, scnSrt.y, scnEnd.x - scnSrt.x, scnEnd.y - scnSrt.y, colors::white);	

			// If this is our first iteration, draw a box on 1
			if (i == 1)
				drawgl::FilledRect(scnSrt.x - 3, scnSrt.y - 3, 6, 6);	
		}
		break;
	}*/
	
	
	// Not using switch due to switch not working correctly
	if (crumbArrayLength < 2) {
			
		// If not, we check if we have 1 point and draw on it
		if (crumbArrayLength == 1) {
			
			// Get the entity vector to screen and draw a rect on it
			Vector scn;	
			draw::WorldToScreen(breadcrumbs[crumbBottom], scn);
			drawgl::FilledRect(scn.x - 3, scn.y - 3, 6, 6);	
		}
		
		// Return as we have nothing else to do
		return;
	}

	// Create book keeping vars to draw with
	Vector scnSrt, scnEnd;
	int tmpCrumb1, tmpCrumb2;

	// For loop to draw through the crumbs
	for (int i = 0; i < crumbArrayLength; i++) {

		// We need 2 crumbs to draw a line, so we get the number in the array for 2 crumbs
		tmpCrumb1 = crumbBottom + i;
		tmpCrumb2 = crumbBottom + i + 1;

		// Correction for array numbers when one goes over our limit
		if (tmpCrumb1 >= MAX_CRUMBS)
			tmpCrumb1 - MAX_CRUMBS;
		if (tmpCrumb2 >= MAX_CRUMBS)
			tmpCrumb2 - MAX_CRUMBS;

		// Take our 2 crumbs and get a position on the screen
		draw::WorldToScreen(breadcrumbs[tmpCrumb1], scnSrt);
		draw::WorldToScreen(breadcrumbs[tmpCrumb2], scnEnd);

		// Draw a line from one crumb to the other
		drawgl::Line(scnSrt.x, scnSrt.y, scnEnd.x - scnSrt.x, scnEnd.y - scnSrt.y, colors::white);	

		// If this is our first iteration, draw a box on 1
		if (i == 0)
			drawgl::FilledRect(scnSrt.x - 3, scnSrt.y - 3, 6, 6);	
	}
	return;
}
#endif
}}}

#endif
