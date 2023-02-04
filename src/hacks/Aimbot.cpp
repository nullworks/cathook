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
#include <hacks/Backtrack.hpp>
#include <PlayerTools.hpp>
#include <settings/Bool.hpp>
#include "common.hpp"
#include <targethelper.hpp>
#include "MiscTemporary.hpp"
#include "hitrate.hpp"
#include "FollowBot.hpp"
#include "Warp.hpp"
#include "AntiCheatBypass.hpp"

namespace hacks::shared::aimbot
{
static settings::Boolean normal_enable{ "aimbot.enable", "false" };
static settings::Button aimkey{ "aimbot.aimkey.button", "<null>" };
static settings::Int aimkey_mode{ "aimbot.aimkey.mode", "1" };
static settings::Boolean autoshoot{ "aimbot.autoshoot", "1" };
static settings::Boolean autoreload{ "aimbot.autoshoot.activate-heatmaker", "false" };
static settings::Boolean autoshoot_disguised{ "aimbot.autoshoot-disguised", "1" };
static settings::Boolean multipoint{ "aimbot.multipoint", "0" };
static settings::Int vischeck_hitboxes{ "aimbot.vischeck-hitboxes", "0" };
static settings::Int hitbox_mode{ "aimbot.hitbox-mode", "0" };
static settings::Float normal_fov{ "aimbot.fov", "0" };
static settings::Int priority_mode{ "aimbot.priority-mode", "0" };
static settings::Boolean wait_for_charge{ "aimbot.wait-for-charge", "0" };

static settings::Boolean silent{ "aimbot.silent", "1" };
static settings::Boolean target_lock{ "aimbot.lock-target", "false" };
#if ENABLE_VISUALS
static settings::Boolean assistance_only{ "aimbot.assistance.only", "0" };
#endif
static settings::Int hitbox{ "aimbot.hitbox", "0" };
static settings::Boolean zoomed_only{ "aimbot.zoomed-only", "1" };
static settings::Boolean only_can_shoot{ "aimbot.can-shoot-only", "1" };

static settings::Boolean extrapolate{ "aimbot.extrapolate", "0" };
static settings::Int normal_slow_aim{ "aimbot.slow", "0" };
static settings::Int miss_chance{ "aimbot.miss-chance", "0" };

static settings::Boolean projectile_aimbot{ "aimbot.projectile.enable", "true" };
static settings::Float proj_gravity{ "aimbot.projectile.gravity", "0" };
static settings::Float proj_speed{ "aimbot.projectile.speed", "0" };
static settings::Float proj_start_vel{ "aimbot.projectile.initial-velocity", "0" };

static settings::Float sticky_autoshoot{ "aimbot.projectile.sticky-autoshoot", "0.5" };

static settings::Boolean aimbot_debug{ "aimbot.debug", "0" };

static settings::Boolean auto_spin_up{ "aimbot.auto.spin-up", "0" };
static settings::Boolean minigun_tapfire{ "aimbot.auto.tapfire", "false" };
static settings::Boolean auto_zoom{ "aimbot.auto.zoom", "0" };
static settings::Boolean auto_unzoom{ "aimbot.auto.unzoom", "0" };

static settings::Boolean backtrackAimbot{ "aimbot.backtrack", "0" };
static settings::Boolean backtrackLastTickOnly("aimbot.backtrack.only-last-tick", "true");
static bool force_backtrack_aimbot = false;
static settings::Boolean backtrackVischeckAll{ "aimbot.backtrack.vischeck-all", "0" };

// TODO maybe these should be moved into "Targeting"
static settings::Float max_range{ "aimbot.target.max-range", "4096" };
static settings::Boolean ignore_vaccinator{ "aimbot.target.ignore-vaccinator", "1" };
static settings::Boolean ignore_deadringer{ "aimbot.target.ignore-deadringer", "1" };
static settings::Boolean buildings_sentry{ "aimbot.target.sentry", "1" };
static settings::Boolean buildings_other{ "aimbot.target.other-buildings", "1" };
static settings::Boolean npcs{ "aimbot.target.npcs", "1" };
static settings::Boolean stickybot{ "aimbot.target.stickybomb", "0" };
static settings::Boolean rageonly{ "aimbot.target.ignore-non-rage", "0" };
static settings::Int teammates{ "aimbot.target.teammates", "0" };

/*
 * 0 Always on
 * 1 Disable if being spectated in first person
 * 2 Disable if being spectated
 */
static settings::Int specmode("aimbot.spectator-mode", "0");
static settings::Boolean specenable("aimbot.spectator.enable", "0");
static settings::Float specfov("aimbot.spectator.fov", "0");
static settings::Int specslow("aimbot.spectator.slow", "0");

settings::Boolean engine_projpred{ "aimbot.debug.engine-pp", "1" };
struct AimbotCalculatedData_s
{
    unsigned long predict_tick{ 0 };
    bool predict_type{ 0 };
    Vector aim_position{ 0 };
    unsigned long vcheck_tick{ 0 };
    bool visible{ false };
    float fov{ 0 };
    int hitbox{ 0 };
} static cd;
int slow_aim;
float fov;
bool enable;
bool projectile_self_damage = false;
#if ENABLE_VISUALS
static settings::Boolean fov_draw{ "aimbot.fov-circle.enable", "0" };
static settings::Float fovcircle_opacity{ "aimbot.fov-circle.opacity", "0.7" };
#endif

int PreviousX, PreviousY;
int CurrentX, CurrentY;

float last_mouse_check = 0;
float stop_moving_time = 0;

// Used to make rapidfire not knock your enemies out of range
unsigned last_target_ignore_timer = 0;
settings::Boolean ignore_cloak{ "aimbot.target.ignore-cloaked-spies", "1" };
// Projectile info
bool projectile_mode{ false };
float cur_proj_speed{ 0.0f };
float cur_proj_grav{ 0.0f };
float cur_proj_start_vel{ 0.0f };

bool shouldbacktrack_cache = false;
// Func to find value of how far to target ents
inline float EffectiveTargetingRange()
{
    if (GetWeaponMode() == weapon_melee)
        return (float) re::C_TFWeaponBaseMelee::GetSwingRange(RAW_ENT(LOCAL_W));
    else if (LOCAL_W->m_iClassID() == CL_CLASS(CTFFlameThrower))
        return 310.0f; // Pyros only have so much until their flames hit
    else if (LOCAL_W->m_iClassID() == CL_CLASS(CTFWeaponFlameBall))
        return 512.0f; // Dragons Fury is fast but short range

    return (float) max_range;
}
inline bool isHitboxMedium(int hitbox)
{
    switch (hitbox)
    {
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
        return true;

    default:
        return false;
    }

    return false;
}
inline bool playerTeamCheck(CachedEntity *entity)

{
    return (int) teammates == 2 || (entity->m_bEnemy() && !teammates) || (!entity->m_bEnemy() && teammates) || (CE_GOOD(LOCAL_W) && LOCAL_W->m_iClassID() == CL_CLASS(CTFCrossbow) && entity->m_iHealth() < entity->m_iMaxHealth());
}
// Am I holding Hitman's Heatmaker ?
inline bool CarryingHeatmaker()
{
    return CE_INT(LOCAL_W, netvar.iItemDefinitionIndex) == 752;
}
// A function to find the best hitbox for a target
inline int BestHitbox(CachedEntity *target)
{
    // Switch based apon the hitbox mode set by the user
    switch (*hitbox_mode)
    {
    case 0:
        // AUTO priority
        return autoHitbox(target);
        break;
    case 1:
    { // AUTO priority, return closest hitbox to crosshair
        return ClosestHitbox(target);
    }
    break;
    case 2:
    { // STATIC priority, return a user chosen hitbox
        return *hitbox;
    }
    break;
    default:
        break;
    }
    // Hitbox machine :b:roke
    return -1;
}
inline float projectileHitboxSize(int projectile_size)
{
    switch (projectile_size)
    {
    case CL_CLASS(CTFRocketLauncher):
    case CL_CLASS(CTFRocketLauncher_Mortar):
    case CL_CLASS(CTFRocketLauncher_AirStrike):
    case CL_CLASS(CTFRocketLauncher_DirectHit):
    case CL_CLASS(CTFPipebombLauncher):
    case CL_CLASS(CTFGrenadeLauncher):
    case CL_CLASS(CTFCannon):
        return 6.5f;
    case CL_CLASS(CTFFlareGun):
    case CL_CLASS(CTFFlareGun_Revenge):
    case CL_CLASS(CTFDRGPomson):
        return 3.0f;
    case CL_CLASS(CTFSyringeGun):
    case CL_CLASS(CTFCompoundBow):
        return 2.0f;
    default:
        return 3.0f;
    }
}
inline void updateShouldBacktrack()
{
    if (hacks::tf2::backtrack::hasData() || projectile_mode || !(*backtrackAimbot || force_backtrack_aimbot))
        shouldbacktrack_cache = false;
    else
        shouldbacktrack_cache = true;
}

inline bool shouldBacktrack(CachedEntity *ent)
{
    if (!shouldbacktrack_cache)
        return false;
    else if (ent && ent->m_Type() != ENTITY_PLAYER)
        return false;
    else if (!tf2::backtrack::getGoodTicks(ent))
        return false;
    return true;
}
void spectatorUpdate()
{
    switch (*specmode)
    {
    // Always on
    default:
    case 0:
        break;
        // Disable if being spectated in first person
    case 1:
    {
        if (g_pLocalPlayer->spectator_state == g_pLocalPlayer->FIRSTPERSON)
        {
            enable   = *specenable;
            slow_aim = *specslow;
            fov      = *specfov;
        }
        break;
    }
        // Disable if being spectated
    case 2:
    {
        if (g_pLocalPlayer->spectator_state != g_pLocalPlayer->NONE)
        {
            enable   = *specenable;
            slow_aim = *specslow;
            fov      = *specfov;
        }
    };
    }
}

#define GET_MIDDLE(c1, c2) (corners[c1] + corners[c2]) / 2.0f

// Get all the valid aim positions
std::vector<Vector> getValidHitpoints(CachedEntity *ent, int hitbox)
{
    // Recorded vischeckable points
    std::vector<Vector> hitpoints;
    auto hb = ent->hitboxes.GetHitbox(hitbox);

    trace_t trace;

    if (IsEntityVectorVisible(ent, hb->center, true, MASK_SHOT_HULL, &trace, true))
    {
        if (trace.hitbox == hitbox)
            hitpoints.push_back(hb->center);
    }
    // Multipoint
    auto bboxmin = hb->bbox->bbmin;
    auto bboxmax = hb->bbox->bbmax;

    auto transform = ent->hitboxes.GetBones()[hb->bbox->bone];
    QAngle rotation;
    Vector origin;

    MatrixAngles(transform, rotation, origin);

    Vector corners[8];
    GenerateBoxVertices(origin, rotation, bboxmin, bboxmax, corners);

    float shrink_size = 1;

    if (!isHitboxMedium(hitbox)) // hitbox should be chosen based on size.
        shrink_size = 3;
    else
        shrink_size = 6;

    // Shrink positions by moving towards opposing corner
    for (int i = 0; i < 8; ++i)
        corners[i] += (corners[7 - i] - corners[i]) / shrink_size;

    // Generate middle points on line segments
    // Define cleans up code

    const Vector line_positions[12] = { GET_MIDDLE(0, 1), GET_MIDDLE(0, 2), GET_MIDDLE(1, 3), GET_MIDDLE(2, 3), GET_MIDDLE(7, 6), GET_MIDDLE(7, 5), GET_MIDDLE(6, 4), GET_MIDDLE(5, 4), GET_MIDDLE(0, 4), GET_MIDDLE(1, 5), GET_MIDDLE(2, 6), GET_MIDDLE(3, 7) };

    // Create combined vector
    std::vector<Vector> positions;

    positions.reserve(sizeof(Vector) * 20);
    positions.insert(positions.end(), corners, &corners[8]);
    positions.insert(positions.end(), line_positions, &line_positions[12]);

    for (int i = 0; i < 20; ++i)
    {
        trace_t trace;

        if (IsEntityVectorVisible(ent, positions[i], true, MASK_SHOT_HULL, &trace, true))
        {
            if (trace.hitbox == hitbox)
                hitpoints.push_back(positions[i]);
        }
    }
    if (*vischeck_hitboxes)
    {
        if (*vischeck_hitboxes == 1 && playerlist::AccessData(ent).state != playerlist::k_EState::RAGE)
        {
            return hitpoints;
        }
        int i                  = 0;
        const u_int8_t max_box = ent->hitboxes.GetNumHitboxes();
        while (hitpoints.empty() && i < max_box) // Prevents returning empty at all costs. Loops through every hitbox
        {
            if (hitbox == i)
            {
                ++i;
                continue;
            }
            hitpoints = getHitpointsVischeck(ent, i);
            ++i;
        }
    }

    return hitpoints;
}
std::vector<Vector> getHitpointsVischeck(CachedEntity *ent, int hitbox)
{
    std::vector<Vector> hitpoints;
    auto hb      = ent->hitboxes.GetHitbox(hitbox);
    auto bboxmin = hb->bbox->bbmin;
    auto bboxmax = hb->bbox->bbmax;

    auto transform = ent->hitboxes.GetBones()[hb->bbox->bone];
    QAngle rotation;
    Vector origin;

    MatrixAngles(transform, rotation, origin);

    Vector corners[8];
    GenerateBoxVertices(origin, rotation, bboxmin, bboxmax, corners);

    float shrink_size = 1;

    if (!isHitboxMedium(hitbox)) // hitbox should be chosen based on size.
        shrink_size = 3;
    else
        shrink_size = 6;

    // Shrink positions by moving towards opposing corner
    for (int i = 0; i < 8; ++i)
        corners[i] += (corners[7 - i] - corners[i]) / shrink_size;

    // Generate middle points on line segments
    // Define cleans up code

    const Vector line_positions[12] = { GET_MIDDLE(0, 1), GET_MIDDLE(0, 2), GET_MIDDLE(1, 3), GET_MIDDLE(2, 3), GET_MIDDLE(7, 6), GET_MIDDLE(7, 5), GET_MIDDLE(6, 4), GET_MIDDLE(5, 4), GET_MIDDLE(0, 4), GET_MIDDLE(1, 5), GET_MIDDLE(2, 6), GET_MIDDLE(3, 7) };

    // Create combined vector
    std::vector<Vector> positions;

    positions.reserve(sizeof(Vector) * 20);
    positions.insert(positions.end(), corners, &corners[8]);
    positions.insert(positions.end(), line_positions, &line_positions[12]);

    for (int i = 0; i < 20; ++i)
    {
        trace_t trace;

        if (IsEntityVectorVisible(ent, positions[i], true, MASK_SHOT_HULL, &trace, true))
        {
            if (trace.hitbox == hitbox)
                hitpoints.push_back(positions[i]);
        }
    }

    return hitpoints;
}
// Get the best point to aim at for a given hitbox
std::optional<Vector> getBestHitpoint(CachedEntity *ent, int hitbox)
{
    auto positions = getValidHitpoints(ent, hitbox);

    std::optional<Vector> best_pos = std::nullopt;
    float max_score                = FLT_MAX;
    for (auto const &position : positions)
    {
        float score = GetFov(g_pLocalPlayer->v_OrigViewangles, g_pLocalPlayer->v_Eye, position);
        if (score < max_score)
        {
            best_pos  = position;
            max_score = score;
        }
    }

    return best_pos;
}

// Reduce Backtrack lag by checking if the ticks hitboxes are within a reasonable FOV range
bool validateTickFOV(tf2::backtrack::BacktrackData &tick)
{
    if (fov)
    {
        bool valid_fov = false;
        for (auto &hitbox : tick.hitboxes)
        {
            float score = GetFov(g_pLocalPlayer->v_OrigViewangles, g_pLocalPlayer->v_Eye, hitbox.center);
            // Check if the FOV is within a 2.0f threshhold
            if (score < fov + 2.0f)
            {
                valid_fov = true;
                break;
            }
        }
        return valid_fov;
    }
    return true;
}

void doAutoZoom(bool target_found)
{
    bool isIdle = target_found ? false : hacks::shared::followbot::isIdle();

    // Keep track of our zoom time
    static Timer zoomTime{};

    // Minigun spun up handler
    if (auto_spin_up && g_pLocalPlayer->weapon()->m_iClassID() == CL_CLASS(CTFMinigun))
    {
        if (target_found)
            zoomTime.update();
        if (isIdle || !zoomTime.check(3000))
            current_user_cmd->buttons |= IN_ATTACK2;
        return;
    }

    if (auto_zoom && g_pLocalPlayer->holding_sniper_rifle && (target_found || isIdle))
    {
        if (target_found)
            zoomTime.update();
        if (not g_pLocalPlayer->bZoomed)
            current_user_cmd->buttons |= IN_ATTACK2;
    }
    else if (!target_found)
    {
        // Auto-Unzoom
        if (auto_unzoom)
            if (g_pLocalPlayer->holding_sniper_rifle && g_pLocalPlayer->bZoomed && zoomTime.check(3000))
                current_user_cmd->buttons |= IN_ATTACK2;
    }
}

// Current Entity
CachedEntity *target_last = 0;
bool aimed_this_tick      = false;
Vector viewangles_this_tick(0.0f);

// If slow aimbot allows autoshoot
bool slow_can_shoot = false;
bool projectileAimbotRequired;

// This array will store calculated projectile/hitscan predictions
// for current frame, to avoid performing them again

// The main "loop" of the aimbot.
static void CreateMove()
{
    enable          = *normal_enable;
    slow_aim        = *normal_slow_aim;
    fov             = *normal_fov;
    aimed_this_tick = false;

    bool aimkey_status = UpdateAimkey();

    if (*specmode != 0)
        spectatorUpdate();
    if (!enable)
    {
        target_last = nullptr;
        return;
    }
    else if (!LOCAL_E->m_bAlivePlayer() || !g_pLocalPlayer->entity)
    {
        target_last = nullptr;
        return;
    }
    else if (!aimkey_status || !ShouldAim())
    {
        target_last = nullptr;
        return;
    }

    doAutoZoom(false);
    if (hacks::tf2::antianticheat::enabled)
        fov = std::min(fov > 0.0f ? fov : FLT_MAX, 10.0f);
    bool should_backtrack    = hacks::tf2::backtrack::backtrackEnabled();
    int get_weapon_mode      = g_pLocalPlayer->weapon_mode;
    projectile_mode          = false;
    projectileAimbotRequired = false;
    bool should_zoom         = *auto_zoom;
    switch (get_weapon_mode)
    {
    case weapon_hitscan:
    {
        if (should_backtrack)
            updateShouldBacktrack();
        target_last = RetrieveBestTarget(aimkey_status);
        if (target_last)
        {
            if (should_zoom)
                doAutoZoom(true);
            int weapon_case = LOCAL_W->m_iClassID();
            if (!hitscanSpecialCases(target_last, weapon_case))
                DoAutoshoot(target_last);
        }
        break;
    }
    case weapon_melee:
    {
        if (should_backtrack)
            updateShouldBacktrack();
        target_last = RetrieveBestTarget(aimkey_status);
        if (target_last)
        {
            DoAutoshoot(target_last);
        }
        break;
    }
    case weapon_projectile:
    case weapon_throwable:
    {
        if (projectile_aimbot)
        {
            projectileAimbotRequired = true;
            projectile_mode          = GetProjectileData(g_pLocalPlayer->weapon(), cur_proj_speed, cur_proj_grav, cur_proj_start_vel);
            if (!projectile_mode)
            {
                target_last = nullptr;
                return;
            }
            if (proj_speed)
                cur_proj_speed = *proj_speed;
            if (proj_gravity)
                cur_proj_grav = *proj_gravity;
            if (proj_start_vel)
                cur_proj_start_vel = *proj_start_vel;
            target_last = RetrieveBestTarget(aimkey_status);
            if (target_last)
            {
                int weapon_case = g_pLocalPlayer->weapon()->m_iClassID();
                if (projectileSpecialCases(target_last, weapon_case))
                {

                    DoAutoshoot(target_last);
                }
            }
        }
        break;
    }
    }
}

bool projectileSpecialCases(CachedEntity *target_entity, int weapon_case)
{

    switch (weapon_case)
    {
    case CL_CLASS(CTFCompoundBow):
    {
        bool release = false;
        if (autoshoot)
            current_user_cmd->buttons |= IN_ATTACK;
        // Grab time when charge began
        float begincharge = CE_FLOAT(g_pLocalPlayer->weapon(), netvar.flChargeBeginTime);
        float charge      = g_GlobalVars->curtime - begincharge;
        if (!begincharge)
            charge = 0.0f;
        int damage        = std::floor(50.0f + 70.0f * fminf(1.0f, charge));
        int charge_damage = std::floor(50.0f + 70.0f * fminf(1.0f, charge)) * 3.0f;
        if (HasCondition<TFCond_Slowed>(LOCAL_E) && (autoshoot || !(current_user_cmd->buttons & IN_ATTACK)) && (!wait_for_charge || (charge >= 1.0f || damage >= target_entity->m_iHealth() || charge_damage >= target_entity->m_iHealth())))
            release = true;
        return release;
        break;
    }
    case CL_CLASS(CTFCannon):
    {
        bool release = false;
        if (autoshoot)
            current_user_cmd->buttons |= IN_ATTACK;
        float detonate_time = CE_FLOAT(LOCAL_W, netvar.flDetonateTime);
        // Currently charging up
        if (detonate_time > g_GlobalVars->curtime)
        {
            if (wait_for_charge)
            {
                // Shoot when a straight shot would result in only 100ms left on fuse upon target hit
                float best_charge = PredictEntity(target_entity).DistTo(g_pLocalPlayer->v_Eye) / cur_proj_speed + 0.1;
                if (detonate_time - g_GlobalVars->curtime <= best_charge)
                    release = true;
            }
            else
                release = true;
        }
        return release;
        break;
    }
    case CL_CLASS(CTFPipebombLauncher):
    {
        float chargebegin = CE_FLOAT(LOCAL_W, netvar.flChargeBeginTime);
        float chargetime  = g_GlobalVars->curtime - chargebegin;

        DoAutoshoot();
        bool currently_charging_pipe = false;

        // Grenade started charging
        if (chargetime < 6.0f && chargetime && chargebegin)
            currently_charging_pipe = true;

        // Grenade was released
        if (!(current_user_cmd->buttons & IN_ATTACK) && currently_charging_pipe)
        {
            currently_charging_pipe = false;
            Aim(target_entity);
            return false;
        }
        break;
    }
    default:
        return true;
    }
    return true;
}
int tapfire_delay = 0;
bool hitscanSpecialCases(CachedEntity *target_entity, int weapon_case)
{
    switch (weapon_case)
    {
    case CL_CLASS(CTFMinigun):
    {
        if (!minigun_tapfire)
            DoAutoshoot(target_entity);
        else
        {
            // Used to keep track of what tick we're in right now
            tapfire_delay++;

            // This is the exact delay needed to hit
            if (17 <= tapfire_delay || target_entity->m_flDistance() <= 1250.0f)
            {
                DoAutoshoot(target_entity);
                tapfire_delay = 0;
            }
            return true;
        }
        return true;
        break;
    }
    default:
        return false;
    }
}
// Just hold m1 if we were aiming at something before and are in rapidfire
static void CreateMoveWarp()
{
    if (hacks::tf2::warp::in_rapidfire && aimed_this_tick)
    {
        current_user_cmd->viewangles     = viewangles_this_tick;
        g_pLocalPlayer->bUseSilentAngles = *silent;
        current_user_cmd->buttons |= IN_ATTACK;
    }
    // Warp should call aimbot normally
    else if (!hacks::tf2::warp::in_rapidfire)
        CreateMove();
}

#if ENABLE_VISUALS
bool MouseMoving()
{
    if ((g_GlobalVars->curtime - last_mouse_check) < 0.02)
    {
        SDL_GetMouseState(&PreviousX, &PreviousY);
    }
    else
    {
        SDL_GetMouseState(&CurrentX, &CurrentY);
        last_mouse_check = g_GlobalVars->curtime;
    }

    if (PreviousX != CurrentX || PreviousY != CurrentY)
        stop_moving_time = g_GlobalVars->curtime + 0.5;

    if (g_GlobalVars->curtime <= stop_moving_time)
        return true;
    else
        return false;
}
#endif

// The first check to see if the player should aim in the first place
bool ShouldAim()
{
    // Checks should be in order: cheap -> expensive

    // Check for +use
    if (current_user_cmd->buttons & IN_USE)
        return false;
    // Check if using action slot item
    else if (g_pLocalPlayer->using_action_slot_item)
        return false;
    // Using a forbidden weapon?
    else if (!g_pLocalPlayer->weapon() || g_pLocalPlayer->weapon()->m_iClassID() == CL_CLASS(CTFKnife) || CE_INT(LOCAL_W, netvar.iItemDefinitionIndex) == 237 || CE_INT(LOCAL_W, netvar.iItemDefinitionIndex) == 265)
        return false;

    // Carrying A building?
    else if (CE_BYTE(g_pLocalPlayer->entity, netvar.m_bCarryingObject))
        return false;
    // Deadringer out?
    else if (CE_BYTE(g_pLocalPlayer->entity, netvar.m_bFeignDeathReady))
        return false;
    else if (g_pLocalPlayer->holding_sapper)
        return false;
    // Is bonked?
    else if (HasCondition<TFCond_Bonked>(g_pLocalPlayer->entity))
        return false;
    // Is taunting?
    else if (HasCondition<TFCond_Taunting>(g_pLocalPlayer->entity))
        return false;
    // Is cloaked
    else if (IsPlayerInvisible(g_pLocalPlayer->entity))
        return false;
    else if (LOCAL_W->m_iClassID() == CL_CLASS(CTFMinigun) && CE_INT(LOCAL_E, netvar.m_iAmmo + 4) == 0)
        return false;
#if ENABLE_VISUALS
    if (assistance_only && !MouseMoving())
        return false;
#endif

    return true;
}

// Function to find a suitable target
CachedEntity *RetrieveBestTarget(bool aimkey_state)
{
    // If we have a previously chosen target, target lock is on, and the aimkey
    // is allowed, then attemt to keep the previous target

    if (target_lock && target_last && aimkey_state)
    {

        if (shouldBacktrack(target_last))
        {
            auto good_ticks_tmp = hacks::tf2::backtrack::getGoodTicks(target_last);
            if (good_ticks_tmp)
            {
                auto good_ticks = *good_ticks_tmp;
                if (backtrackLastTickOnly)
                {
                    good_ticks.clear();
                    good_ticks.push_back(good_ticks_tmp->back());
                }
                for (auto &bt_tick : good_ticks)
                {
                    if (!validateTickFOV(bt_tick))
                        continue;
                    hacks::tf2::backtrack::MoveToTick(bt_tick);
                    if (IsTargetStateGood(target_last) && Aim(target_last))
                        return target_last;
                    // Restore if bad target
                    hacks::tf2::backtrack::RestoreEntity(target_last->m_IDX);
                }
            }

            // Check if previous target is still good
            else if (!shouldbacktrack_cache && IsTargetStateGood(target_last) && Aim(target_last))
            {
                // If it is then return it again
                return target_last;
            }
        }
    }
    // No last_target found, reset the timer.
    hacks::shared::aimbot::last_target_ignore_timer = 0;

    float target_highest_score, scr = 0.0f;
    CachedEntity *target_highest_ent                            = nullptr;
    target_highest_score                                        = -256;
    std::optional<hacks::tf2::backtrack::BacktrackData> bt_tick = std::nullopt;
    for (auto const &ent : entity_cache::valid_ents)
    {
        // Check for null and dormant
        // Check whether the current ent is good enough to target
        bool isTargetGood = false;

        static std::optional<hacks::tf2::backtrack::BacktrackData> temp_bt_tick = std::nullopt;
        if (shouldBacktrack(ent))
        {
            auto good_ticks_tmp = tf2::backtrack::getGoodTicks(ent);
            if (good_ticks_tmp)
            {
                auto good_ticks = *good_ticks_tmp;
                if (backtrackLastTickOnly)
                {
                    good_ticks.clear();
                    good_ticks.push_back(good_ticks_tmp->back());
                }
                for (auto &bt_tick : good_ticks)
                {
                    if (!validateTickFOV(bt_tick))
                        continue;
                    hacks::tf2::backtrack::MoveToTick(bt_tick);
                    if (IsTargetStateGood(ent) && Aim(ent))
                    {
                        isTargetGood = true;
                        temp_bt_tick = bt_tick;
                        break;
                    }
                    hacks::tf2::backtrack::RestoreEntity(ent->m_IDX);
                }
            }
        }
        else
        {
            if (IsTargetStateGood(ent) && Aim(ent))
                isTargetGood = true;
        }
        if (isTargetGood) // Melee mode straight up won't swing if the target is too far away. No need to prioritize based on distance. Just use whatever the user chooses.
        {
            switch ((int) priority_mode)
            {
            case 0: // Smart Priority
            {
                scr = GetScoreForEntity(ent);
                break;
            }
            case 1: // Fov Priority
            {
                scr = 360.0f - cd.fov;
                break;
            }
            case 2:
            {
                scr = 4096.0f - cd.aim_position.DistTo(g_pLocalPlayer->v_Eye);
                break;
            }
            case 3: // Health Priority (Lowest)
            {
                scr = 450.0f - ent->m_iHealth();
                break;
            }
            case 4: // Distance Priority (Furthest Away)
            {
                scr = cd.aim_position.DistTo(g_pLocalPlayer->v_Eye);
                break;
            }
            case 5: // Health Priority (Highest)
            {
                scr = ent->m_iHealth() * 4;
                break;
            }
            case 6: // Fast
            {

                return ent;
            }
            default:
                break;
            }
            // Crossbow logic
            if (!ent->m_bEnemy() && ent->m_Type() == ENTITY_PLAYER && CE_GOOD(LOCAL_W) && LOCAL_W->m_iClassID() == CL_CLASS(CTFCrossbow))
            {
                scr = ((ent->m_iMaxHealth() - ent->m_iHealth()) / ent->m_iMaxHealth()) * (*priority_mode == 2 ? 16384.0f : 2000.0f);
            }
            // Compare the top score to our current ents score
            if (scr > target_highest_score)
            {
                target_highest_score = scr;
                target_highest_ent   = ent;
                bt_tick              = temp_bt_tick;
            }
        }

        // Restore tick
        if (shouldBacktrack(ent))
            hacks::tf2::backtrack::RestoreEntity(ent->m_IDX);
    }
    if (target_highest_ent && bt_tick)
        hacks::tf2::backtrack::MoveToTick(*bt_tick);
    return target_highest_ent;
}

// A second check to determine whether a target is good enough to be aimed at
bool IsTargetStateGood(CachedEntity *entity)
{
    PROF_SECTION(PT_aimbot_targetstatecheck);

    const int current_type = entity->m_Type();
    bool is_player         = false;
    switch (current_type)
    {

    case (ENTITY_PLAYER):
    {
        // Local player check
        if (entity == LOCAL_E)
            return false;
        // Dead
        else if (!entity->m_bAlivePlayer())
            return false;
        // Teammates
        else if (!playerTeamCheck(entity))
            return false;
        else if (!player_tools::shouldTarget(entity))
            return false;
        // Invulnerable players, ex: uber, bonk
        else if (IsPlayerInvulnerable(entity))
            return false;
        // Distance
        is_player             = true;
        float targeting_range = EffectiveTargetingRange();
        if (entity->m_flDistance() - 40 > targeting_range && tickcount > hacks::shared::aimbot::last_target_ignore_timer) // m_flDistance includes the collision box. You have to subtract it (Should be the same for every model)
            return false;

        // Rage only check
        if (rageonly)
        {
            if (playerlist::AccessData(entity).state != playerlist::k_EState::RAGE)
            {
                return false;
            }
        }

        // Wait for charge
        if (wait_for_charge && g_pLocalPlayer->holding_sniper_rifle)
        {
            float cdmg  = CE_FLOAT(LOCAL_W, netvar.flChargedDamage) * 3;
            float maxhs = 450.0f;
            if (CE_INT(LOCAL_W, netvar.iItemDefinitionIndex) == 230 || HasCondition<TFCond_Jarated>(entity))
            {
                cdmg  = int(CE_FLOAT(LOCAL_W, netvar.flChargedDamage) * 1.35f);
                maxhs = 203.0f;
            }
            bool maxCharge = cdmg >= maxhs;

            // Darwins damage correction, Darwins protects against 15% of
            // damage
            //                if (HasDarwins(entity))
            //                    cdmg = (cdmg * .85) - 1;
            // Vaccinator damage correction, Vac charge protects against 75%
            // of damage
            if (IsPlayerInvisible(entity))
                cdmg = (cdmg * .80) - 1;

            else if (HasCondition<TFCond_UberBulletResist>(entity))
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

            // Check if player will die from headshot or if target has more
            // than 450 health and sniper has max chage
            float hsdmg = 150.0f;
            if (CE_INT(LOCAL_W, netvar.iItemDefinitionIndex) == 230)
                hsdmg = int(50.0f * 1.35f);

            int health = entity->m_iHealth();
            if (!(health <= hsdmg || health <= cdmg || !g_pLocalPlayer->bZoomed || (maxCharge && health > maxhs)))
            {
                return false;
            }
        }

        // Some global checks

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
                    if (ignore_cloak && !(HasCondition<TFCond_OnFire>(entity)) && !(HasCondition<TFCond_CloakFlicker>(entity)))
                        return false;
                }
            }
        }
        // Vaccinator
        if (ignore_vaccinator && IsPlayerResistantToCurrentWeapon(entity))
            return false;

