/*
 * AutoDetonator.cpp
 *
 *  Created on: Mar 21, 2018
 *      Author: nullifiedcat & Lighty
 */

#include "common.hpp"

namespace hacks
{
namespace tf
{
namespace autodetonator
{

// Vars for user settings
static CatVar enabled(CV_SWITCH, XORSTR("detonator_enabled"), XORSTR("0"),
                      XORSTR("Auto-Detonator-detonator"),
                      XORSTR("Master auto detonator switch"));
static CatVar legit(CV_SWITCH, XORSTR("detonator_legit"), XORSTR("0"), XORSTR("Ignore invis"),
                    XORSTR("Ignores invis spies"));

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
    if ((CE_INT(ent, 0x894) & 0xFFF) != LOCAL_W->m_IDX)
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
        // Dont detonate on friendly players
        if (playerlist::IsFriendly(playerlist::AccessData(ent).state))
            return false;

        IF_GAME(IsTF())
        {
            // Dont target invulnerable players, ex: uber, bonk
            if (IsPlayerInvulnerable(ent))
                return false;

            // If settings allow, ignore taunting players
            if (ignore_taunting && HasCondition<TFCond_Taunting>(ent))
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
void CreateMove()
{
    // Check user settings if auto detonator is enabled
    if (!enabled)
        return;

    // Check if player is pyro
    if (g_pLocalPlayer->clazz != tf_pyro)
        return;

    // Clear the arrays
    flares.clear();
    targets.clear();

    // Cycle through the ents and search for valid ents
    for (int i = 0; i < HIGHEST_ENTITY; i++)
    {
        // Assign the for loops tick number to an ent
        CachedEntity *ent = ENTITY(i);
        // Check for dormancy and if valid
        if (CE_BAD(ent))
            continue;
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
                    g_pUserCmd->buttons |= IN_ATTACK2;

                    return;
                }
            }
        }
    }
    // End of function, just return
    return;
}
}
}
}
