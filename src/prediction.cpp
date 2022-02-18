/*
 * prediction.cpp
 *
 *  Created on: Dec 5, 2016
 *      Author: nullifiedcat
 */
#include "common.hpp"
#include "navparser.hpp"
#include <settings/Bool.hpp>
#include <boost/circular_buffer.hpp>

namespace hacks::shared::aimbot
{
extern settings::Boolean engine_projpred;
}

static settings::Boolean debug_pp_extrapolate{ "debug.pp-extrapolate", "false" };
static settings::Boolean debug_pp_draw{ "debug.pp-draw", "false" };
static settings::Boolean debug_pp_draw_engine{ "debug.pp-draw.engine", "false" };
static settings::Int debug_pp_steps{ "debug.pp-steps", "66" };
// The higher the sample size, the more previous positions we will take into account to calculate the next position. Lower = Faster reaction Higher = Stability
static settings::Int sample_size("debug.strafepred.samplesize", "10");
// TODO there is a Vector() object created each call.

Vector SimpleLatencyPrediction(CachedEntity *ent, int hb)
{
    if (!ent)
        return Vector();
    Vector result;
    GetHitbox(ent, hb, result);
    float latency = g_IEngine->GetNetChannelInfo()->GetLatency(FLOW_OUTGOING) + g_IEngine->GetNetChannelInfo()->GetLatency(FLOW_INCOMING);
    Vector velocity;
    velocity::EstimateAbsVelocity(RAW_ENT(ent), velocity);
    result += velocity * latency;
    return result;
}

float PlayerGravityMod(CachedEntity *player)
{
    //      int movetype = CE_INT(player, netvar.movetype);
    //      if (movetype == MOVETYPE_FLY || movetype == MOVETYPE_NOCLIP) return
    //      0.0f;
    if (HasCondition<TFCond_Parachute>(player))
        return 0.448f;
    return 1.0f;
}

struct StrafePredictionData
{
    bool strafing_dir;
    float radius;
    Vector center;
};

// StrafePredictionData strafepred_data;
static std::array<boost::circular_buffer<Vector>, MAX_PLAYERS> previous_positions;
static ConVar *sv_gravity = nullptr;

// Function for calculating the center and radius of a circle that is supposed to represent a players starfing pattern
static std::optional<StrafePredictionData> findCircle(const Vector &current, const Vector &past1, const Vector &past2)
{
    float yDelta_a = past1.y - current.y;
    float xDelta_a = past1.x - current.x;
    float yDelta_b = past2.y - past1.y;
    float xDelta_b = past2.x - past1.x;
    Vector2D Center;

    float aSlope = yDelta_a / xDelta_a;
    float bSlope = yDelta_b / xDelta_b;

    Vector2D AB_Mid = Vector2D((current.x + past1.x) / 2, (current.y + past1.y) / 2);
    Vector2D BC_Mid = Vector2D((past1.x + past2.x) / 2, (past1.y + past2.y) / 2);

    if (yDelta_a == 0)
    {
        Center.x = AB_Mid.x;

        if (xDelta_b == 0)
            return std::nullopt;
        else
        {
            Center.y = BC_Mid.y + (BC_Mid.x - Center.x) / bSlope;
        }
    }
    else if (yDelta_b == 0)
    {
        Center.x = BC_Mid.x;
        if (xDelta_a == 0)
            return std::nullopt;
        else
            Center.y = AB_Mid.y + (AB_Mid.x - Center.x) / aSlope;
    }
    else if (xDelta_a == 0)
        return std::nullopt;
    else if (xDelta_b == 0)
        return std::nullopt;
    else
    {
        Center.x = (aSlope * bSlope * (AB_Mid.y - BC_Mid.y) - aSlope * BC_Mid.x + bSlope * AB_Mid.x) / (bSlope - aSlope);
        Center.y = AB_Mid.y - (Center.x - AB_Mid.x) / aSlope;
    }

    float Radius = current.AsVector2D().DistTo(Center);

    if (Radius < 5.0f || Radius > 500.0f)
        return std::nullopt;

    float Angle         = atan2(current.y - Center.y, current.x - Center.x);
    float PreviousAngle = atan2(past1.y - Center.y, past1.x - Center.x);

    Vector Center3D(Center.x, Center.y, current.z);

    return StrafePredictionData{ Angle < PreviousAngle, Radius, Center3D };
}

