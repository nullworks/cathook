/*
 * HBunnyhop.cpp
 *
 *  Created on: Oct 6, 2016
 *      Author: nullifiedcat
 */

#include "Bunnyhop.h"

#include "../common.h"
#include "../sdk.h"

namespace hacks { namespace shared { namespace bunnyhop {

// Var for user settings
CatVar enabled(CV_SWITCH, "bhop_enabled", "0", "Bunnyhop", "Enable Bunnyhop. All extra features like autojump and perfect jump limit were temporary removed.");

int iTicksLastJump = 0;
	
// Function called by game for movement
void CreateMove() {
	// Check user settings if bhop is enabled
	if (!enabled) return;
	if (!g_pUserCmd) return;
	
	// Check if there is usercommands
	if (!g_pUserCmd->command_number) return;

	// Check if local player is airborne, if true then return
	//if (CE_INT(g_pLocalPlayer->entity, netvar.movetype) == MOVETYPE_FLY) return;
	
	// Grab netvar for flags from the local player
	int flags = CE_INT(g_pLocalPlayer->entity, netvar.iFlags);

	// var for "if on ground" from the flags netvar
	bool ground = (flags & (1 << 0));
	// Var for if the player is pressing jump
	bool jump = (g_pUserCmd->buttons & IN_JUMP);

	// Check if player is not on the ground and player is holding their jump key
	if (!ground && jump) {
		// If the ticks since last jump are greater or equal to 7, then force the player to stop jumping
		// The bot disables jump untill player hits the ground or lets go of jump
		if (iTicksLastJump++ >= 9) g_pUserCmd->buttons = g_pUserCmd->buttons &~ IN_JUMP;
	}

	// If the players jump cmd has been used, then we reset our var
	if (!jump) iTicksLastJump = 0;
	
	// Finish the function with return
	return;
}

}}}