        cd.hitbox = BestHitbox(entity);
        if (*vischeck_hitboxes && !*multipoint && is_player)
        {
            if (*vischeck_hitboxes == 1 && playerlist::AccessData(entity).state != playerlist::k_EState::RAGE || (projectileAimbotRequired && 0.01f < cur_proj_grav))
            {
                return true;
            }

            else
            {
                int i = 0;
                trace_t first_tracer;

                if (IsEntityVectorVisible(entity, entity->hitboxes.GetHitbox(cd.hitbox)->center, true, MASK_SHOT_HULL, &first_tracer, true))
                    return true;

                const u_int8_t max_box = entity->hitboxes.GetNumHitboxes();
                while (i < max_box) // Prevents returning empty at all costs. Loops through every hitbox
                {
                    if (i == cd.hitbox)
                    {
                        ++i;
                        continue;
                    }
                    trace_t test_trace;

                    Vector centered_hitbox = entity->hitboxes.GetHitbox(i)->center;

                    if (IsEntityVectorVisible(entity, centered_hitbox, true, MASK_SHOT_HULL, &test_trace, true))
                    {
                        cd.hitbox = i;
                        return true;
                    }
                    ++i;
                }
                return false; // It looped through every hitbox and found nothing. It isn't visible.
            }
        }
        return true;
        break;
    }
    // Check for buildings
    case (ENTITY_BUILDING):
    {
        // Enabled check
        if (!(buildings_other || buildings_sentry))
            return false;
        // Teammates, Even with friendly fire enabled, buildings can NOT be
        // damaged
        else if (!entity->m_bEnemy())
            return false;
        // Distance
        else if (EffectiveTargetingRange())
        {
            if (entity->m_flDistance() - 40 > EffectiveTargetingRange() && tickcount > hacks::shared::aimbot::last_target_ignore_timer)
                return false;
        }

        // Building type
        else if (!(buildings_other && buildings_sentry))
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

        // Vis and fov check
        return true;
    }
    case (ENTITY_NPC):
    {
        // NPCs (Skeletons, Merasmus, etc)

        // NPC targeting is disabled
        if (!npcs)
            return false;

        // Cannot shoot this
        else if (entity->m_iTeam() == LOCAL_E->m_iTeam())
            return false;

        // Distance
        float targeting_range = EffectiveTargetingRange();

        if (entity->m_flDistance() - 40 > targeting_range && tickcount > hacks::shared::aimbot::last_target_ignore_timer)
            return false;

        // Grab the prediction var

        return true;
        break;
    }
    default:
        break;
    }
    // Check for stickybombs
    if (entity->m_iClassID() == CL_CLASS(CTFGrenadePipebombProjectile))
    {
        // Enabled
        if (!stickybot)
            return false;

        // Only hitscan weapons can break stickys so check for them.
        else if (!(GetWeaponMode() == weapon_hitscan || GetWeaponMode() == weapon_melee))
            return false;

        // Distance
        float targeting_range = EffectiveTargetingRange();
        if (entity->m_flDistance() > targeting_range)
            return false;

        // Teammates, Even with friendly fire enabled, stickys can NOT be
        // destroied
        if (!entity->m_bEnemy())
            return false;

        // Check if target is a pipe bomb
        if (CE_INT(entity, netvar.iPipeType) != 1)
            return false;

        // Moving Sticky?
        Vector velocity;
        velocity::EstimateAbsVelocity(RAW_ENT(entity), velocity);
        if (!velocity.IsZero())
            return false;

        // Grab the prediction var

        // Vis and fov check

        return true;
    }

    return false;
}

