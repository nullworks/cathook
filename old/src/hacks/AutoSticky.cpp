/*
 * AutoSticky.cpp
 *
 *  Created on: Dec 2, 2016
 *      Author: nullifiedcat
 */

#include "AutoSticky.h"

#include "../common.h"
#include "../sdk.h"

namespace hacks { namespace tf { namespace autosticky {

// Vars for user settings
CatVar enabled(CV_SWITCH, "sticky_enabled", "0", "AutoSticky", "Master AutoSticky switch");
CatVar buildings(CV_SWITCH, "sticky_buildings", "1", "Detonate buildings", "Stickies react to buildings");
CatVar legit(CV_SWITCH, "sticky_legit", "0", "Legit", "Stickys only detonate when you see them\nAlso ignores invis spies");

// A storage array for ents
std::vector<CachedEntity*> bombs;
std::vector<CachedEntity*> targets;
	
// Function to tell when an ent is the local players own bomb
bool IsBomb(CachedEntity* ent) {
	// Check if ent is a stickybomb
	if (ent->m_iClassID != CL_CLASS(CTFGrenadePipebombProjectile)) return false;
	if (CE_INT(ent, netvar.iPipeType) != 1) return false;
	
	// Check if the stickybomb is the players own
	if ((CE_INT(ent, netvar.hThrower) & 0xFFF) != g_pLocalPlayer->entity->m_IDX) return false;
	
	// Check passed, return true
	return true;
}

// Function to check ent if it is a good target
bool IsTarget(CachedEntity* ent) {
	// Check if target is The local player
	if (ent == LOCAL_E) return false;

	// Check if target is an enemy
	if (!ent->m_bEnemy) return false;
	
	// Player specific
	if (ent->m_Type == ENTITY_PLAYER) {
		// Dont detonate on dead players
		if (!ent->m_bAlivePlayer) return false;
		// Dont detonate on friendly players
		if (playerlist::IsFriendly(playerlist::AccessData(ent).state)) return false;
		
		IF_GAME (IsTF()) {
			// Dont target invulnerable players, ex: uber, bonk
			if (IsPlayerInvulnerable(ent)) return false;
			
			// If settings allow, ignore taunting players
			//if (ignore_taunting && HasCondition<TFCond_Taunting>(ent)) return false;
			
			// If settings allow, dont target cloaked players
			if (legit && IsPlayerInvisible(ent)) return false;
		}
		
		// Target is good
		return true;
		
	// Building specific
	} else if (ent->m_Type == ENTITY_BUILDING) {
		return buildings;
	}
	
	// Target isnt a good type
	return false;
}
	
// Function called by game for movement
void CreateMove() {
	// Check user settings if auto-sticky is enabled
	if (!enabled) return;
	
	// Check if game is a tf game
	//IF_GAME (!IsTF()) return;
	
	// Check if player is demoman
	if (g_pLocalPlayer->clazz != tf_demoman) return;
	
	// Check for sticky jumper, which is item 265, if true, return
	if (HasWeapon(LOCAL_E, 265)) return;
		
	// Clear the arrays
	bombs.clear();
	targets.clear();
		
	// Cycle through the ents and search for valid ents
	for (int i = 0; i < HIGHEST_ENTITY; i++) {
		// Assign the for loops tick number to an ent
		CachedEntity* ent = ENTITY(i);
		// Check for dormancy and if valid
		if (CE_BAD(ent)) continue;
		// Check if ent is a bomb or suitable target and push to respective arrays
		if (IsBomb(ent)) {
			bombs.push_back(ent);
		} else if (IsTarget(ent)) {
			targets.push_back(ent);
		}
	}
		
	// Loop once for every bomb in the array
	for (auto bomb : bombs) {
		// Loop through every target for a given bomb
		for (auto target : targets) {
			// Check distance to the target to see if the sticky will hit
			if (bomb->m_vecOrigin.DistToSqr(target->m_vecOrigin) < 16900) {
				// Vis check the target from the bomb
            	if (VisCheckEntFromEnt(bomb, target)) {
					// Check user settings if legit mode is off, if legit mode is off then detonate 
                    if (!legit) {					
						// Check for scottish, id 130, if true then aim at bomb
						if (HasWeapon(LOCAL_E, 130)) {
							// Aim at bomb
							AimAt(g_pLocalPlayer->v_Eye, bomb->m_vecOrigin, g_pUserCmd);
							// Use silent
							g_pLocalPlayer->bUseSilentAngles = true;
						}
						
						// Detonate
                    	g_pUserCmd->buttons |= IN_ATTACK2;
						
						// Return as its a waste to check anymore, we detonated and all the rest of the stickys are gone
						return;
						
					// Since legit mode is on, check if the sticky can see the local player
					} else if (VisCheckEntFromEnt(bomb, LOCAL_E)) {
						// Check for scottish, id 130, if true then aim at bomb
						if (HasWeapon(LOCAL_E, 130)) {
							// Aim at bomb
							AimAt(g_pLocalPlayer->v_Eye, bomb->m_vecOrigin, g_pUserCmd);
							// Use silent
							g_pLocalPlayer->bUseSilentAngles = true;
						}
						
						// Detonate
                    	g_pUserCmd->buttons |= IN_ATTACK2;
						
						// Return as its a waste to check anymore, we detonated and all the rest of the stickys are gone
						return;
					}
                }
			}
		}
	}
	// End of function, just return
	return;
}

}}}