// Applies strafe predictions to the next position
void applyStrafePrediction(Vector &pos, StrafePredictionData &data, float prediction_distance)
{
    // Calculate our current angle on the circle
    float curangle = atan2(pos.y - data.center.y, pos.x - data.center.x);

    // Ok so here we calculate the angle based on the distance the normal prediction threw out, stored in prediction_distance
    // central angle = arclength / radius
    float newangle = prediction_distance / data.radius;

    // We then use that angle to calculate the actual 2d position and insert that into PosOnCircle
    Vector PosOnCircle;
    if (!data.strafing_dir)
        PosOnCircle = Vector(data.center.x + data.radius * cos(curangle + newangle), data.center.y + data.radius * sin(curangle + newangle), 0);
    else
        PosOnCircle = Vector(data.center.x + data.radius * cos(curangle - newangle), data.center.y + data.radius * sin(curangle - newangle), 0);

    pos.x = PosOnCircle.x;
    pos.y = PosOnCircle.y;
}

// Check if this target is eligible for strafe prediction and return an object containing info if it is
std::optional<StrafePredictionData> initializeStrafePrediction(CachedEntity *ent)
{
    if (g_pLocalPlayer->weapon_mode == weapon_projectile && ent->m_Type() == ENTITY_PLAYER && ent->m_IDX > 0 && ent->m_IDX <= g_GlobalVars->maxClients && !(CE_INT(ent, netvar.iFlags) & FL_ONGROUND))
    {
        auto &buffer = previous_positions.at(ent->m_IDX - 1);
        if (buffer.full())
            return findCircle(buffer.front(), buffer.at(buffer.capacity() / 2), buffer.back());
    }
    return std::nullopt;
}

