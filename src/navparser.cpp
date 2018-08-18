#include "navparser.hpp"

namespace nav
{
static CNavFile navfile(nullptr);
std::vector<CNavArea> areas;
// std::vector<CNavArea> SniperAreas;
bool init             = false;
bool pathfinding      = true;
bool ReadyForCommands = false;
std::vector<std::pair<int, int>> ignoredConnections;

static settings::Bool enabled{ "misc.pathing", "true" };

// Todo fix
int FindInVector(int id)
{
    for (int i = 0; i < areas.size(); i++)
    {
        if (areas.at(i).m_id == id)
            return i;
    }
}

struct MAP : public micropather::Graph
{
    std::unique_ptr<micropather::MicroPather> pather;
    bool IsIgnored(int currState, int connectionID)
    {
        for (int i = 0; i < ignoredConnections.size(); i++)
        {
            if (ignoredConnections.at(i).first == currState &&
                ignoredConnections.at(i).second == connectionID)
            {
                return true;
            }
        }
        return false;
    }
    Vector GetClosestCornerToArea(CNavArea *CornerOf, CNavArea *Target)
    {
        std::array<Vector, 4> corners;
        corners.at(0) = CornerOf->m_nwCorner; // NW
        corners.at(1) = CornerOf->m_seCorner; // SE
        corners.at(2) = Vector{ CornerOf->m_seCorner.x, CornerOf->m_nwCorner.y,
                                CornerOf->m_nwCorner.z }; // NE
        corners.at(3) = Vector{ CornerOf->m_nwCorner.x, CornerOf->m_seCorner.y,
                                CornerOf->m_seCorner.z }; // SW

        Vector bestVec;
        float bestDist = FLT_MAX;

        for (int i = 0; i < corners.size(); i++)
        {
            float dist = corners.at(i).DistTo(Target->m_center);
            if (dist < bestDist)
            {
                bestVec  = corners.at(i);
                bestDist = dist;
            }
        }

        Vector bestVec2;
        float bestDist2 = FLT_MAX;

        for (int i = 0; i < corners.size(); i++)
        {
            if (corners.at(i) == bestVec2)
                continue;
            float dist = corners.at(i).DistTo(Target->m_center);
            if (dist < bestDist2)
            {
                bestVec2  = corners.at(i);
                bestDist2 = dist;
            }
        }
        return (bestVec + bestVec2) / 2;
    }

    float GetZBetweenAreas(CNavArea *start, CNavArea *end)
    {
        float z1 = GetClosestCornerToArea(start, end).z;
        float z2 = GetClosestCornerToArea(end, start).z;

        return z2 - z1;
    }
    float LeastCostEstimate(void *stateStart, void *stateEnd)
    {
        CNavArea *start = static_cast<CNavArea *>(stateStart);
        CNavArea *end   = static_cast<CNavArea *>(stateEnd);
        float dist      = start->m_center.DistTo(end->m_center);
        return dist;
    }
    void AdjacentCost(void *state, MP_VECTOR<micropather::StateCost> *adjacent)
    {
        CNavArea *area   = static_cast<CNavArea *>(state);
        auto &neighbours = area->m_connections;
        for (auto i : neighbours)
        {
            if (GetZBetweenAreas(area, i.area) > 42)
                continue;
            if (IsIgnored(area->m_id, i.area->m_id))
                continue;
            micropather::StateCost cost;
            cost.state =
                static_cast<void *>(&areas.at(FindInVector(i.area->m_id)));
            cost.cost = area->m_center.DistTo(i.area->m_center);
            adjacent->push_back(cost);
        }
    }
    void PrintStateInfo(void *state)
    {
    }
    MAP(size_t size)
    {
        pather =
            std::make_unique<micropather::MicroPather>(this, size, 6, true);
    }

