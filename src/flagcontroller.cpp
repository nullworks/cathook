#include "flagcontroller.hpp"
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
static InitRoutine init(
    []()
    {
        EC::Register(EC::CreateMove, Update, "flagcontroller_update");
        EC::Register(EC::LevelInit, LevelInit, "flagcontroller_levelinit");
    });
} // namespace flagcontroller
