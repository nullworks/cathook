#include "CaptureLogic.hpp"
#include "common.hpp"

namespace flagcontroller
{

std::array<flag_info, 2> flags;
bool is_ctf = true;

// Check if a flag is good or not
bool isGoodFlag(CachedEntity *flag)
{
    if (CE_INVALID(flag) || flag->m_iClassID() != CL_CLASS(CCaptureFlag))
        return false;
    return true;
}

void Update()
{
    // Not ctf, no need to update
    if (!is_ctf)
        return;
    // Find flags if missing
    if (!flags[0].ent || !flags[1].ent)
        for (auto const &ent : entity_cache::valid_ents)
        {
            // We cannot identify a bad entity as a flag due to the unreliability of it
            if (ent->m_iClassID() != CL_CLASS(CCaptureFlag))
                continue;

            // Store flags
            if (!flags[0].ent)
                flags[0].ent = ent;
            else if (ent != flags[0].ent)
                flags[1].ent = ent;
        }
    // Update flag data
    for (auto &flag : flags)
    {
        // Not inited
        if (!flag.ent)
            continue;

        // Bad Flag, reset
        if (!isGoodFlag(flag.ent))
        {
            flag = flag_info();
            continue;
        }

        // Cannot use "bad" flag, but it is still potentially valid
        if (CE_BAD(flag.ent))
            continue;

        int flag_type = CE_INT(flag.ent, netvar.m_nFlagType);

        // Only CTF support for now
        if (flag_type != TF_FLAGTYPE_CTF)
            continue;

        // Assign team if missing
        if (flag.team == TEAM_UNK)
            flag.team = flag.ent->m_iTeam();

        // Assign spawn point if it is missing and the flag is at spawn
        if (!flag.spawn_pos)
        {
            int flag_status = CE_INT(flag.ent, netvar.m_nFlagStatus);

            // Flag is home
            if (flag_status == TF_FLAGINFO_HOME)
                flag.spawn_pos = flag.ent->m_vecOrigin();
        }
    }
}

void LevelInit()
{
    // Resez everything
    for (auto &flag : flags)
        flag = flag_info();
    is_ctf = true;
}

// Get the info for the flag
flag_info getFlag(int team)
{
    for (auto &flag : flags)
    {
        if (flag.team == team)
            return flag;
    }
    // None found
    return flag_info();
}

// Get the Position of a flag on a specific team
Vector getPosition(CachedEntity *flag)
{
    return flag->m_vecOrigin();
}

std::optional<Vector> getPosition(int team)
{
    auto flag = getFlag(team);
    if (isGoodFlag(flag.ent))
        return getPosition(flag.ent);
    // No good flag
    return std::nullopt;
}

// Get the person carrying the flag
CachedEntity *getCarrier(CachedEntity *flag)
{
    int entidx = HandleToIDX(CE_INT(flag, netvar.m_hOwnerEntity));
    // None/Invalid
    if (IDX_BAD(entidx))
        return nullptr;
    CachedEntity *carrier = ENTITY(entidx);
    // Carrier is invalid
    if (CE_BAD(carrier) || carrier->m_Type() != ENTITY_PLAYER)
        return nullptr;
    return carrier;
}

// Wrapper for when you don't have the entity
CachedEntity *getCarrier(int team)
{
    auto flag = getFlag(team);
    // Only use good flags
    if (isGoodFlag(flag.ent))
        return getCarrier(flag.ent);
    return nullptr;
}

// Get the status of the flag (Home, being carried, dropped)
ETFFlagStatus getStatus(CachedEntity *flag)
{
    return (ETFFlagStatus) CE_INT(flag, netvar.m_nFlagStatus);
}

ETFFlagStatus getStatus(int team)
{
    auto flag = getFlag(team);
    // Only use good flags
    if (isGoodFlag(flag.ent))
        return getStatus(flag.ent);
    // Mark as home if nothing is found
    return TF_FLAGINFO_HOME;
}
} // namespace flagcontroller

namespace plcontroller
{

// Valid_ents that controls all the payloads for each team. Red team is first, then comes blue team.
static std::array<std::vector<CachedEntity *>, 2> payloads;
static Timer update_payloads{};

void Update()
{
    // We should update the payload list
    if (update_payloads.test_and_set(3000))
    {
        // Reset entries
        for (auto &entry : payloads)
            entry.clear();

        for (auto const &ent : entity_cache::valid_ents)
        {

            // Not the object we need or invalid (team)
            if (ent->m_iClassID() != CL_CLASS(CObjectCartDispenser) || ent->m_iTeam() < TEAM_RED || ent->m_iTeam() > TEAM_BLU)
                continue;
            int team = ent->m_iTeam();

            // Add new entry for the team
            payloads.at(team - TEAM_RED).push_back(ent);
        }
    }
}
std::optional<Vector> getClosestPayload(Vector source, int team)
{
    // Invalid team
    if (team < TEAM_RED || team > TEAM_BLU)
        return std::nullopt;
    // Convert to index
    int index  = team - TEAM_RED;
    auto entry = payloads[index];

    float best_distance = FLT_MAX;
    std::optional<Vector> best_pos;

    // Find best payload
    for (auto &payload : entry)
    {
        if (CE_BAD(payload) || payload->m_iClassID() != CL_CLASS(CObjectCartDispenser))
            continue;
        if (payload->m_vecOrigin().DistTo(source) < best_distance)
        {
            best_pos      = payload->m_vecOrigin();
            best_distance = payload->m_vecOrigin().DistTo(source);
        }
    }
    return best_pos;
}

void LevelInit()
{
    for (auto &entry : payloads)
        entry.clear();
}
} // namespace plcontroller

