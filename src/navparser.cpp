#include "common.hpp"
#include "navparser.hpp"
#include <thread>
#include "micropather.h"
#include <pwd.h>
#include <boost/functional/hash.hpp>
#include <boost/container/flat_set.hpp>
#include <chrono>
#include "soundcache.hpp"
#include "MiscTemporary.hpp"
#include <CNavFile.h>

namespace nav
{

static settings::Boolean enabled{ "misc.pathing", "true" };
// Whether or not to run vischecks at pathtime
static settings::Boolean vischecks{ "misc.pathing.pathtime-vischecks", "true" };
static settings::Boolean vischeckBlock{ "misc.pathing.pathtime-vischeck-block", "false" };
static settings::Boolean draw{ "misc.pathing.draw", "false" };
static settings::Boolean look{ "misc.pathing.look-at-path", "false" };
static settings::Int stuck_time{ "misc.pathing.stuck-time", "4000" };
static settings::Int unreachable_time{ "misc.pathing.unreachable-time", "1000" };

static std::vector<CNavArea *> crumbs;
static Vector startPoint, endPoint;

enum ignore_status : uint8_t
{
    // Status is unknown
    unknown = 0,
    // Something like Z check failed, these are unchanging
    const_ignored,
    // LOS between areas is given
    vischeck_success,
    // No LOS between areas
    vischeck_failed,
    // Failed to actually walk thru connection
    explicit_ignored,
    // Danger like sentry gun or sticky
    danger_found
};

void ResetPather();
void repath();
void DoSlowAim(Vector &input_angle);

struct ignoredata
{
    ignore_status status{ unknown };
    float stucktime{ 0.0f };
    Timer ignoreTimeout{};
};

Vector GetClosestCornerToArea(CNavArea *CornerOf, const Vector &target)
{
    std::array<Vector, 4> corners{
        CornerOf->m_nwCorner,                                                       // NW
        CornerOf->m_seCorner,                                                       // SE
        { CornerOf->m_seCorner.x, CornerOf->m_nwCorner.y, CornerOf->m_nwCorner.z }, // NE
        { CornerOf->m_nwCorner.x, CornerOf->m_seCorner.y, CornerOf->m_seCorner.z }  // SW
    };

    Vector *bestVec = &corners[0], *bestVec2 = bestVec;
    float bestDist = corners[0].DistTo(target), bestDist2 = bestDist;

    for (size_t i = 1; i < corners.size(); i++)
    {
        float dist = corners[i].DistTo(target);
        if (dist < bestDist)
        {
            bestVec  = &corners[i];
            bestDist = dist;
        }
        if (corners[i] == *bestVec2)
            continue;

        if (dist < bestDist2)
        {
            bestVec2  = &corners[i];
            bestDist2 = dist;
        }
    }
    return (*bestVec + *bestVec2) / 2;
}

float getZBetweenAreas(CNavArea *start, CNavArea *end)
{
    float z1 = GetClosestCornerToArea(start, end->m_center).z;
    float z2 = GetClosestCornerToArea(end, start->m_center).z;

    return z2 - z1;
}

static std::unordered_map<std::pair<CNavArea *, CNavArea *>, ignoredata, boost::hash<std::pair<CNavArea *, CNavArea *>>> ignores;
namespace ignoremanager
{
static bool vischeck(CNavArea *begin, CNavArea *end)
{
    Vector first  = begin->m_center;
    Vector second = end->m_center;
    first.z += 42;
    second.z += 42;
    return IsVectorVisible(first, second, true, LOCAL_E, MASK_PLAYERSOLID);
}
static ignore_status runIgnoreChecks(CNavArea *begin, CNavArea *end)
{
    if (getZBetweenAreas(begin, end) > 42)
        return const_ignored;
    if (!vischecks)
        return vischeck_success;
    if (vischeck(begin, end))
        return vischeck_success;
    else
        return vischeck_failed;
}
static void updateDanger()
{
    for (size_t i = 0; i <= HIGHEST_ENTITY; i++)
    {
        CachedEntity *ent = ENTITY(i);
        if (CE_INVALID(ent))
            continue;
        if (ent->m_iClassID() == CL_CLASS(CObjectSentrygun))
        {
            if (!ent->m_bEnemy())
                continue;
            if (HasCondition<TFCond_Disguised>(LOCAL_E))
                continue;
            Vector loc = GetBuildingPosition(ent);
            if (RAW_ENT(ent)->IsDormant())
            {
                auto vec = ent->m_vecDormantOrigin();
                if (vec)
                {
                    loc -= RAW_ENT(ent)->GetCollideable()->GetCollisionOrigin();
                    loc += *vec;
                }
                else
                    continue;
            }
            // It's still building, ignore
            else if (CE_BYTE(ent, netvar.m_bBuilding) || CE_BYTE(ent, netvar.m_bPlacing))
                continue;

            // Keep track of good spots
            std::vector<CNavArea *> spot_list{};

            // Don't blacklist if local player is standing in it
            bool local_player_in_range = false;

            // Local player's nav area
            auto local_area = findClosestNavSquare(LOCAL_E->m_vecOrigin());

            // Actual building check
            for (auto &i : navfile->m_areas)
            {
                Vector area = i.m_center;
                area.z += 41.5f;
                if (loc.DistTo(area) > 1100)
                    continue;
                // Check if sentry can see us
                if (!IsVectorVisible(loc, area, true))
                    continue;
                // local player's nav area?
                if (local_area == &i)
                {
                    local_player_in_range = true;
                    break;
                }
                spot_list.push_back(&i);
            }

            // Local player is in the sentry range, let him nav
            if (local_player_in_range)
                continue;

            // Ignore these
            for (auto &i : spot_list)
            {
                ignoredata &data = ignores[{ i, nullptr }];
                data.status      = danger_found;
                data.ignoreTimeout.update();
                data.ignoreTimeout.last -= std::chrono::seconds(17);
            }
        }
        else if (ent->m_iClassID() == CL_CLASS(CTFGrenadePipebombProjectile))
        {
            if (!ent->m_bEnemy())
                continue;
            if (CE_INT(ent, netvar.iPipeType) == 1)
                continue;
            Vector loc = ent->m_vecOrigin();

            // Keep track of good spots
            std::vector<CNavArea *> spot_list{};

            // Don't blacklist if local player is standing in it
            bool local_player_in_range = false;

            // Local player's nav area
            auto local_area = findClosestNavSquare(LOCAL_E->m_vecOrigin());

            // Actual Sticky check
            for (auto &i : navfile->m_areas)
            {
                Vector area = i.m_center;
                area.z += 41.5f;
                if (loc.DistTo(area) > 130)
                    continue;
                // Check if in Sticky vis range
                if (!IsVectorVisible(loc, area, true))
                    continue;
                // local player's nav area?
                if (local_area == &i)
                {
                    local_player_in_range = true;
                    break;
                }
                spot_list.push_back(&i);
            }

            // Local player is in the sentry range, let him nav
            if (local_player_in_range)
                continue;

            // Ignore these
            for (auto &i : spot_list)
            {
                ignoredata &data = ignores[{ i, nullptr }];
                data.status      = danger_found;
                data.ignoreTimeout.update();
                data.ignoreTimeout.last -= std::chrono::seconds(17);
            }
        }
    }
}

static void checkPath()
{
    bool perform_repath = false;
    // Vischecks
    for (size_t i = 0; i < crumbs.size() - 1; i++)
    {
        CNavArea *begin = crumbs[i];
        CNavArea *end   = crumbs[i + 1];
        if (!begin || !end)
            continue;
        ignoredata &data = ignores[{ begin, end }];
        if (data.status == vischeck_failed)
            return;
        if (!vischeck(begin, end))
        {
            data.status = vischeck_failed;
            data.ignoreTimeout.update();
            perform_repath = true;
        }
        else if (ignores[{ end, nullptr }].status == danger_found)
        {
            perform_repath = true;
        }
    }
    if (perform_repath)
        repath();
}
// 0 = Not ignored, 1 = low priority, 2 = ignored
static int isIgnored(CNavArea *begin, CNavArea *end)
{
    if (ignores[{ end, nullptr }].status == danger_found)
        return 2;
    ignore_status status = ignores[{ begin, end }].status;
    if (status == unknown)
        status = runIgnoreChecks(begin, end);
    if (status == vischeck_success)
        return 0;
    else if (status == vischeck_failed)
        return 1;
    else
        return 2;
}
static bool addTime(ignoredata &connection, ignore_status status)
{
    connection.status = status;
    connection.ignoreTimeout.update();

    return true;
}
static bool addTime(CNavArea *begin, CNavArea *end, ignore_status status)
{
    logging::Info("Ignored Connection %i-%i", begin->m_id, end->m_id);
    return addTime(ignores[{ begin, end }], status);
}
static bool addTime(CNavArea *begin, CNavArea *end, Timer &time)
{
    if (!begin || !end)
    {
        // We can't reach the destination vector. Destination vector might
        // be out of bounds/reach.
        clearInstructions();
        return true;
    }
    using namespace std::chrono;
    // Check if connection is already known
    if (ignores.find({ begin, end }) == ignores.end())
    {
        ignores[{ begin, end }] = {};
    }
    ignoredata &connection = ignores[{ begin, end }];
    connection.stucktime += duration_cast<milliseconds>(system_clock::now() - time.last).count();
    if (connection.stucktime >= *stuck_time)
    {
        logging::Info("Ignored Connection %i-%i", begin->m_id, end->m_id);
        return addTime(connection, explicit_ignored);
    }
    return false;
}
static void reset()
{
    ignores.clear();
    ResetPather();
}
static void updateIgnores()
{
    static Timer update{};
    static Timer last_pather_reset{};
    static bool reset_pather = false;
    if (!update.test_and_set(500))
        return;
    updateDanger();
    if (crumbs.empty())
    {
        for (auto &i : ignores)
        {
            switch (i.second.status)
            {
            case explicit_ignored:
                if (i.second.ignoreTimeout.check(60000))
                {
                    i.second.status    = unknown;
                    i.second.stucktime = 0;
                    reset_pather       = true;
                }
                break;
            case unknown:
                break;
            case danger_found:
                if (i.second.ignoreTimeout.check(20000))
                {
                    i.second.status = unknown;
                    reset_pather    = true;
                }
                break;
            case vischeck_failed:
            case vischeck_success:
            default:
                if (i.second.ignoreTimeout.check(30000))
                {
                    i.second.status    = unknown;
                    i.second.stucktime = 0;
                    reset_pather       = true;
                }
                break;
            }
        }
    }
    else
        checkPath();
    if (reset_pather && last_pather_reset.test_and_set(10000))
    {
        reset_pather = false;
        ResetPather();
    }
}
static bool isSafe(CNavArea *area)
{
    return !(ignores[{ area, nullptr }].status == danger_found);
}
}; // namespace ignoremanager

struct Graph : public micropather::Graph
{
    std::unique_ptr<micropather::MicroPather> pather;

