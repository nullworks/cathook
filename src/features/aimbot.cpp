
/*
 *
 *	The cheats main aimbot system.
 *	
 *
 *
 */

#include "../framework/bones.hpp"	// So we can aim at bones
#include "esp.hpp"	// SetEspColor()

#include "aimbot.hpp"

namespace features { namespace aimbot {

CatEnum aimbot_menu({"Aimbot"}); // Menu locator for esp settings
CatVarBool enabled(aimbot_menu, "aimbot", true, "Enable Aimbot", "Main aimbot switch");
// Main preferences
CatEnum priority_mode_enum({ "SMART", "FOV", "DISTANCE", "HEALTH" });
CatVarInt priority_mode(aimbot_menu, priority_mode_enum, "aimbot_prioritymode", 1, "Priority mode", "Priority mode.\nSMART: Basically Auto-Threat.n\FOV, DISTANCE, HEALTH are self-explainable.n\HEALTH picks the weakest enemy");
CatVarFloat fov(aimbot_menu, "aimbot_fov", 0, "Aimbot FOV", "FOV range for aimbot to lock targets.", 180.0f);
CatEnum teammates_enum({"ENEMY ONLY", "TEAMMATE ONLY", "BOTH"});
CatVarInt teammates(teammates_enum, "aimbot_teammates", 0, "Teammates", "Use to choose which team/s to target");
CatEnum hitbox_enum({ // Update this as needed
	"HEAD", "TOP SPINE", "UPPER SPINE", "MIDDLE SPINE", "BOTTOM SPINE", "PELVIS",
	"UPPER ARM L", "UPPER ARM R", "MIDDLE ARM L", "MIDDLE ARM R", "LOWER ARM L", "LOWER ARM R",
	"UPPER LEG L", "UPPER LEG R", "MIDDLE LEG L", "MIDDLE LEG R", "LOWER LEG L", "LOWER LEG R",
});
CatVarInt hitbox(aimbot_menu, hitbox_enum, "aimbot_hitbox", 0, "Hitbox", "Hitbox to use if mode is set to static");
	
// A function to find a place to aim for on the target
CatVector RetriveAimpoint(CatEntity* entity) {
	if (CE_BAD(entity)) return CatVector();
	
	// Check if we can use bones
	if (!entity->bones.empty()) {
		
		// Get our best bone
		static CatVector tmp;
		if (framework::bones::GetCatBoneFromEnt(entity, (int)hitbox, tmp))
			return tmp;
	}
			
	// Check for collision
	if (entity->collision != CatBox())
		// We can use the center collision for an aimpoint
		return entity->collision.center();
			
	// Without anything else, all we can use is the origin
	return entity->origin;
}

// Target Selection
	
// A second check to determine whether a target is good enough to be aimed at
bool IsTargetGood(CatEntity* entity) {
	if (CE_BAD(entity)) return false;
			
	// Local player check
	if (g_LocalPlayer.entity == entity) return false;
	// Dead
	if (!entity->alive) return false;
	// Teammates
	switch((int)teammates) {
	case 0:	// Enemy only
		if (!entity->Enemy()) return false; break;
	case 1:	// Ally only
		if (entity->Enemy()) return false;
	}
	
	// Fov check
	if ((float)fov > 0.0f && util::GetFov(RetriveAimpoint(entity)) > (float)fov) return false;
	
	// Hey look! Target passed all checks
	return true;
}
// Function to find a suitable target
CatEntity* RetrieveBestTarget() {
	
	// Book keepers for highest target
	CatEntity* highest_ent = nullptr;
	float highest_score = -1024;

	// Loop through all entitys
	for (int i = 0; i < entity_cache::HIGHEST_ENTITY; i++) {
		// Get our ent and ensure its okay to use
		CatEntity* entity = entity_cache::Get(i);
		if (CE_BAD(entity)) continue;
		
		// Check whether or not we can target the ent
		if (!IsTargetGood(entity)) continue;

		// Get score based on priority mode
		float score;
		switch ((int)priority_mode) {
		case 0: // SMART Priority
			score = 0; break; // TODO, fix
		case 1: // Fov Priority
			score = 360.0f - util::GetFov(RetriveAimpoint(entity)); break;
		case 2: // Distance priority
			score = 4096.0f - entity->Distance(); break;
		case 3: // Health Priority
			score = 1024.0f - entity->health;
		}

		// Compare the top score to our current ents score
		if (score > highest_score) {
			highest_score = score;
			highest_ent = entity;
		}
	}
	
	return highest_ent;
}
// A check to determine whether the local player should aimbot
bool ShouldAim() {	

	// Good check
	if (CE_BAD(g_LocalPlayer.entity)) return false;
	// Alive check
	if (!g_LocalPlayer.entity->alive) return false;
	
	return true;
}

// Input a vector to aim at it
void AimAt(const CatVector& point) {
	g_LocalPlayer.real_angles = util::VectorAngles(g_LocalPlayer.camera_position, point);
}
// Input an entity to aim at it
void AimAt(CatEntity* entity) {
	if (CE_BAD(entity)) return;
	AimAt(RetriveAimpoint(entity));
}
	
// The main "loop" of the aimbot.
void WorldTick() {
	if (!enabled) return; // Main enabled check
	
	// Get our best target
	CatEntity* target = RetrieveBestTarget();
	if (CE_BAD(target)) return; 			// Check whether we found a target
		
	// Check if our local player is ready to aimbot
	if (!ShouldAim()) return;
			
	// Set targets color
	esp::SetEspColor(target, colors::pink);	// Colors are cool
	
	AimAt(target);
}

}}
	

	