// A function to aim at a specific entitiy
bool Aim(CachedEntity *entity)
{
    if (*miss_chance > 0 && UniformRandomInt(0, 99) < *miss_chance)
        return true;

    // Get angles from eye to target
    Vector is_it_good = PredictEntity(entity);
    if (!projectileAimbotRequired)
        if (!IsEntityVectorVisible(entity, is_it_good, true, MASK_SHOT_HULL, nullptr, true))
            return false;

    Vector angles = GetAimAtAngles(g_pLocalPlayer->v_Eye, is_it_good, LOCAL_E);

    if (projectileAimbotRequired) // unfortunately you have to check this twice, otherwise you'd have to run GetAimAtAngles far too early
        if (!didProjectileHit(getShootPos(angles), is_it_good, entity, projectileHitboxSize(LOCAL_W->m_iClassID()), (0.01f < cur_proj_grav)))
            return false;
    if (fov > 0 && cd.fov > fov)
        return false;
    // Slow aim
    if (slow_aim)
        DoSlowAim(angles);

#if ENABLE_VISUALS
    if (entity->m_Type() == ENTITY_PLAYER)
        hacks::shared::esp::SetEntityColor(entity, colors::target);
#endif
    // Set angles
    current_user_cmd->viewangles = angles;

    if (silent && !slow_aim)
        g_pLocalPlayer->bUseSilentAngles = true;
    // Set tick count to targets (backtrack messes with this)
    if (!shouldBacktrack(entity) && nolerp && entity->m_IDX <= g_IEngine->GetMaxClients())
        current_user_cmd->tick_count = TIME_TO_TICKS(CE_FLOAT(entity, netvar.m_flSimulationTime));
    aimed_this_tick      = true;
    viewangles_this_tick = angles;
    // Finish function
    return true;
}

