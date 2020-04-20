#include "common.hpp"
#include "navparser.hpp"
#include "NavBot.hpp"
#include "PlayerTools.hpp"
#include "Aimbot.hpp"
#include "FollowBot.hpp"
#include "soundcache.hpp"
#include "Misc.hpp"
#include "MiscTemporary.hpp"

namespace hacks::tf2::NavBot
{
// -Rvars-
static settings::Boolean enabled("navbot.enabled", "false");
static settings::Boolean stay_near("navbot.stay-near", "true");
static settings::Boolean heavy_mode("navbot.other-mode", "false");
static settings::Boolean spy_mode("navbot.spy-mode", "false");
static settings::Boolean engineer_mode("navbot.engineer-mode", "false");
static settings::Boolean get_health("navbot.get-health-and-ammo", "true");
static settings::Float jump_distance("navbot.autojump.trigger-distance", "300");
static settings::Boolean autojump("navbot.autojump.enabled", "false");
static settings::Boolean primary_only("navbot.primary-only", "true");
static settings::Int spy_ignore_time("navbot.spy-ignore-time", "5000");

// -Forward declarations-
bool init(bool first_cm);
static bool navToSniperSpot();
static bool navToBuildingSpot();
static bool stayNear();
static bool stayNearEngineer();
static bool getDispenserHealthAndAmmo(int metal = -1);
static bool getHealthAndAmmo(int metal = -1);
static void autoJump();
static void updateSlot();
static void update_building_spots();
static bool engineerLogic();
static std::pair<CachedEntity *, float> getNearestPlayerDistance(bool vischeck = true);
using task::current_engineer_task;
using task::current_task;

// -Variables-
static std::vector<std::pair<CNavArea *, Vector>> sniper_spots;
static std::vector<std::pair<CNavArea *, Vector>> building_spots;
static std::vector<CNavArea *> blacklisted_build_spots;
// Our Buildings. We need this so we can remove them on object_destroyed.
static std::vector<CachedEntity *> local_buildings;
// Needed for blacklisting
static CNavArea *current_build_area;
// How long should the bot wait until pathing again?
static Timer wait_until_path{};
// Engineer version of above
static Timer wait_until_path_engineer{};
// Time before following target cloaked spy again
static std::array<Timer, PLAYER_ARRAY_SIZE> spy_cloak{};
// Don't spam spy path thanks
static Timer spy_path{};
// Big wait between updating Engineer building spots
static Timer engineer_update{};
// Recheck Building Area
static Timer engineer_recheck{};
// Timer for resetting Build attempts
static Timer build_timer{};
// Timer for wait between rotation and checking if we can place
static Timer rotation_timer{};
// Uses to check how long until we should resend the "build" command
static Timer build_command_timer{};
// Dispenser Nav cooldown
static Timer dispenser_nav_timer{};
// Last Yaw used for building
static float build_current_rotation = -180.0f;
// How many times have we tried to place?
static int build_attempts = 0;
// Enum for Building types
enum Building
{
    None      = -1,
    Dispenser = 0,
    TP_Entrace,
    Sentry,
    TP_Exit
};

// Successfully built? (Unknown == Bot is still trying to build and isn't sure if it will work or not yet)
enum success_build
{
    Failure = 0,
    Unknown,
    Success
};

// What is the bot currently doing
namespace task
{
Task current_task                   = task::none;
engineer_task current_engineer_task = engineer_task::nothing;
} // namespace task

constexpr bot_class_config DIST_SPY{ 10.0f, 50.0f, 1000.0f };
constexpr bot_class_config DIST_OTHER{ 100.0f, 200.0f, 300.0f };
constexpr bot_class_config DIST_SNIPER{ 1000.0f, 1500.0f, 3000.0f };
constexpr bot_class_config DIST_ENGINEER{ 600.0f, 1000.0f, 2500.0f };

// Gunslinger Engineers really don't care at all
constexpr bot_class_config DIST_GUNSLINGER_ENGINEER{ 100.0f, 300.0f, 500.0f };

inline bool HasGunslinger(CachedEntity *ent)
{
    return HasWeapon(ent, 142);
}
static void CreateMove()
{
    if (CE_BAD(LOCAL_E) || !LOCAL_E->m_bAlivePlayer() || !LOCAL_E->m_bAlivePlayer())
        return;
    if (!init(false))
        return;
    // blocking actions implement their own functions and shouldn't be interrupted by anything else
    bool blocking = std::find(task::blocking_tasks.begin(), task::blocking_tasks.end(), task::current_task.id) != task::blocking_tasks.end();

    if (!nav::ReadyForCommands || blocking)
        wait_until_path.update();
    else
        current_task = task::none;

    if (autojump)
        autoJump();
    if (primary_only)
        updateSlot();
    if (engineer_mode)
    {
        if (CE_GOOD(LOCAL_E))
        {
            for (int i = g_IEngine->GetMaxClients() + 1; i < MAX_ENTITIES; i++)
            {
                CachedEntity *ent = ENTITY(i);
                if (!ent || CE_BAD(ent) || ent->m_bEnemy() || !ent->m_bAlivePlayer())
                    continue;
                if (HandleToIDX(CE_INT(ent, netvar.m_hBuilder)) != LOCAL_E->m_IDX)
                    continue;
                if (std::find(local_buildings.begin(), local_buildings.end(), ent) == local_buildings.end())
                    local_buildings.push_back(ent);
            }
            update_building_spots();
        }
    }

    if (get_health)
    {
        int metal = -1;
        if (engineer_mode && g_pLocalPlayer->clazz == tf_engineer)
            metal = CE_INT(LOCAL_E, netvar.m_iAmmo + 12);
        if ((dispenser_nav_timer.test_and_set(1000) && getDispenserHealthAndAmmo(metal)))
            return;
        if (getHealthAndAmmo(metal))
            return;
    }

    // Engineer stuff
    if (engineer_mode && g_pLocalPlayer->clazz == tf_engineer)
    {
        if (engineerLogic())
            return;
    }
    // Reset Engi task stuff if not engineer
    else if (!engineer_mode && task::current_task == task::engineer)
    {
        task::current_task          = task::none;
        task::current_engineer_task = task::nothing;
    }

    if (blocking)
        return;
    // Spy can just walk into the enemy
    if (spy_mode)
    {
        if (spy_path.check(1000))
        {
            auto nearest = getNearestPlayerDistance(false);
            if (CE_VALID(nearest.first) && nearest.first->m_vecDormantOrigin())
            {
                if (current_task != task::stay_near)
                {
                    if (nav::navTo(*nearest.first->m_vecDormantOrigin(), 6, true, false))
                    {
                        spy_path.update();
                        current_task = task::stay_near;
                        return;
                    }
                }
                else
                {
                    if (nav::navTo(*nearest.first->m_vecDormantOrigin(), 6, false, false))
                    {
                        spy_path.update();
                        return;
                    }
                }
            }
        }
        if (current_task == task::stay_near)
            return;
    }
    // Try to stay near enemies to increase efficiency
    if ((stay_near || heavy_mode) && !spy_mode)
        if (stayNear())
            return;
    // We don't have anything else to do. Just nav to sniper spots.
    if (navToSniperSpot())
        return;
    // Uhh... Just stand around I guess?
}

bool init(bool first_cm)
{
    static bool inited = false;
    if (first_cm)
        inited = false;
    if (!enabled)
        return false;
    if (!nav::prepare())
        return false;
    if (!inited)
    {
        blacklisted_build_spots.clear();
        local_buildings.clear();
        sniper_spots.clear();
        // Add all sniper spots to vector
        for (auto &area : nav::navfile->m_areas)
        {
            for (auto hide : area.m_hidingSpots)
                if (hide.IsGoodSniperSpot() || hide.IsIdealSniperSpot() || hide.IsExposed())
                    sniper_spots.emplace_back(&area, hide.m_pos);
        }
        inited = true;
    }
    return true;
}

struct area_struct
{
    // The Area
    CNavArea *navarea;
    // Distance away from enemies
    float min_distance;
    // Valid enemies to area
    std::vector<Vector *> enemy_list;
};

void update_building_spots()
{
    if (engineer_update.test_and_set(10000))
    {
        building_spots.clear();
        // Store in here to reduce operations
        std::vector<Vector> enemy_positions;
        // Stores valid areas, the float is the minimum distance away from enemies, needed for sorting later
        std::vector<area_struct> areas;

        for (int i = 0; i <= g_IEngine->GetMaxClients(); i++)
        {
            CachedEntity *ent = ENTITY(i);
            // Grab only Enemies and only if they are in soundcache
            if (!ent || CE_INVALID(ent) || !ent->m_bAlivePlayer() || !ent->m_bEnemy())
                continue;
            if (ent->m_vecDormantOrigin())
                enemy_positions.push_back(*ent->m_vecDormantOrigin());
        }
        auto config = &DIST_ENGINEER;
        if (HasGunslinger(LOCAL_E))
            config = &DIST_GUNSLINGER_ENGINEER;
        for (auto &area : nav::navfile->m_areas)
        {
            // Blacklisted for building
            if (std::find(blacklisted_build_spots.begin(), blacklisted_build_spots.end(), &area) != blacklisted_build_spots.end())
                continue;

            // These positions we should vischeck
            std::vector<Vector *> vischeck_positions;

            // Minimum distance the area was away from enemies
            float min_dist_away = FLT_MAX;

            // Area Center
            auto area_pos = area.m_center;
            // Don't want to instantly hit the floor
            area_pos.z += 42.0f;

            // Found enemy below min/above max range away from area
            bool enemy_found  = false;
            bool out_of_reach = true;

            for (auto &pos : enemy_positions)
            {
                auto dist = area_pos.DistTo(pos);
                if (dist < config->min)
                {
                    enemy_found = true;
                    break;
                }
                // Found someone within min and max range
                if (dist < config->max)
                {
                    out_of_reach = false;
                    // Should vischeck this one
                    vischeck_positions.push_back(&pos);
                    if (dist < min_dist_away)
                        min_dist_away = dist;
                }
            }
            // Too close/Too far away
            if (enemy_found || out_of_reach)
                continue;

            // Area is valid (Distance wise)
            areas.push_back({ &area, min_dist_away, vischeck_positions });
        }

        // Sort, be as close to preferred as possible
        std::sort(areas.begin(), areas.end(), [&](area_struct a, area_struct b) { return std::abs(a.min_distance - config->preferred) < std::abs(b.min_distance - config->preferred); });

        // Still need to do vischeck stuff
        for (auto &area : areas)
        {
            // Is the enemy able to see the area?
            bool can_see_area = false;
            // Area Center
            auto area_pos = area.navarea->m_center;
            // Don't want to instantly hit the floor
            area_pos.z += 42.0f;
            // Loop all valid enemies
            for (auto pos : area.enemy_list)
            {
                if (IsVectorVisible(area_pos, *pos))
                {
                    can_see_area = true;
                    break;
                }
            }
            // Someone can see the area. Abort! (Gunslinger Engineer does not care)
            if (can_see_area && !HasGunslinger(LOCAL_E))
                continue;
            // All good!
            building_spots.push_back(std::pair<CNavArea *, Vector>(area.navarea, area.navarea->m_center));
        }
    }
}

static bool navToSniperSpot()
{
    // Don't path if you already have commands. But also don't error out.
    if (!nav::ReadyForCommands || current_task != task::none)
        return true;
    // Wait arround a bit before pathing again
    if (!wait_until_path.check(2000))
        return false;
    // Max 10 attempts
    for (int attempts = 0; attempts < 10; attempts++)
    {
        // Get a random sniper spot
        auto random = select_randomly(sniper_spots.begin(), sniper_spots.end());
        // Check if spot is considered safe (no sentry, no sticky)
        if (!nav::isSafe(random.base()->first))
            continue;
        // Try to nav there
        if (nav::navTo(random.base()->second, 1, true, true, false))
        {
            current_task = { task::sniper_spot, 1 };
        }
    }
    return false;
}

static bool navToBuildingSpot()
{
    // Don't path if you already have commands. But also don't error out.
    if (!nav::ReadyForCommands || (current_task != task::engineer && current_task != task::none))
        return true;
    // Wait a bit before pathing again
    if (!wait_until_path_engineer.test_and_set(2000))
        return false;
    // Max 10 attempts
    for (int attempts = 0; attempts < 10 && attempts < building_spots.size(); attempts++)
    {
        // Get a Building spot
        auto &area = building_spots[attempts];
        // Check if spot is considered safe (no sentry, no sticky)
        if (!nav::isSafe(area.first))
            continue;
        // Try to nav there
        if (nav::navTo(area.second, 5, true, true, false))
        {
            current_task          = { task::engineer, 5 };
            current_build_area    = area.first;
            current_engineer_task = task::engineer_task::goto_build_spot;
            return true;
        }
    }
    return false;
}

static Building selectBuilding()
{
    int metal        = CE_INT(LOCAL_E, netvar.m_iAmmo + 12);
    int metal_sentry = 130;

    // We have a mini sentry, costs less
    if (HasGunslinger(LOCAL_E))
        metal_sentry = 100;

    // Do we already have these?
    bool sentry_built    = false;
    bool dispenser_built = false;
    // Loop all buildings
    for (auto &building : local_buildings)
    {
        if (building->m_iClassID() == CL_CLASS(CObjectSentrygun))
            sentry_built = true;
        else if (building->m_iClassID() == CL_CLASS(CObjectDispenser))
            dispenser_built = true;
    }

    if (metal >= metal_sentry && !sentry_built)
        return Sentry;
    else if (metal >= 100 && !dispenser_built)
        return Dispenser;
    return None;
}

static success_build buildBuilding()
{
    int metal = CE_INT(LOCAL_E, netvar.m_iAmmo + 12);
    // Out of Metal
    if (metal < 100)
        return Failure;
    // Last building
    static Building last_building = selectBuilding();
    // Get best building to build right now
    Building building = selectBuilding();
    // Reset Rotation on these conditions
    if (building != last_building || build_timer.check(1000))
    {
        // We changed the target building, means it was successful!
        if (!build_timer.check(1000))
            return Success;

        // No building
        if (building == None)
            return Failure;

        build_attempts         = 0;
        build_current_rotation = -180.0f;
        build_timer.update();
    }
    // No building
    if (building == None)
        return Failure;

    last_building = building;
    build_timer.update();
    if (rotation_timer.test_and_set(300))
    {
        // Look slightly downwards for building process
        current_user_cmd->viewangles.x = 20.0f;
        // Set Yaw
        current_user_cmd->viewangles.y = build_current_rotation;
        // Rotate
        build_current_rotation += 20.0f;
        build_attempts++;
        // Put building in hand if not already
        if (hacks::shared::misc::getCarriedBuilding() == -1 && build_command_timer.test_and_set(50))
            g_IEngine->ClientCmd_Unrestricted(format("build ", building).c_str());
    }
    else if (rotation_timer.check(200))
    {
        if (hacks::shared::misc::getCarriedBuilding() != -1)
        {
            int carried_building = hacks::shared::misc::getCarriedBuilding();
            // It works! Place building
            if (CE_INT(ENTITY(carried_building), netvar.m_bCanPlace))
                current_user_cmd->buttons |= IN_ATTACK;
        }
    }
    // Bad area
    if (build_attempts >= 14)
    {
        blacklisted_build_spots.push_back(current_build_area);
        return Failure;
    }
    return Unknown;
}

static bool navToBuilding(CachedEntity *target = nullptr)
{
    if (local_buildings.size())
    {
        int priority = 5;
        if (current_engineer_task == task::staynear_engineer)
            priority = 7;
        // Just grab target and nav there
        if (target)
            if (nav::navTo(target->m_vecOrigin(), priority, true))
            {
                current_task          = { task::engineer, priority };
                current_engineer_task = task::engineer_task::goto_building;
                return true;
            }
        // Nav to random building
        for (auto &building : local_buildings)
        {
            if (nav::navTo(building->m_vecOrigin(), priority, true))
            {
                current_task          = { task::engineer, priority };
                current_engineer_task = task::engineer_task::goto_building;
                return true;
            }
        }
    }
    return false;
}

static bool engineerLogic()
{
    std::vector<CachedEntity *> new_building_list;
    for (auto &building : local_buildings)
        if (CE_VALID(building) && !CE_INT(building, netvar.m_bPlacing))
            new_building_list.push_back(building);
    local_buildings = new_building_list;

    // Overwrites and Not yet running engineer task
    if ((current_task != task::engineer || current_engineer_task == task::engineer_task::nothing || current_engineer_task == task::engineer_task::staynear_engineer) && current_task != task::health && current_task != task::ammo)
    {
        // Already have a building
        if (local_buildings.size())
        {
            int metal = CE_INT(LOCAL_E, netvar.m_iAmmo + 12);
            if (metal)
                for (auto &building : local_buildings)
                    // Hey hit the building thanks (gunslinger engineer shouldn't care)
                    if (hacks::tf2::misc_aimbot::ShouldHitBuilding(building) && !HasGunslinger(LOCAL_E))
                    {
                        if (navToBuilding(building))
                            return true;
                    }

            // Let's terrify some people (gunslinger engineer)
            if (HasGunslinger(LOCAL_E))
                stayNearEngineer();

            else if (selectBuilding() != None)
            {
                // If we're near our buildings and have the metal, build another one
                for (auto &building : local_buildings)
                    if (building->m_vecOrigin().DistTo(LOCAL_E->m_vecOrigin()) <= 300.0f)
                    {
                        current_task          = { task::engineer, 4 };
                        current_engineer_task = task::engineer_task::build_building;
                        return true;
                    }
                    // We're too far away, go to building
                    else if (navToBuilding())
                        return true;
            }
            // If it's metal we're missing, get some metal
            /*else if (metal < 100)
            {
                if ((dispenser_nav_timer.test_and_set(1000) && getDispenserHealthAndAmmo(metal)) || getHealthAndAmmo(metal))
                    return true;
            }*/
            // Else just Roam around the map and kill people
            else if (stayNearEngineer())
                return true;
        }
        // Nav to a Building spot
        else if (navToBuildingSpot())
        {
            engineer_recheck.update();
            return false;
        }
    }
    // Metal
    int metal = CE_INT(LOCAL_E, netvar.m_iAmmo + 12);
    /*if ((dispenser_nav_timer.test_and_set(1000) && getDispenserHealthAndAmmo(metal)) || getHealthAndAmmo(metal))
        return true;*/
    switch (current_engineer_task)
    {
    // Upgrade/repair
    case (task::engineer_task::upgradeorrepair_building):
    {
        if (metal)
            if (local_buildings.size())
                for (auto &building : local_buildings)
                {
                    if (building->m_vecOrigin().DistTo(LOCAL_E->m_vecOrigin()) <= 300.0f)
                    {
                        if (hacks::tf2::misc_aimbot::ShouldHitBuilding(building))
                            break;
                    }
                }

        current_task          = { task::engineer, 4 };
        current_engineer_task = task::engineer_task::nothing;
    }
    // Going to existing building
    case (task::engineer_task::goto_building):
    {
        if (nav::ReadyForCommands)
        {
            bool found = false;
            if (local_buildings.size())
                for (auto &building : local_buildings)
                {
                    if (building->m_vecOrigin().DistTo(LOCAL_E->m_vecOrigin()) <= 300.0f)
                    {
                        if (metal && hacks::tf2::misc_aimbot::ShouldHitBuilding(building))
                        {
                            current_task          = { task::engineer, 4 };
                            current_engineer_task = task::engineer_task::upgradeorrepair_building;
                        }
                        if (current_engineer_task != task::engineer_task::upgradeorrepair_building)
                        {
                            current_task          = { task::engineer, 4 };
                            current_engineer_task = task::engineer_task::nothing;
                        }
                        found = true;
                    }
                }
            if (found)
                break;

            current_task          = { task::engineer, 4 };
            current_engineer_task = task::engineer_task::nothing;
        }
        break;
    }
    // Going to spot to build
    case (task::engineer_task::goto_build_spot):
    {
        // We Arrived, time to (try) to build!
        if (nav::ReadyForCommands)
        {
            current_task          = { task::engineer, 4 };
            current_engineer_task = task::engineer_task::build_building;
        }
        else if (engineer_recheck.test_and_set(15000))
        {
            if (navToBuildingSpot())
            {
                engineer_recheck.update();
                return true;
            }
        }
        break;
    }
    // Build building
    case (task::engineer_task::build_building):
    {
        auto status = buildBuilding();
        // Failed, Get a new Task
        if (status == Failure)
        {
            current_task          = { task::engineer, 4 };
            current_engineer_task = task::engineer_task::nothing;
        }
        else if (status == Success)
        {

            current_task          = { task::engineer, 4 };
            current_engineer_task = task::engineer_task::nothing;
            return true;
        }
        // Still building
        else if (status == Unknown)
            return true;
        break;
    }
    default:
        break;
    }
    return false;
}

static std::pair<CachedEntity *, float> getNearestPlayerDistance(bool vischeck)
{
    float distance         = FLT_MAX;
    CachedEntity *best_ent = nullptr;
    for (int i = 1; i <= g_IEngine->GetMaxClients(); i++)
    {
        CachedEntity *ent = ENTITY(i);
        if (CE_VALID(ent) && ent->m_vecDormantOrigin() && ent->m_bAlivePlayer() && ent->m_bEnemy() && g_pLocalPlayer->v_Origin.DistTo(ent->m_vecOrigin()) < distance && player_tools::shouldTarget(ent) && (!vischeck || VisCheckEntFromEnt(LOCAL_E, ent)))
        {
            if (hacks::shared::aimbot::ignore_cloak && IsPlayerInvisible(ent))
            {
                spy_cloak[i].update();
                continue;
            }
            if (!spy_cloak[i].check(*spy_ignore_time))
                continue;
            distance = g_pLocalPlayer->v_Origin.DistTo(*ent->m_vecDormantOrigin());
            best_ent = ent;
        }
    }
    return { best_ent, distance };
}

namespace stayNearHelpers
{
// Check if the location is close enough/far enough and has a visual to target
static bool isValidNearPosition(Vector vec, Vector target, const bot_class_config &config)
{
    vec.z += 40;
    target.z += 40;
    float dist = vec.DistTo(target);
    if (dist < config.min || dist > config.max)
        return false;
    if (!IsVectorVisible(vec, target, true, LOCAL_E, MASK_PLAYERSOLID))
        return false;
    return true;
}

// Returns true if began pathing
static bool stayNearPlayer(CachedEntity *&ent, const bot_class_config &config, CNavArea *&result, bool engineer = false)
{
    if (!CE_VALID(ent))
        return false;
    auto position = ent->m_vecDormantOrigin();
    if (!position)
        return false;
    // Get some valid areas
    std::vector<CNavArea *> areas;
    for (auto &area : nav::navfile->m_areas)
    {
        if (!isValidNearPosition(area.m_center, *position, config))
            continue;
        areas.push_back(&area);
    }
    if (areas.empty())
        return false;

    const Vector ent_orig = *position;
    // Area dist to target should be as close as possible to config.preferred
    std::sort(areas.begin(), areas.end(), [&](CNavArea *a, CNavArea *b) { return std::abs(a->m_center.DistTo(ent_orig) - config.preferred) < std::abs(b->m_center.DistTo(ent_orig) - config.preferred); });

    size_t size = 20;
    if (areas.size() < size)
        size = areas.size();

    // Get some areas that are close to the player
    std::vector<CNavArea *> preferred_areas(areas.begin(), areas.end());
    preferred_areas.resize(size / 2);
    if (preferred_areas.empty())
        return false;
    std::sort(preferred_areas.begin(), preferred_areas.end(), [](CNavArea *a, CNavArea *b) { return a->m_center.DistTo(g_pLocalPlayer->v_Origin) < b->m_center.DistTo(g_pLocalPlayer->v_Origin); });

    preferred_areas.resize(size / 4);
    if (preferred_areas.empty())
        return false;
    for (auto &i : preferred_areas)
    {
        if (nav::navTo(i->m_center, 7, true, false))
        {
            result = i;
            if (engineer)
            {
                current_task          = { task::engineer, 4 };
                current_engineer_task = task::staynear_engineer;
            }
            else
                current_task = task::stay_near;
            return true;
        }
    }

    for (size_t attempts = 0; attempts < size / 4; attempts++)
    {
        auto it = select_randomly(areas.begin(), areas.end());
        if (nav::navTo((*it.base())->m_center, 7, true, false))
        {
            result = *it.base();
            if (engineer)
            {
                current_task          = { task::engineer, 4 };
                current_engineer_task = task::staynear_engineer;
            }
            else
                current_task = task::stay_near;
            return true;
        }
    }
    return false;
}

// Loop thru all players and find one we can path to
static bool stayNearPlayers(const bot_class_config &config, CachedEntity *&result_ent, CNavArea *&result_area)
{
    std::vector<CachedEntity *> players;
    for (int i = 1; i <= g_IEngine->GetMaxClients(); i++)
    {
        CachedEntity *ent = ENTITY(i);
        if (CE_INVALID(ent) || !g_pPlayerResource->isAlive(ent->m_IDX) || !ent->m_bEnemy() || !player_tools::shouldTarget(ent))
            continue;
        if (hacks::shared::aimbot::ignore_cloak && IsPlayerInvisible(ent))
        {
            spy_cloak[i].update();
            continue;
        }
        if (!spy_cloak[i].check(*spy_ignore_time))
            continue;
        if (ent->m_vecDormantOrigin())
            players.push_back(ent);
    }
    if (players.empty())
        return false;
    std::sort(players.begin(), players.end(), [](CachedEntity *a, CachedEntity *b) {
        Vector position_a = *a->m_vecDormantOrigin(), position_b = *b->m_vecDormantOrigin();
        return position_a.DistTo(g_pLocalPlayer->v_Origin) < position_b.DistTo(g_pLocalPlayer->v_Origin);
    });
    for (auto player : players)
    {
        if (stayNearPlayer(player, config, result_area))
        {
            result_ent = player;
            return true;
        }
    }
    return false;
}
} // namespace stayNearHelpers

// stayNear()'s Little Texan brother
static bool stayNearEngineer()
{
    static CachedEntity *last_target = nullptr;
    static CNavArea *last_area       = nullptr;

    // What distances do we have to use?
    bot_class_config config = DIST_ENGINEER;
    if (HasGunslinger(LOCAL_E))
        config = DIST_GUNSLINGER_ENGINEER;

    // Check if someone is too close to us and then target them instead
    std::pair<CachedEntity *, float> nearest = getNearestPlayerDistance();
    if (nearest.first && nearest.first != last_target && nearest.second < config.min)
        if (stayNearHelpers::stayNearPlayer(nearest.first, config, last_area, true))
        {
            last_target = nearest.first;
            return true;
        }
    bool valid_dormant = false;
    if (CE_VALID(last_target) && RAW_ENT(last_target)->IsDormant())
    {
        if (last_target->m_vecDormantOrigin())
            valid_dormant = true;
    }
    if (current_task == task::stay_near)
    {
        static Timer invalid_area_time{};
        static Timer invalid_target_time{};
        // Do we already have a stay near target? Check if its still good.
        if (CE_GOOD(last_target) || valid_dormant)
            invalid_target_time.update();
        else
            invalid_area_time.update();
        // Check if we still have LOS and are close enough/far enough
        Vector position;
        if (CE_GOOD(last_target) || valid_dormant)
        {
            position = *last_target->m_vecDormantOrigin();
        }
        if ((CE_GOOD(last_target) || valid_dormant) && stayNearHelpers::isValidNearPosition(last_area->m_center, position, config))
            invalid_area_time.update();

        if ((CE_GOOD(last_target) || valid_dormant) && (!g_pPlayerResource->isAlive(last_target->m_IDX) || !last_target->m_bEnemy() || !player_tools::shouldTarget(last_target) || !spy_cloak[last_target->m_IDX].check(*spy_ignore_time) || (hacks::shared::aimbot::ignore_cloak && IsPlayerInvisible(last_target))))
        {
            if (hacks::shared::aimbot::ignore_cloak && IsPlayerInvisible(last_target))
                spy_cloak[last_target->m_IDX].update();
            nav::clearInstructions();
            current_engineer_task = task::engineer_task::nothing;
        }
        else if (invalid_area_time.test_and_set(300))
        {
            current_engineer_task = task::engineer_task::nothing;
        }
        else if (invalid_target_time.test_and_set(5000))
        {
            current_engineer_task = task::engineer_task::nothing;
        }
    }
    // Are we doing nothing? Check if our current location can still attack our
    // last target
    if (current_engineer_task != task::engineer_task::staynear_engineer && (CE_GOOD(last_target) || valid_dormant) && g_pPlayerResource->isAlive(last_target->m_IDX) && last_target->m_bEnemy())
    {
        if (hacks::shared::aimbot::ignore_cloak && IsPlayerInvisible(last_target))
            spy_cloak[last_target->m_IDX].update();
        if (spy_cloak[last_target->m_IDX].check(*spy_ignore_time))
        {
            Vector position = *last_target->m_vecDormantOrigin();

            if (stayNearHelpers::isValidNearPosition(g_pLocalPlayer->v_Origin, position, config))
                return true;
            // If not, can we try pathing to our last target again?
            if (stayNearHelpers::stayNearPlayer(last_target, config, last_area, true))
                return true;
        }
        last_target = nullptr;
    }

    static Timer wait_until_stay_near{};
    if (current_task == task::engineer_task::staynear_engineer)
    {
        return true;
    }
    else if (wait_until_stay_near.test_and_set(4000))
    {
        // We're doing nothing? Do something!
        return stayNearHelpers::stayNearPlayers(config, last_target, last_area);
    }

    return false;
}
// Main stay near function
static bool stayNear()
{
    static CachedEntity *last_target = nullptr;
    static CNavArea *last_area       = nullptr;

    // What distances do we have to use?
    const bot_class_config *config;
    if (spy_mode)
    {
        config = &DIST_SPY;
    }
    else if (heavy_mode)
    {
        config = &DIST_OTHER;
    }
    else
    {
        config = &DIST_SNIPER;
    }

    // Check if someone is too close to us and then target them instead
    std::pair<CachedEntity *, float> nearest = getNearestPlayerDistance();
    if (nearest.first && nearest.first != last_target && nearest.second < config->min)
        if (stayNearHelpers::stayNearPlayer(nearest.first, *config, last_area))
        {
            last_target = nearest.first;
            return true;
        }
    bool valid_dormant = false;
    if (CE_VALID(last_target) && RAW_ENT(last_target)->IsDormant())
    {
        if (last_target->m_vecDormantOrigin())
            valid_dormant = true;
    }
    if (current_task == task::stay_near)
    {
        static Timer invalid_area_time{};
        static Timer invalid_target_time{};
        // Do we already have a stay near target? Check if its still good.
        if (CE_GOOD(last_target) || valid_dormant)
            invalid_target_time.update();
        else
            invalid_area_time.update();
        // Check if we still have LOS and are close enough/far enough
        Vector position;
        if (CE_GOOD(last_target) || valid_dormant)
        {
            position = *last_target->m_vecDormantOrigin();
        }
        if ((CE_GOOD(last_target) || valid_dormant) && stayNearHelpers::isValidNearPosition(last_area->m_center, position, *config))
            invalid_area_time.update();

        if ((CE_GOOD(last_target) || valid_dormant) && (!g_pPlayerResource->isAlive(last_target->m_IDX) || !last_target->m_bEnemy() || !player_tools::shouldTarget(last_target) || !spy_cloak[last_target->m_IDX].check(*spy_ignore_time) || (hacks::shared::aimbot::ignore_cloak && IsPlayerInvisible(last_target))))
        {
            if (hacks::shared::aimbot::ignore_cloak && IsPlayerInvisible(last_target))
                spy_cloak[last_target->m_IDX].update();
            nav::clearInstructions();
            current_task = task::none;
        }
        else if (invalid_area_time.test_and_set(300))
        {
            current_task = task::none;
        }
        else if (invalid_target_time.test_and_set(5000))
        {
            current_task = task::none;
        }
    }
    // Are we doing nothing? Check if our current location can still attack our
    // last target
    if (current_task != task::stay_near && (CE_GOOD(last_target) || valid_dormant) && g_pPlayerResource->isAlive(last_target->m_IDX) && last_target->m_bEnemy())
    {
        if (hacks::shared::aimbot::ignore_cloak && IsPlayerInvisible(last_target))
            spy_cloak[last_target->m_IDX].update();
        if (spy_cloak[last_target->m_IDX].check(*spy_ignore_time))
        {
            Vector position = *last_target->m_vecDormantOrigin();

            if (stayNearHelpers::isValidNearPosition(g_pLocalPlayer->v_Origin, position, *config))
                return true;
            // If not, can we try pathing to our last target again?
            if (stayNearHelpers::stayNearPlayer(last_target, *config, last_area))
                return true;
        }
        last_target = nullptr;
    }

    static Timer wait_until_stay_near{};
    if (current_task == task::stay_near)
    {
        return true;
    }
    else if (wait_until_stay_near.test_and_set(1000))
    {
        // We're doing nothing? Do something!
        return stayNearHelpers::stayNearPlayers(*config, last_target, last_area);
    }

    return false;
}

static inline bool hasLowAmmo()
{
    if (CE_BAD(LOCAL_W))
        return false;
    int *weapon_list = (int *) ((uint64_t)(RAW_ENT(LOCAL_E)) + netvar.hMyWeapons);
    if (!weapon_list)
        return false;
    if (g_pLocalPlayer->holding_sniper_rifle && CE_INT(LOCAL_E, netvar.m_iAmmo + 4) <= 5)
        return true;
    for (int i = 0; weapon_list[i]; i++)
    {
        int handle = weapon_list[i];
        int eid    = handle & 0xFFF;
        if (eid > MAX_PLAYERS && eid <= HIGHEST_ENTITY)
        {
            IClientEntity *weapon = g_IEntityList->GetClientEntity(eid);
            if (weapon and re::C_BaseCombatWeapon::IsBaseCombatWeapon(weapon) && re::C_TFWeaponBase::UsesPrimaryAmmo(weapon) && !re::C_TFWeaponBase::HasPrimaryAmmo(weapon))
                return true;
        }
    }
    return false;
}

static std::vector<Vector> getDispensers()
{
    std::vector<Vector> dispensers;
    for (int i = 1; i <= HIGHEST_ENTITY; i++)
    {
        CachedEntity *ent = ENTITY(i);
        if (CE_INVALID(ent))
            continue;
        if (!ent->m_vecDormantOrigin())
            continue;
        if (ent->m_iClassID() != CL_CLASS(CObjectDispenser) || ent->m_bEnemy())
            continue;
        if (CE_BYTE(ent, netvar.m_bHasSapper))
            continue;
        if (CE_BYTE(ent, netvar.m_bBuilding))
            continue;
        if (CE_BYTE(ent, netvar.m_bPlacing))
            continue;
        dispensers.push_back(*ent->m_vecDormantOrigin());
    }
    std::sort(dispensers.begin(), dispensers.end(), [](Vector &a, Vector &b) { return g_pLocalPlayer->v_Origin.DistTo(a) < g_pLocalPlayer->v_Origin.DistTo(b); });
    return dispensers;
}

static bool getDispenserHealthAndAmmo(int metal)
{
    // Timeout for standing next to dispenser
    static Timer dispenser_timeout{};
    // Cooldown after standing next to one for too long
    static Timer dispenser_cooldown{};
    float health = static_cast<float>(LOCAL_E->m_iHealth()) / LOCAL_E->m_iMaxHealth();
    bool lowAmmo = hasLowAmmo();
    if (metal != -1)
    {
        lowAmmo = metal < 100 && selectBuilding() == None;
        if (current_engineer_task == task::engineer_task::upgradeorrepair_building)
            lowAmmo = metal == 0;
    }
    // Check if we should cancel this task
    if (current_task == task::dispenser)
    {
        if (health > 0.99f && !lowAmmo)
        {
            nav::clearInstructions();
            current_task = task::none;
            return false;
        }
        if (health > 0.64f && !lowAmmo)
            current_task.priority = 3;
    }

    // Check if we're standing next to a dispenser for too long.
    if (current_task == task::dispenser)
    {
        auto dispensers = getDispensers();
        // If near enough to dispenser
        if (dispensers.size() && dispensers[0].DistTo(g_pLocalPlayer->v_Origin) < 60.0f)
        {
            // Standing next to it for too long
            if (dispenser_timeout.check(10000))
            {
                dispenser_cooldown.update();
                current_task = task::none;
                nav::clearInstructions();
                return false;
            }
        }
        else
        {
            dispenser_timeout.update();
        }
    }

    // If Low ammo/Health
    if (current_task != task::dispenser)
        if (health < 0.64f || (current_task.priority < 3 && health < 0.99f) || lowAmmo)
        {
            std::vector<Vector> dispensers = getDispensers();

            for (auto &dispenser : dispensers)
            {
                // Nav To Dispenser
                if (nav::navTo(dispenser, health < 0.64f || lowAmmo ? 11 : 3, true, false))
                {
                    // On Success, update task
                    current_task = { task::dispenser, health < 0.64f || lowAmmo ? 10 : 3 };
                }
            }
        }
    if (current_task == task::dispenser && current_task.priority == 10)
        return true;
    else
        return false;
}

static bool getHealthAndAmmo(int metal)
{
    float health = static_cast<float>(LOCAL_E->m_iHealth()) / LOCAL_E->m_iMaxHealth();
    bool lowAmmo = hasLowAmmo();
    if (metal != -1)
    {
        lowAmmo = metal < 100 && selectBuilding() == None;
        if (current_engineer_task == task::engineer_task::upgradeorrepair_building)
            lowAmmo = metal == 0;
    }
    // Check if we should cancel this task
    if (current_task == task::health || current_task == task::ammo)
    {
        if (health > 0.99f && !lowAmmo)
        {
            nav::clearInstructions();
            current_task = task::none;
            return false;
        }
        if (health > 0.64f && !lowAmmo)
            current_task.priority = 3;
    }

    // If Low Ammo/Health
    if (current_task != task::health && current_task != task::ammo)
        if (health < 0.64f || (current_task.priority < 3 && health < 0.99f) || lowAmmo)
        {
            bool gethealth;
            if (health < 0.64f)
                gethealth = true;
            else if (lowAmmo)
                gethealth = false;
            else
                gethealth = true;

            if (gethealth)
            {
                std::vector<Vector> healthpacks;
                for (int i = 1; i <= HIGHEST_ENTITY; i++)
                {
                    CachedEntity *ent = ENTITY(i);
                    if (CE_BAD(ent))
                        continue;
                    if (ent->m_ItemType() != ITEM_HEALTH_SMALL && ent->m_ItemType() != ITEM_HEALTH_MEDIUM && ent->m_ItemType() != ITEM_HEALTH_LARGE)
                        continue;
                    healthpacks.push_back(ent->m_vecOrigin());
                }
                std::sort(healthpacks.begin(), healthpacks.end(), [](Vector &a, Vector &b) { return g_pLocalPlayer->v_Origin.DistTo(a) < g_pLocalPlayer->v_Origin.DistTo(b); });
                for (auto &pack : healthpacks)
                {
                    if (nav::navTo(pack, health < 0.64f || lowAmmo ? 10 : 3, true, false))
                    {
                        current_task = { task::health, health < 0.64f ? 10 : 3 };
                    }
                }
            }
            else
            {
                std::vector<Vector> ammopacks;
                for (int i = 1; i <= HIGHEST_ENTITY; i++)
                {
                    CachedEntity *ent = ENTITY(i);
                    if (CE_BAD(ent))
                        continue;
                    if (ent->m_ItemType() != ITEM_AMMO_SMALL && ent->m_ItemType() != ITEM_AMMO_MEDIUM && ent->m_ItemType() != ITEM_AMMO_LARGE)
                        continue;
                    ammopacks.push_back(ent->m_vecOrigin());
                }
                std::sort(ammopacks.begin(), ammopacks.end(), [](Vector &a, Vector &b) { return g_pLocalPlayer->v_Origin.DistTo(a) < g_pLocalPlayer->v_Origin.DistTo(b); });
                for (auto &pack : ammopacks)
                {
                    if (nav::navTo(pack, health < 0.64f || lowAmmo ? 9 : 3, true, false))
                    {
                        current_task = { task::ammo, 10 };
                        return true;
                    }
                }
            }
        }
    if ((current_task == task::health || current_task == task::ammo) && current_task.priority == 10)
        return true;
    else
        return false;
}

static void autoJump()
{
    static Timer last_jump{};
    if (!last_jump.test_and_set(200))
        return;

    if (getNearestPlayerDistance().second <= *jump_distance)
        current_user_cmd->buttons |= IN_JUMP | IN_DUCK;
}

enum slots
{
    primary   = 1,
    secondary = 2,
    melee     = 3
};
static slots getBestSlot(slots active_slot)
{
    auto nearest = getNearestPlayerDistance(false);
    switch (g_pLocalPlayer->clazz)
    {
    case tf_scout:
        return primary;
    case tf_heavy:
        return primary;
    case tf_medic:
        return secondary;
    case tf_spy:
    {
        if (nearest.second > 200 && active_slot == primary)
            return active_slot;
        else if (nearest.second >= 250)
            return primary;
        else
            return melee;
    }
    case tf_sniper:
    {
        if (nearest.second <= 300 && nearest.first->m_iHealth() < 75)
            return secondary;
        else if (nearest.second <= 400 && nearest.first->m_iHealth() < 75)
            return active_slot;
        else
            return primary;
    }
    case tf_pyro:
    {
        if (nearest.second > 450 && active_slot == secondary)
            return active_slot;
        else if (nearest.second <= 550)
            return primary;
        else
            return secondary;
    }
    case tf_engineer:
    {
        if (current_task == task::engineer)
        {
            // We cannot build the building if we keep switching away from the PDA
            if (current_engineer_task == task::engineer_task::build_building)
                return active_slot;
            // Use wrench to repair/upgrade
            if (current_engineer_task == task::engineer_task::upgradeorrepair_building)
                return melee;
        }
        return primary;
    }
    default:
    {
        if (nearest.second <= 400)
            return secondary;
        else if (nearest.second <= 500)
            return active_slot;
        else
            return primary;
    }
    }
}

static void updateSlot()
{
    static Timer slot_timer{};
    if (!slot_timer.test_and_set(300))
        return;
    if (CE_GOOD(LOCAL_E) && CE_GOOD(LOCAL_W) && !g_pLocalPlayer->life_state)
    {
        IClientEntity *weapon = RAW_ENT(LOCAL_W);
        // IsBaseCombatWeapon()
        if (re::C_BaseCombatWeapon::IsBaseCombatWeapon(weapon))
        {
            int slot    = re::C_BaseCombatWeapon::GetSlot(weapon) + 1;
            int newslot = getBestSlot(static_cast<slots>(slot));
            if (slot != newslot)
                g_IEngine->ClientCmd_Unrestricted(format("slot", newslot).c_str());
        }
    }
}

class ObjectDestroyListener : public IGameEventListener2
{
    virtual void FireGameEvent(IGameEvent *event)
    {
        if (!isHackActive() || !engineer_mode)
            return;
        // Get index of destroyed object
        int index = event->GetInt("index");
        // Destroyed Entity
        CachedEntity *ent = ENTITY(index);
        // Get Entry in the vector
        auto it = std::find(local_buildings.begin(), local_buildings.end(), ent);
        // If found, erase
        if (it != local_buildings.end())
            local_buildings.erase(it);
    }
};

ObjectDestroyListener &listener()
{
    static ObjectDestroyListener object{};
    return object;
}

static InitRoutine runinit([]() {
    g_IEventManager2->AddListener(&listener(), "object_destroyed", false);
    EC::Register(EC::CreateMove, CreateMove, "navbot", EC::early);
    EC::Register(
        EC::Shutdown, []() { g_IEventManager2->RemoveListener(&listener()); }, "navbot_shutdown");
});

void change(settings::VariableBase<bool> &, bool)
{
    nav::clearInstructions();
}

static InitRoutine routine([]() { enabled.installChangeCallback(change); });
} // namespace hacks::tf2::NavBot