    ~MAP()
    {
    }
};

std::unique_ptr<MAP> TF2MAP;

void Init()
{
    // TODO: Improve performance
    std::string lvlname(g_IEngine->GetLevelName());
    int dotpos = lvlname.find('.');
    lvlname    = lvlname.substr(0, dotpos);

    std::string lvldir("/home/");
    passwd *pwd = getpwuid(getuid());
    lvldir.append(pwd->pw_name);
    lvldir.append("/.steam/steam/steamapps/common/Team Fortress 2/tf/");
    lvldir.append(lvlname);
    lvldir.append(".nav");
    logging::Info(format("Pathing: Nav File location: ", lvldir).c_str());

    areas.clear();
    navfile = CNavFile(lvldir.c_str());
    if (!navfile.m_isOK)
        logging::Info("Pathing: Invalid Nav File");
    else
    {
        size_t size = navfile.m_areas.size();
        logging::Info(
            format("Pathing: Number of areas to index:", size).c_str());
        areas.reserve(size);
        for (auto i : navfile.m_areas)
            areas.push_back(i);
        if (size > 7000)
            size = 7000;
        TF2MAP = std::make_unique<MAP>(size);
    }
    pathfinding = true;
}
std::string lastmap;
bool Prepare()
{
    if (!enabled)
        return false;
    if (!init)
    {
        if (lastmap == g_IEngine->GetLevelName())
        {
            init = true;
        }
        else
        {
            lastmap     = g_IEngine->GetLevelName();
            pathfinding = false;
            init        = true;
            Init();
        }
    }
    if (!pathfinding)
        return false;
    return true;
}

std::vector<int> localAreas(6);
int findClosestNavSquare(Vector vec)
{
    if (localAreas.size() > 5)
        localAreas.erase(localAreas.begin());
    std::vector<std::pair<int, CNavArea *>> overlapping;
    for (int i = 0; i < areas.size(); i++)
    {
        if (areas.at(i).IsOverlapping(vec))
        {
            if (std::count(localAreas.begin(), localAreas.end(), i) < 2)
                overlapping.push_back({ i, &areas.at(i) });
        }
    }

    float bestDist = FLT_MAX;
    int bestSquare = -1;
    for (int i = 0; i < overlapping.size(); i++)
    {
        float dist = overlapping.at(i).second->m_center.DistTo(vec);
        if (dist < bestDist)
        {
            bestDist   = dist;
            bestSquare = overlapping.at(i).first;
        }
    }
    if (bestSquare != -1)
    {
        if (vec == g_pLocalPlayer->v_Origin)
            localAreas.push_back(bestSquare);
        return bestSquare;
    }
    for (int i = 0; i < areas.size(); i++)
    {
        float dist = areas.at(i).m_center.DistTo(vec);
        if (dist < bestDist)
        {
            if (std::count(localAreas.begin(), localAreas.end(), i) < 2)
            {
                bestDist   = dist;
                bestSquare = i;
            }
        }
    }
    if (vec == g_pLocalPlayer->v_Origin)
        localAreas.push_back(bestSquare);
    return bestSquare;
}

std::vector<Vector> findPath(Vector loc, Vector dest)
{
    if (areas.empty())
        return std::vector<Vector>(0);
    int id_loc  = findClosestNavSquare(loc);
    int id_dest = findClosestNavSquare(dest);
    if (id_loc == -1 || id_dest == -1)
        return std::vector<Vector>(0);
    float cost;
    micropather::MPVector<void *> pathNodes;
    int result = TF2MAP->pather->Solve(static_cast<void *>(&areas.at(id_loc)),
                                       static_cast<void *>(&areas.at(id_dest)),
                                       &pathNodes, &cost);
    logging::Info(format(result).c_str());
    if (result == 1)
        return std::vector<Vector>(0);
    std::vector<Vector> path;
    for (int i = 0; i < pathNodes.size(); i++)
    {
        path.push_back(static_cast<CNavArea *>(pathNodes[i])->m_center);
    }
    path.push_back(dest);
    return path;
}

static Timer inactivity{};
static Timer lastJump{};
static std::vector<Vector> crumbs;
static bool ensureArrival;

bool NavTo(Vector dest, bool navToLocalCenter, bool persistent)
{
    if (CE_BAD(LOCAL_E))
        return false;
    if (!Prepare())
        return false;
    auto path = findPath(g_pLocalPlayer->v_Origin, dest);
    if (path.empty())
        return false;
    crumbs.clear();
    crumbs = std::move(path);
    if (!navToLocalCenter)
        crumbs.erase(crumbs.begin());
    inactivity.update();
    ensureArrival = persistent;
    localAreas.clear();
    return true;
}

static Vector lastArea = { 0.0f, 0.0f, 0.0f };
void ignoreConnection()
{
    if (crumbs.size() < 1)
        return;

    CNavArea *currnode = nullptr;
    for (int i = 0; i < areas.size(); i++)
    {
        if (areas.at(i).m_center == lastArea)
        {
            currnode = &areas.at(i);
            break;
        }
    }
    if (!currnode)
        return;

    CNavArea *nextnode = nullptr;
    for (int i = 0; i < areas.size(); i++)
    {
        if (areas.at(i).m_center == crumbs.at(0))
        {
            nextnode = &areas.at(i);
            break;
        }
    }
    if (!nextnode)
        return;

    for (auto i : currnode->m_connections)
    {
        if (i.area->m_id == nextnode->m_id)
        {
            ignoredConnections.push_back({ currnode->m_id, nextnode->m_id });
            TF2MAP->pather->Reset();
            return;
        }
    }
}

static Timer ignoreReset{};
void clearIgnores()
{
    if (ignoreReset.test_and_set(180000))
    {
        ignoredConnections.clear();
        if (TF2MAP && TF2MAP->pather)
            TF2MAP->pather->Reset();
    }
}

void CreateMove()
{
    if (!enabled)
        return;
    if (CE_BAD(LOCAL_E))
        return;
    clearIgnores();
    if (crumbs.empty())
    {
        ReadyForCommands = true;
        ensureArrival    = false;
        return;
    }
    ReadyForCommands = false;
    if (g_pLocalPlayer->v_Origin.DistTo(Vector{crumbs.at(0).x, crumbs.at(0).y, g_pLocalPlayer->v_Origin.z}) < 30.0f)
    {
        lastArea = crumbs.at(0);
        crumbs.erase(crumbs.begin());
        inactivity.update();
    }
    if (crumbs.empty())
        return;
    if (crumbs.at(0).z - g_pLocalPlayer->v_Origin.z > 18 &&
        lastJump.test_and_set(200))
        current_user_cmd->buttons |= IN_JUMP;
    if (inactivity.test_and_set(5000))
    {
        ignoreConnection();
        if (ensureArrival)
        {
            logging::Info("Pathing: NavBot inactive for too long. Ignoring "
                          "connection and finding another path...");
            // NavTo(crumbs.back(), true, true);
            crumbs = findPath(g_pLocalPlayer->v_Origin, crumbs.back());
            inactivity.update();
        }
        else
        {
            logging::Info(
                "Pathing: NavBot inactive for too long. Canceling tasks and "
                "ignoring connection...");
            crumbs.clear();
        }
        return;
    }
    WalkTo(crumbs.at(0));
}

CatCommand navinit("nav_init", "Debug nav init", [](const CCommand &args) {
    lastmap = "";
    init    = false;
    Prepare();
});

Vector loc;

CatCommand navset("nav_set", "Debug nav set",
                  [](const CCommand &args) { loc = LOCAL_E->m_vecOrigin(); });
CatCommand navprint("nav_print", "Debug nav print", [](const CCommand &args) {
    logging::Info(
        format(findClosestNavSquare(g_pLocalPlayer->v_Origin)).c_str());
});

CatCommand navfind("nav_find", "Debug nav find", [](const CCommand &args) {
    std::vector<Vector> path = findPath(g_pLocalPlayer->v_Origin, loc);
    if (path.empty())
    {
        logging::Info("Pathing: No path found");
        return;
    }
    std::string output = "Pathing: Path found! Path: ";
    for (int i = 0; i < path.size(); i++)
    {
        output.append(format(path.at(i).x, ",", format(path.at(i).y), " "));
    }
    logging::Info(output.c_str());
});

CatCommand navpath("nav_path", "Debug nav path", [](const CCommand &args) {
    if (NavTo(loc, true, true))
    {
        logging::Info("Pathing: Success! Walking to path...");
    }
    else
    {
        logging::Info("Pathing: Failed!");
    }
});

} // namespace nav