    Graph()
    {
        pather = std::make_unique<micropather::MicroPather>(this, 3000, 6, true);
    }
    ~Graph() override
    {
    }
    void AdjacentCost(void *state, MP_VECTOR<micropather::StateCost> *adjacent) override
    {
        CNavArea *center = static_cast<CNavArea *>(state);
        for (auto &i : center->m_connections)
        {
            CNavArea *neighbour = i.area;
            int isIgnored       = ignoremanager::isIgnored(center, neighbour);
            if (isIgnored == 2)
                continue;
            float distance = center->m_center.DistTo(i.area->m_center);
            if (isIgnored == 1)
            {
                if (*vischeckBlock)
                    continue;
                distance += 50000;
            }
            adjacent->emplace_back(micropather::StateCost{ reinterpret_cast<void *>(neighbour), distance });
        }
    }
    float LeastCostEstimate(void *stateStart, void *stateEnd) override
    {
        CNavArea *start = reinterpret_cast<CNavArea *>(stateStart);
        CNavArea *end   = reinterpret_cast<CNavArea *>(stateEnd);
        return start->m_center.DistTo(end->m_center);
    }
    void PrintStateInfo(void *) override
    {
    }
};

// Navfile containing areas
std::unique_ptr<CNavFile> navfile;
// Status
std::atomic<init_status> status;

// See "Graph", does pathing and stuff I guess
static Graph Map;

void initThread()
{
    char *p, cwd[PATH_MAX + 1], nav_path[PATH_MAX + 1], lvl_name[256];

    std::strncpy(lvl_name, g_IEngine->GetLevelName(), 255);
    lvl_name[255] = 0;
    p             = std::strrchr(lvl_name, '.');
    if (!p)
    {
        logging::Info("Failed to find dot in level name");
        return;
    }
    *p = 0;
    p  = getcwd(cwd, sizeof(cwd));
    if (!p)
    {
        logging::Info("Failed to get current working directory: %s", strerror(errno));
        return;
    }
    std::snprintf(nav_path, sizeof(nav_path), "%s/tf/%s.nav", cwd, lvl_name);
    logging::Info("Pathing: Nav File location: %s", nav_path);
    navfile = std::make_unique<CNavFile>(nav_path);
    if (!navfile->m_isOK)
    {
        navfile.reset();
        status = unavailable;
        return;
    }
    logging::Info("Pather: Initing with %i Areas", navfile->m_areas.size());
    status = on;
}

void init()
{
    endPoint.Invalidate();
    ignoremanager::reset();
    status = initing;
    std::thread thread;
    thread = std::thread(initThread);
    thread.detach();
}

bool prepare()
{
    if (!enabled)
        return false;
    init_status fast_status = status;
    if (fast_status == on)
        return true;
    if (fast_status == off)
    {
        init();
    }
    return false;
}

// This prevents the bot from gettings completely stuck in some cases
static std::vector<CNavArea *> findClosestNavSquare_localAreas(6);

// Function for getting closest Area to player, aka "LocalNav"
CNavArea *findClosestNavSquare(const Vector &vec)
{
    bool isLocal = vec == g_pLocalPlayer->v_Origin;
    if (isLocal && findClosestNavSquare_localAreas.size() > 5)
        findClosestNavSquare_localAreas.erase(findClosestNavSquare_localAreas.begin());

    float ovBestDist = FLT_MAX, bestDist = FLT_MAX;
    // If multiple candidates for LocalNav have been found, pick the closest
    CNavArea *ovBestSquare = nullptr, *bestSquare = nullptr;
    for (auto &i : navfile->m_areas)
    {
        // Make sure we're not stuck on the same area for too long
        if (isLocal && std::count(findClosestNavSquare_localAreas.begin(), findClosestNavSquare_localAreas.end(), &i) >= 3)
        {
            continue;
        }
        float dist = i.m_center.DistTo(vec);
        if (dist < bestDist)
        {
            bestDist   = dist;
            bestSquare = &i;
        }
        // Check if we are within x and y bounds of an area
        if (ovBestDist >= dist || !i.IsOverlapping(vec) || !IsVectorVisible(vec, i.m_center, true, LOCAL_E, MASK_PLAYERSOLID))
        {
            continue;
        }
        ovBestDist   = dist;
        ovBestSquare = &i;
    }
    if (!ovBestSquare)
        ovBestSquare = bestSquare;

    if (isLocal)
        findClosestNavSquare_localAreas.push_back(ovBestSquare);

    return ovBestSquare;
}

std::vector<CNavArea *> findPath(const Vector &start, const Vector &end)
{
    using namespace std::chrono;

    if (status != on)
        return {};

    CNavArea *local, *dest;
    if (!(local = findClosestNavSquare(start)) || !(dest = findClosestNavSquare(end)))
        return {};

    logging::Info("Start: (%f,%f,%f)", local->m_center.x, local->m_center.y, local->m_center.z);
    logging::Info("End: (%f,%f,%f)", dest->m_center.x, dest->m_center.y, dest->m_center.z);
    float cost;
    std::vector<CNavArea *> pathNodes;

    time_point begin_pathing = high_resolution_clock::now();
    int result               = Map.pather->Solve(reinterpret_cast<void *>(local), reinterpret_cast<void *>(dest), reinterpret_cast<std::vector<void *> *>(&pathNodes), &cost);
    long long timetaken      = duration_cast<nanoseconds>(high_resolution_clock::now() - begin_pathing).count();
    logging::Info("Pathing: Pather result: %i. Time taken (NS): %lld", result, timetaken);
    // If no result found, return empty Vector
    if (result == micropather::MicroPather::NO_SOLUTION)
        return {};

    return pathNodes;
}

static Vector loc(0.0f, 0.0f, 0.0f);
static CNavArea *last_area = nullptr;
bool ReadyForCommands      = true;
static Timer inactivity{};
int curr_priority         = 0;
static bool ensureArrival = false;

bool navTo(const Vector &destination, int priority, bool should_repath, bool nav_to_local, bool is_repath)
{
    if (!prepare() || priority < curr_priority)
        return false;

    auto path = findPath(g_pLocalPlayer->v_Origin, destination);
    if (path.empty())
    {
        clearInstructions();
        return false;
    }
    auto crumb = crumbs.begin();
    if (crumb != crumbs.end() && ignoremanager::addTime(last_area, *crumb, inactivity))
        ResetPather();

    auto path_it = path.begin();
    last_area    = *path_it;
    if (!nav_to_local)
    {
        path.erase(path_it);
        if (path.empty())
            return false;
    }
    inactivity.update();
    if (!is_repath)
        findClosestNavSquare_localAreas.clear();

    ensureArrival    = should_repath;
    ReadyForCommands = false;
    curr_priority    = priority;
    crumbs           = std::move(path);
    endPoint         = destination;
    return true;
}

void repath()
{
    if (!ensureArrival)
        return;

    Vector last;
    if (!crumbs.empty())
        last = crumbs.back()->m_center;
    else if (endPoint.IsValid())
        last = endPoint;
    else
        return;

    clearInstructions();
    ResetPather();
    navTo(last, curr_priority, true, true, true);
}

static Timer last_jump{};
// Main movement function, gets path from NavTo
static void cm()
{
    if (!enabled || status != on)
        return;
    if (CE_BAD(LOCAL_E) || CE_BAD(LOCAL_W))
        return;
    if (!LOCAL_E->m_bAlivePlayer())
    {
        // Clear path if player dead
        clearInstructions();
        return;
    }
    ignoremanager::updateIgnores();
    auto crumb = crumbs.begin();
    const Vector *crumb_vec;
    // Crumbs empty, prepare for next instruction
    if (crumb == crumbs.end())
    {
        if (endPoint.IsValid())
            crumb_vec = &endPoint;
        else
        {
            curr_priority    = 0;
            ReadyForCommands = true;
            ensureArrival    = false;
            return;
        }
    }
    else
        crumb_vec = &(*crumb)->m_center;

    ReadyForCommands = false;
    // Remove old crumbs
    if (g_pLocalPlayer->v_Origin.DistTo(*crumb_vec) < 50.0f)
    {
        inactivity.update();
        if (crumb_vec == &endPoint)
        {
            endPoint.Invalidate();
            return;
        }
        last_area = *crumb;
        crumbs.erase(crumb);
        crumb = crumbs.begin();
        if (crumb == crumbs.end())
        {
            if (!endPoint.IsValid())
            {
                logging::Info("navparser.cpp cm -> endPoint.IsValid() == false");
                return;
            }
            crumb_vec = &endPoint;
        }
    }
    if (look && LookAtPathTimer.check(1000))
    {
        Vector next{ crumb_vec->x, crumb_vec->y, g_pLocalPlayer->v_Eye.z };
        next = GetAimAtAngles(g_pLocalPlayer->v_Eye, next);
        DoSlowAim(next);
        current_user_cmd->viewangles = next;
    }
    // Detect when jumping is necessary
    if ((!(g_pLocalPlayer->holding_sniper_rifle && g_pLocalPlayer->bZoomed) && crumb_vec->z - g_pLocalPlayer->v_Origin.z > 18 && last_jump.test_and_set(200)) || (last_jump.test_and_set(200) && inactivity.check(*stuck_time / 2)))
    {
        current_user_cmd->buttons |= IN_JUMP;
    }
    // Walk to next crumb
    WalkTo(*crumb_vec);
    /* If can't go through for some time (doors aren't instantly opening)
     * ignore that connection
     * Or if inactive for too long
     */
    if (inactivity.check(*stuck_time) || (inactivity.check(*unreachable_time) && !IsVectorVisible(g_pLocalPlayer->v_Origin, *crumb_vec + Vector(.0f, .0f, 41.5f), false, LOCAL_E, MASK_PLAYERSOLID)))
    {
        /* crumb is invalid if endPoint is used */
        if (crumb_vec != &endPoint)
            ignoremanager::addTime(last_area, *crumb, inactivity);

        repath();
        return;
    }
}

#if ENABLE_VISUALS
static void drawcrumbs()
{
    if (!enabled || !draw)
        return;
    if (CE_BAD(LOCAL_E) || CE_BAD(LOCAL_W))
        return;
    if (!LOCAL_E->m_bAlivePlayer())
        return;
    if (crumbs.size() < 2)
        return;
    for (size_t i = 0; i < crumbs.size(); i++)
    {
        Vector wts1, wts2, *o1, *o2;
        if (crumbs.size() - 1 == i)
        {
            if (!endPoint.IsValid())
                break;

            o2 = &endPoint;
        }
        else
            o2 = &crumbs[i + 1]->m_center;

        o1 = &crumbs[i]->m_center;
        if (draw::WorldToScreen(*o1, wts1) && draw::WorldToScreen(*o2, wts2))
        {
            draw::Line(wts1.x, wts1.y, wts2.x - wts1.x, wts2.y - wts1.y, colors::white, 0.3f);
        }
    }
    Vector wts;
    if (!draw::WorldToScreen(crumbs[0]->m_center, wts))
        return;
    draw::Rectangle(wts.x - 4, wts.y - 4, 8, 8, colors::white);
    draw::RectangleOutlined(wts.x - 4, wts.y - 4, 7, 7, colors::white, 1.0f);
}
#endif

static InitRoutine runinit([]() {
    EC::Register(EC::CreateMove, cm, "cm_navparser", EC::average);
#if ENABLE_VISUALS
    EC::Register(EC::Draw, drawcrumbs, "draw_navparser", EC::average);
#endif
});

void ResetPather()
{
    Map.pather->Reset();
}

bool isSafe(CNavArea *area)
{
    return ignoremanager::isSafe(area);
}

static CatCommand nav_find("nav_find", "Debug nav find", []() {
    auto path = findPath(g_pLocalPlayer->v_Origin, loc);
    if (path.empty())
    {
        logging::Info("Pathing: No path found");
        return;
    }
    std::string output = "Pathing: Path found! Path: ";
    for (int i = 0; i < path.size(); i++)
    {
        output.append(format(path[i]->m_center.x, ",", format(path[i]->m_center.y), " "));
    }
    logging::Info(output.c_str());
});

static CatCommand nav_set("nav_set", "Debug nav find", []() { loc = g_pLocalPlayer->v_Origin; });

static CatCommand nav_init("nav_init", "Debug nav init", []() {
    status = off;
    prepare();
});

static CatCommand nav_path("nav_path", "Debug nav path", []() { navTo(loc); });

static CatCommand nav_path_no_local("nav_path_no_local", "Debug nav path", []() { navTo(loc, 5, false, false); });

static CatCommand nav_reset_ignores("nav_reset_ignores", "Reset all ignores.", []() { ignoremanager::reset(); });

void DoSlowAim(Vector &input_angle)
{
    static float slow_change_dist_y{};
    static float slow_change_dist_p{};

    auto viewangles = current_user_cmd->viewangles;

    // Yaw
    if (viewangles.y != input_angle.y)
    {

        // Check if input angle and user angle are on opposing sides of yaw so
        // we can correct for that
        bool slow_opposing = false;
        if ((input_angle.y < -90 && viewangles.y > 90) || (input_angle.y > 90 && viewangles.y < -90))
            slow_opposing = true;

        // Direction
        bool slow_dir = false;
        if (slow_opposing)
        {
            if (input_angle.y > 90 && viewangles.y < -90)
                slow_dir = true;
        }
        else if (viewangles.y > input_angle.y)
            slow_dir = true;

        // Speed, check if opposing. We dont get a new distance due to the
        // opposing sides making the distance spike, so just cheap out and reuse
        // our last one.
        if (!slow_opposing)
            slow_change_dist_y = std::abs(viewangles.y - input_angle.y) / 5;

        // Move in the direction of the input angle
        if (slow_dir)
            input_angle.y = viewangles.y - slow_change_dist_y;
        else
            input_angle.y = viewangles.y + slow_change_dist_y;
    }

    // Pitch
    if (viewangles.x != input_angle.x)
    {
        // Get speed
        slow_change_dist_p = std::abs(viewangles.x - input_angle.x) / 5;

        // Move in the direction of the input angle
        if (viewangles.x > input_angle.x)
            input_angle.x = viewangles.x - slow_change_dist_p;
        else
            input_angle.x = viewangles.x + slow_change_dist_p;
    }

    // Clamp as we changed angles
    fClampAngle(input_angle);
}

void clearInstructions()
{
    crumbs.clear();
    endPoint.Invalidate();
    curr_priority = 0;
}
static CatCommand nav_stop("nav_cancel", "Cancel Navigation", []() { clearInstructions(); });
} // namespace nav
