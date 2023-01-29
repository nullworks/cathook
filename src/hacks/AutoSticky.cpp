/*
 * AutoSticky.cpp
 *
 *  Created on: Dec 2, 2016
 *      Author: nullifiedcat
 */

#include "common.hpp"
#include <PlayerTools.hpp>
#include <settings/Bool.hpp>
#include "soundcache.hpp"

namespace hacks::tf::autosticky
{
static settings::Boolean enable{ "autosticky.enable", "false" };
static settings::Boolean buildings{ "autosticky.buildings", "true" };
static settings::Int legit{ "autosticky.legit-mode", "0" };
static settings::Boolean dontblowmeup{ "autosticky.dontblowmeup", "true" };
static settings::Int min_dist{ "autosticky.min-dist", "130" };

// A storage array for ents
static std::vector<CachedEntity *> bombs;
static std::vector<CachedEntity *> targets;

bool shouldExplode;

// Function to tell when an ent is the local players own bomb
bool IsBomb(CachedEntity *ent)
{
    // Check if ent is a stickybomb
    if (ent->m_iClassID() != CL_CLASS(CTFGrenadePipebombProjectile))
        return false;
    if (CE_INT(ent, netvar.iPipeType) != 1)
        return false;

    // Check if the stickybomb is the players own
    if (HandleToIDX(CE_INT(ent, netvar.hThrower)) != g_pLocalPlayer->entity->m_IDX)
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

            // If settings allow, ignore taunting players
            // if (ignore_taunting && HasCondition<TFCond_Taunting>(ent)) return
            // false;

            // If settings don't allow, dont target cloaked players
            if (*legit && IsPlayerInvisible(ent))
                return false;
        }

        // Target is good
        return true;

        // Building specific
    }
    else if (ent->m_Type() == ENTITY_BUILDING)
    {
        return *buildings;
    }

    // Target isnt a good type
    return false;
}

static int wait_ticks = 0;
static void CreateMove()
{
    // Check user settings if auto-sticky is enabled
    if (!enable)
        return;

    // Check if game is a tf game
    // IF_GAME (!IsTF()) return;

    // Check if player is demoman
    if (g_pLocalPlayer->clazz != tf_demoman)
        return;

    // Check for sticky jumper, which is item 265, if true, return
    if (HasWeapon(LOCAL_E, 265))
        return;

    // Can't detonate while attacking
    if (current_user_cmd->buttons & IN_ATTACK || wait_ticks)
    {
        if (!wait_ticks)
            wait_ticks = 3;
        wait_ticks--;
        return;
    }

    // Clear the arrays
    bombs.clear();
    targets.clear();

    // Cycle through the ents and search for valid ents
    for (auto const &ent : entity_cache::valid_ents)
    {
        // Assign the for loops index to an ent

        // Check if ent is a bomb or suitable target and push to respective
        // arrays
        if (IsBomb(ent))
        {
            bombs.push_back(ent);
        }
        else if (IsTarget(ent))
        {
            targets.push_back(ent);
        }
    }

    // Loop through every target for a given bomb
    bool found = false;
    // Is Scottish resistance
    bool IsScottishResistance = HasWeapon(LOCAL_E, 130);
    // Loop once for every bomb in the array
    for (auto bomb : bombs)
    {
        // Don't blow yourself up
        if (dontblowmeup)
        {
            auto collideable = RAW_ENT(LOCAL_E)->GetCollideable();

            auto position = LOCAL_E->m_vecOrigin() + (collideable->OBBMins() + collideable->OBBMaxs()) / 2;
            if (bomb->m_vecOrigin().DistTo(position) < 130)
            {
                // Vis check the target from the bomb
                if (IsVectorVisible(bomb->m_vecOrigin(), position, true))
                    return;
            }
        }
        for (auto target : targets)
        {
            // Check distance to the target to see if the sticky will hit
            auto position = target->m_vecDormantOrigin();
            if (!position)
                continue;
            auto collideable = RAW_ENT(target)->GetCollideable();

            position = *position + (collideable->OBBMins() + collideable->OBBMaxs()) / 2;

            if (!found)
                if (bomb->m_vecOrigin().DistTo(*position) < *min_dist)
                {
                    // Vis check the target from the bomb
                    if (IsVectorVisible(bomb->m_vecOrigin(), *position, true))
                    {
                        // Check user settings if legit mode is off, if legit mode
                        // is off then detonate
                        shouldExplode = false;
                        if (!*legit)
                            shouldExplode = true;
                        else if (*legit == 1 && CE_GOOD(target) && IsVectorVisible(g_pLocalPlayer->v_Eye, bomb->m_vecOrigin(), true))
                            shouldExplode = true;
                        else if (*legit == 2 && CE_GOOD(target) && IsVectorVisible(g_pLocalPlayer->v_Eye, bomb->m_vecOrigin(), true) && IsVectorVisible(g_pLocalPlayer->v_Eye, *position, true))
                            shouldExplode = true;

                        if (shouldExplode)
                        {
                            // Aim at bomb
                            if (IsScottishResistance)
                                AimAt(g_pLocalPlayer->v_Eye, bomb->m_vecOrigin(), current_user_cmd);
                            // Use silent
                            g_pLocalPlayer->bUseSilentAngles = true;

                            // Continue the loop in case local player is within the explosion radius
                            found = true;
                            continue;
                        }
                    }
                }
        }
    }
    if (found)
        current_user_cmd->buttons |= IN_ATTACK2;
}

static InitRoutine EC([]() { EC::Register(EC::CreateMove, CreateMove, "auto_sticky", EC::average); });
} // namespace hacks::tf::autosticky
