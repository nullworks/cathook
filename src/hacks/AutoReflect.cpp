/*
 * AutoReflect.cpp
 *
 *  Created on: Nov 18, 2016
 *      Author: nullifiedcat
 */

#include "common.hpp"
#include <hacks/AutoReflect.hpp>
#include <glez/draw.hpp>
#include <settings/Bool.hpp>

static settings::Bool enable{ "autoreflect.enable", "false" };
static settings::Bool idle_only{ "autoreflect.idle-only", "false" };
static settings::Bool legit{ "autoreflect.legit", "false" };
static settings::Bool dodgeball{ "autoreflect.dodgeball", "false" };
static settings::Button blastkey{ "autoreflect.button", "<null>" };
static settings::Bool stickies{ "autoreflect.stickies", "false" };
static settings::Bool teammates{ "autoreflect.teammate", "false" };
static settings::Float fov{ "autoreflect.fov", "85" };

#if ENABLE_VISUALS
static settings::Bool fov_draw{ "autoreflect.draw-fov", "false" };
static settings::Float fovcircle_opacity{ "autoreflect.draw-fov-opacity",
                                          "0.7" };
#endif

namespace hacks::tf::autoreflect
{

// Function called by game for movement
void CreateMove()
{
    // Check if user settings allow Auto Reflect
    if (!enable)
        return;
    if (blastkey && !blastkey.isKeyDown())
        return;

    // Check if player is using a flame thrower
    if (g_pLocalPlayer->weapon()->m_iClassID() != CL_CLASS(CTFFlameThrower) && CE_INT(LOCAL_W, netvar.iItemDefinitionIndex) != 528)
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
    for (int i = 0; i < HIGHEST_ENTITY; i++)
    {

        // Find an ent from the for loops current tick
        CachedEntity *ent = ENTITY(i);

        // Check if null or dormant
        if (CE_BAD(ent))
            continue;

        // Check if ent should be reflected
        if (!ShouldReflect(ent))
            continue;

        // Some extrapolating due to reflect timing being latency based
        // Grab latency
        float latency =
            g_IEngine->GetNetChannelInfo()->GetLatency(FLOW_INCOMING) +
            g_IEngine->GetNetChannelInfo()->GetLatency(FLOW_OUTGOING);
        // Create a vector variable to store our velocity
        Vector velocity;
        // Grab Velocity of projectile
        velocity::EstimateAbsVelocity(RAW_ENT(ent), velocity);
        // Predict a vector for where the projectile will be
        Vector predicted_proj = ent->m_vecOrigin() + (velocity * latency);
        ;

        // Dont vischeck if ent is stickybomb or if dodgeball mode is enabled
        if (!IsEntStickyBomb(ent) && !dodgeball)
        {
            // Vis check the predicted vector
            if (!IsVectorVisible(g_pLocalPlayer->v_Origin, predicted_proj))
                continue;
        } /*else {
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
            if (GetFov(g_pLocalPlayer->v_OrigViewangles, g_pLocalPlayer->v_Eye,
                       predicted_proj) > (float) fov)
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
        AimAt(g_pLocalPlayer->v_Eye, closest_vec, current_user_cmd);
        // Use silent angles
        g_pLocalPlayer->bUseSilentAngles = true;
    }

    // Airblast
    current_user_cmd->buttons |= IN_ATTACK2;
}

// Function to determine whether an ent is good to reflect
bool ShouldReflect(CachedEntity *ent)
{
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
        if (CE_INT(ent, (ent->m_bGrenadeProjectile()
                             ?
                             /* NetVar for grenades */ netvar.Grenade_iDeflected
                             :
                             /* For rockets */ netvar.Rocket_iDeflected)))
            return false;
    }

    // Check if the projectile is a sticky bomb and if the user settings allow
    // it to be reflected
    if (IsEntStickyBomb(ent) && !stickies)
        return false;

    // Target passed the test, return true
    return true;
}

bool IsEntStickyBomb(CachedEntity *ent)
{
    // Check if the projectile is a sticky bomb
    if (ent->m_iClassID() == CL_CLASS(CTFGrenadePipebombProjectile))
    {
        if (CE_INT(ent, netvar.iPipeType) == 1)
        {
            // Ent passed and should be reflected
            return true;
        }
    }
    // Ent didnt pass the test so return false
    return false;
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
            if (LOCAL_E->m_bAlivePlayer())
            {
                rgba_t color = GUIColor();
                color.a      = float(fovcircle_opacity);

                int width, height;
                g_IEngine->GetScreenSize(width, height);

                // Math
                float mon_fov = (float(width) / float(height) / (4.0f / 3.0f));
                float fov_real =
                    RAD2DEG(2 * atanf(mon_fov * tanf(DEG2RAD(draw::fov / 2))));
                float radius = tan(DEG2RAD(float(fov)) / 2) /
                               tan(DEG2RAD(fov_real) / 2) * (width);

                glez::draw::circle(width / 2, height / 2, radius, color, 1,
                                   100);
            }
        }
    }
#endif
}
} // namespace hacks::tf::autoreflect
