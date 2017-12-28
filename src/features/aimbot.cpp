
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
static CatEnum priority_mode_enum({ "SMART", "FOV", "DISTANCE", "HEALTH" });
static CatVarEnum priority_mode(aimbot_menu, priority_mode_enum, "aimbot_prioritymode", 1, "Priority mode", "Priority mode.\nSMART: Basically Auto-Threat.\nFOV, DISTANCE, HEALTH are self-explainable.\nHEALTH picks the weakest enemy");
static CatVarFloat fov(aimbot_menu, "aimbot_fov", 0, "Aimbot FOV", "FOV range for aimbot to lock targets.", 180.0f);
static CatEnum teammates_enum({"ENEMY ONLY", "TEAMMATE ONLY", "BOTH"});
static CatVarEnum teammates(aimbot_menu, teammates_enum, "aimbot_teammates", 0, "Teammates", "Use to choose which team/s to target");
// Other
static CatVarKey aimkey(aimbot_menu, "aimbot_aimkey", CATKEY_E, "Aimkey", "If an aimkey is set, aimbot only works while key is depressed.");
static CatVarBool autoshoot(aimbot_menu, "aimbot_autoshoot", true, "Auto-shoot", "Automaticly shoots when it can");
static CatEnum hitbox_mode_enum({"AUTO", "AUTO-HEAD", "AUTO-CLOSEST", "HEAD", "CENTER"});
CatVarEnum hitbox_mode(aimbot_menu, hitbox_mode_enum, "aimbot_hitbox_mode", 3, "Hitbox Mode", "Hitbox selection mode\n"
																																																		 "AUTO: Automaticly chooses best hitbox\n"
																																																		 "AUTO-HEAD: Head is first priority, but will aim anywhere else if not possible\n"
																																																		 "AUTO-CLOSEST: Aims to the closest hitbox to your crosshair\n"
																																																		 "HEAD: Head only\n"
																																																		 "CENTER: Aims directly in the center of the entity");
static CatVarBool debug(aimbot_menu, "aimbot_debug", true, "debug", "gives debug info about aimbot");


// Hitbox selection


// Somewhere to store the auto-hitbox function
CMFunction<CatVector(const CatEntity&)> GetAutoHitbox {[](auto){ return CatVector(); }};

// A function to find a place to aim for on the target
CatVector RetriveAimpoint(const CatEntity& entity, int mode = hitbox_mode) {

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
			if (!bones::GetBone(entity, i, tmp)) continue;
			// Vis check
			if (!trace::TraceEnt(entity, g_LocalPlayer.GetCamera(), tmp)) continue;
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
			if (!bones::GetBone(entity, i, tmp)) continue;
			// Get FOV
			int fov = util::GetFov(tmp);
			// Check if fov is lower than our current best
			if (fov > closest_fov) continue;
			// Vis check
			if (!trace::TraceEnt(entity, g_LocalPlayer.GetCamera(), tmp)) continue;
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
		if (bones::GetBone(entity, EBone_head, tmp)) return tmp;
		break;
	}
	}

	// Center fallback, uses center of collision box

	// Check if collision box is set
	if (entity.collision != CatBox())
		// We can use the center collision for an aimpoint
		return entity.collision.center();

	// Without anything else, all we can use is the origin
	return entity.origin;
}


// Target Selection

// For modules to add their target selection stuff
std::vector<bool(*)(const CatEntity&)> TargetSelectionModule;

// A second check to determine whether a target is good enough to be aimed at
static std::pair<bool, CatVector> IsTargetGood(const CatEntity& entity) {
	if (CE_BAD(entity)) return std::make_pair(false, CatVector());

	// Local player check
	if (g_LocalPlayer.entity == &entity) return std::make_pair(false, CatVector());
	// Dead
	if (!entity.alive) return std::make_pair(false, CatVector());
	// Teammates
	if (!(teammates == 2 || (teammates == 0) ? entity.Enemy() : !entity.Enemy())) return std::make_pair(false, CatVector());

	// Get our best Aimpoint
	CatVector aimpoint = RetriveAimpoint(entity);

	// Fov check
	if (fov > 0.0f && util::GetFov(aimpoint) > (float)fov) return std::make_pair(false, aimpoint);

	// Vis check
	if (!trace::TraceEnt(entity, g_LocalPlayer.GetCamera(), aimpoint)) return std::make_pair(false, aimpoint);

	// Hey look! Target passed all checks
	return std::make_pair(true, aimpoint);
}
// Function to find a suitable target
static std::pair<const CatEntity*, CatVector> RetrieveBestTarget() {

	// Book keepers for highest target
	const CatEntity* highest_ent = nullptr;
	CatVector highest_aimpoint;
	float highest_score = -1024;

	// Loop through all entitys
	for (const auto& entity : g_CatEntitys) {
		// Ensure ent is okay to use
		if (CE_BAD(entity)) continue;

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
			score = 4096.0f - entity.Distance(); break;
		case 3: // Health Priority
			score = 1024.0f - entity.health;
		}

		// Compare the top score to our current ents score
		if (score > highest_score) {
			highest_score = score;
			highest_ent = &entity;
			highest_aimpoint = tmp.second;
		}
	}

	return std::make_pair(highest_ent, highest_aimpoint);
}
// A check to determine whether the local player should aimbot
static bool ShouldAim() {

	// It would be prefered to have a local ent before we shoot
	if (!g_LocalPlayer.entity) return false;
	// Good check
	if (!g_LocalPlayer.entity->exists || g_LocalPlayer.entity->dormant) return false;
	// Alive check
	if (!g_LocalPlayer.entity->alive) return false;
	// aimkey
	if (aimkey && !input::pressed_buttons[aimkey]) return false;

	return true;
}

// The main "loop" of the aimbot.
static void WorldTick() {
	if (!enabled) return; // Main enabled check

	// Get our best target
	auto target = RetrieveBestTarget();
	if (!target.first) return; // Check whether we found a target

	// Set targets color
	esp::SetEspColor(*target.first, colors::pink);	// Colors are cool

	// Check if our local player is ready to aimbot
	if (!ShouldAim()) return;

	// Autoshoot
	g_LocalPlayer.Attack();

	// Aim at player
	g_LocalPlayer.SetCameraAngle(util::VectorAngles(g_LocalPlayer.GetCamera(), target.second));
}

static void DrawTick() {

}

void Init() {
	wtickmgr_on(WorldTick);
	drawmgr_on(DrawTick);
}

}}