Vector PredictStep(Vector pos, Vector &vel, const Vector &acceleration, std::pair<Vector, Vector> *minmax, float steplength, StrafePredictionData *strafepred, bool vischeck, std::optional<float> grounddistance)
{
    PROF_SECTION(PredictNew)
    Vector result = pos;

    // If we should do strafe prediction, then we still need to do the calculations, but instead of applying them we simply calculate the distance traveled and use that info together with strafe pred
    if (strafepred && (grounddistance ? *grounddistance > 0.1f : (minmax ? DistanceToGround(pos, minmax->first, minmax->second) : DistanceToGround(pos)) > 0.1f))
    {
        auto newpos = result + (acceleration / 2.0f) * pow(steplength, 2) + vel * steplength;
        // Strafe pred does not predict Z! The player can't control his Z anyway, so it is pointless.
        result.z = newpos.z;
        applyStrafePrediction(result, *strafepred, newpos.DistTo(result));
    }
    else
        result += (acceleration / 2.0f) * pow(steplength, 2) + vel * steplength;
    vel += acceleration * steplength;

    bool moved_upwards = false;
    if (vischeck)
    {
        Vector modify = result;
        // Standing Player height is 83
        modify.z   = pos.z + 83.0f;
        Vector low = modify;
        low.z -= 8912.0f;

        {
            PROF_SECTION(PredictTraces)

            // First, ensure we're not slightly below the floor, up to 18 HU will snap up
            trace_t upwards_trace;
            Ray_t ray;
            Vector endpos = result;
            endpos.z += 18;
            if (minmax)
                ray.Init(endpos, result, minmax->first, minmax->second);
            else
                ray.Init(endpos, result);
            g_ITrace->TraceRay(ray, MASK_PLAYERSOLID, &trace::filter_no_player, &upwards_trace);

            // We hit something, snap to it
            if (upwards_trace.DidHit() && !upwards_trace.startsolid)
            {
                result         = upwards_trace.endpos;
                grounddistance = 0.0f;
                moved_upwards  = true;
            }

            // Now check actual ground distance
            else
            {
                trace_t trace;
                if (minmax)
                    ray.Init(modify, low, minmax->first, minmax->second);
                else
                    ray.Init(modify, low);
                g_ITrace->TraceRay(ray, MASK_PLAYERSOLID, &trace::filter_no_player, &trace);

                float dist = pos.z - trace.endpos.z;
                if (trace.m_pEnt && grounddistance > -18.0f)
                    grounddistance = dist;
            }
        }
    }
    if (grounddistance)
        if (result.z < pos.z - *grounddistance)
            result.z = pos.z - *grounddistance;

    // Check if we hit a wall, if so, snap to it and distance ourselves a bit from it
    if (vischeck && !moved_upwards)
    {
        {
            PROF_SECTION(PredictTraces)
            Ray_t ray;
            trace_t trace;
            if (minmax)
                ray.Init(pos, result, minmax->first, minmax->second);
            else
                ray.Init(pos, result);
            g_ITrace->TraceRay(ray, MASK_PLAYERSOLID, &trace::filter_no_player, &trace);

            // Hit a wall, scratch along it
            if (trace.DidHit())
            {
                Vector hitpos      = trace.endpos;
                Vector normal_wall = trace.plane.normal;
                Vector normal_trace(hitpos.x - pos.x, hitpos.y - pos.y, 0.0f);
                normal_trace = normal_trace.Normalized();

                // Angle of impact determines speed
                float impact_angle = acos(normal_trace.Dot(normal_wall));

                // Ignore floor planes (They have no components we can use)
                if (!normal_wall.AsVector2D().IsZero(0.001f))
                {
                    // We can get a plane in Normal form and determine the direction from there
                    // aka n1*x+n2*y+n3*z=d
                    float d = normal_wall.Dot(hitpos);

                    Vector point1;
                    Vector point2;

                    // The above will be invalid due to a division by 0
                    if (normal_wall.y == 0.0f)
                    {
                        point1 = Vector((d - normal_wall.y * vel.y - normal_wall.z * hitpos.z) / normal_wall.x, vel.y, hitpos.z);
                        point2 = Vector((d - normal_wall.y * 2.0f * vel.y - normal_wall.z * hitpos.z) / normal_wall.x, vel.y * 2.0f, hitpos.z);
                    }
                    else
                    {
                        point1 = Vector(vel.x, (d - normal_wall.x * vel.x - normal_wall.z * hitpos.z) / normal_wall.y, hitpos.z);
                        point2 = Vector(vel.x * 2.0f, (d - normal_wall.x * vel.x * 2.0f - normal_wall.z * hitpos.z) / normal_wall.y, hitpos.z);
                    }

                    hitpos += normal_wall * vel * steplength;
                    result = hitpos;
                    // Adjust velocity depending on angle
                    float speed = vel.Length2D() * (PI - impact_angle);

                    // Adjust new velocity
                    Vector2D new_vel = (point2.AsVector2D() - point1.AsVector2D());
                    // Ensure we have no 0 length
                    if (new_vel.Length())
                    {
                        new_vel /= new_vel.Length();
                        vel.x = new_vel.x * speed;
                        vel.y = new_vel.y * speed;
                    }
                }
            }
        }
    }
    return result;
}

std::vector<Vector> Predict(CachedEntity *player, Vector pos, float offset, Vector vel, Vector acceleration, std::pair<Vector, Vector> minmax, int count, bool vischeck)
{
    std::vector<Vector> positions;
    positions.reserve(count);

    pos.z -= offset;

    float dist       = DistanceToGround(pos, minmax.first, minmax.second);
    auto strafe_pred = initializeStrafePrediction(player);

    for (int i = 0; i < count; i++)
    {
        if (vischeck)
            pos = PredictStep(pos, vel, acceleration, &minmax, g_GlobalVars->interval_per_tick, strafe_pred ? &*strafe_pred : nullptr);
        else
            pos = PredictStep(pos, vel, acceleration, &minmax, g_GlobalVars->interval_per_tick, strafe_pred ? &*strafe_pred : nullptr, false, dist);
        positions.push_back({ pos.x, pos.y, pos.z + offset });
    }
    return positions;
}

