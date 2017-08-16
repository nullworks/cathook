/*
 * HAimbot.cpp
 *
 *  Created on: Oct 9, 2016
 *      Author: nullifiedcat
 */

#include "../common.h"

namespace hacks { namespace shared { namespace aimbot {
	
// User settings are stored and used by these vars
static CatVar enabled(CV_SWITCH, "aimbot_enabled", "0", "Enable Aimbot", "Main aimbot switch");
static CatVar aimkey(CV_KEY, "aimbot_aimkey", "0", "Aimkey", "Aimkey. Look at Aimkey Mode too!");
static CatEnum aimkey_modes_enum({ "DISABLED", "AIMKEY", "REVERSE", "TOGGLE" });
static CatVar aimkey_mode(aimkey_modes_enum, "aimbot_aimkey_mode", "1", "Aimkey mode", "DISABLED: aimbot is always active\nAIMKEY: aimbot is active when key is down\nREVERSE: aimbot is disabled when key is down\nTOGGLE: pressing key toggles aimbot");
static CatVar autoshoot(CV_SWITCH, "aimbot_autoshoot", "1", "Autoshoot", "Shoot automatically when the target is locked, isn't compatible with 'Enable when attacking'");
static CatEnum hitbox_mode_enum({ "AUTO-HEAD", "AUTO-CLOSEST", "STATIC" });
static CatVar hitbox_mode(hitbox_mode_enum, "aimbot_hitboxmode", "0", "Hitbox Mode", "Defines hitbox selection mode");
static CatVar fov(CV_FLOAT, "aimbot_fov", "0", "Aimbot FOV", "FOV range for aimbot to lock targets. \"Smart FOV\" coming eventually.", 180.0f);
static CatEnum priority_mode_enum({ "SMART", "FOV", "DISTANCE", "HEALTH" });
static CatVar priority_mode(priority_mode_enum, "aimbot_prioritymode", "0", "Priority mode", "Priority mode.\n"
		"SMART: Basically Auto-Threat. Will be tweakable eventually. "
		"FOV, DISTANCE, HEALTH are self-explainable. HEALTH picks the weakest enemy");
static CatVar wait_for_charge(CV_SWITCH, "aimbot_charge", "0", "Wait for sniper rifle charge", "Aimbot waits until it has enough charge to kill");
static CatVar ignore_vaccinator(CV_SWITCH, "aimbot_ignore_vaccinator", "1", "Ignore Vaccinator", "Hitscan weapons won't fire if enemy is vaccinated against bullets");
static CatVar ignore_hoovy(CV_SWITCH, "aimbot_ignore_hoovy", "0", "Ignore Hoovies", "Aimbot won't attack hoovies");
static CatVar ignore_cloak(CV_SWITCH, "aimbot_ignore_cloak", "1", "Ignore cloaked", "Don't aim at invisible enemies");
static CatVar ignore_deadringer(CV_SWITCH, "aimbot_ignore_deadringer", "1", "Ignore deadringer", "Don't aim at deadringed enemies");
static CatVar buildings_sentry(CV_SWITCH, "aimbot_buildings_sentry", "1", "Aim Sentry", "Should aimbot aim at sentryguns?");
static CatVar buildings_other(CV_SWITCH, "aimbot_buildings_other", "1", "Aim Other building", "Should aimbot aim at other buildings");
static CatVar stickybot(CV_SWITCH, "aimbot_stickys", "0", "Aim Sticky", "Should aimbot aim at stickys");
static CatEnum teammates_enum({ "ENEMY ONLY", "TEAMMATE ONLY", "BOTH" });
static CatVar teammates(teammates_enum, "aimbot_teammates", "0", "Aim at teammates", "Use to choose which team/s to target");
static CatVar silent(CV_SWITCH, "aimbot_silent", "1", "Silent", "Your screen doesn't get snapped to the point where aimbot aims at");
static CatVar target_lock(CV_SWITCH, "aimbot_target_lock", "0", "Target Lock", "Keeps your previously chosen target untill target check fails");
static CatEnum hitbox_enum({
		"HEAD", "PELVIS", "SPINE 0", "SPINE 1", "SPINE 2", "SPINE 3", "UPPER ARM L", "LOWER ARM L",
		"HAND L", "UPPER ARM R", "LOWER ARM R", "HAND R", "HIP L", "KNEE L", "FOOT L", "HIP R",
		"KNEE R", "FOOT R" });
static CatVar hitbox(hitbox_enum, "aimbot_hitbox", "0", "Hitbox", "Hitbox to aim at. Ignored if AutoHitbox is on");
static CatVar zoomed_only(CV_SWITCH, "aimbot_zoomed", "1", "Zoomed only", "Don't autoshoot with unzoomed rifles");
static CatVar only_can_shoot(CV_SWITCH, "aimbot_only_when_can_shoot", "1", "Active when can shoot", "Aimbot only activates when you can instantly shoot, sometimes making the autoshoot invisible for spectators");
static CatVar attack_only(CV_SWITCH, "aimbot_enable_attack_only", "0", "Active when attacking", "Basically makes Mouse1 an AimKey, isn't compatible with AutoShoot");
static CatVar max_range(CV_INT, "aimbot_maxrange", "0", "Max distance",
		"Max range for aimbot\n"
		"900-1100 range is efficient for scout/widowmaker engineer", 4096.0f);
static CatVar extrapolate(CV_SWITCH, "aimbot_extrapolate", "0", "Latency extrapolation", "(NOT RECOMMENDED) latency extrapolation");
static CatVar slowaim(CV_SWITCH, "aimbot_slow", "0", "Slow Aim", "Slowly moves your crosshair onto the target for more legit play\nDisables silent aimbot");
static CatVar slowaim_smoothing(CV_INT, "aimbot_slow_smooth", "10", "Slow Aim Smooth", "How slow the slow aim's aiming should be", 50);
static CatVar slowaim_autoshoot(CV_INT, "aimbot_slow_autoshoot", "10", "Slow Aim Threshhold", "Distance to autoshoot while smooth aiming", 25);
static CatVar projectile_aimbot(CV_SWITCH, "aimbot_projectile", "1", "Projectile aimbot", "If you turn it off, aimbot won't try to aim with projectile weapons");
static CatVar proj_fov(CV_SWITCH, "aimbot_proj_fovpred", "0", "Projectile FOV mode", "If disabled, FOV restrictions apply to current target position");
static CatVar proj_visibility(CV_SWITCH, "aimbot_proj_vispred", "0", "Projectile visibility prediction", "If enabled, projectile aimbot will perform additional visibility checking and won't try to predict enemies behind walls");
static CatVar proj_gravity(CV_FLOAT, "aimbot_proj_gravity", "0", "Projectile gravity",
		"Force override projectile gravity. Useful for debugging.", 1.0f);
static CatVar proj_speed(CV_FLOAT, "aimbot_proj_speed", "0", "Projectile speed",
		"Force override projectile speed.\n"
		"Can be useful for playing with MvM upgrades or on x10 servers "
		"since there is no \"automatic\" projectile speed detection in "
		"cathook. Yet.");
static CatVar huntsman_autoshoot(CV_FLOAT, "aimbot_huntsman_charge", "0.5", "Huntsman autoshoot", "Minimum charge for autoshooting with huntsman.\n"
		"Set it to 0.01 if you want to shoot as soon as you start pulling the arrow", 0.01f, 1.0f);
static CatVar huntsman_full_auto(CV_SWITCH, "aimbot_full_auto_huntsman", "1", "Auto Huntsman", "Autoshoot will pull huntsman's string");
// Debug vars
static CatVar aimbot_debug(CV_SWITCH, "aimbot_debug", "0", "Aimbot Debug", "Display simple debug info for aimbot");
static CatVar engine_projpred(CV_SWITCH, "debug_aimbot_engine_pp", "0", "Engine ProjPred");
// Followbot vars
static CatVar auto_spin_up(CV_SWITCH, "aimbot_spin_up", "0", "Auto Spin Up", "Spin up minigun if you can see target, useful for followbots");
static CatVar auto_zoom(CV_SWITCH, "aimbot_auto_zoom", "0", "Auto Zoom", "Automatically zoom in if you can see target, useful for followbots");

static CatVar fovcircle_opacity(CV_FLOAT, "aimbot_fov_draw_opacity", "0.7", "FOV Circle Opacity", "Defines opacity of FOV circle", 0.0f, 1.0f);
static CatVar rageonly(CV_SWITCH, "aimbot_rage_only", "0", "Ignore non-rage targets", "Use playerlist to set up rage targets");

static CatVar miss_chance(CV_FLOAT, "aimbot_miss_chance", "0", "Miss chance", "From 0 to 1. Aimbot will NOT aim in these % cases", 0.0f, 1.0f);
static CatVar auto_unzoom(CV_SWITCH, "aimbot_auto_unzoom", "0", "Auto Un-zoom", "Automatically unzoom");

// Current Entity
int target_eid { 0 };
CachedEntity* target = 0;
CachedEntity* target_last = 0;
bool foundTarget = false;
// Projectile info
bool projectile_mode { false };
float cur_proj_speed { 0.0f };
float cur_proj_grav { 0.0f };
// If slow aimbot allows autoshoot
bool slowCanShoot = false;
bool silent_huntsman { false };

// This array will store calculated projectile/hitscan predictions
// for current frame, to avoid performing them again
AimbotCalculatedData_s calculated_data_array[2048] {};

// The main "loop" of the aimbot.
void CreateMove() {
	
	// Check if aimbot is enabled
	if (!enabled) return;
	
	if (auto_unzoom) {
		if (g_pLocalPlayer->holding_sniper_rifle) {
			if (g_pLocalPlayer->bZoomed) {
				if (g_GlobalVars->curtime - g_pLocalPlayer->flZoomBegin > 5.0f)
					g_pUserCmd->buttons |= IN_ATTACK2;
			}
		}
	}

	// Refresh projectile info
	int huntsman_ticks = 0;
	projectile_mode = (GetProjectileData(g_pLocalPlayer->weapon(), cur_proj_speed, cur_proj_grav));
	if (proj_speed)
		cur_proj_speed = (float)proj_speed;
	if (proj_gravity)
		cur_proj_grav = (float)proj_gravity;
	
	// Attemt to reduce huntsman_ticks by 1 untill it reaches 0
	if (huntsman_ticks) {
		// Disable attack
		g_pUserCmd->buttons |= IN_ATTACK;
		// Returns 0 - Something higher than 0
		huntsman_ticks = max(0, huntsman_ticks - 1);
	}
	
	// Save should aim info
	// We do this as we need to pass whether the aimkey allows aiming to both the find target and aiming system. If we just call the func than toggle aimkey would break so we save it to a var to use it twice
	bool aimkey_status = UpdateAimkey();
	
	// Refresh our best target
	CachedEntity* target = RetrieveBestTarget(aimkey_status);
	
	// Check target for dormancy and if there even is a target at all
	if (CE_GOOD(target) && foundTarget) {
		
		IF_GAME (IsTF()) {
			if (auto_zoom) {
				if (g_pLocalPlayer->holding_sniper_rifle) {
					if (not g_pLocalPlayer->bZoomed) {
						g_pUserCmd->buttons |= IN_ATTACK2;
					}
				}
			}
		}

#if ENABLE_VISUALS == 1
		// Set target esp color to pink
		hacks::shared::esp::SetEntityColor(target, colors::pink);
#endif
		
		// Check if player can aim and if aimkey allows aiming
		// We also preform a CanShoot check here per the old canshoot method
		if (ShouldAim() && aimkey_status && GetCanAim(1)) {
			
			// Check if player isnt using a huntsman
			if (g_pLocalPlayer->weapon()->m_iClassID != CL_CLASS(CTFCompoundBow)) {

				// We check if we need to do a canshoot check as we might want to shoot but not aim, so do that check here
				if (GetCanAim(2)) {
					// Check the flNextPrimaryAttack netvar to tell when to aim
					if (CanShoot()) Aim(target);
				} else {
					// If settings dont allow canShoot check, then just aim
					Aim(target);
				}
				// Attemt to auto-shoot
				if (CanAutoShoot()) g_pUserCmd->buttons |= IN_ATTACK;

			// If player is using huntsman, we use a different system for autoshooting 
			} else {
				
				// Grab time when charge began
				float begincharge = CE_FLOAT(g_pLocalPlayer->weapon(), netvar.flChargeBeginTime);
				// Reset current charge count
				float charge = 0;
				// If bow is not charged, reset the charge time keeper
				if (begincharge != 0) {
					charge = g_GlobalVars->curtime - begincharge;
					// Keep the charge time keeper kept at 1 second
					if (charge > 1.0f) charge = 1.0f;
					silent_huntsman = true;
				}
				// If current charge is equal to or more than the user limit allows, then release the huntsman
				if (charge >= (float)huntsman_autoshoot) {
					// Stop user attacking
					g_pUserCmd->buttons &= ~IN_ATTACK;
					// temporarily stop the anti-aim to allow the projectile to pass due to attack not being used
					hacks::shared::antiaim::SetSafeSpace(3);
				// If user shouldnt release hunstman, attack for user here
				} else if (autoshoot && huntsman_full_auto) {
					huntsman_ticks = 3;
					g_pUserCmd->buttons |= IN_ATTACK;
				}
				// If player released the huntsman, aim here
				if (!(g_pUserCmd->buttons & IN_ATTACK) && silent_huntsman) {
					Aim(target);
					silent_huntsman = false;
				}
			}
		}
	}
	
	// If settings allow and slowaim is disabled, Use silent angles
	if (silent && !slowaim) g_pLocalPlayer->bUseSilentAngles = true;
	return;
}
	
// The first check to see if the player should aim in the first place
bool ShouldAim() {
	// Checks should be in order: cheap -> expensive
	
	// Check for +attack if settings allow it
	if (attack_only && !(g_pUserCmd->buttons & IN_ATTACK)) {
		return false;
	}
	// Check for +use
	if (g_pUserCmd->buttons & IN_USE) return false;
	// Check if using action slot item 
	if (g_pLocalPlayer->using_action_slot_item) return false;

	IF_GAME (IsTF2()) {
		// Check if Carrying A building
		if (CE_BYTE(g_pLocalPlayer->entity, netvar.m_bCarryingObject)) return false;
		// Check if deadringer out
		if (CE_BYTE(g_pLocalPlayer->entity, netvar.m_bFeignDeathReady)) return false;
		// If zoomed only is on, check if zoomed
		if (zoomed_only && g_pLocalPlayer->holding_sniper_rifle) {
			if (!g_pLocalPlayer->bZoomed && !(g_pUserCmd->buttons & IN_ATTACK)) return false;
		}
		// Check if player is taunting
		if (HasCondition<TFCond_Taunting>(g_pLocalPlayer->entity)) return false;
		// Check if player is cloaked
		if (IsPlayerInvisible(g_pLocalPlayer->entity)) return false;
		// Disable aimbot with stickbomb launcher
		if (g_pLocalPlayer->weapon()->m_iClassID == CL_CLASS(CTFPipebombLauncher)) return false;
	}
	
	IF_GAME (IsTF2()) {
		switch (GetWeaponMode()) {
		case weapon_hitscan:
			break;
		case weapon_melee:
			break;
		// Check if player is using a projectile based weapon without the setting enabled
		case weapon_projectile:
			if (!projectile_aimbot) return false;
			break;
		// Check if player doesnt have a weapon usable by aimbot
		default:
			return false;
		};
	}
	
	IF_GAME (IsTF()) {
		// Check if player is zooming
		if (g_pLocalPlayer->bZoomed) {
			if (!(g_pUserCmd->buttons & (IN_ATTACK | IN_ATTACK2))) {
				if (!CanHeadshot()) return false;
			}
		}
		
		// Minigun spun up handler
		if (g_pLocalPlayer->weapon()->m_iClassID == CL_CLASS(CTFMinigun)) {
			int weapon_state = CE_INT(g_pLocalPlayer->weapon(), netvar.iWeaponState);
			// If user setting for autospin isnt true, then we check if minigun is already zoomed 
			if ((weapon_state == MinigunState_t::AC_STATE_IDLE || weapon_state == MinigunState_t::AC_STATE_STARTFIRING) && !auto_spin_up) {
				return false;
			}
			if (auto_spin_up) {
				g_pUserCmd->buttons |= IN_ATTACK2;
			}
			if (!(g_pUserCmd->buttons & (IN_ATTACK2 | IN_ATTACK))) {
				return false;
			}
		}

		// Check if crithack allows attacking
		if (!AllowAttacking())
			return false;
	}
	return true;
}
	
// Function to find a suitable target
CachedEntity* RetrieveBestTarget(bool aimkey_state) {
	
	// If we have a previously chosen target, target lock is on, and the aimkey is allowed, then attemt to keep the previous target
	if (target_lock && foundTarget && aimkey_state) {
		if (CE_GOOD(target_last)) {
			// Check if previous target is still good
			if (IsTargetStateGood(target_last)) {
				// If it is then return it again
				return target_last;
			}
		}
	}

	// We dont have a target currently so we must find one, reset statuses
	foundTarget = false;
	target_last = nullptr;

	// Book keeping vars
	float target_highest_score, scr;
	CachedEntity* ent;
	CachedEntity* target_highest_ent = 0;
	target_highest_score = -256;
	// Loop that checks all ents whether it is a good target or not
	for (int i = 0; i < HIGHEST_ENTITY; i++) {
		ent = ENTITY(i);
		// Check for null and dormant
		if (CE_BAD(ent)) continue;
		// Check whether the current ent is good enough to target
		if (IsTargetStateGood(ent)) {
			// Get a score for the entity
			// Distance Priority, Uses this is melee is used
			if (GetWeaponMode() == weaponmode::weapon_melee || (int)priority_mode == 2) {
				scr = 4096.0f - calculated_data_array[i].aim_position.DistTo(g_pLocalPlayer->v_Eye);
			} else {
				switch ((int)priority_mode) {
					case 0: // Smart Priority
						scr = GetScoreForEntity(ent);
						break;
					case 1: // Fov Priority
						scr = 360.0f - calculated_data_array[ent->m_IDX].fov;
						break;
					case 3: // Health Priority
						scr = 450.0f - ent->m_iHealth;
						break;
				}
			}
			// Compare the top score to our current ents score
			if (scr > target_highest_score) {
				// Set foundTarget status to true
				foundTarget = true;
				// Save found target info to vars
				target_highest_score = scr;
				target_highest_ent = ent;
			}
		}
	}
	// Save the ent for future use with target lock
	target_last = target_highest_ent;
	// When our for loop finishes, return our ent
	return target_highest_ent;
}
	
// A second check to determine whether a target is good enough to be aimed at
bool IsTargetStateGood(CachedEntity* entity) {
	
	// Check for Players
	if (entity->m_Type == ENTITY_PLAYER) {
		// Check if target is The local player
		if (entity == LOCAL_E) return false;
		// Dont aim at dead player
		if (!entity->m_bAlivePlayer) return false;
		// Dont aim at teammates
		if ((int)teammates != 2 && ((!entity->m_bEnemy && !teammates) || (entity->m_bEnemy && teammates))) return false;
		// Check if player is too far away
		if (EffectiveTargetingRange()) {
			if (entity->m_flDistance > EffectiveTargetingRange()) return false;
		}
		if (rageonly) {
			if (playerlist::AccessData(entity).state != playerlist::k_EState::RAGE) {
				return false;
			}
		}
		IF_GAME (IsTF()) {
			// If settings allow waiting for charge, and current charge cant kill target, dont aim unless at 100% and the player wants to fire at 100% anyways
			if (wait_for_charge && g_pLocalPlayer->holding_sniper_rifle) {
				// Grab netvar for current charge damage and multiply by 3 for headshot
				float cdmg = CE_FLOAT(LOCAL_W, netvar.flChargedDamage) * 3;
				bool maxCharge = cdmg >= 450.0F;

				// Darwins damage correction, Darwins protects against 15% of damage
				if (HasDarwins(entity))
				  cdmg = (cdmg * .85) - 1;
				// Vaccinator damage correction, Vac charge protects against 75% of damage
				if (HasCondition<TFCond_UberBulletResist>(entity)) {
				  cdmg = (cdmg * .25) - 1;
				// Passive bullet resist protects against 10% of damage
				} else if (HasCondition<TFCond_SmallBulletResist>(entity)) {
				  cdmg = (cdmg * .90) - 1;
				}
				// Invis damage correction, Invis spies get protection from 10% of damage
				if (IsPlayerInvisible(entity))
				  cdmg = (cdmg * .80) - 1;
				
				// Check if player will die from headshot or if target has more than 450 health and sniper has max chage
				if ( !(entity->m_iHealth <= 150.0F || entity->m_iHealth <= cdmg || !g_pLocalPlayer->bZoomed || (maxCharge && entity->m_iHealth > 450.0F)) ) {
				  return false;
				}
			}
			
			// If settings allow, ignore taunting players
			if (ignore_taunting && HasCondition<TFCond_Taunting>(entity)) return false;
			// Dont target invulnerable players, ex: uber, bonk
			if (IsPlayerInvulnerable(entity)) return false;
			// Checks for cloaked/deadringed players
			if (ignore_cloak || ignore_deadringer) {
				if (IsPlayerInvisible(entity)) {
					// Determine whether cloaked player is using deadringer and checks user settings accordingly
					// Item id for deadringer is 59 as of time of creation
					if (HasWeapon(entity, 59)) {
						if (ignore_deadringer) return false;
					} else {
						if (ignore_cloak) return false;
					}
				}
			}
			// If settings allow, dont target vaccinated players
			if (g_pLocalPlayer->weapon_mode == weaponmode::weapon_hitscan || LOCAL_W->m_iClassID == CL_CLASS(CTFCompoundBow))
				if (ignore_vaccinator && HasCondition<TFCond_UberBulletResist>(entity)) return false;
		}
		// Dont target players marked as friendly
		if (playerlist::IsFriendly(playerlist::AccessData(entity).state)) return false;
		IF_GAME (IsTF()) {
			// If settings allow, ignore hoovys
			if (ignore_hoovy && IsHoovy(entity)) {
				return false;
			}
		}
		// Preform hitbox prediction
		int hitbox = BestHitbox(entity);
		AimbotCalculatedData_s& cd = calculated_data_array[entity->m_IDX];
		cd.hitbox = hitbox;
		
		// If VisCheck fails, dont target player
		if (!VischeckPredictedEntity(entity)) return false;

		// Check if player is within fov. Fov check can be preformed as it is a minimal fps drop and it even needs to be predicted beforehand by the vischeck anyways
		if ((float)fov > 0.0f && cd.fov > (float)fov) return false;
		
		// Target passed the tests so return true
		return true;
		
	// Check for buildings
	} else if (entity->m_Type == ENTITY_BUILDING) {
		// Check if building aimbot is enabled
		if ( !(buildings_other || buildings_sentry) ) return false;
		// Check if enemy building
		if (!entity->m_bEnemy) return false;
		// Check if building is within range
		if (EffectiveTargetingRange()) {
			if (entity->m_flDistance > (int)EffectiveTargetingRange()) return false;
		}
		
		// If needed, Check if building type is allowed
		if ( !(buildings_other && buildings_sentry) ) {
			// Check if target is a sentrygun
			if ( entity->m_iClassID == CL_CLASS(CObjectSentrygun) ) {
				// If sentrys are not allowed, dont target
				if (!buildings_sentry) return false;
			} else {
				// If target is not a sentry, check if other buildings are allowed
				if (!buildings_other) return false;
			}			
		}
	
		// Grab the prediction var
		AimbotCalculatedData_s& cd = calculated_data_array[entity->m_IDX];
				
		// If VisCheck fails, dont target building
		if (!VischeckPredictedEntity(entity)) return false;
		
		// Check if building is within fov
		if ((float)fov > 0.0f && cd.fov > (float)fov) return false;
		
		// Target passed the tests so return true
		return true;
	
	// Check for stickybombs
	} else if (entity->m_iClassID == CL_CLASS(CTFGrenadePipebombProjectile)) {
		// Check if sticky aimbot is enabled
		if (!stickybot) return false;
		
		// Only hitscan weapons can break stickys so check for them.
		if (!(GetWeaponMode() == weapon_hitscan || GetWeaponMode() == weapon_melee)) return false;

		// Check if target is within range
		if (EffectiveTargetingRange()) {
			if (entity->m_flDistance > (int)EffectiveTargetingRange()) return false;
		}
		
		// Check if thrower is a teammate
		if (!entity->m_bEnemy) return false;
		
		// Check if target is a pipe bomb
		if (CE_INT(entity, netvar.iPipeType) != 1) return false;
		
		// Grab the prediction var
		AimbotCalculatedData_s& cd = calculated_data_array[entity->m_IDX];
				
		// If VisCheck fails, dont target building
		if (!VischeckPredictedEntity(entity)) return false;
		
		// Check if building is within fov
		if ((float)fov > 0.0f && cd.fov > (float)fov) return false;
			
		// Target passed the tests so return true
		return true;
		
	} else {
		// If target is not player or building, return false
		return false;
	}
	// An impossible error so just return false
	return false;
}
	
// A function to aim at a specific entitiy
void Aim(CachedEntity* entity) {
	if (float(miss_chance) > 0.0f) {
		if ((rand() % 100) < float(miss_chance) * 100.0f) {
			return;
		}
	}

	// Dont aim at a bad entity
	if (CE_BAD(entity)) return;

	// Create some vars
	Vector angles, tr;
	int hitbox;
	
	// Grab the targets vector, and vector it for the eye angles 
	tr = (PredictEntity(entity) - g_pLocalPlayer->v_Eye);
	VectorAngles(tr, angles);
	
	// Clamp angles
	fClampAngle(angles);
    
    // Slow the aiming to the aim vector if true
	if (slowaim) slowAim(angles, g_pUserCmd->viewangles);
    
	// Set angles
    g_pUserCmd->viewangles = angles;
	
	// Finish function
	return;
}

// A function to check whether player can autoshoot
bool CanAutoShoot() {
	
	// First check whether user settings allow autoshoot
	if (autoshoot) {
		
		// A var for weapons not to use with autoshoot
		static int forbiddenWeapons[] = { CL_CLASS(CTFCompoundBow), CL_CLASS(CTFKnife) };
		int weapon_class;
		bool attack = true;
		
		// If your using a sniper rifle, are zoomed and cant headshot, then return false
		IF_GAME (IsTF()) {
			if (g_pLocalPlayer->clazz == tf_class::tf_sniper) {
				if (g_pLocalPlayer->holding_sniper_rifle) {
					if (zoomed_only && !CanHeadshot()) attack = false;
				}
			}
		}
		
		// Check if zoomed, and zoom if not, then zoom
		/*IF_GAME (IsTF()) {
			if (g_pLocalPlayer->clazz == tf_class::tf_sniper) {
				if (g_pLocalPlayer->holding_sniper_rifle) {
					if (auto_zoom && !HasCondition<TFCond_Zoomed>(LOCAL_E)) {
						g_pUserCmd->buttons |= IN_ATTACK2;
						attack = false;
					}
				}
			}
		}*/
		
		// Check if ambassador can headshot
		IF_GAME (IsTF2()) {
			if (IsAmbassador(g_pLocalPlayer->weapon())) {
				// Check if ambasador can headshot
				if (!AmbassadorCanHeadshot()) return false;	
			}
		}

		// Don't autoshoot with the knife or bow!
		weapon_class = g_pLocalPlayer->weapon()->m_iClassID;
        for (int i = 0; i < 2; i++) {
			if (weapon_class == forbiddenWeapons[i]) {
				attack = false;
				break;
			}
		}
		
		//Autoshoot breaks Slow aimbot, so use a workaround to detect when it can
		if (slowaim && !slowCanShoot) attack = false;
		
		// Return what 
		return attack;
	} else
		// Return false due to setting not allowing autoshoot
		return false;
}
	
// Grab a vector for a specific ent
const Vector& PredictEntity(CachedEntity* entity) {
	// Pull out predicted data
	AimbotCalculatedData_s& cd = calculated_data_array[entity->m_IDX];
	Vector& result = cd.aim_position;
	// If predicted vector has already been calculated this tick, then return it.
	if (cd.predict_tick == tickcount) return result;
	// If ent is a player, find a corresponding vector
	if ((entity->m_Type == ENTITY_PLAYER)) {
		// If using projectiles, predict a vector
		if (projectile_mode) {
			// Use prediction engine if user settings allow
			if (engine_projpred)
				result = ProjectilePrediction_Engine(entity, cd.hitbox, cur_proj_speed, cur_proj_grav, 0);
			else
				result = ProjectilePrediction(entity, cd.hitbox, cur_proj_speed, cur_proj_grav, PlayerGravityMod(entity));
		} else {
			// If using extrapolation, then predict a vector
			if (extrapolate)
				result = SimpleLatencyPrediction(entity, cd.hitbox);
			// else just grab strait from the hitbox
			else
				GetHitbox(entity, cd.hitbox, result);
		}
	// If ent is a building, find a corresponding vector
	} else if (entity->m_Type == ENTITY_BUILDING) {
		result = GetBuildingPosition(entity);
	// If ent isnt a special type, just use origin
	} else {
		result = entity->m_vecOrigin;
	}
	// Reset the predicted tickcount for the ent
	cd.predict_tick = tickcount;
	// Pre-Calculate fov and store to array
	cd.fov = GetFov(g_pLocalPlayer->v_OrigViewangles, g_pLocalPlayer->v_Eye, result);
	// Return the found vector
	return result;
}

// A function to find the best hitbox for a target
int BestHitbox(CachedEntity* target) {

	// Switch based apon the hitbox mode set by the user
	switch ((int)hitbox_mode) {
	case 0: { // AUTO-HEAD priority
		// The best hitbox var
		int preferred = hitbox;
		// Var to keep if we can bodyshot
		bool headonly = false;
		// Save the local players current weapon to a var
		int ci = g_pLocalPlayer->weapon()->m_iClassID;
		IF_GAME (IsTF()) {
			// Set our default hitbox for pelvis 
			preferred = hitbox_t::pelvis;
			// If user is using a sniper rifle, Set headonly to whether we can headshot or not, 
			if (g_pLocalPlayer->holding_sniper_rifle) {
				headonly = CanHeadshot();
			// If player is using a compund bow, set headonly to true
			} else if (ci == CL_CLASS(CTFCompoundBow)) {
				headonly = true;
			// If player is using an ambassador, set headonly to true
			} else if (IsAmbassador(g_pLocalPlayer->weapon())) {
				// We only want to aim for the head if the ambassador can headshot
				headonly = AmbassadorCanHeadshot();
				// 18 health is a good number to use as thats the usual minimum damage it can do with a bodyshot, but damage could potentially be higher
				if (target->m_iHealth <= 18 || IsPlayerCritBoosted(g_pLocalPlayer->entity)) headonly = false;
			// If player is using a rocket based weapon, prefer the hip
			} else if (ci == CL_CLASS(CTFRocketLauncher) ||
				ci == CL_CLASS(CTFRocketLauncher_AirStrike) ||
					ci == CL_CLASS(CTFRocketLauncher_DirectHit) ||
					ci == CL_CLASS(CTFRocketLauncher_Mortar)) {
				preferred = hitbox_t::hip_L;
			}
			// If target is off the ground and local player is using projectile weapons other than the bow, use the higher hitbox, spine_3
			if (GetWeaponMode() == weaponmode::weapon_projectile) {
				// Grab netvar for flags and save to a var
				int flags = CE_INT(target, netvar.iFlags);
				// Extract ground var from flags
				bool ground = (flags & (1 << 0));
				if (!ground) {
					if (g_pLocalPlayer->weapon()->m_iClassID != CL_CLASS(CTFCompoundBow)) {
						preferred = hitbox_t::spine_3;
					}
				}
			}
			
			// Bodyshot handling
			if (g_pLocalPlayer->holding_sniper_rifle) {
				
				// Grab netvar for current charge damage
				float cdmg = CE_FLOAT(LOCAL_W, netvar.flChargedDamage);
				// Set our baseline bodyshot damage
				float bdmg = 50;
				// Darwins damage correction
				if (HasDarwins(target)) {
					// Darwins protects against 15% of damage
					bdmg = (bdmg * .85) - 1;
					cdmg = (cdmg * .85) - 1;
				}
				// Vaccinator damage correction
				if (HasCondition<TFCond_UberBulletResist>(target)) {
					// Vac charge protects against 75% of damage
					bdmg = (bdmg * .25) - 1;
					cdmg = (cdmg * .25) - 1;
				} else if (HasCondition<TFCond_SmallBulletResist>(target)) {
					// Passive bullet resist protects against 10% of damage
					bdmg = (bdmg * .90) - 1;
					cdmg = (cdmg * .90) - 1;
				}
				// Invis damage correction
				if (IsPlayerInvisible(target)) {
					// Invis spies get protection from 10% of damage
					bdmg = (bdmg * .80) - 1;
					cdmg = (cdmg * .80) - 1;
				}
				// If can headshot and if bodyshot kill from charge damage, or if crit boosted and they have 150 health, or if player isnt zoomed, or if the enemy has less than 40, due to darwins, and only if they have less than 150 health will it try to bodyshot
				if (CanHeadshot() && (cdmg >= target->m_iHealth || IsPlayerCritBoosted(g_pLocalPlayer->entity) || !g_pLocalPlayer->bZoomed || target->m_iHealth <= bdmg)  && target->m_iHealth <= 150) {
					// We dont need to hit the head as a bodyshot will kill
					preferred = hitbox_t::spine_1;
					headonly = false;
				}
			}
		// In counter-strike source, headshots are what we want
		} else IF_GAME (IsCSS()) {
			headonly = true;
		}
		// If headonly is true, return the var here
		if (headonly) {
			IF_GAME (IsTF())
				return hitbox_t::head;
			IF_GAME (IsCSS())
				return 12;
		}
		// If the prefered hitbox vis check passes, use it
		if (target->hitboxes.VisibilityCheck(preferred)) return preferred;
		// Else attempt to find a hitbox at all
		for (int i = projectile_mode ? 1 : 0; i < target->hitboxes.GetNumHitboxes(); i++) {
			if (target->hitboxes.VisibilityCheck(i)) return i;
		}
	} break;	
	case 1: { // AUTO-CLOSEST priority
		// Return closest hitbox to crosshair
		return ClosestHitbox(target);
	} break;
	case 2: { // STATIC priority
		// Return a user chosen hitbox
		return (int)hitbox;
	} break;
	}
	// without a good hitbox, just return -1 in its place
	return -1;
}

// Function to find the closesnt hitbox to the crosshair for a given ent
int ClosestHitbox(CachedEntity* target) {
	// FIXME this will break multithreading if it will be ever implemented. When implementing it, these should be made non-static
	int closest;
	float closest_fov, fov;

	closest = -1;
	closest_fov = 256;
	for (int i = 0; i < target->hitboxes.GetNumHitboxes(); i++) {
		fov = GetFov(g_pLocalPlayer->v_OrigViewangles, g_pLocalPlayer->v_Eye, target->hitboxes.GetHitbox(i)->center);
		if (fov < closest_fov || closest == -1) {
			closest = i;
			closest_fov = fov;
		}
	}
	return closest;
}

// Function to get predicted visual checks 
bool VischeckPredictedEntity(CachedEntity* entity) {
	// Retrieve predicted data
	AimbotCalculatedData_s& cd = calculated_data_array[entity->m_IDX];
	// If vis check data is up-to-date with tick count then return it
	if (cd.vcheck_tick == tickcount) return cd.visible;
	// Reset vis check tick and calculate new vis data
	cd.vcheck_tick = tickcount;
	cd.visible = IsEntityVectorVisible(entity, PredictEntity(entity));
	// Return found check
	return cd.visible;
}


// A helper function to find a user angle that isnt directly on the target angle, effectively slowing the aiming process
void slowAim(Vector &inputAngle, Vector userAngle) {
    // Initialize vars for slow aim
    int slowfliptype;
    int slowdir;
    float changey = 0;
    float changex = 0;
        
    // Determine whether to move the mouse at all for the yaw
    if (userAngle.y != inputAngle.y) {
        
        // Fliping The main axis to prevent 360s from happening when the bot trys to cross -180y and 180y
        slowfliptype = 0;
        if ( ((inputAngle.y < -90) && (userAngle.y > 90)) && (slowfliptype == 0) ) {
            slowfliptype = 1;
            inputAngle.y = inputAngle.y - 90;
            userAngle.y = userAngle.y + 90;
        }
        if ( ((inputAngle.y > 90) && (userAngle.y < -90)) && (slowfliptype == 0) ) {
            slowfliptype = 2;
            inputAngle.y = inputAngle.y + 90;
            userAngle.y = userAngle.y - 90;
        }
        
        // Math to calculate how much to move the mouse
        changey = (std::abs(userAngle.y - inputAngle.y)) / ((int)slowaim_smoothing) ;
        // Use stronger shunting due to the flip
        if (slowfliptype != 0) changey = ((( std::abs(userAngle.y - inputAngle.y) ) / ((int)slowaim_smoothing * (int)slowaim_smoothing)) / (int)slowaim_smoothing) ;
        
        // Determine the direction to move in before reseting the flipped angles
        slowdir = 0;
        if ((userAngle.y > inputAngle.y) && (slowdir == 0)) slowdir = 1;
        if ((userAngle.y < inputAngle.y) && (slowdir == 0)) slowdir = 2;

        // Reset Flipped angles and fix directions
        if (slowfliptype == 1) {
            inputAngle.y = inputAngle.y + 90;
            userAngle.y = userAngle.y - 90;
            slowdir = 2;
        }
        if (slowfliptype == 2) {
            inputAngle.y = inputAngle.y - 90;
            userAngle.y = userAngle.y + 90;
            slowdir = 1;
        }
        
        // Move in the direction determined before the fliped angles
        if (slowdir == 1) inputAngle.y = userAngle.y - changey;
        if (slowdir == 2) inputAngle.y = userAngle.y + changey;
    }
    
    // Determine whether to move the mouse at all for the pitch
    if (userAngle.x != inputAngle.x) {
        changex = (std::abs(userAngle.x - inputAngle.x)) / ((int)slowaim_smoothing) ;
        
        // Determine the direction to move in
        if (userAngle.x > inputAngle.x) inputAngle.x = userAngle.x - changex; 
        if (userAngle.x < inputAngle.x) inputAngle.x = userAngle.x + changex;
    }

    // Check if can autoshoot with slowaim
    slowCanShoot = false;
    if (changey < (0.02*(int)slowaim_autoshoot) && changex < (0.02*(int)slowaim_autoshoot)) slowCanShoot = true;
}

// A function that determins whether aimkey allows aiming
bool UpdateAimkey() {
	static bool aimkey_flip = false;
	static bool pressed_last_tick = false;
	bool key_down;
	bool allowAimkey = true;
	// Check if aimkey is used
	if (aimkey && aimkey_mode) {
		// Grab whether the aimkey is depressed
		key_down = g_IInputSystem->IsButtonDown((ButtonCode_t)(int)aimkey);
		// Switch based on the user set aimkey mode
		switch ((int)aimkey_mode) {
		// Only while key is depressed, enable
		case 1:
			if (!key_down) {
				allowAimkey = false;
			}
			break;
		// Only while key is not depressed, enable
		case 2:
			if (key_down) {
				allowAimkey = false;
			}
			break;
		// Aimkey acts like a toggle switch
		case 3:
			if (!pressed_last_tick && key_down) aimkey_flip = !aimkey_flip;
			if (!aimkey_flip) {
				allowAimkey = false;
			}
		}
		pressed_last_tick = key_down;
	}
	// Return whether the aimkey allows aiming
	return allowAimkey;
}

// A function called at 2 points in the create move function
// First time is when the aimbot Determines if it should aim and autoshoot
// Second time is for when the aimbot determines only when it should aim and always autoshoots
// Using either mode has problems with some weapons so we compramise by using a combo of the 2
// The point of using this function for 2 uses is to not make duplicate funcs for essentialy the same thing
bool GetCanAim(int mode) {
	
	// User setting check
	switch (mode) {
	case 1: // The first check when the aimbot checks if it can aim or shoot
			
		// If user settings dont allow, Always aim at the point this is called
		if (!only_can_shoot) return true;
		// Always aim with melee weapons
		if (GetWeaponMode() == weaponmode::weapon_melee) return true;
		break;
			
	case 2: // Second check when the aimbot checks if it can aim, and will shoot regardless of the output here
		
		// dont check if should aim with melee weapons
		if (GetWeaponMode() == weaponmode::weapon_melee) return false;
			
		// At the point this is called, we dont want to check for can shoot if user settings dont allow
		if (!only_can_shoot) return false; 
	}
	
	// Weapons that should attack continuously
	bool using_wep_on_list = 
		g_pLocalPlayer->weapon()->m_iClassID == CL_CLASS(CTFPistol_Scout) || 
		g_pLocalPlayer->weapon()->m_iClassID == CL_CLASS(CTFMinigun) ||
		g_pLocalPlayer->weapon()->m_iClassID == CL_CLASS(CTFSyringeGun) ||
		g_pLocalPlayer->weapon()->m_iClassID == CL_CLASS(CTFSMG) ||
		g_pLocalPlayer->weapon()->m_iClassID == CL_CLASS(CTFRevolver) ||
		g_pLocalPlayer->weapon()->m_iClassID == CL_CLASS(CTFFlameThrower);
	
	switch (mode) {
	case 1: // The first check when the aimbot checks if it can aim or shoot

		// If the player isnt using these weaps, then check for can shoot. If the weapon isnt on the list, then we need to check if the player should aim here and return the result
		if (!using_wep_on_list) return CanShoot();
		break;
			
	case 2: // Second check when the aimbot checks if it can aim, and will shoot regardless of the output here
			
		// Return whether we are using one of the weapons on the list because we want to tell the aimbot that we should check for canshoot
		return using_wep_on_list;
	}

	// Mode wasnt input correctly, just return true and hope for the best
	return true;
}

// Func to find value of how far to target ents
float EffectiveTargetingRange() {
	// Melees use a close range, TODO add dynamic range for demoknight swords
	if (GetWeaponMode() == weapon_melee) {
		return 100.0f;
	// Pyros only have so much untill their flames hit
	} else if (g_pLocalPlayer->weapon()->m_iClassID == CL_CLASS(CTFFlameThrower)) {
		return 185.0f;
	}
	// Else return user settings
	return (float)max_range;
}
	
// A function used by gui elements to determine the current target
CachedEntity* CurrentTarget() {
	// If a target is found return it
	if (foundTarget)
		return ENTITY(target_eid);
	// Else return a nullpointer
	return nullptr;
}
	
// Used for when you join and leave maps to reset aimbot vars
void Reset() {
	target_last = nullptr;
	projectile_mode = false;
}

#if ENABLE_VISUALS == 1

// Function called when we need to draw to screen
static CatVar fov_draw(CV_SWITCH, "aimbot_fov_draw", "0", "Draw Fov Ring", "Draws a ring to represent your current aimbot fov");
void DrawText() {
	// Dont draw to screen when aimbot is disabled
	if (!enabled) return;
	
	// Fov ring to represent when a target will be shot
	// Not perfect but does a good job of representing where its supposed to be
	// Broken from kathook merge, TODO needs to be adapted for imgui
	if (fov_draw) {
		// It cant use fovs greater than 180, so we check for that
		if (float(fov) > 0.0f && float(fov) < 180) {
			// Dont show ring while player is dead
			if (LOCAL_E->m_bAlivePlayer) {
				rgba_t color = GUIColor();
				color.a = float(fovcircle_opacity);

				// Grab the screen resolution and save to some vars
				int width, height;
				g_IEngine->GetScreenSize(width, height);
				
				// Some math to find radius of the fov circle
				float mon_fov = (float(width) / float(height) / (4.0f / 3.0f));
				float fov_real = RAD2DEG(2 * atanf(mon_fov * tanf(DEG2RAD(draw::fov / 2))));

				float radius = tan(DEG2RAD(float(fov)) / 2) / tan(DEG2RAD(fov_real) / 2) * (width);
				
				// Draw a circle with our newfound circle
				float px = 0;
				float py = 0;
				constexpr float steps = 120;
				for (int i = 0; i < steps; i++) {
					float ang = 2 * PI * (float(i) / steps);
					float x = width / 2 + radius * cos(ang);
					float y = height / 2 + radius * sin(ang);
					if (!i) {
						ang = 2 * PI * (float(steps - 1) / steps);
						px = width / 2 + radius * cos(ang);
						py = height / 2 + radius * sin(ang);
					}
					drawgl::Line(px, py, x - px, y - py, color);
					px = x;
					py = y;
				}
			}
		}
	}	
	// Dont run the following unless debug is enabled
	if (!aimbot_debug) return;
	for (int i = 1; i < 32; i++) {
		CachedEntity* ent = ENTITY(i);
		if (CE_GOOD(ent)) {
			Vector screen;
			Vector oscreen;
			if (draw::WorldToScreen(calculated_data_array[i].aim_position, screen) && draw::WorldToScreen(ent->m_vecOrigin, oscreen)) {
				drawgl::FilledRect(screen.x - 2, screen.y - 2, 4, 4);
				drawgl::Line(oscreen.x, oscreen.y, screen.x - oscreen.x, screen.y - oscreen.y);
			}
		}
	}
}

#endif

}}}