// A function to check whether player can autoshoot
bool begancharge = false;
int begansticky  = 0;
void DoAutoshoot(CachedEntity *target_entity)
{
    // Enable check
    if (!autoshoot)
        return;
    else if (IsPlayerDisguised(g_pLocalPlayer->entity) && !autoshoot_disguised)
        return;
    // Handle Huntsman/Loose cannon
    else if (g_pLocalPlayer->weapon()->m_iClassID() == CL_CLASS(CTFCompoundBow) || g_pLocalPlayer->weapon()->m_iClassID() == CL_CLASS(CTFCannon))
    {
        if (!only_can_shoot)
        {
            if (!begancharge)
            {
                current_user_cmd->buttons |= IN_ATTACK;
                begancharge = true;
                return;
            }
        }
        begancharge = false;
        current_user_cmd->buttons &= ~IN_ATTACK;
        hacks::shared::antiaim::SetSafeSpace(5);
        return;
    }
    else
        begancharge = false;
    if (g_pLocalPlayer->weapon()->m_iClassID() == CL_CLASS(CTFPipebombLauncher))
    {
        float chargebegin = CE_FLOAT(LOCAL_W, netvar.flChargeBeginTime);
        float chargetime  = g_GlobalVars->curtime - chargebegin;

        // Release Sticky if > chargetime, 3.85 is the max second chargetime,
        // but we also need to consider the release time supplied by the user
        if ((chargetime >= 3.85f * *sticky_autoshoot) && begansticky > 3)
        {
            current_user_cmd->buttons &= ~IN_ATTACK;
            hacks::shared::antiaim::SetSafeSpace(5);
            begansticky = 0;
        }
        // Else just keep charging
        else
        {
            current_user_cmd->buttons |= IN_ATTACK;
            begansticky++;
        }
        return;
    }
    else
        begansticky = 0;
    bool attack = true;

    // Rifle check

    if (g_pLocalPlayer->clazz == tf_class::tf_sniper)
    {
        if (g_pLocalPlayer->holding_sniper_rifle)
        {
            if (zoomed_only && !CanHeadshot())
                attack = false;
        }
    }

    // Ambassador check

    else if (IsAmbassador(g_pLocalPlayer->weapon()))
    {
        // Check if ambasador can headshot
        if (!AmbassadorCanHeadshot() && wait_for_charge)
            attack = false;
    }

    // Autoshoot breaks with Slow aimbot, so use a workaround to detect when it
    // can
    else if (slow_aim && !slow_can_shoot)
        attack = false;

    // Dont autoshoot without anything in clip
    else if (CE_INT(g_pLocalPlayer->weapon(), netvar.m_iClip1) == 0)
        attack = false;

    if (attack)
    {
        // TO DO: Sending both reload and attack will activate the hitmans heatmaker ability
        // Don't activate it only on first kill (or somehow activate it before a shot)
        current_user_cmd->buttons |= IN_ATTACK | (*autoreload && CarryingHeatmaker() ? IN_RELOAD : 0);
        if (target_entity)
        {
            auto hitbox = cd.hitbox;
            hitrate::AimbotShot(target_entity->m_IDX, hitbox != head);
        }
        *bSendPackets = true;
    }
    if (LOCAL_W->m_iClassID() == CL_CLASS(CTFLaserPointer))
        current_user_cmd->buttons |= IN_ATTACK2;
}