#if ENABLE_VISUALS
void Prediction_PaintTraverse()
{
    if (g_Settings.bInvalid)
        return;
    if (debug_pp_draw || debug_pp_draw_engine)
    {
        if (!sv_gravity)
        {
            sv_gravity = g_ICvar->FindVar("sv_gravity");
            if (!sv_gravity)
                return;
        }

        for (int i = 1; i < g_GlobalVars->maxClients; i++)
        {
            auto ent = ENTITY(i);
            if (CE_BAD(ent) || !ent->m_bAlivePlayer())
                continue;

            Vector velocity;
            velocity::EstimateAbsVelocity(RAW_ENT(ent), velocity);

            if (debug_pp_draw_engine)
            {
                std::vector<Vector> data;
                Vector original_origin = ent->m_vecOrigin();
                Vector new_origin      = original_origin;

                Vector new_velocity = velocity;

                Vector mins = RAW_ENT(ent)->GetCollideable()->OBBMins();
                Vector maxs = RAW_ENT(ent)->GetCollideable()->OBBMaxs();

                for (int i = 0; i < 64; i++)
                {
                    const_cast<Vector &>(RAW_ENT(ent)->GetAbsOrigin()) = new_origin;
                    CE_VECTOR(ent, 0x354)                              = new_origin;
                    ent->m_vecOrigin()                                 = new_origin;
                    new_origin                                         = EnginePrediction(ent, g_GlobalVars->interval_per_tick, &new_velocity);
                    if (DistanceToGround(new_origin, mins, maxs) > 0.0f)
                        new_velocity.z -= sv_gravity->GetFloat() * PlayerGravityMod(ent) * g_GlobalVars->interval_per_tick;

                    data.push_back(new_origin);
                }
                CE_VECTOR(ent, 0x354)                              = original_origin;
                ent->m_vecOrigin()                                 = original_origin;
                const_cast<Vector &>(RAW_ENT(ent)->GetAbsOrigin()) = original_origin;

                Vector previous_screen;
                if (!draw::WorldToScreen(ent->m_vecOrigin(), previous_screen))
                    continue;
                rgba_t color = colors::FromRGBA8(0, 0, 255, 255);
                for (size_t j = 0; j < data.size(); j++)
                {
                    Vector screen;
                    if (draw::WorldToScreen(data[j], screen))
                    {
                        draw::Line(screen.x, screen.y, previous_screen.x - screen.x, previous_screen.y - screen.y, color, 2);
                        previous_screen = screen;
                    }
                    else
                    {
                        break;
                    }
                    color.b -= 1.0f / 20.0f;
                }
                /*if (!ent->m_bEnemy())
                    continue;
                auto pos  = ProjectilePrediction_Engine(ent, hitbox_t::spine_3, 1980.0f, 0.0f, 1.0f, 0.0f);
                auto pos2 = ProjectilePrediction(ent, hitbox_t::spine_3, 1980.0f, 0.0f, 1.0f, 0.0f);

                Vector aaa;
                if (draw::WorldToScreen(pos.first, aaa))
                    draw::Rectangle(aaa.x, aaa.y, 5, 5, colors::yellow);
                if (draw::WorldToScreen(pos2.first, aaa))
                    draw::Rectangle(aaa.x, aaa.y, 5, 5, colors::orange);*/
            }
            if (debug_pp_draw)
            {

                auto data = Predict(ent, ent->m_vecOrigin(), 0.0f, velocity, Vector(0, 0, -sv_gravity->GetFloat()), std::make_pair(RAW_ENT(ent)->GetCollideable()->OBBMins(), RAW_ENT(ent)->GetCollideable()->OBBMaxs()), 64);
                Vector previous_screen;
                if (!draw::WorldToScreen(ent->m_vecOrigin(), previous_screen))
                    continue;
                rgba_t color = colors::FromRGBA8(255, 0, 0, 255);
                for (size_t j = 0; j < data.size(); j++)
                {
                    Vector screen;
                    if (draw::WorldToScreen(data[j], screen))
                    {
                        draw::Line(screen.x, screen.y, previous_screen.x - screen.x, previous_screen.y - screen.y, color, 2);
                        previous_screen = screen;
                    }
                    else
                    {
                        break;
                    }
                    color.r -= 1.0f / 20.0f;
                }

                /*if (!ent->m_bEnemy())
                    continue;
                auto pos = ProjectilePrediction(ent, hitbox_t::spine_3, 1980.0f, 0.0f, 1.0f, 0.0f);

                Vector aaa;
                if (draw::WorldToScreen(pos, aaa))
                    draw::Rectangle(aaa.x, aaa.y, 5, 5, colors::yellow);*/
            }
        }
    }
}
#endif

