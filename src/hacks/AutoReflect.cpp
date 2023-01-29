/*
 * AutoReflect.cpp
 *
 *  Created on: Nov 18, 2016
 *      Author: nullifiedcat
 */

#include "common.hpp"
#include <settings/Bool.hpp>

namespace hacks::tf::autoreflect
{
static settings::Boolean enable{ "autoreflect.enable", "false" };
static settings::Boolean idle_only{ "autoreflect.idle-only", "false" };
static settings::Boolean legit{ "autoreflect.legit", "false" };
static settings::Boolean dodgeball{ "autoreflect.dodgeball", "false" };
static settings::Button blastkey{ "autoreflect.button", "<null>" };

static settings::Boolean proj_default{ "autoreflect.default", "false" };
static settings::Boolean stickies{ "autoreflect.stickies", "true" };
static settings::Boolean pipes{ "autoreflect.pipes", "true" };
static settings::Boolean flares{ "autoreflect.flares", "false" };
static settings::Boolean arrows{ "autoreflect.arrows", "false" };
static settings::Boolean jars{ "autoreflect.jars", "false" };
static settings::Boolean healingbolts{ "autoreflect.healingbolts", "false" };
static settings::Boolean rockets{ "autoreflect.rockets", "true" };
static settings::Boolean sentryrockets{ "autoreflect.sentryrockets", "true" };
static settings::Boolean cleavers{ "autoreflect.cleavers", "false" };
static settings::Boolean teammates{ "autoreflect.teammate", "false" };
static settings::Boolean teammates_fire{ "autoreflect.teammatesfire", "false" };

static settings::Float fov{ "autoreflect.fov", "85" };

#if ENABLE_VISUALS
static settings::Boolean fov_draw{ "autoreflect.draw-fov", "false" };
static settings::Float fovcircle_opacity{ "autoreflect.draw-fov-opacity", "0.7" };
#endif

// Function to determine whether an ent is good to reflect
static bool ShouldReflect(CachedEntity *ent)
{
    // Check if entity is a burning teammate
    if (ent->m_Type() == ENTITY_PLAYER && teammates_fire)
    {
        // check if player is local player
        if (ent->m_IDX == LOCAL_E->m_IDX)
            return false;
        // check if player is on the same team
        if (ent->m_iTeam() != g_pLocalPlayer->team)
            return false;
        // check if player is on fire
        if (!HasCondition<TFCond_OnFire>(ent))
            return false;

        float dist = ent->m_flDistance();

        // this isnt the correct distance value but it works, and looks legit.
        if (dist < 1000.0f)
            return *teammates_fire;
    }

    // Check if the entity is a projectile
    if (ent->m_Type() != ENTITY_PROJECTILE)
        return false;

    if (!teammates)
    {
        // Check if the projectile is your own teams
        if (!ent->m_bEnemy())
            return false;
    }

    // We dont want to do these checks in dodgeball, it breakes if we do
    if (!dodgeball)
    {
        // If projectile is already deflected, don't deflect it again.
        if (CE_INT(ent, (ent->m_bGrenadeProjectile() ?
                                                     /* NetVar for grenades */ netvar.Grenade_iDeflected
                                                     :
                                                     /* For rockets */ netvar.Rocket_iDeflected)))
            return false;
    }

    // Checks if the projectile is x and if the user settings allow it to be reflected
    // If not, returns false

    switch (ent->m_iClassID())
    {
    default:
    {
        // Target is not known, return default value
        return *proj_default;
    }
    case CL_CLASS(CTFGrenadePipebombProjectile):
    {
        if (CE_INT(ent, netvar.iPipeType) == 1)
            return *stickies;
        if (CE_INT(ent, netvar.iPipeType) == 0)
            return *pipes;
    }
    case CL_CLASS(CTFProjectile_Arrow):
    {
        return *arrows;
    }
    case CL_CLASS(CTFProjectile_Jar) || ent->m_iClassID() == CL_CLASS(CTFProjectile_JarMilk):
    {
        return *jars;
    }
    case CL_CLASS(CTFProjectile_HealingBolt):
    {
        return *healingbolts;
    }
    case CL_CLASS(CTFProjectile_Rocket):
    {
        return *rockets;
    }
    case CL_CLASS(CTFProjectile_SentryRocket):
    {
        return *sentryrockets;
    }
    case CL_CLASS(CTFProjectile_Cleaver):
    {
        return *cleavers;
    }
    case CL_CLASS(CTFGrapplingHook):
    {
        return false;
    }
    }
}

// Function called by game for movement
static void CreateMove()
{
    // Check if user settings allow Auto Reflect
    if (!enable || CE_BAD(LOCAL_W) || (blastkey && !blastkey.isKeyDown()))
        return;

    // Check if player is using a flame thrower
    if (LOCAL_W->m_iClassID() != CL_CLASS(CTFFlameThrower) && LOCAL_W->m_iClassID() != CL_CLASS(CTFWeaponFlameBall) && CE_INT(LOCAL_W, netvar.iItemDefinitionIndex) != 528)
        return;

    // Check for phlogistinator, which is item 594
    if (HasWeapon(LOCAL_E, 594))
        return;

    // If user settings allow, return if local player is in attack
    if (idle_only && (current_user_cmd->buttons & IN_ATTACK))
        return;

    // Create some book-keeping vars
    float closest_dist = 0.0f;
    Vector closest_vec;
    // Loop to find the closest entity
    for (auto const &ent : entity_cache::valid_ents)
    {

        // Find an ent from the for loops current tick

        // Check if null or dormant

        // Check if ent should be reflected
        if (!ShouldReflect(ent))
            continue;

        // Some extrapolating due to reflect timing being latency based
        // Grab latency
        float latency = g_IEngine->GetNetChannelInfo()->GetLatency(FLOW_INCOMING) + g_IEngine->GetNetChannelInfo()->GetLatency(FLOW_OUTGOING);
        // Create a vector variable to store our velocity
        Vector velocity;
        // Grab Velocity of projectile
        velocity::EstimateAbsVelocity(RAW_ENT(ent), velocity);
        // Predict a vector for where the projectile will be
        Vector predicted_proj = ent->m_vecOrigin() + (velocity * latency);

        // Dont vischeck if ent is stickybomb or if dodgeball mode is enabled
        if (ent->m_iClassID() == CL_CLASS(CTFGrenadePipebombProjectile) && !dodgeball)
        {
            if (CE_INT(ent, netvar.iPipeType) == 1)
            {
                // Vis check the predicted vector
                if (!IsVectorVisible(g_pLocalPlayer->v_Origin, predicted_proj))
                    continue;
            }
        }

        /*else {
           // Stickys are weird, we use a different way to vis check them
           // Vis checking stickys are wonky, I quit, just ignore the check >_>
           //if (!VisCheckEntFromEnt(ent, LOCAL_E)) continue;
       }*/

        // Calculate distance
        float dist = predicted_proj.DistToSqr(g_pLocalPlayer->v_Origin);

        // If legit mode is on, we check to see if reflecting will work if we
        // dont aim at the projectile
        if (legit)
        {
            if (GetFov(g_pLocalPlayer->v_OrigViewangles, g_pLocalPlayer->v_Eye, predicted_proj) > (float) fov)
                continue;
        }

        // Compare our info to the others and determine if its the best, if we
        // dont have a projectile already, then we save it here
        if (dist < closest_dist || closest_dist == 0.0f)
        {
            closest_dist = dist;
            closest_vec  = predicted_proj;
        }
    }

    // Determine whether the closest projectile is whithin our parameters,
    // preferably 185 units should be our limit, sqr is around the number below
    if (closest_dist == 0 || closest_dist > 34400)
        return;

    // We dont want to aim if legit is true
    if (!legit)
    {
        // Aim at predicted projectile
        AimAt(g_pLocalPlayer->v_Eye, closest_vec, current_user_cmd, false);
        // Use silent angles
        g_pLocalPlayer->bUseSilentAngles = true;
    }

    // Airblast
    current_user_cmd->buttons |= IN_ATTACK2;
}

void Draw()
{
#if ENABLE_VISUALS
    // Dont draw to screen when reflect is disabled
    if (!enable)
        return;
    // Don't draw to screen when legit is disabled
    if (!legit)
        return;

    // Fov ring to represent when a projectile will be reflected
    if (fov_draw)
    {
        // It cant use fovs greater than 180, so we check for that
        if (*fov > 0.0f && *fov < 180)
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
#endif
}

static InitRoutine EC(
    []()
    {
        EC::Register(EC::CreateMove, CreateMove, "cm_auto_reflect", EC::average);
        EC::Register(EC::CreateMoveWarp, CreateMove, "cmw_auto_reflect", EC::average);
#if ENABLE_VISUALS
        EC::Register(EC::Draw, Draw, "draw_auto_reflect", EC::average);
#endif
    });
} // namespace hacks::tf::autoreflect
