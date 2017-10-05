 
/*
 *
 *	This contains the entity for the local player and related options.
 *
 */

#include "localplayers.hpp"

CLocalPlayer::Reset() {

	eye_position = CatVector(0, 0, 0); 
	attack = false;
}

// Var for our local player
static CLocalPlayer g_LocalPlayer;