Vector EnginePrediction(CachedEntity *entity, float time, Vector *vecVelocity)
{
    Vector result          = entity->m_vecOrigin();
    IClientEntity *ent     = RAW_ENT(entity);
    Vector old_vecVelocity = NET_VECTOR(ent, 0x110);
    Vector old_absVelocity = NET_VECTOR(ent, 0x14c);

    typedef void (*SetupMoveFn)(IPrediction *, IClientEntity *, CUserCmd *, class IMoveHelper *, CMoveData *);
    typedef void (*FinishMoveFn)(IPrediction *, IClientEntity *, CUserCmd *, CMoveData *);

    void **predictionVtable  = *((void ***) g_IPrediction);
    SetupMoveFn oSetupMove   = (SetupMoveFn) (*(unsigned *) (predictionVtable + 19));
    FinishMoveFn oFinishMove = (FinishMoveFn) (*(unsigned *) (predictionVtable + 20));

    // CMoveData *pMoveData = (CMoveData*)(sharedobj::client->lmap->l_addr +
    // 0x1F69C0C);  CMoveData movedata {};
    auto pMoveData = std::make_unique<CMoveData>();

    float frameTime = g_GlobalVars->frametime;
    float curTime   = g_GlobalVars->curtime;

    CUserCmd fakecmd{};

    Vector vel /* = NET_VECTOR(ent, 0x14c)*/;
    velocity::EstimateAbsVelocity(RAW_ENT(entity), vel);

    Vector vel_angles;
    VectorAngles(vel, vel_angles);

    float cosang = cos(vel_angles.y);
    float sinang = sin(vel_angles.y);

    float fmove = vel.x * cosang - vel.y * sinang;
    float smove = vel.x * sinang + vel.y * cosang;

    fakecmd.command_number = last_cmd_number;
    fakecmd.forwardmove    = fmove;
    fakecmd.sidemove       = smove;
    Vector oldangles       = CE_VECTOR(entity, netvar.m_angEyeAngles);
    // static Vector zerov{ 0, 0, 0 };
    // CE_VECTOR(entity, netvar.m_angEyeAngles) = zerov;

    CUserCmd *original_cmd = NET_VAR(ent, 4188, CUserCmd *);

    NET_VAR(ent, 4188, CUserCmd *) = &fakecmd;

    g_GlobalVars->curtime   = g_GlobalVars->interval_per_tick * NET_INT(ent, netvar.nTickBase);
    g_GlobalVars->frametime = time;

    Vector old_origin = entity->m_vecOrigin();
    // Apply Velocity overwrite
    /*if (vecVelocity)
    {
        NET_VECTOR(ent, 0x14c) = *vecVelocity;
        NET_VECTOR(ent, 0x110) = *vecVelocity;
    }*/

    NET_VECTOR(ent, 0x354) = entity->m_vecOrigin();

    *g_PredictionRandomSeed = MD5_PseudoRandom(current_user_cmd->command_number) & 0x7FFFFFFF;
    g_IGameMovement->StartTrackPredictionErrors(reinterpret_cast<CBasePlayer *>(ent));
    oSetupMove(g_IPrediction, ent, &fakecmd, nullptr, pMoveData.get());

    if (vecVelocity)
        pMoveData->m_vecVelocity = *vecVelocity;

    g_IGameMovement->ProcessMovement(reinterpret_cast<CBasePlayer *>(ent), pMoveData.get());
    oFinishMove(g_IPrediction, ent, &fakecmd, pMoveData.get());
    g_IGameMovement->FinishTrackPredictionErrors(reinterpret_cast<CBasePlayer *>(ent));

    NET_VAR(ent, 4188, CUserCmd *) = original_cmd;

    g_GlobalVars->frametime = frameTime;
    g_GlobalVars->curtime   = curTime;

    result                                    = ent->GetAbsOrigin();
    NET_VECTOR(ent, 0x354)                    = old_origin;
    NET_VECTOR(ent, 0x110)                    = old_vecVelocity;
    NET_VECTOR(ent, 0x14c)                    = old_absVelocity;
    CE_VECTOR(entity, netvar.m_angEyeAngles)  = oldangles;
    const_cast<Vector &>(ent->GetAbsOrigin()) = old_origin;
    const_cast<QAngle &>(ent->GetAbsAngles()) = VectorToQAngle(oldangles);

    return result;
}

