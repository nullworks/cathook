/*
 * AutoDetonator.cpp
 *
 *  Created on: Mar 21, 2018
 *      Author: nullifiedcat & Lighty
 */

#include <PlayerTools.hpp>
#include <settings/Bool.hpp>
#include "common.hpp"

namespace hacks::tf::autodetonator
{
static settings::Boolean enable{ "auto-detonator.enable", "0" };
static settings::Boolean legit{ "auto-detonator.ignore-cloaked", "0" };

// A storage array for ents
std::vector<CachedEntity *> flares;
std::vector<CachedEntity *> targets;

// Function to tell when an ent is the local players own flare
bool IsFlare(CachedEntity *ent)
{
    // Check if ent is a flare
    if (ent->m_iClassID() != CL_CLASS(CTFProjectile_Flare))
        return false;

    // Check if we're the owner of the flare
    if (HandleToIDX((CE_INT(ent, netvar.m_hOwnerEntity))) != LOCAL_W->m_IDX)
        return false;

    // Check passed, return true
    return true;
}

// Function to check ent if it is a good target
bool IsTarget(CachedEntity *ent)
{
    // Check if target is The local player
    if (ent == LOCAL_E)
        return false;

    // Check if target is an enemy
    if (!ent->m_bEnemy())
        return false;

    // Player specific
    if (ent->m_Type() == ENTITY_PLAYER)
    {
        // Dont detonate on dead players
        if (!ent->m_bAlivePlayer())
            return false;

        // Global checks
        if (!player_tools::shouldTarget(ent))
            return false;
        IF_GAME(IsTF())
        {
            // Dont target invulnerable players, ex: uber, bonk
            if (IsPlayerInvulnerable(ent))
                return false;

            // If settings allow, dont target cloaked players
            if (legit && IsPlayerInvisible(ent))
                return false;
        }

        // Target is good
        return true;
    }
    // Target isnt a good type
    return false;
}

// Function called by game for movement
static void CreateMove()
{
    // Check if it enabled in settings, weapon entity exists and current
    // class is pyro
    if (!enable || CE_BAD(LOCAL_W) || g_pLocalPlayer->clazz != tf_pyro)
        return;

    // Clear the arrays
    flares.clear();
    targets.clear();

    // Cycle through the ents and search for valid ents
    for (auto const &ent : entity_cache::valid_ents)
    {

        // Check if ent is a flare or suitable target and push to respective
        // arrays
        if (IsFlare(ent))
        {
            flares.push_back(ent);
        }
        else if (IsTarget(ent))
        {
            targets.push_back(ent);
        }
    }
    for (auto flare : flares)
    {
        // Loop through every target
        for (auto target : targets)
        {
            // Check distance to the target to see if the flare will hit
            if (flare->m_vecOrigin().DistToSqr(target->m_vecOrigin()) < 22000)
            {
                // Vis check the target from the flare
                if (VisCheckEntFromEnt(flare, target))
                {
                    // Detonate
                    current_user_cmd->buttons |= IN_ATTACK2;

                    return;
                }
            }
        }
    }
    // End of function, just return
    return;
}

static InitRoutine EC([]() { EC::Register(EC::CreateMove, CreateMove, "auto_detonator", EC::average); });
} // namespace hacks::tf::autodetonator