namespace cpcontroller
{

std::array<cp_info, MAX_CONTROL_POINTS+1> controlpoint_data;
CachedEntity *objective_resource = nullptr;

struct point_ignore
{
    std::string mapname;
    int point_idx;
    point_ignore(std::string str, int idx) : mapname{ str }, point_idx{ idx } {};
};

// TODO: Find a way to fix these edge-cases.
// clang-format off
std::array<point_ignore, 1> ignore_points
{
    point_ignore("cp_steel", 4)
};
// clang-format on

// This function updates the Entity used for the Object resource
void UpdateObjectiveResource()
{
    // Already set and valid
    if (CE_GOOD(objective_resource) && objective_resource->m_iClassID() == CL_CLASS(CTFObjectiveResource))
        return;
    // Find ObjectiveResource and gamerules
    for (auto const &ent : entity_cache::valid_ents)
    {
        if (ent->m_iClassID() != CL_CLASS(CTFObjectiveResource))
            continue;
        // Found it
        objective_resource = ent;
        break;
    }
}

// A Bunch of defines for netvars that don't deserve their own function
#define GET_NUM_CONTROL_POINTS() (CE_INT(objective_resource, netvar.m_iNumControlPoints))
#define GET_OWNING_TEAM(index) ((&CE_INT(objective_resource, netvar.m_iOwningTeam))[index])
#define GET_BASE_CONTROL_POINT_FOR_TEAM(team) ((&CE_INT(objective_resource, netvar.m_iBaseControlPoints))[team])
#define GET_CP_LOCKED(index) ((&CE_VAR(objective_resource, netvar.m_bCPLocked, bool))[index])
#define IN_MINI_ROUND(index) ((&CE_VAR(objective_resource, netvar.m_bInMiniRound, bool))[index])

bool TeamCanCapPoint(int index, int team)
{
    int arr_index = index + team * MAX_CONTROL_POINTS;
    return (&CE_VAR(objective_resource, netvar.m_bTeamCanCap, bool))[arr_index];
}

int GetPreviousPointForPoint(int index, int team, int previndex)
{
    int iIntIndex = previndex + (index * MAX_PREVIOUS_POINTS) + (team * MAX_CONTROL_POINTS * MAX_PREVIOUS_POINTS);
    return (&CE_INT(objective_resource, netvar.m_iPreviousPoints))[iIntIndex];
}

int GetFarthestOwnedControlPoint(int team)
{
    int iOwnedEnd = GET_BASE_CONTROL_POINT_FOR_TEAM(team);
    if (iOwnedEnd == -1)
        return -1;

    int iNumControlPoints = GET_NUM_CONTROL_POINTS();
    int iWalk             = 1;
    int iEnemyEnd         = iNumControlPoints - 1;
    if (iOwnedEnd != 0)
    {
        iWalk     = -1;
        iEnemyEnd = 0;
    }

    // Walk towards the other side, and find the farthest owned point
    int iFarthestPoint = iOwnedEnd;
    for (int iPoint = iOwnedEnd; iPoint != iEnemyEnd; iPoint += iWalk)
    {
        // If we've hit a point we don't own, we're done
        if (GET_OWNING_TEAM(iPoint) != team)
            break;

        iFarthestPoint = iPoint;
    }

    return iFarthestPoint;
}

// Can we cap this point?
bool isPointUseable(int index, int team)
{
    // We Own it, can't cap it
    if (GET_OWNING_TEAM(index) == team)
        return false;

    // Can we cap the point?
    if (!TeamCanCapPoint(index, team))
        return false;

    // We are playing a sectioned map, check if the CP is in it
    if (CE_VAR(objective_resource, netvar.m_bPlayingMiniRounds, bool) && !IN_MINI_ROUND(index))
        return false;

    // Is the point locked?
    if (GET_CP_LOCKED(index))
        return false;

    // Linear cap means that it won't require previous points, bail
    static auto tf_caplinear = g_ICvar->FindVar("tf_caplinear");
    if (tf_caplinear && !tf_caplinear->GetBool())
        return true;

    // Any previous points necessary?
    int iPointNeeded = GetPreviousPointForPoint(index, team, 0);

    // Points set to require themselves are always cappable
    if (iPointNeeded == index)
        return true;

    // No required points specified? Require all previous points.
    if (iPointNeeded == -1)
    {
        // No Mini rounds
        if (!CE_VAR(objective_resource, netvar.m_bPlayingMiniRounds, bool))
        {
            // No custom previous point, team must own all previous points
            int iFarthestPoint = GetFarthestOwnedControlPoint(team);
            return (abs(iFarthestPoint - index) <= 1);
        }
        // We got a section map
        else
        {
            // Tf2 itself does not seem to have any more code for this, so here goes
            return true;
        }
    }

    // Loop through each previous point and see if the team owns it
    for (int iPrevPoint = 0; iPrevPoint < MAX_PREVIOUS_POINTS; iPrevPoint++)
    {
        iPointNeeded = GetPreviousPointForPoint(index, team, iPrevPoint);
        if (iPointNeeded != -1)
        {
            // We don't own the needed points
            if (GET_OWNING_TEAM(iPointNeeded) != team)
                return false;
        }
    }
    return true;
}

// Don't constantly update the cap status
static Timer capstatus_update{};
// Update the control points
void UpdateControlPoints()
{
    // No objective ressource, can't run
    if (!objective_resource)
        return;
    int num_cp = CE_INT(objective_resource, netvar.m_iNumControlPoints);
    // No control points
    if (!num_cp)
        return;
    // Clear the invalid controlpoints
    if (num_cp <= MAX_CONTROL_POINTS)
        for (int i = num_cp; i < MAX_CONTROL_POINTS; i++)
            controlpoint_data[i] = cp_info();

    for (int i = 0; i < num_cp; i++)
    {
        auto &data    = controlpoint_data[i];
        data.cp_index = i;

        // Update position (m_vCPPositions[index])
        data.position = (&CE_VAR(objective_resource, netvar.m_vCPPositions, Vector))[i];
    }

    if (capstatus_update.test_and_set(1000))
        for (int i = 0; i < num_cp; i++)
        {
            auto &data = controlpoint_data[i];
            // Check accessibility for both teams, requires alot of checks
            data.can_cap.at(0) = isPointUseable(i, TEAM_RED);
            data.can_cap.at(1) = isPointUseable(i, TEAM_BLU);
        }
}

// Get the closest controlpoint to cap
std::optional<Vector> getClosestControlPoint(Vector source, int team)
{
    // No resource for it
    if (!objective_resource)
        return std::nullopt;
    // Check if it's a cp map
    static auto tf_gamemode_cp = g_ICvar->FindVar("tf_gamemode_cp");
    if (!tf_gamemode_cp)
    {
        tf_gamemode_cp = g_ICvar->FindVar("tf_gamemode_cp");
        return std::nullopt;
    }
    if (!tf_gamemode_cp->GetBool())
        return std::nullopt;

    // Map team to 0-1 and check If Valid
    int team_idx = team - TEAM_RED;
    if (team_idx < 0 || team_idx > 1)
        return std::nullopt;

    // No controlpoints
    if (!GET_NUM_CONTROL_POINTS())
        return std::nullopt;

    int ignore_index = -1;
    // Do the points need checking because of the map?
    auto levelname = GetLevelName();
    for (auto &ignore : ignore_points)
    {
        // Try to find map name in bad point array
        if (levelname.find(ignore.mapname) != levelname.npos)
            ignore_index = ignore.point_idx;
    }

    // Find the best and closest control point
    std::optional<Vector> best_cp;
    float best_distance = FLT_MAX;
    for (auto &cp : controlpoint_data)
    {
        // Ignore this point
        if (cp.cp_index == ignore_index)
            continue;
        // They can cap
        if (cp.can_cap.at(team_idx))
        {
            // Is it closer?
            if (cp.position && (*cp.position).DistTo(source) < best_distance)
            {
                best_distance = (*cp.position).DistTo(source);
                best_cp       = cp.position;
            }
        }
    }

    return best_cp;
}

void LevelInit()
{
    for (auto &cp : controlpoint_data)
        cp = cp_info();
    objective_resource = nullptr;
}

void Update()
{
    UpdateControlPoints();
    UpdateObjectiveResource();
}
} // namespace cpcontroller

// Main handlers
static void CreateMove()
{
    flagcontroller::Update();
    plcontroller::Update();
    cpcontroller::Update();
}

void LevelInit()
{
    flagcontroller::LevelInit();
    plcontroller::LevelInit();
    cpcontroller::LevelInit();
}

static InitRoutine init(
    []()
    {
        EC::Register(EC::CreateMove, CreateMove, "capturelogic_update");
        EC::Register(EC::LevelInit, LevelInit, "capturelogic_levelinit");
    });