std::pair<Vector, Vector> ProjectilePrediction_Engine(CachedEntity *ent, int hb, float speed, float gravity, float entgmod, float proj_startvelocity)
{
    Vector origin = ent->m_vecOrigin();
    Vector velocity;
    velocity::EstimateAbsVelocity(RAW_ENT(ent), velocity);
    Vector hitbox;
    GetHitbox(ent, hb, hitbox);
    Vector hitbox_offset = hitbox - origin;

    if (!sv_gravity)
        sv_gravity = g_ICvar->FindVar("sv_gravity");

    if (speed == 0.0f || !sv_gravity) 
        return { Vector(), Vector() };
    
    float currenttime = g_pLocalPlayer->v_Eye.DistTo(hitbox) / speed - 1.5f;
    if (currenttime <= 0.0f) 
        currenttime = 0.01f;
    
    float besttime          = currenttime;
    float mindelta          = 65536.0f;
    Vector bestpos          = origin;
    Vector current          = origin;
    Vector current_velocity = velocity;
    int maxsteps            = (int) debug_pp_steps;
    float steplength        = g_GlobalVars->interval_per_tick;

    Vector ent_mins = RAW_ENT(ent)->GetCollideable()->OBBMins();
    Vector ent_maxs = RAW_ENT(ent)->GetCollideable()->OBBMaxs();

    for (int steps = 0; steps < maxsteps; steps++, currenttime += steplength)
    {
        ent->m_vecOrigin()                                 = current;
        const_cast<Vector &>(RAW_ENT(ent)->GetAbsOrigin()) = current;
        CE_VECTOR(ent, 0x354)                              = current;
        current                                            = EnginePrediction(ent, steplength, &current_velocity);

        // Apply velocity if not touching the ground
        if (!(CE_INT(ent, netvar.iFlags) & (1 << 0)))
            current_velocity.z -= sv_gravity->GetFloat() * entgmod * steplength;

        float rockettime = g_pLocalPlayer->v_Eye.DistTo(current) / speed;
        // Compensate for ping
        rockettime += g_IEngine->GetNetChannelInfo()->GetLatency(FLOW_OUTGOING) + cl_interp->GetFloat();
        float timedelta = fabs(currenttime > rockettime ? currenttime - rockettime : rockettime - currenttime);
        if (timedelta < mindelta)
        {
            besttime = currenttime;
            bestpos  = current;
            mindelta = timedelta;
        }
    }
    // logging::Info("besttime: %f, currenttime: %f, old currenttime: %f", besttime, currenttime, currenttime - steplength * maxsteps);
    const_cast<Vector &>(RAW_ENT(ent)->GetAbsOrigin()) = origin;
    CE_VECTOR(ent, 0x354)                              = origin;
    // Compensate for ping
    besttime += g_IEngine->GetNetChannelInfo()->GetLatency(FLOW_OUTGOING) + cl_interp->GetFloat();
    bestpos.z += (sv_gravity->GetFloat() / 2.0f * besttime * besttime * gravity);
    // S = at^2/2 ; t = sqrt(2S/a)*/
    Vector result            = bestpos + hitbox_offset;
    Vector result_initialvel = result;
    result_initialvel.z -= proj_startvelocity * besttime;
    /*logging::Info("[Pred][%d] delta: %.2f   %.2f   %.2f", result.x - origin.x,
                  result.y - origin.y, result.z - origin.z);*/
    return { result, result_initialvel };
}
std::pair<Vector, Vector> BuildingPrediction(CachedEntity *building, Vector vec, float speed, float gravity, float proj_startvelocity)
{
    if (!vec.z || CE_BAD(building))
        return { Vector(), Vector() };
    Vector result = vec;

    if (!sv_gravity)
        sv_gravity = g_ICvar->FindVar("sv_gravity");

    if (speed == 0.0f || !sv_gravity)
        return { Vector(), Vector() };

    trace::filter_no_player.SetSelf(RAW_ENT(building));
    
    // Buildings do not move. We don't need to do any steps here
    float time = g_pLocalPlayer->v_Eye.DistTo(result) / speed;
    // Compensate for ping
    time += g_IEngine->GetNetChannelInfo()->GetLatency(FLOW_OUTGOING) + cl_interp->GetFloat();
    
    result.z += (sv_gravity->GetFloat() / 2.0f * time * time * gravity);
    Vector result_initialvel = result;
    result_initialvel.z -= proj_startvelocity * time;
    // S = at^2/2 ; t = sqrt(2S/a)*/
    return { result, result_initialvel };
}