// Grab a vector for a specific ent
Vector PredictEntity(CachedEntity *entity)
{
    // Pull out predicted data
    Vector &result            = cd.aim_position;
    const short int curr_type = entity->m_Type();

    // Players

    // If using projectiles, predict a vector
    switch (curr_type)
    {
    case ENTITY_PLAYER:
    {
        if (projectileAimbotRequired)
        {
            std::pair<Vector, Vector> tmp_result;
            // Use prediction engine if user settings allow
            if (engine_projpred)
                tmp_result = ProjectilePrediction_Engine(entity, cd.hitbox, cur_proj_speed, cur_proj_grav, PlayerGravityMod(entity), cur_proj_start_vel);
            else
                tmp_result = ProjectilePrediction(entity, cd.hitbox, cur_proj_speed, cur_proj_grav, PlayerGravityMod(entity), cur_proj_start_vel);

            // Don't use the intial velocity compensated one in vischecks
            result = tmp_result.second;
        }
        else
        {
            // If using extrapolation, then predict a vector
            if (extrapolate)
                result = SimpleLatencyPrediction(entity, cd.hitbox);
            // else just grab strait from the hitbox
            else
            {
                // Allow multipoint logic to run
                if (!*multipoint)
                {
                    result = entity->hitboxes.GetHitbox(cd.hitbox)->center;
                    break;
                }

                std::optional<Vector> best_pos = getBestHitpoint(entity, cd.hitbox);
                if (best_pos)
                    result = *best_pos;
            }
        }
        break;
    }
        // Buildings
    case ENTITY_BUILDING:
    {
        if (cur_proj_grav != 0)
        {
            std::pair<Vector, Vector> temp_result = BuildingPrediction(entity, GetBuildingPosition(entity), cur_proj_speed, cur_proj_grav, cur_proj_start_vel);
            result                                = temp_result.second;
        }
        else
            result = GetBuildingPosition(entity);
        break;
    }
        // NPCs (Skeletons, merasmus, etc)
    case ENTITY_NPC:
    {
        result = entity->hitboxes.GetHitbox(std::max(0, entity->hitboxes.GetNumHitboxes() / 2 - 1))->center;
        break;
    }

        // Other
    default:
    {
        result = entity->m_vecOrigin();
        break;
    }
    }
    cd.predict_tick = tickcount;
    cd.fov          = GetFov(g_pLocalPlayer->v_OrigViewangles, g_pLocalPlayer->v_Eye, result);

    // Return the found vector
    return result;
}
int notVisibleHitbox(CachedEntity *target, int preferred)
{
    if (target->hitboxes.VisibilityCheck(preferred))
        return preferred;
    // Else attempt to find any hitbox at all
    else
        return hitbox_t::spine_1;
}
int autoHitbox(CachedEntity *target)
{

    int preferred     = 3;
    int target_health = target->m_iHealth(); // This was used way too many times. Due to how pointers work (defrencing)+the compiler already dealing with tons of AIDS global variables it likely derefrenced it every time it was called.
    int ci            = LOCAL_W->m_iClassID();

    if (CanHeadshot()) // Nothing else zooms in this game you have to be holding a rifle for this to be true.
    {
        float cdmg = CE_FLOAT(LOCAL_W, netvar.flChargedDamage);
        float bdmg = 50;
        // Vaccinator damage correction, protects against 20% of damage
        if (CarryingHeatmaker())
        {
            bdmg = (bdmg * .80) - 1;
            cdmg = (cdmg * .80) - 1;
        }
        // Vaccinator damage correction, protects against 75% of damage
        if (HasCondition<TFCond_UberBulletResist>(target))
        {
            bdmg = (bdmg * .25) - 1;
            cdmg = (cdmg * .25) - 1;
        }
        // Passive bullet resist protects against 10% of damage
        else if (HasCondition<TFCond_SmallBulletResist>(target))
        {
            bdmg = (bdmg * .90) - 1;
            cdmg = (cdmg * .90) - 1;
        }
        // Invis damage correction, Invis spies get protection from 10%
        // of damage
        else if (IsPlayerInvisible(target)) // You can't be invisible and under the effects of the vacc
        {
            bdmg = (bdmg * .80) - 1;
            cdmg = (cdmg * .80) - 1;
        }
        // If can headshot and if bodyshot kill from charge damage, or
        // if crit boosted and they have 150 health, or if player isnt
        // zoomed, or if the enemy has less than 40, due to darwins, and
        // only if they have less than 150 health will it try to
        // bodyshot
        if (std::floor(cdmg) >= target_health || IsPlayerCritBoosted(g_pLocalPlayer->entity) || (target_health <= std::floor(bdmg) && target_health <= 150))
        {
            // We dont need to hit the head as a bodyshot will kill
            preferred = hitbox_t::spine_1;
            return preferred;
        }

        return hitbox_t::head;
    }

    // Hunstman
    else if (ci == CL_CLASS(CTFCompoundBow))
    {
        float begincharge = CE_FLOAT(g_pLocalPlayer->weapon(), netvar.flChargeBeginTime);
        float charge      = g_GlobalVars->curtime - begincharge;
        int damage        = std::floor(50.0f + 70.0f * fminf(1.0f, charge));
        if (damage >= target_health)
            return hitbox_t::spine_1;
        else
            return hitbox_t::head;
    }

    // Ambassador
    else if (IsAmbassador(g_pLocalPlayer->weapon()))
    {

        // 18 health is a good number to use as thats the usual minimum
        // damage it can do with a bodyshot, but damage could
        // potentially be higher

        if (target_health <= 18 || IsPlayerCritBoosted(g_pLocalPlayer->entity) || target->m_flDistance() > 1200)
            return hitbox_t::spine_1;
        else if (AmbassadorCanHeadshot())
            return hitbox_t::head;
    }

    // Rockets and stickies should aim at the foot if the target is on the ground
    else if (ci == CL_CLASS(CTFPipebombLauncher) || ci == CL_CLASS(CTFRocketLauncher) || ci == CL_CLASS(CTFParticleCannon) || ci == CL_CLASS(CTFRocketLauncher_AirStrike) || ci == CL_CLASS(CTFRocketLauncher_Mortar) || ci == CL_CLASS(CTFRocketLauncher_DirectHit))
    {
        bool ground = CE_INT(target, netvar.iFlags) & (1 << 0);
        if (ground)
            preferred = notVisibleHitbox(target, hitbox_t::foot_L); // Only time it is worth the penalty
    }
    return preferred;
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
    for (int i = 0; i < target->hitboxes.GetNumHitboxes(); ++i)
    {
        fov = GetFov(g_pLocalPlayer->v_OrigViewangles, g_pLocalPlayer->v_Eye, target->hitboxes.GetHitbox(i)->center);
        if (fov < closest_fov || closest == -1)
        {
            closest     = i;
            closest_fov = fov;
        }
    }
    return closest;
}

