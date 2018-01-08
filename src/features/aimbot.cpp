
/*
 *
 *	The cheats main aimbot system.
 *
 *
 *
 */

#include "../framework/gameticks.hpp" // To run our stuff
#include "../framework/entitys.hpp"	// Contains entity and bone info
#include "../framework/input.hpp" // to get userinput for aimkeys
#include "../framework/trace.hpp" // so we can vis check
#include "../gui/hudstrings/sidestrings.hpp"
#include "esp.hpp"	// SetEspColor()

#include "aimbot.hpp"

namespace features { namespace aimbot {

static CatEnum aimbot_menu({"Aimbot"}); // Menu locator for esp settings
static CatVarBool enabled(aimbot_menu, "aimbot", true, "Enable Aimbot", "Main aimbot switch");
// Target Selection
static CatVarEnum priority_mode(aimbot_menu, {"SMART", "FOV", "DISTANCE", "HEALTH"}, "aimbot_prioritymode", 1, "Priority mode", "Priority mode.\nSMART: Basically Auto-Threat.\nFOV, DISTANCE, HEALTH are self-explainable.\nHEALTH picks the weakest enemy");
static CatVarFloat fov(aimbot_menu, "aimbot_fov", 2, "Aimbot FOV", "FOV range for aimbot to lock targets.", 180.0f);
static CatVarEnum teammates(aimbot_menu, {"ENEMY ONLY", "TEAMMATE ONLY", "BOTH"}, "aimbot_teammates", 0, "Teammates", "Use to choose which team/s to target");
static CatVarBool target_lock(aimbot_menu, "aimbot_targetlock", false, "Target lock", "Once aimbot finds a target, it will continue to use that target untill that target is no longer valid");
// Aiming
static CatVarKey aimkey(aimbot_menu, "aimbot_aimkey", CATKEY_E, "Aimkey", "If an aimkey is set, aimbot only works while key is depressed.");
static CatVarBool autoshoot(aimbot_menu, "aimbot_autoshoot", true, "Auto-shoot", "Automaticly shoots when it can");
static CatVarEnum hitbox_mode(aimbot_menu, {"AUTO", "AUTO-HEAD", "AUTO-CLOSEST", "HEAD", "CENTER"}, "aimbot_hitbox_mode", 1, "Hitbox Mode", "Hitbox selection mode\n"
																																																																			 		  "AUTO: Automaticly chooses best hitbox\n"
																																																																					  "AUTO-HEAD: Head is first priority, but will aim anywhere else if not possible\n"
																																																																					  "AUTO-CLOSEST: Aims to the closest hitbox to your crosshair\n"
																																																																					  "HEAD: Head only\n"
																																																																					  "CENTER: Aims directly in the center of the entity");
static CatVarEnum silent_aim(aimbot_menu, {"OFF", "SNAPBACK", "MODULE"}, "aimbot_silent", 0, "Silent aimbot", "SNAPBACK: Snaps the aimbot back after aiming\n"
																																																							"MODULE: Uses the modules own version of silent, if any");
static CatVarBool debug(aimbot_menu, "aimbot_debug", true, "debug", "gives debug info about aimbot");


// Hitbox selection


// Somewhere to store the auto-hitbox function
CMFunction<CatVector(CatEntity*)> GetAutoHitbox {[](auto){ return CatVector(); }};

// A function to find a place to aim for on the target
CatVector RetriveAimpoint(CatEntity* entity, int mode = hitbox_mode) {

	// Check if we can use bones
	// Get our best bone
	switch(mode) {
	case 0: { // AUTO
		return GetAutoHitbox(entity);
	}
	case 1: { // AUTO-HEAD
		// Head is first bone, should be fine to iterate through them
		for (int i = 0; i < EBone_count; i++) {
			// Get our bone
			CatVector tmp;
			if (!entity->GetBone(i, tmp)) continue;
			// Vis check
			if (auto local_ent = GetLocalPlayer())
				if (!trace::TraceEnt(entity, local_ent->GetCamera(), tmp))
					continue;
			return tmp;
		}
		break;
	}
	case 2: { // AUTO-CLOSEST
		// Book-keepers for the best one we have found
		CatVector closest;
		float closest_fov = 360;
		for (int i = 0; i < EBone_count; i++) {
			// Get our bone
			CatVector tmp;
			if (!entity->GetBone(i, tmp)) continue;
			// Get FOV
			int fov = util::GetFov(tmp);
			// Check if fov is lower than our current best
			if (fov > closest_fov) continue;
			// Vis check
			if (auto* local_ent = GetLocalPlayer())
				if (!trace::TraceEnt(entity, local_ent->GetCamera(), tmp))
					continue;
			// Set the new current best
			closest = tmp;
			closest_fov = fov;
		}
		// Check if we have anything set, then return if true
		if (closest != CatVector())
			return closest;
		break;
	}
	case 3: { // HEAD
		CatVector tmp;
		if (entity->GetBone(EBone_head, tmp)) return tmp;
		break;
	}
	}

	// Center fallback, uses center of collision box

	// Check if collision box is set
	if (entity->GetCollision() != CatBox())
		// We can use the center collision for an aimpoint
		return entity->GetCollision().center();

	// Without anything else, all we can use is the origin
	return entity->GetOrigin();
}

// Target Selection

// For modules to add their target selection stuff
std::vector<bool(*)(CatEntity*)> TargetSelectionModule;

// A second check to determine whether a target is good enough to be aimed at
static std::pair<bool, CatVector> IsTargetGood(CatEntity* entity) {
	if (entity->GetDormant()) return std::make_pair(false, CatVector());

	// Local player check
	if (GetLocalPlayer() == entity) return std::make_pair(false, CatVector());
	// Dead
	if (!entity->GetAlive()) return std::make_pair(false, CatVector());
	// Teammates
	//if (!(teammates == 2 || (teammates == 0) ? entity.Enemy() : !entity.Enemy())) return std::make_pair(false, CatVector());

	// Do the custom stuff
	for (auto tmp : TargetSelectionModule)
		if (!tmp(entity)) return std::make_pair(false, CatVector());

	// Get our best Aimpoint
	CatVector aimpoint = RetriveAimpoint(entity);

	// Fov check
	if (fov > 0.0f && util::GetFov(aimpoint) > (float)fov) return std::make_pair(false, aimpoint);

	// Vis check
	if (auto  local_ent = GetLocalPlayer())
		if (!trace::TraceEnt(entity, local_ent->GetCamera(), aimpoint))
			return std::make_pair(false, aimpoint);

	// Hey look! Target passed all checks
	return std::make_pair(true, aimpoint);
}

CatEntity* last_target = nullptr;
// Function to find a suitable target
static std::pair<CatEntity*, CatVector> RetrieveBestTarget() {

	// If target lock is enabled, check last entity.
	if (target_lock && last_target) {
		auto tmp = IsTargetGood(last_target);
		if (tmp.first)
			return tmp;
		last_target = nullptr;
	}

	// Book keepers for highest target
	CatEntity* highest_ent = nullptr;
	CatVector highest_aimpoint;
	float highest_score = -1024;

	// Loop through all entitys
	for (int i = 0; i < GetEntityCount(); i++) {
		CatEntity* entity = GetEntity(i);
		if (!entity) continue;

		// Check whether or not we can target the ent
		auto tmp = IsTargetGood(entity);
		if (!tmp.first) continue;

		// Get score based on priority mode
		float score = 0;
		switch (priority_mode) {
		case 0: // SMART Priority
			//score = 0; break; // TODO, fix
		case 1: // Fov Priority
			score = 360.0f - util::GetFov(tmp.second); break;
		case 2: // Distance priority
			//score = 4096.0f - entity.Distance(); break;
		case 3: // Health Priority
			score = 1024.0f - entity->GetHealth();
		}

		// Compare the top score to our current ents score
		if (score > highest_score) {
			highest_score = score;
			highest_ent = entity;
			highest_aimpoint = tmp.second;
		}
	}

	return std::make_pair(highest_ent, highest_aimpoint);
}
// A check to determine whether the local player should aimbot
static bool ShouldAim() {

	// It would be prefered to have a local ent before we shoot
	auto local_ent = GetLocalPlayer();
	if (!local_ent || local_ent->GetDormant()) return false;
	// Alive check
	if (!local_ent->GetAlive()) return false;
	// aimkey
	if (aimkey && !input::pressed_buttons[aimkey]) return false;

	return true;
}

// The main "loop" of the aimbot.
static void WorldTick() {
	// Main enabled check
	if (!enabled) {
		last_target = nullptr;
		return;
	}

	// Get our best target
	auto target = RetrieveBestTarget();
	if (!target.first) { // Check whether we found a target
		last_target = nullptr;
		return;
	}

	// Check if our local player is ready to aimbot
	if (!ShouldAim()) {
		last_target = nullptr;
		return;
	}

	// Set our last target
	last_target = target.first;

	// Get local ent for use below
	auto local_ent = GetLocalPlayer();

	// Autoshoot
	if (autoshoot) local_ent->Attack();

	// Aim at player
	switch(silent_aim){
	case 0: // OFF
	local_ent->SetCameraAngle(util::VectorAngles(local_ent->GetCamera(), target.second)); break;
	case 1: // SNAPBACK

	case 2: // MODULE
	}
}

static void DrawTick() {

}

void Init() {
	wtickmgr_on(WorldTick);
	drawmgr_on(DrawTick);
}

}}