std::pair<Vector, Vector> ProjectilePrediction(CachedEntity *ent, int hb, float speed, float gravitymod, float entgmod, float proj_startvelocity)
{
    Vector origin = ent->m_vecOrigin();
    Vector hitbox;
    GetHitbox(ent, hb, hitbox);
    Vector hitbox_offset = hitbox - origin;

    if (!sv_gravity)
        sv_gravity = g_ICvar->FindVar("sv_gravity");

    if (speed == 0.0f || !sv_gravity)
        return { Vector(), Vector() };

    // TODO ProjAim
    float medianTime  = g_pLocalPlayer->v_Eye.DistTo(hitbox) / speed;
    float range       = 1.5f;
    float currenttime = medianTime - range;
    if (currenttime <= 0.0f)
        currenttime = 0.01f;
    float besttime = currenttime;
    float mindelta = 65536.0f;
    Vector bestpos = origin;
    Vector current = origin;
    int maxsteps   = (int) debug_pp_steps;
    bool onground  = false;
    if (ent->m_Type() == ENTITY_PLAYER)
    {
        if (CE_INT(ent, netvar.iFlags) & FL_ONGROUND)
            onground = true;
    }

    Vector velocity;
    velocity::EstimateAbsVelocity(RAW_ENT(ent), velocity);

    Vector acceleration = { 0.0f, 0.0f, -sv_gravity->GetFloat() * entgmod };
    float steplength    = ((float) (2 * range) / (float) maxsteps);
    auto minmax         = std::make_pair(RAW_ENT(ent)->GetCollideable()->OBBMins(), RAW_ENT(ent)->GetCollideable()->OBBMaxs());

    Vector last = origin;

    auto strafe_pred = initializeStrafePrediction(ent);

    float currenttime_before = currenttime;
    for (int steps = 0; steps < maxsteps; steps++, currenttime += steplength)
    {
        current = last = PredictStep(last, velocity, acceleration, &minmax, steplength, strafe_pred ? &*strafe_pred : nullptr);

        if (onground)
        {
            float toground = DistanceToGround(current, minmax.first, minmax.second);
            current.z -= toground;
        }

        float rockettime = g_pLocalPlayer->v_Eye.DistTo(current) / speed;
        // Compensate for ping
        rockettime += g_IEngine->GetNetChannelInfo()->GetLatency(FLOW_OUTGOING) + cl_interp->GetFloat();
        if (fabs(currenttime > rockettime ? currenttime - rockettime : rockettime - currenttime) < mindelta)
        {
            besttime = currenttime;
            bestpos  = current;
            mindelta = fabs(rockettime - currenttime);
        }
    }
    // Compensate for ping
    besttime += g_IEngine->GetNetChannelInfo()->GetLatency(FLOW_OUTGOING) + cl_interp->GetFloat();
    bestpos.z += (sv_gravity->GetFloat() / 2.0f * besttime * besttime * gravitymod);
    // S = at^2/2 ; t = sqrt(2S/a)*/
    Vector result            = bestpos + hitbox_offset;
    Vector initialvel_result = result;
    initialvel_result.z -= proj_startvelocity * (besttime - currenttime_before);
    /*logging::Info("[Pred][%d] delta: %.2f   %.2f   %.2f", result.x - origin.x,
                  result.y - origin.y, result.z - origin.z);*/
    return { result, initialvel_result };
}

