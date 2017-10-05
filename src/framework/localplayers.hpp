 
/*
 *
 *	Header for local player mgr
 *
 */

#include "entitys.hpp"

class CLocalPlayer {
public:
	Reset();
	CatEntity* CatEntity = nullptr;	// Contains the cat entity of our local player
	
	
	CatVector eye_position = CatVector(0, 0, 0); 	// Point where the users camera is 
	bool attack = false;							// Used to control if attacking should happen
	bool PreventAttacking = false;	// Used to prevent attacking if nessesary
	bool cam_in_thirdperson = false;	// Set to true if your camera is in thirdperson
	bool currently_attacking = false;	// Controls whether aimbot should aim. Good for flicks instead of lock ons
};

extern CLocalPlayer g_LocalPlayer;
