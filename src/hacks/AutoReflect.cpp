/*
 * AutoReflect.cpp
 *
 *  Created on: Nov 18, 2016
 *      Author: nullifiedcat
 */

#include "common.hpp"
#include <hacks/AutoReflect.hpp>

namespace hacks
{
namespace tf
{
namespace autoreflect
{

// Vars for user settings
static CatVar enabled(CV_SWITCH, XORSTR("reflect_enabled"), XORSTR("0"), XORSTR("Auto Reflect"),
                      XORSTR("Master AutoReflect switch"));
static CatVar idle_only(CV_SWITCH, XORSTR("reflect_only_idle"), XORSTR("0"),
                        XORSTR("Only when not shooting"),
                        XORSTR("Don't AutoReflect if you're holding M1"));
static CatVar
    legit(CV_SWITCH, XORSTR("reflect_legit"), XORSTR("0"), XORSTR("Legit Reflect"),
          XORSTR("Only Auto-airblasts projectiles that you can see, doesnt move "
          "your crosshair"));
static CatVar dodgeball(CV_SWITCH, XORSTR("reflect_dodgeball"), XORSTR("0"), XORSTR("Dodgeball Mode"),
                        XORSTR("Allows auto-reflect to work in dodgeball servers"));
static CatVar blastkey(CV_KEY, XORSTR("reflect_key"), XORSTR("0"), XORSTR("Reflect Key"),
                       XORSTR("Hold this key to activate auto-airblast"));
static CatVar stickies(CV_SWITCH, XORSTR("reflect_stickybombs"), XORSTR("0"),
                       XORSTR("Reflect stickies"), XORSTR("Reflect Stickybombs"));
static CatVar teammates(CV_SWITCH, XORSTR("reflect_teammates"), XORSTR("0"),
                        XORSTR("Reflect teammates projectiles"),
                        XORSTR("Useful in dodgeball with "
                        "free-for-all enabled"));
static CatVar fov(CV_FLOAT, XORSTR("reflect_fov"), XORSTR("85"), XORSTR("Reflect FOV"), XORSTR("Reflect FOV"),
                  180.0f);
static CatVar fov_draw(CV_SWITCH, XORSTR("reflect_fov_draw"), XORSTR("0"), XORSTR("Draw Fov Ring"),
                       XORSTR("Draws a ring to represent your current reflect fov"));
static CatVar fovcircle_opacity(CV_FLOAT, XORSTR("reflect_fov_draw_opacity"), XORSTR("0.7"),
                                XORSTR("FOV Circle Opacity"),
                                XORSTR("Defines opacity of FOV circle"), 0.0f, 1.0f);
// TODO setup proj sorting
// TODO CatVar big_proj(CV_SWITCH, XORSTR("reflect_big_projectile"), XORSTR("0"), XORSTR("Reflect big
// projectilesXORSTR(", ")Reflect RocketsXORSTR(");
// TODO CatVar small_proj(CV_SWITCH, XORSTR("reflect_small_projectile"), XORSTR("0"), XORSTR("Reflect
// small projectilesXORSTR(", ")Reflect Huntsman arrows, Crusaders boltsXORSTR(");
// TODO CatVar misc_proj(CV_SWITCH, XORSTR("reflect_misc_projectile"), XORSTR("0"), XORSTR("Reflect
// otherXORSTR(", ")Reflect jarate, milkXORSTR(");

// Function called by game for movement
void CreateMove()
{
    // Check if user settings allow Auto Reflect
    if (!enabled)
        return;
    if (blastkey && !blastkey.KeyDown())
        return;

    // Check if player is using a flame thrower
    if (g_pLocalPlayer->weapon()->m_iClassID() != CL_CLASS(CTFFlameThrower))
        return;

    // Check for phlogistinator, which is item 594
    if (HasWeapon(LOCAL_E, 594))
        return;

    // If user settings allow, return if local player is in attack
    if (idle_only && (g_pUserCmd->buttons & IN_ATTACK))
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
        AimAt(g_pLocalPlayer->v_Eye, closest_vec, g_pUserCmd);
        // Use silent angles
        g_pLocalPlayer->bUseSilentAngles = true;
    }

    // Airblast
    g_pUserCmd->buttons |= IN_ATTACK2;

    // Function is finished, return
    return;
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
    if (!enabled)
        return;
    // Don't draw to screen when legit is disabled
    if (!legit)
        return;

    // Fov ring to represent when a projectile will be reflected
    if (fov_draw)
    {
        // It cant use fovs greater than 180, so we check for that
        if (float(fov) > 0.0f && float(fov) < 180)
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

                draw_api::draw_circle(width / 2, height / 2, radius, color, 1,
                                      100);
            }
        }
    }
#endif
}
}
}
}