float DistanceToGround(CachedEntity *ent)
{
    Vector origin = ent->m_vecOrigin();
    Vector mins   = RAW_ENT(ent)->GetCollideable()->OBBMins();
    Vector maxs   = RAW_ENT(ent)->GetCollideable()->OBBMins();
    return DistanceToGround(origin, mins, maxs);
}

float DistanceToGround(Vector origin, Vector mins, Vector maxs)
{
    // First, ensure we're not slightly below the floor, up to 18 HU will snap up
    trace_t upwards_trace;
    Ray_t ray;
    Vector endpos = origin;
    endpos.z += 18;
    ray.Init(endpos, origin, mins, maxs);
    g_ITrace->TraceRay(ray, MASK_PLAYERSOLID, &trace::filter_no_player, &upwards_trace);

    // We hit something, snap to it
    if (upwards_trace.DidHit() && !upwards_trace.startsolid)
        return -std::fabs(origin.z - upwards_trace.endpos.z);

    trace_t ground_trace;
    endpos.z = origin.z - 8192;
    ray.Init(origin, endpos, mins, maxs);
    g_ITrace->TraceRay(ray, MASK_PLAYERSOLID, &trace::filter_no_player, &ground_trace);
    return std::fabs(origin.z - ground_trace.endpos.z);
}

float DistanceToGround(Vector origin)
{
    // First, ensure we're not slightly below the floor, up to 18 HU will snap up
    trace_t upwards_trace;
    Ray_t ray;
    Vector endpos = origin;
    endpos.z += 18;
    ray.Init(endpos, origin);
    g_ITrace->TraceRay(ray, MASK_PLAYERSOLID, &trace::filter_no_player, &upwards_trace);

    // We hit something, snap to it
    if (upwards_trace.DidHit() && !upwards_trace.startsolid)
        return -std::fabs(origin.z - upwards_trace.endpos.z);

    trace_t ground_trace;
    endpos.z = origin.z - 8192;
    ray.Init(origin, endpos);
    g_ITrace->TraceRay(ray, MASK_PLAYERSOLID, &trace::filter_no_player, &ground_trace);
    return std::fabs(origin.z - ground_trace.endpos.z);
}

static InitRoutine init(
    []()
    {
        previous_positions.fill(boost::circular_buffer<Vector>(*sample_size));
        sample_size.installChangeCallback([](settings::VariableBase<int> &, int after) { previous_positions.fill(boost::circular_buffer<Vector>(after)); });
        EC::Register(
            EC::CreateMove,
            []()
            {
                // Don't run if we don't use it
                if (!hacks::shared::aimbot::engine_projpred && !debug_pp_draw)
                    return;
                for (int i = 1; i < g_GlobalVars->maxClients; i++)
                {
                    auto ent     = ENTITY(i);
                    auto &buffer = previous_positions.at(i - 1);

                    if (CE_BAD(LOCAL_E) || CE_BAD(ent) || !ent->m_bAlivePlayer())
                    {
                        buffer.clear();
                        continue;
                    }
                    Vector vel;
                    velocity::EstimateAbsVelocity(RAW_ENT(ent), vel);
                    if (vel.IsZero())
                    {
                        buffer.clear();
                        continue;
                    }

                    buffer.push_front(ent->m_vecOrigin());
                }
            },
            "cm_prediction", EC::very_early);
    });