// A helper function to find a user angle that isnt directly on the target
// angle, effectively slowing the aiming process
void DoSlowAim(Vector &input_angle)
{
    auto viewangles   = current_user_cmd->viewangles;
    Vector slow_delta = { 0, 0, 0 };

    // Don't bother if we're already on target
    if (viewangles != input_angle)
    {
        slow_delta = input_angle - viewangles;

        while (slow_delta.y > 180)
            slow_delta.y -= 360;
        while (slow_delta.y < -180)
            slow_delta.y += 360;

        slow_delta /= slow_aim;
        input_angle = viewangles + slow_delta;

        // Clamp as we changed angles
        fClampAngle(input_angle);
    }
    // 0.17 is a good amount in general
    slow_can_shoot = false;
    if (std::abs(slow_delta.y) < 0.17 && std::abs(slow_delta.x) < 0.17)
        slow_can_shoot = true;
}

// A function that determins whether aimkey allows aiming
bool UpdateAimkey()
{
    static bool aimkey_flip       = false;
    static bool pressed_last_tick = false;
    bool allow_aimkey             = true;
    static bool last_allow_aimkey = true;

    // Check if aimkey is used
    if (aimkey && aimkey_mode)
    {
        // Grab whether the aimkey is depressed
        bool key_down = aimkey.isKeyDown();
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
        // Huntsman and Loose Cannon need special logic since we aim upon m1 being released
        if (!autoshoot && CE_GOOD(LOCAL_W) && (LOCAL_W->m_iClassID() == CL_CLASS(CTFCompoundBow) || LOCAL_W->m_iClassID() == CL_CLASS(CTFCannon)))
        {
            if (!allow_aimkey && last_allow_aimkey)
            {
                allow_aimkey      = true;
                last_allow_aimkey = false;
            }
            else
                last_allow_aimkey = allow_aimkey;
        }
        else
            last_allow_aimkey = allow_aimkey;
        pressed_last_tick = key_down;
    }
    // Return whether the aimkey allows aiming
    return allow_aimkey;
}

