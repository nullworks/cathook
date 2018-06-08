/*
 * HAimbot.cpp
 *
 *  Created on: Oct 9, 2016
 *      Author: nullifiedcat
 */

#include <hacks/Aimbot.hpp>
#include <hacks/CatBot.hpp>
#include <hacks/AntiAim.hpp>
#include <hacks/ESP.hpp>
#include "common.hpp"

namespace hacks
{
namespace shared
{
namespace aimbot
{

// User settings are stored and used by these vars
static CatVar enabled(CV_SWITCH, XORSTR("aimbot_enabled"), XORSTR("0"), XORSTR("Enable Aimbot"),
                      XORSTR("Main aimbot switch"));
static CatVar aimkey(CV_KEY, XORSTR("aimbot_aimkey"), XORSTR("0"), XORSTR("Aimkey"),
                     XORSTR("Aimkey. Look at Aimkey Mode too!"));
static CatEnum aimkey_modes_enum({ XORSTR("DISABLED"), XORSTR("AIMKEY"), XORSTR("REVERSE"), XORSTR("TOGGLE") });
static CatVar aimkey_mode(aimkey_modes_enum, XORSTR("aimbot_aimkey_mode"), XORSTR("1"),
                          XORSTR("Aimkey mode"),
                          XORSTR("DISABLED: aimbot is always active\nAIMKEY: aimbot ")
                          XORSTR("is active when key is down\nREVERSE: aimbot is ")
                          XORSTR("disabled when key is down\nTOGGLE: pressing key ")
                          XORSTR("toggles aimbot"));
static CatVar autoshoot(CV_SWITCH, XORSTR("aimbot_autoshoot"), XORSTR("1"), XORSTR("Autoshoot"),
                        XORSTR("Shoot automatically when the target is locked, isn't ")
                        XORSTR("compatible with 'Enable when attacking'");
static CatVar autoshoot_disguised(CV_SWITCH, XORSTR("aimbot_autoshoot_disguised"), XORSTR("1"),
                                  XORSTR("Autoshoot while disguised"),
                                  XORSTR("Shoot automatically if disguised."));
static CatVar multipoint(CV_SWITCH, XORSTR("aimbot_multipoint"), XORSTR("0"), XORSTR("Multipoint"),
                         XORSTR("Multipoint aimbot"));
static CatEnum hitbox_mode_enum({ XORSTR("AUTO"), XORSTR("AUTO-CLOSEST"), XORSTR("STATIC") });
static CatVar hitbox_mode(hitbox_mode_enum, XORSTR("aimbot_hitboxmode"), XORSTR("0"),
                          XORSTR("Hitbox Mode"), XORSTR("Defines hitbox selection mode"));
static CatVar fov(
    CV_FLOAT, XORSTR("aimbot_fov"), XORSTR("0"), XORSTR("Aimbot FOV"),
    XORSTR("FOV range for aimbot to lock targets. \"Smart FOV\" coming eventually."),
    180.0f);
static CatVar fovcircle_opacity(CV_FLOAT, XORSTR("aimbot_fov_draw_opacity"), XORSTR("0.7"),
                                XORSTR("FOV Circle Opacity"),
                                XORSTR("Defines opacity of FOV circle"), 0.0f, 1.0f);
// Selective and related
static CatEnum priority_mode_enum({ XORSTR("SMART"), XORSTR("FOV"), XORSTR("DISTANCE"), XORSTR("HEALTH") });
static CatVar priority_mode(priority_mode_enum, XORSTR("aimbot_prioritymode"), XORSTR("0"),
                            XORSTR("Priority mode"),
                            XORSTR("Priority mode.\n")
                            XORSTR("SMART: Basically Auto-Threat. ")
                            XORSTR("FOV, DISTANCE, HEALTH are self-explainable. ")
                            XORSTR("HEALTH picks the weakest enemy"));
static CatVar
    wait_for_charge(CV_SWITCH, XORSTR("aimbot_charge"), XORSTR("0"),
                    XORSTR("Wait for sniper rifle charge"),
                    XORSTR("Aimbot waits until it has enough charge to kill"));
static CatVar ignore_vaccinator(
    CV_SWITCH, XORSTR("aimbot_ignore_vaccinator"), XORSTR("1"), XORSTR("Ignore Vaccinator"),
    XORSTR("Hitscan weapons won't fire if enemy is vaccinated against bullets"));
static CatVar ignore_hoovy(CV_SWITCH, XORSTR("aimbot_ignore_hoovy"), XORSTR("0"),
                           XORSTR("Ignore Hoovies"), XORSTR("Aimbot won't attack hoovies"));
static CatVar ignore_cloak(CV_SWITCH, XORSTR("aimbot_ignore_cloak"), XORSTR("1"),
                           XORSTR("Ignore cloaked"), XORSTR("Don't aim at invisible enemies"));
static CatVar ignore_deadringer(CV_SWITCH, XORSTR("aimbot_ignore_deadringer"), XORSTR("1"),
                                XORSTR("Ignore deadringer"),
                                XORSTR("Don't aim at deadringed enemies"));
static CatVar buildings_sentry(CV_SWITCH, XORSTR("aimbot_buildings_sentry"), XORSTR("1"),
                               XORSTR("Aim Sentry"),
                               XORSTR("Should aimbot aim at sentryguns?"));
static CatVar buildings_other(CV_SWITCH, XORSTR("aimbot_buildings_other"), XORSTR("1"),
                              XORSTR("Aim Other building"),
                              XORSTR("Should aimbot aim at other buildings"));
static CatVar stickybot(CV_SWITCH, XORSTR("aimbot_stickys"), XORSTR("0"), XORSTR("Aim Sticky"),
                        XORSTR("Should aimbot aim at stickys"));
static CatVar rageonly(CV_SWITCH, XORSTR("aimbot_rage_only"), XORSTR("0"),
                       XORSTR("Ignore non-rage targets"),
                       XORSTR("Use playerlist to set up rage targets"));
static CatEnum teammates_enum({ XORSTR("ENEMY ONLY"), XORSTR("TEAMMATE ONLY"), XORSTR("BOTH") });
static CatVar teammates(teammates_enum, XORSTR("aimbot_teammates"), XORSTR("0"),
                        XORSTR("Aim at teammates"),
                        XORSTR("Use to choose which team/s to target"));
// Other prefrences
static CatVar
    silent(CV_SWITCH, XORSTR("aimbot_silent"), XORSTR("1"), XORSTR("Silent"),
           XORSTR("Your screen doesn't get snapped to the point where aimbot aims at"));
static CatVar target_lock(
    CV_SWITCH, XORSTR("aimbot_target_lock"), XORSTR("0"), XORSTR("Target Lock"),
    XORSTR("Keeps your previously chosen target untill target check fails"));
static CatEnum hitbox_enum({ XORSTR("HEAD"), XORSTR("PELVIS"), XORSTR("SPINE 0"), XORSTR("SPINE 1"), XORSTR("SPINE 2"),
                             XORSTR("SPINE 3"), XORSTR("UPPER ARM L"), XORSTR("LOWER ARM L"), XORSTR("HAND L"),
                             XORSTR("UPPER ARM R"), XORSTR("LOWER ARM R"), XORSTR("HAND R"), XORSTR("HIP L"),
                             XORSTR("KNEE L"), XORSTR("FOOT L"), XORSTR("HIP R"), XORSTR("KNEE R"), XORSTR("FOOT R") });
static CatVar hitbox(hitbox_enum, XORSTR("aimbot_hitbox"), XORSTR("0"), XORSTR("Hitbox"),
                     XORSTR("Hitbox to aim at. Ignored if AutoHitbox is on"));
static CatVar zoomed_only(CV_SWITCH, XORSTR("aimbot_zoomed"), XORSTR("0"), XORSTR("Zoomed only"),
                          XORSTR("Don't autoshoot with unzoomed rifles"));
static CatVar only_can_shoot(CV_SWITCH, XORSTR("aimbot_only_when_can_shoot"), XORSTR("1"),
                             XORSTR("Active when can shoot"),
                             XORSTR("Aimbot only activates when you can instantly ")
                             XORSTR("shoot, sometimes making the autoshoot invisible ")
                             XORSTR("for spectators"));
static CatVar
    max_range(CV_INT, XORSTR("aimbot_maxrange"), XORSTR("0"), XORSTR("Max distance"),
              XORSTR("Max range for aimbot\n")
              XORSTR("900-1100 range is efficient for scout/widowmaker engineer"),
              4096.0f);
static CatVar extrapolate(CV_SWITCH, XORSTR("aimbot_extrapolate"), XORSTR("0"),
                          XORSTR("Latency extrapolation"),
                          XORSTR("(NOT RECOMMENDED) latency extrapolation"));
static CatVar slow_aim(CV_INT, XORSTR("aimbot_slow"), XORSTR("0"), XORSTR("Slow Aim"),
                       XORSTR("Slowly moves your crosshair onto the target for more ")
                       XORSTR("legit play\nDisables silent aimbot"),
                       0, 50);
static CatVar projectile_aimbot(
    CV_SWITCH, XORSTR("aimbot_projectile"), XORSTR("1"), XORSTR("Projectile aimbot"),
    XORSTR("If you turn it off, aimbot won't try to aim with projectile weapons"));
static CatVar
    proj_gravity(CV_FLOAT, XORSTR("aimbot_proj_gravity"), XORSTR("0"), XORSTR("Projectile gravity"),
                 XORSTR("Force override projectile gravity. Useful for debugging."),
                 1.0f);
static CatVar
    proj_speed(CV_FLOAT, XORSTR("aimbot_proj_speed"), XORSTR("0"), XORSTR("Projectile speed"),
               XORSTR("Force override projectile speed.\n")
               XORSTR("Can be useful for playing with MvM upgrades or on x10 servers ")
               XORSTR("since there is no \"automatic\" projectile speed detection in ")
               XORSTR("cathook. Yet."));
static CatVar
    huntsman_autoshoot(CV_FLOAT, XORSTR("aimbot_huntsman_charge"), XORSTR("0.5"),
                       XORSTR("Huntsman autoshoot"),
                       XORSTR("Minimum charge for autoshooting with huntsman.\n")
                       XORSTR("Set it to 0.01 if you want to shoot as soon as you ")
                       XORSTR("start pulling the arrow"),
                       0.01f, 1.0f);
static CatVar miss_chance(CV_FLOAT, XORSTR("aimbot_miss_chance"), XORSTR("0"), XORSTR("Miss chance"),
                          XORSTR("From 0 to 1. Aimbot will NOT aim in these % cases"),
                          0.0f, 1.0f);
// Debug vars
static CatVar aimbot_debug(CV_SWITCH, XORSTR("aimbot_debug"), XORSTR("0"), XORSTR("Aimbot Debug"),
                           XORSTR("Display simple debug info for aimbot"));
static CatVar engine_projpred(CV_SWITCH, XORSTR("debug_aimbot_engine_pp"), XORSTR("0"),
                              XORSTR("Engine ProjPred"));
// Followbot vars
static CatVar auto_spin_up(
    CV_SWITCH, XORSTR("aimbot_spin_up"), XORSTR("0"), XORSTR("Auto Spin Up"),
    XORSTR("Spin up minigun if you can see target, useful for followbots"));
static CatVar auto_zoom(
    CV_SWITCH, XORSTR("aimbot_auto_zoom"), XORSTR("0"), XORSTR("Auto Zoom"),
    XORSTR("Automatically zoom in if you can see target, useful for followbots"));
static CatVar auto_unzoom(CV_SWITCH, XORSTR("aimbot_auto_unzoom"), XORSTR("0"), XORSTR("Auto Un-zoom"),
                          XORSTR("Automatically unzoom"));

// Current Entity
int target_eid{ 0 };
CachedEntity *target      = 0;
CachedEntity *target_last = 0;
bool foundTarget          = false;
// Projectile info
bool projectile_mode{ false };
float cur_proj_speed{ 0.0f };
float cur_proj_grav{ 0.0f };
// If slow aimbot allows autoshoot
bool slow_can_shoot = false;

// This array will store calculated projectile/hitscan predictions
// for current frame, to avoid performing them again
AimbotCalculatedData_s calculated_data_array[2048]{};

// The main XORSTR("loop") of the aimbot.
void CreateMove()
{
    PROF_SECTION(PT_aimbot_cm);
    if (!enabled)
        return;

    // Auto-Unzoom
    if (auto_unzoom)
    {
        if (g_pLocalPlayer->holding_sniper_rifle)
        {
            if (g_pLocalPlayer->bZoomed)
            {
                if (g_GlobalVars->curtime - g_pLocalPlayer->flZoomBegin > 5.0f)
                    g_pUserCmd->buttons |= IN_ATTACK2;
            }
        }
    }

    // Refresh projectile info
    projectile_mode = (GetProjectileData(g_pLocalPlayer->weapon(),
                                         cur_proj_speed, cur_proj_grav));
    if (proj_speed)
        cur_proj_speed = float(proj_speed);
    if (proj_gravity)
        cur_proj_grav = float(proj_gravity);

    // We do this as we need to pass whether the aimkey allows aiming to both
    // the find target and aiming system. If we just call the func than toggle
    // aimkey would break so we save it to a var to use it twice
    bool aimkey_status = UpdateAimkey();

    // Refresh our best target
    CachedEntity *target_entity = RetrieveBestTarget(aimkey_status);
    if (CE_BAD(target_entity) || !foundTarget)
        return;

    if (!g_IEntityList->GetClientEntity(target_entity->m_IDX))
        return;
    if (!target_entity->hitboxes.GetHitbox(
            calculated_data_array[target_entity->m_IDX].hitbox))
        return;
    // Auto-zoom
    IF_GAME(IsTF())
    {
        if (auto_zoom)
        {
            if (g_pLocalPlayer->holding_sniper_rifle)
            {
                if (not g_pLocalPlayer->bZoomed)
                {
                    g_pUserCmd->buttons |= IN_ATTACK2;
                }
            }
        }
    }

#if ENABLE_VISUALS
    static effect_chams::EffectChams Effectchams;
    hacks::shared::esp::SetEntityColor(target_entity, colors::pink);
    Effectchams.SetEntityColor(target_entity, colors::pink);
#endif

    // Local player check + Aimkey
    if (!ShouldAim() || !aimkey_status)
        return;

    // Attemt to auto-shoot

    // flNextPrimaryAttack meme
    if (only_can_shoot)
    {

        // Handle Compound bow
        if (g_pLocalPlayer->weapon()->m_iClassID() == CL_CLASS(CTFCompoundBow))
        {
            DoAutoshoot();
            static bool currently_charging_huntsman = false;

            // Hunstman started charging
            if (CE_FLOAT(g_pLocalPlayer->weapon(), netvar.flChargeBeginTime) !=
                0)
                currently_charging_huntsman = true;

            // Huntsman was released
            if (!(g_pUserCmd->buttons & IN_ATTACK) &&
                currently_charging_huntsman)
            {
                currently_charging_huntsman = false;
                Aim(target_entity);
            }
            else
                return;

            // Not release type weapon
        }
        else if (GetWeaponMode() == weapon_melee)
        {
            DoAutoshoot();
            Aim(target_entity);
        }
        else if (CanShoot() &&
                 CE_INT(g_pLocalPlayer->weapon(), netvar.m_iClip1) != 0)
        {
            Aim(target_entity);
            DoAutoshoot();
        }
    }
    else
    {
        DoAutoshoot();
        Aim(target_entity);
    }

    return;
}

// The first check to see if the player should aim in the first place
bool ShouldAim()
{
    // Checks should be in order: cheap -> expensive

    // Check for +use
    if (g_pUserCmd->buttons & IN_USE)
        return false;
    // Check if using action slot item
    if (g_pLocalPlayer->using_action_slot_item)
        return false;

    IF_GAME(IsTF2())
    {
        // Carrying A building?
        if (CE_BYTE(g_pLocalPlayer->entity, netvar.m_bCarryingObject))
            return false;
        // Deadringer out?
        if (CE_BYTE(g_pLocalPlayer->entity, netvar.m_bFeignDeathReady))
            return false;
        // If zoomed only is on, check if zoomed
        if (zoomed_only && g_pLocalPlayer->holding_sniper_rifle)
        {
            if (!g_pLocalPlayer->bZoomed && !(g_pUserCmd->buttons & IN_ATTACK))
                return false;
        }
        // Is taunting?
        if (HasCondition<TFCond_Taunting>(g_pLocalPlayer->entity))
            return false;
        // Is cloaked
        if (IsPlayerInvisible(g_pLocalPlayer->entity))
            return false;
        // Disable aimbot with stickbomb launcher
        if (g_pLocalPlayer->weapon()->m_iClassID() ==
            CL_CLASS(CTFPipebombLauncher))
            return false;
    }

    IF_GAME(IsTF2())
    {
        switch (GetWeaponMode())
        {
        case weapon_hitscan:
            break;
        case weapon_melee:
            break;
        // Check if player is using a projectile based weapon without the
        // setting enabled
        case weapon_projectile:
            if (!projectile_aimbot)
                return false;
            break;
        // Check if player doesnt have a weapon usable by aimbot
        default:
            return false;
        };
    }

    IF_GAME(IsTF())
    {
        // Check if player is zooming
        if (g_pLocalPlayer->bZoomed)
        {
            if (!(g_pUserCmd->buttons & (IN_ATTACK | IN_ATTACK2)))
            {
                if (!CanHeadshot())
                    return false;
            }
        }

        // Minigun spun up handler
        if (g_pLocalPlayer->weapon()->m_iClassID() == CL_CLASS(CTFMinigun))
        {
            int weapon_state =
                CE_INT(g_pLocalPlayer->weapon(), netvar.iWeaponState);
            // If user setting for autospin isnt true, then we check if minigun
            // is already zoomed
            if ((weapon_state == MinigunState_t::AC_STATE_IDLE ||
                 weapon_state == MinigunState_t::AC_STATE_STARTFIRING) &&
                !auto_spin_up)
            {
                return false;
            }
            if (auto_spin_up)
            {
                g_pUserCmd->buttons |= IN_ATTACK2;
            }
            if (!(g_pUserCmd->buttons & (IN_ATTACK2 | IN_ATTACK)))
            {
                return false;
            }
        }
    }
    return true;
}

// Function to find a suitable target
CachedEntity *RetrieveBestTarget(bool aimkey_state)
{

    // If we have a previously chosen target, target lock is on, and the aimkey
    // is allowed, then attemt to keep the previous target
    if (target_lock && foundTarget && aimkey_state)
    {
        if (CE_GOOD(target_last))
        {
            // Check if previous target is still good
            if (IsTargetStateGood(target_last))
            {
                // If it is then return it again
                return target_last;
            }
        }
    }

    // We dont have a target currently so we must find one, reset statuses
    foundTarget = false;
    target_last = nullptr;

    float target_highest_score, scr;
    CachedEntity *ent;
    CachedEntity *target_highest_ent = 0;
    target_highest_score             = -256;

    for (int i = 0; i < HIGHEST_ENTITY; i++)
    {
        ent = ENTITY(i);
        if (CE_BAD(ent))
            continue; // Check for null and dormant
        // Check whether the current ent is good enough to target
        if (IsTargetStateGood(ent))
        {

            // Distance Priority, Uses this is melee is used
            if (GetWeaponMode() == weaponmode::weapon_melee ||
                (int) priority_mode == 2)
            {
                scr = 4096.0f -
                      calculated_data_array[i].aim_position.DistTo(
                          g_pLocalPlayer->v_Eye);
            }
            else
            {
                switch ((int) priority_mode)
                {
                case 0: // Smart Priority
                    scr = GetScoreForEntity(ent);
                    break;
                case 1: // Fov Priority
                    scr = 360.0f - calculated_data_array[ent->m_IDX].fov;
                    break;
                case 3: // Health Priority
                    scr = 450.0f - ent->m_iHealth();
                    break;
                default:
                    break;
                }
            }
            // Compare the top score to our current ents score
            if (scr > target_highest_score)
            {
                foundTarget          = true;
                target_highest_score = scr;
                target_highest_ent   = ent;
            }
        }
    }
    // Save the ent for future use with target lock
    target_last = target_highest_ent;

    return target_highest_ent;
}

// A second check to determine whether a target is good enough to be aimed at
bool IsTargetStateGood(CachedEntity *entity)
{
    PROF_SECTION(PT_aimbot_targetstatecheck);

    // Checks for Players
    if (entity->m_Type() == ENTITY_PLAYER)
    {
        // Local player check
        if (entity == LOCAL_E)
            return false;
        // Dead
        if (!entity->m_bAlivePlayer())
            return false;
        // Teammates
        if ((int) teammates != 2 && ((!entity->m_bEnemy() && !teammates) ||
                                     (entity->m_bEnemy() && teammates)))
            return false;
        // Distance
        if (EffectiveTargetingRange())
        {
            if (entity->m_flDistance() > EffectiveTargetingRange())
                return false;
        }
        // Rage only check
        if (rageonly)
        {
            if (playerlist::AccessData(entity).state !=
                playerlist::k_EState::RAGE)
            {
                return false;
            }
        }
        IF_GAME(IsTF())
        {
            // don't aim if holding sapper
            if (g_pLocalPlayer->holding_sapper)
                return false;

            // Wait for charge
            if (wait_for_charge && g_pLocalPlayer->holding_sniper_rifle)
            {
                float cdmg     = CE_FLOAT(LOCAL_W, netvar.flChargedDamage) * 3;
                bool maxCharge = cdmg >= 450.0F;

                // Darwins damage correction, Darwins protects against 15% of
                // damage
                //                if (HasDarwins(entity))
                //                    cdmg = (cdmg * .85) - 1;
                // Vaccinator damage correction, Vac charge protects against 75%
                // of damage
                if (HasCondition<TFCond_UberBulletResist>(entity))
                {
                    cdmg = (cdmg * .25) - 1;
                    // Passive bullet resist protects against 10% of damage
                }
                else if (HasCondition<TFCond_SmallBulletResist>(entity))
                {
                    cdmg = (cdmg * .90) - 1;
                }
                // Invis damage correction, Invis spies get protection from 10%
                // of damage
                if (IsPlayerInvisible(entity))
                    cdmg = (cdmg * .80) - 1;

                // Check if player will die from headshot or if target has more
                // than 450 health and sniper has max chage
                if (!(entity->m_iHealth() <= 150.0F ||
                      entity->m_iHealth() <= cdmg || !g_pLocalPlayer->bZoomed ||
                      (maxCharge && entity->m_iHealth() > 450.0F)))
                {
                    return false;
                }
            }

            // Taunting
            if (ignore_taunting && HasCondition<TFCond_Taunting>(entity))
                return false;
            // Invulnerable players, ex: uber, bonk
            if (IsPlayerInvulnerable(entity))
                return false;
            // cloaked/deadringed players
            if (ignore_cloak || ignore_deadringer)
            {
                if (IsPlayerInvisible(entity))
                {
                    // Item id for deadringer is 59 as of time of creation
                    if (HasWeapon(entity, 59))
                    {
                        if (ignore_deadringer)
                            return false;
                    }
                    else
                    {
                        if (ignore_cloak)
                            return false;
                    }
                }
            }
            // Vaccinator
            if (g_pLocalPlayer->weapon_mode == weaponmode::weapon_hitscan ||
                LOCAL_W->m_iClassID() == CL_CLASS(CTFCompoundBow))
                if (ignore_vaccinator &&
                    HasCondition<TFCond_UberBulletResist>(entity))
                    return false;
        }
        // Friendly player
        if (playerlist::IsFriendly(playerlist::AccessData(entity).state))
            return false;
        IF_GAME(IsTF())
        {
            // Hoovys
            if (ignore_hoovy && IsHoovy(entity))
            {
                return false;
            }
        }
        if (hacks::shared::catbot::should_ignore_player(entity))
            return false;
        // Preform hitbox prediction
        int hitbox                 = BestHitbox(entity);
        AimbotCalculatedData_s &cd = calculated_data_array[entity->m_IDX];
        cd.hitbox                  = hitbox;

        // Vis check + fov check
        if (!VischeckPredictedEntity(entity))
            return false;
        if ((float) fov > 0.0f && cd.fov > (float) fov)
            return false;

        return true;

        // Check for buildings
    }
    else if (entity->m_Type() == ENTITY_BUILDING)
    {
        // Don't aim if holding sapper
        if (g_pLocalPlayer->holding_sapper)
            return false;
        // Enabled check
        if (!(buildings_other || buildings_sentry))
            return false;
        // Teammates, Even with friendly fire enabled, buildings can NOT be
        // damaged
        if (!entity->m_bEnemy())
            return false;
        // Distance
        if (EffectiveTargetingRange())
        {
            if (entity->m_flDistance() > (int) EffectiveTargetingRange())
                return false;
        }

        // Building type
        if (!(buildings_other && buildings_sentry))
        {
            // Check if target is a sentrygun
            if (entity->m_iClassID() == CL_CLASS(CObjectSentrygun))
            {
                if (!buildings_sentry)
                    return false;
                // Other
            }
            else
            {
                if (!buildings_other)
                    return false;
            }
        }

        // Grab the prediction var
        AimbotCalculatedData_s &cd = calculated_data_array[entity->m_IDX];

        // Vis and fov checks
        if (!VischeckPredictedEntity(entity))
            return false;
        if ((float) fov > 0.0f && cd.fov > (float) fov)
            return false;

        return true;

        // Check for stickybombs
    }
    else if (entity->m_iClassID() == CL_CLASS(CTFGrenadePipebombProjectile))
    {
        // Enabled
        if (!stickybot)
            return false;

        // Only hitscan weapons can break stickys so check for them.
        if (!(GetWeaponMode() == weapon_hitscan ||
              GetWeaponMode() == weapon_melee))
            return false;

        // Distance
        if (EffectiveTargetingRange())
        {
            if (entity->m_flDistance() > (int) EffectiveTargetingRange())
                return false;
        }

        // Teammates, Even with friendly fire enabled, stickys can NOT be
        // destroied
        if (!entity->m_bEnemy())
            return false;

        // Check if target is a pipe bomb
        if (CE_INT(entity, netvar.iPipeType) != 1)
            return false;

        // Moving Sticky?
        if (!CE_VECTOR(entity, netvar.vVelocity).IsZero(1.0f))
            return false;

        // Grab the prediction var
        AimbotCalculatedData_s &cd = calculated_data_array[entity->m_IDX];

        // Vis and fov check
        if (!VischeckPredictedEntity(entity))
            return false;
        if ((float) fov > 0.0f && cd.fov > (float) fov)
            return false;

        return true;
    }
    else
    {
        // Target not valid
        return false;
    }
    // An impossible error so just return false
    return false;
}

// A function to aim at a specific entitiy
void Aim(CachedEntity *entity)
{
    if (float(miss_chance) > 0.0f)
    {
        if ((rand() % 100) < float(miss_chance) * 100.0f)
        {
            return;
        }
    }

    // Dont aim at a bad entity
    if (CE_BAD(entity))
        return;

    // Get angles
    Vector tr = (PredictEntity(entity) - g_pLocalPlayer->v_Eye);

    // Multipoint
    if (multipoint && !projectile_mode)
    {
        // Get hitbox num
        AimbotCalculatedData_s &cd = calculated_data_array[entity->m_IDX];
        float minx, maxx, miny, maxy, minz, maxz, centerx, centery, centerz;
        auto hitbox = entity->hitboxes.GetHitbox(cd.hitbox);
        // get positions
        minx    = hitbox->min.x;
        miny    = hitbox->min.y;
        maxx    = hitbox->max.x;
        maxy    = hitbox->max.y;
        minz    = hitbox->min.z;
        maxz    = hitbox->max.z;
        centerx = hitbox->center.x;
        centery = hitbox->center.y;
        centerz = hitbox->center.z;

        // Shrink positions
        std::vector<Vector> positions;
        minx += (maxx - minx) / 6;
        maxx -= (maxx - minx) / 6;
        maxy -= (maxy - miny) / 6;
        miny += (maxy - miny) / 6;
        maxz -= (maxz - minz) / 6;
        minz += (maxz - minz) / 6;
        // Create Vectors
        positions.push_back({ minx, centery, minz });
        positions.push_back({ maxx, centery, minz });
        positions.push_back({ minx, centery, maxz });
        positions.push_back({ maxx, centery, maxz });
        positions.push_back({ centerx, miny, minz });
        positions.push_back({ centerx, maxy, minz });
        positions.push_back({ centerx, miny, maxz });
        positions.push_back({ centerx, maxy, maxz });
        positions.push_back({ minx, miny, centerz });
        positions.push_back({ maxx, maxy, centerz });
        positions.push_back({ minx, miny, centerz });
        positions.push_back({ maxx, maxy, centerz });
        positions.push_back(hitbox->center);
        for (auto pos : positions)
            if (IsVectorVisible(g_pLocalPlayer->v_Eye, pos))
            {
                tr = (pos - g_pLocalPlayer->v_Eye);
                break;
            }
    }
    Vector angles;
    VectorAngles(tr, angles);
    // Clamping is important
    fClampAngle(angles);

    // Slow aim
    if (slow_aim)
        DoSlowAim(angles);

    // Set angles
    g_pUserCmd->viewangles = angles;

    if (silent && !slow_aim)
        g_pLocalPlayer->bUseSilentAngles = true;

    // Finish function
    return;
}

// A function to check whether player can autoshoot
bool begancharge = false;
void DoAutoshoot()
{
    // Enable check
    if (!autoshoot)
        return;
    if (IsPlayerDisguised(g_pLocalPlayer->entity) && !autoshoot_disguised)
        return;
    // Handle Compound bow
    if (g_pLocalPlayer->weapon()->m_iClassID() == CL_CLASS(CTFCompoundBow))
    {

        // Grab time when charge began
        float begincharge =
            CE_FLOAT(g_pLocalPlayer->weapon(), netvar.flChargeBeginTime);

        // Release hunstman if over huntsmans limit
        if ((g_GlobalVars->curtime - begincharge >=
             (float) huntsman_autoshoot) &&
            begancharge)
        {
            g_pUserCmd->buttons &= ~IN_ATTACK;
            hacks::shared::antiaim::SetSafeSpace(3);
            begancharge = false;
            // Pull string if charge isnt enough
        }
        else
        {
            g_pUserCmd->buttons |= IN_ATTACK;
            begancharge = true;
        }
        return;
    }
    else
        begancharge = false;
    bool attack     = true;

    // Rifle check
    IF_GAME(IsTF())
    {
        if (g_pLocalPlayer->clazz == tf_class::tf_sniper)
        {
            if (g_pLocalPlayer->holding_sniper_rifle)
            {
                if (zoomed_only && !CanHeadshot())
                    attack = false;
            }
        }
    }

    // Ambassador check
    IF_GAME(IsTF2())
    {
        if (IsAmbassador(g_pLocalPlayer->weapon()))
        {
            // Check if ambasador can headshot
            if (!AmbassadorCanHeadshot() && wait_for_charge)
                attack = false;
        }
    }

    // Forbidden weapons check
    if (g_pLocalPlayer->weapon()->m_iClassID() == CL_CLASS(CTFKnife))
        attack = false;

    // Autoshoot breaks with Slow aimbot, so use a workaround to detect when it
    // can
    if (slow_aim && !slow_can_shoot)
        attack = false;

    // Dont autoshoot without anything in clip
    if (CE_INT(g_pLocalPlayer->weapon(), netvar.m_iClip1) == 0)
        attack = false;

    if (attack)
        g_pUserCmd->buttons |= IN_ATTACK;
    hacks::shared::antiaim::SetSafeSpace(1);

    return;
}

// Grab a vector for a specific ent
const Vector &PredictEntity(CachedEntity *entity)
{
    // Pull out predicted data
    AimbotCalculatedData_s &cd = calculated_data_array[entity->m_IDX];
    Vector &result             = cd.aim_position;
    if (cd.predict_tick == tickcount)
        return result;

    // Players
    if ((entity->m_Type() == ENTITY_PLAYER))
    {
        // If using projectiles, predict a vector
        if (projectile_mode &&
            (g_pLocalPlayer->weapon_mode == weapon_projectile ||
             g_pLocalPlayer->weapon_mode == weapon_throwable))
        {
            // Use prediction engine if user settings allow
            if (engine_projpred)
                result = ProjectilePrediction_Engine(
                    entity, cd.hitbox, cur_proj_speed, cur_proj_grav, 0);
            else
                result = ProjectilePrediction(entity, cd.hitbox, cur_proj_speed,
                                              cur_proj_grav,
                                              PlayerGravityMod(entity));
        }
        else
        {
            // If using extrapolation, then predict a vector
            if (extrapolate)
                result = SimpleLatencyPrediction(entity, cd.hitbox);
            // else just grab strait from the hitbox
            else
                GetHitbox(entity, cd.hitbox, result);
        }
        // Buildings
    }
    else if (entity->m_Type() == ENTITY_BUILDING)
    {
        result = GetBuildingPosition(entity);
        // Other
    }
    else
    {
        result = entity->m_vecOrigin();
    }

    cd.predict_tick = tickcount;
    cd.fov =
        GetFov(g_pLocalPlayer->v_OrigViewangles, g_pLocalPlayer->v_Eye, result);
    // Return the found vector
    return result;
}

// A function to find the best hitbox for a target
int BestHitbox(CachedEntity *target)
{

    // Switch based apon the hitbox mode set by the user
    switch ((int) hitbox_mode)
    {
    case 0:
    { // AUTO-HEAD priority
        int preferred = hitbox;
        bool headonly = false; // Var to keep if we can bodyshot

        IF_GAME(IsTF())
        {
            int ci    = g_pLocalPlayer->weapon()->m_iClassID();
            preferred = hitbox_t::pelvis;
            // Sniper rifle
            if (g_pLocalPlayer->holding_sniper_rifle)
            {
                headonly = CanHeadshot();
                // Hunstman
            }
            else if (ci == CL_CLASS(CTFCompoundBow))
            {
                headonly = true;
                // Ambassador
            }
            else if (IsAmbassador(g_pLocalPlayer->weapon()))
            {
                headonly = AmbassadorCanHeadshot();
                // 18 health is a good number to use as thats the usual minimum
                // damage it can do with a bodyshot, but damage could
                // potentially be higher
                if (target->m_iHealth() <= 18 ||
                    IsPlayerCritBoosted(g_pLocalPlayer->entity))
                    headonly = false;
                // Rocket launcher
            }
            else if (ci == CL_CLASS(CTFRocketLauncher) ||
                     ci == CL_CLASS(CTFRocketLauncher_AirStrike) ||
                     ci == CL_CLASS(CTFRocketLauncher_DirectHit) ||
                     ci == CL_CLASS(CTFRocketLauncher_Mortar))
            {
                preferred = hitbox_t::hip_L;
            }
            // Airborn projectile
            if (GetWeaponMode() == weaponmode::weapon_projectile)
            {
                bool ground = CE_INT(target, netvar.iFlags) & (1 << 0);
                if (!ground)
                {
                    if (g_pLocalPlayer->weapon()->m_iClassID() !=
                        CL_CLASS(CTFCompoundBow))
                    {
                        preferred = hitbox_t::spine_3;
                    }
                }
            }

            // Bodyshot handling
            if (g_pLocalPlayer->holding_sniper_rifle)
            {

                float cdmg = CE_FLOAT(LOCAL_W, netvar.flChargedDamage);
                float bdmg = 50;
                // Darwins damage correction, protects against 15% of damage
                //                if (HasDarwins(target))
                //                {
                //                    bdmg = (bdmg * .85) - 1;
                //                    cdmg = (cdmg * .85) - 1;
                //                }
                // Vaccinator damage correction, protects against 75% of damage
                if (HasCondition<TFCond_UberBulletResist>(target))
                {
                    bdmg = (bdmg * .25) - 1;
                    cdmg = (cdmg * .25) - 1;
                    // Passive bullet resist protects against 10% of damage
                }
                else if (HasCondition<TFCond_SmallBulletResist>(target))
                {
                    bdmg = (bdmg * .90) - 1;
                    cdmg = (cdmg * .90) - 1;
                }
                // Invis damage correction, Invis spies get protection from 10%
                // of damage
                if (IsPlayerInvisible(target))
                {
                    bdmg = (bdmg * .80) - 1;
                    cdmg = (cdmg * .80) - 1;
                }
                // If can headshot and if bodyshot kill from charge damage, or
                // if crit boosted and they have 150 health, or if player isnt
                // zoomed, or if the enemy has less than 40, due to darwins, and
                // only if they have less than 150 health will it try to
                // bodyshot
                if (CanHeadshot() &&
                    (cdmg >= target->m_iHealth() ||
                     IsPlayerCritBoosted(g_pLocalPlayer->entity) ||
                     !g_pLocalPlayer->bZoomed || target->m_iHealth() <= bdmg) &&
                    target->m_iHealth() <= 150)
                {
                    // We dont need to hit the head as a bodyshot will kill
                    preferred = hitbox_t::spine_1;
                    headonly  = false;
                }
            }
            // In counter-strike source, headshots are what we want
        }
        else IF_GAME(IsCSS())
        {
            headonly = true;
        }
        // Head only
        if (headonly)
        {
            IF_GAME(IsTF())
            return hitbox_t::head;
            IF_GAME(IsCSS())
            return 12;
        }
        // If the prefered hitbox vis check passes, use it
        if (target->hitboxes.VisibilityCheck(preferred))
            return preferred;
        // Else attempt to find a hitbox at all
        for (int i = projectile_mode ? 1 : 0;
             i < target->hitboxes.GetNumHitboxes(); i++)
        {
            if (target->hitboxes.VisibilityCheck(i))
                return i;
        }
    }
    break;
    case 1:
    { // AUTO-CLOSEST priority, Return closest hitbox to crosshair
        return ClosestHitbox(target);
    }
    break;
    case 2:
    { // STATIC priority, Return a user chosen hitbox
        return (int) hitbox;
    }
    break;
    default:
        break;
    }
    // Hitbox machine :b:roke
    return -1;
}

// Function to find the closesnt hitbox to the crosshair for a given ent
int ClosestHitbox(CachedEntity *target)
{
    // FIXME this will break multithreading if it will be ever implemented. When
    // implementing it, these should be made non-static
    int closest;
    float closest_fov, fov = 0.0f;

    closest     = -1;
    closest_fov = 256;
    for (int i = 0; i < target->hitboxes.GetNumHitboxes(); i++)
    {
        fov = GetFov(g_pLocalPlayer->v_OrigViewangles, g_pLocalPlayer->v_Eye,
                     target->hitboxes.GetHitbox(i)->center);
        if (fov < closest_fov || closest == -1)
        {
            closest     = i;
            closest_fov = fov;
        }
    }
    return closest;
}

// Function to get predicted visual checks
bool VischeckPredictedEntity(CachedEntity *entity)
{
    // Retrieve predicted data
    AimbotCalculatedData_s &cd = calculated_data_array[entity->m_IDX];
    if (cd.vcheck_tick == tickcount)
        return cd.visible;

    // Update info
    cd.vcheck_tick = tickcount;
    cd.visible     = IsEntityVectorVisible(entity, PredictEntity(entity));
    return cd.visible;
}

// A helper function to find a user angle that isnt directly on the target
// angle, effectively slowing the aiming process
void DoSlowAim(Vector &input_angle)
{
    static float slow_change_dist_p = 0;
    static float slow_change_dist_y = 0;

    // Yaw
    if (g_pUserCmd->viewangles.y != input_angle.y)
    {

        // Check if input angle and user angle are on opposing sides of yaw so
        // we can correct for that
        bool slow_opposing = false;
        if (input_angle.y < -90 && g_pUserCmd->viewangles.y > 90 ||
            input_angle.y > 90 && g_pUserCmd->viewangles.y < -90)
            slow_opposing = true;

        // Direction
        bool slow_dir = false;
        if (slow_opposing)
        {
            if (input_angle.y > 90 && g_pUserCmd->viewangles.y < -90)
                slow_dir = true;
        }
        else if (g_pUserCmd->viewangles.y > input_angle.y)
            slow_dir = true;

        // Speed, check if opposing. We dont get a new distance due to the
        // opposing sides making the distance spike, so just cheap out and reuse
        // our last one.
        if (!slow_opposing)
            slow_change_dist_y =
                std::abs(g_pUserCmd->viewangles.y - input_angle.y) /
                (int) slow_aim;

        // Move in the direction of the input angle
        if (slow_dir)
            input_angle.y = g_pUserCmd->viewangles.y - slow_change_dist_y;
        else
            input_angle.y = g_pUserCmd->viewangles.y + slow_change_dist_y;
    }

    // Pitch
    if (g_pUserCmd->viewangles.x != input_angle.x)
    {
        // Get speed
        slow_change_dist_p =
            std::abs(g_pUserCmd->viewangles.x - input_angle.x) / (int) slow_aim;

        // Move in the direction of the input angle
        if (g_pUserCmd->viewangles.x > input_angle.x)
            input_angle.x = g_pUserCmd->viewangles.x - slow_change_dist_p;
        else
            input_angle.x = g_pUserCmd->viewangles.x + slow_change_dist_p;
    }

    // 0.17 is a good amount in general
    slow_can_shoot = false;
    if (slow_change_dist_y < 0.17 && slow_change_dist_p < 0.17)
        slow_can_shoot = true;

    // Clamp as we changed angles
    fClampAngle(input_angle);
}

// A function that determins whether aimkey allows aiming
bool UpdateAimkey()
{
    static bool aimkey_flip       = false;
    static bool pressed_last_tick = false;
    bool allow_aimkey             = true;

    // Check if aimkey is used
    if (aimkey && aimkey_mode)
    {
        // Grab whether the aimkey is depressed
        bool key_down =
            g_IInputSystem->IsButtonDown((ButtonCode_t)(int) aimkey);
        switch ((int) aimkey_mode)
        {
        case 1: // Only while key is depressed
            if (!key_down)
                allow_aimkey = false;
            break;
        case 2: // Only while key is not depressed, enable
            if (key_down)
                allow_aimkey = false;
            break;
        case 3: // Aimkey acts like a toggle switch
            if (!pressed_last_tick && key_down)
                aimkey_flip = !aimkey_flip;
            if (!aimkey_flip)
                allow_aimkey = false;
            break;
        default:
            break;
        }
        pressed_last_tick = key_down;
    }
    // Return whether the aimkey allows aiming
    return allow_aimkey;
}

// Func to find value of how far to target ents
float EffectiveTargetingRange()
{
    if (GetWeaponMode() == weapon_melee)
        return (float) re::C_TFWeaponBaseMelee::GetSwingRange(RAW_ENT(LOCAL_W));
    if (g_pLocalPlayer->weapon()->m_iClassID() == CL_CLASS(CTFFlameThrower))
        return 185.0f; // Pyros only have so much untill their flames hit

    return (float) max_range;
}

// A function used by gui elements to determine the current target
CachedEntity *CurrentTarget()
{
    if (foundTarget)
        return target; // Doesnt work for some reason

    return nullptr;
}

// Used for when you join and leave maps to reset aimbot vars
void Reset()
{
    target_last     = nullptr;
    projectile_mode = false;
}

#if ENABLE_VISUALS
static CatVar fov_draw(CV_SWITCH, XORSTR("aimbot_fov_draw"), XORSTR("0"), XORSTR("Draw Fov Ring"),
                       XORSTR("Draws a ring to represent your current aimbot fov"));
void DrawText()
{
    // Dont draw to screen when aimbot is disabled
    if (!enabled)
        return;

    // Fov ring to represent when a target will be shot
    if (fov_draw)
    {
        // It cant use fovs greater than 180, so we check for that
        if (float(fov) > 0.0f && float(fov) < 180)
        {
            // Dont show ring while player is dead
            if (LOCAL_E->m_bAlivePlayer())
            {
                rgba_t color = GUIColor();
                color.a      = float(fovcircle_opacity);

                int width, height;
                g_IEngine->GetScreenSize(width, height);

                // Math
                float mon_fov = (float(width) / float(height) / (4.0f / 3.0f));
                float fov_real =
                    RAD2DEG(2 * atanf(mon_fov * tanf(DEG2RAD(draw::fov / 2))));
                float radius = tan(DEG2RAD(float(fov)) / 2) /
                               tan(DEG2RAD(fov_real) / 2) * (width);

                draw_api::draw_circle(width / 2, height / 2, radius, color, 1,
                                      100);
            }
        }
    }
    // Debug stuff
    if (!aimbot_debug)
        return;
    for (int i = 1; i < 32; i++)
    {
        CachedEntity *ent = ENTITY(i);
        if (CE_GOOD(ent))
        {
            Vector screen;
            Vector oscreen;
            if (draw::WorldToScreen(calculated_data_array[i].aim_position,
                                    screen) &&
                draw::WorldToScreen(ent->m_vecOrigin(), oscreen))
            {
                draw_api::draw_rect(screen.x - 2, screen.y - 2, 4, 4,
                                    colors::white);
                draw_api::draw_line(oscreen.x, oscreen.y, screen.x - oscreen.x,
                                    screen.y - oscreen.y, colors::EntityF(ent),
                                    0.5f);
            }
        }
    }
}
#endif
}
}
}
