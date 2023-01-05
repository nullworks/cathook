#include "payloadcontroller.hpp"
#include "common.hpp"

namespace plcontroller
{

// Array that controls all the payloads for each team. Red team is first, then comes blue team.
static std::array<std::vector<CachedEntity *>, 2> payloads;
static Timer update_payloads{};

void Update()
{
    // We should update the payload list
    if (update_payloads.test_and_set(3000))
    {
        // Reset entries
        for (auto const &entry : payloads)
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
    for (auto payload : entry)
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
    for (auto const &entry : payloads)
        entry.clear();
}

static InitRoutine init(
    []()
    {
        EC::Register(EC::CreateMove, Update, "plcreatemove");
        EC::Register(EC::LevelInit, LevelInit, "levelinit_plcontroller");
    });
} // namespace plcontroller