// Used mostly by navbot to not accidentally look at path when aiming
bool isAiming()
{
    return aimed_this_tick;
}

// A function used by gui elements to determine the current target
CachedEntity *CurrentTarget()
{
    return target_last;
}

// Used for when you join and leave maps to reset aimbot vars
void Reset()
{
    target_last     = nullptr;
    projectile_mode = false;
}

#if ENABLE_VISUALS
static void DrawText()
{
    // Dont draw to screen when aimbot is disabled
    if (!enable)
        return;

    // Fov ring to represent when a target will be shot
    if (fov_draw)
    {
        // It cant use fovs greater than 180, so we check for that
        if (float(fov) > 0.0f && float(fov) < 180)
        {
            // Dont show ring while player is dead
            if (CE_GOOD(LOCAL_E) && LOCAL_E->m_bAlivePlayer())
            {
                rgba_t color = colors::gui;
                color.a      = float(fovcircle_opacity);

                int width, height;
                g_IEngine->GetScreenSize(width, height);

                // Math
                float mon_fov  = (float(width) / float(height) / (4.0f / 3.0f));
                float fov_real = RAD2DEG(2 * atanf(mon_fov * tanf(DEG2RAD(draw::fov / 2))));
                float radius   = tan(DEG2RAD(float(fov)) / 2) / tan(DEG2RAD(fov_real) / 2) * (width);

                draw::Circle(width / 2, height / 2, radius, color, 1, 100);
            }
        }
    }
    // Debug stuff
    if (!aimbot_debug)
        return;
    for (auto const &ent : entity_cache::player_cache)
    {
        if (CE_GOOD(ent))
        {
            Vector screen;
            Vector oscreen;
            if (draw::WorldToScreen(cd.aim_position, screen) && draw::WorldToScreen(ent->m_vecOrigin(), oscreen))
            {
                draw::Rectangle(screen.x - 2, screen.y - 2, 4, 4, colors::white);
                draw::Line(oscreen.x, oscreen.y, screen.x - oscreen.x, screen.y - oscreen.y, colors::EntityF(ent), 0.5f);
            }
        }
    }
}
#endif
void rvarCallback(settings::VariableBase<float> &, float after)
{
    force_backtrack_aimbot = after >= 200.0f;
}
static InitRoutine EC(
    []()
    {
        hacks::tf2::backtrack::latency.installChangeCallback(rvarCallback);
        EC::Register(EC::LevelInit, Reset, "INIT_Aimbot", EC::average);
        EC::Register(EC::LevelShutdown, Reset, "RESET_Aimbot", EC::average);
        EC::Register(EC::CreateMove, CreateMove, "CM_Aimbot", EC::late);
        EC::Register(EC::CreateMoveWarp, CreateMoveWarp, "CMW_Aimbot", EC::late);
#if ENABLE_VISUALS
        EC::Register(EC::Draw, DrawText, "DRAW_Aimbot", EC::average);
#endif
    });

} // namespace hacks::shared::aimbot
