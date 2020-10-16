/*
 * prediction.cpp
 *
 *  Created on: Dec 5, 2016
 *      Author: nullifiedcat
 */
#include "common.hpp"
#include <settings/Bool.hpp>

static settings::Boolean debug_pp_extrapolate{ "debug.pp-extrapolate", "false" };
static settings::Boolean debug_pp_draw{ "debug.pp-draw", "false" };
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

bool PerformProjectilePrediction(CachedEntity *target, int hitbox)
{
    Vector src, vel, hit;
    ;
    // src = vfunc<Vector(*)(IClientEntity*)>(RAW_ENT(target),
    // 299)(RAW_ENT(target));

    return true;
}

Vector Predict(Vector &pos, Vector &vel, Vector acceleration, std::optional<float> grounddistance, float &time)
{
    PROF_SECTION(PredictNew);
    Vector result = pos;
    result += (acceleration / 2.0f) * pow(time, 2) + vel * time;
    vel += acceleration * time;
    if (grounddistance)
        if (result.z < pos.z - *grounddistance)
            result.z = pos.z - *grounddistance;
    return result;
}

Vector PredictStep(Vector pos, Vector &vel, Vector acceleration, std::pair<Vector, Vector> &minmax, float time, float steplength, bool vischeck, std::optional<float> grounddistance)
{
    PROF_SECTION(PredictNew)
    Vector result = pos;

    result += (acceleration / 2.0f) * pow(steplength, 2) + vel * steplength;
    vel += acceleration * steplength;
    if (vischeck)
    {
        Vector modify = result;
        // Standing Player height is 83
        modify.z   = pos.z + 83.0f;
        Vector low = modify;
        low.z -= 8912.0f;

        {
            PROF_SECTION(PredictTraces)
            Ray_t ray;
            trace_t trace;
            ray.Init(modify, low, minmax.first, minmax.second);
            g_ITrace->TraceRay(ray, MASK_PLAYERSOLID, &trace::filter_no_player, &trace);

            float dist = pos.z - trace.endpos.z;
            if (trace.m_pEnt && std::fabs(dist) < 63.0f)
                grounddistance = dist;
        }
    }
    if (grounddistance)
        if (result.z < pos.z - *grounddistance)
            result.z = pos.z - *grounddistance;
    return result;
}

std::vector<Vector> Predict(Vector pos, float offset, Vector vel, Vector acceleration, std::pair<Vector, Vector> minmax, float time, int count, bool vischeck)
{
    std::vector<Vector> positions;
    positions.reserve(count);
    Vector prev;

    pos.z -= offset;

    float dist = DistanceToGround(pos, minmax.first, minmax.second);
    Vector prediction;

    for (int i = 0; i < count; i++)
    {
        time += 1.0f * g_GlobalVars->interval_per_tick;
        if (positions.size() > 0)
        {
            prev = positions.back();
            if (vischeck)
                prediction = PredictStep(prediction, vel, acceleration, minmax, time);
            else
                prediction = PredictStep(prediction, vel, acceleration, minmax, time, g_GlobalVars->interval_per_tick, false, dist);
            positions.push_back({ prediction.x, prediction.y, prediction.z + offset });
        }
        else
        {
            prediction = Predict(pos, vel, acceleration, dist, time);
            prediction.z += offset;
            positions.push_back(prediction);
        }
    }
    return positions;
}

#if ENABLE_VISUALS
void Prediction_PaintTraverse()
{
    if (g_Settings.bInvalid)
        return;
    if (debug_pp_draw)
    {
        static ConVar *sv_gravity = g_ICvar->FindVar("sv_gravity");
        if (!sv_gravity)
        {
            sv_gravity = g_ICvar->FindVar("sv_gravity");
            if (!sv_gravity)
                return;
        }
        for (int i = 1; i <= g_GlobalVars->maxClients; i++)
        {
            auto ent = ENTITY(i);
            if (CE_BAD(ent) || !ent->m_bAlivePlayer())
                continue;
            Vector velocity;
            velocity::EstimateAbsVelocity(RAW_ENT(ent), velocity);
            auto data = Predict(ent->m_vecOrigin(), 0.0f, velocity, Vector(0, 0, -sv_gravity->GetFloat()), std::make_pair(RAW_ENT(ent)->GetCollideable()->OBBMins(), RAW_ENT(ent)->GetCollideable()->OBBMaxs()), 0.0f, 32);
            Vector previous_screen;
            if (!draw::WorldToScreen(ent->m_vecOrigin(), previous_screen))
                continue;
            rgba_t color = colors::FromRGBA8(255, 0, 0, 255);
            for (int j = 0; j < data.size(); j++)
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
        }
    }
}
#endif
Vector EnginePrediction(CachedEntity *entity, float time)
{
    Vector result      = entity->m_vecOrigin();
    IClientEntity *ent = RAW_ENT(entity);

    typedef void (*SetupMoveFn)(IPrediction *, IClientEntity *, CUserCmd *, class IMoveHelper *, CMoveData *);
    typedef void (*FinishMoveFn)(IPrediction *, IClientEntity *, CUserCmd *, CMoveData *);

    void **predictionVtable  = *((void ***) g_IPrediction);
    SetupMoveFn oSetupMove   = (SetupMoveFn)(*(unsigned *) (predictionVtable + 19));
    FinishMoveFn oFinishMove = (FinishMoveFn)(*(unsigned *) (predictionVtable + 20));

    // CMoveData *pMoveData = (CMoveData*)(sharedobj::client->lmap->l_addr +
    // 0x1F69C0C);  CMoveData movedata {};
    auto pMoveData = std::make_unique<CMoveData>();

    float frameTime = g_GlobalVars->frametime;
    float curTime   = g_GlobalVars->curtime;

    CUserCmd fakecmd{};
    memset(&fakecmd, 0, sizeof(CUserCmd));

    Vector vel;
    velocity::EstimateAbsVelocity(RAW_ENT(entity), vel);
    fakecmd.command_number = last_cmd_number;
    fakecmd.forwardmove    = vel.x;
    fakecmd.sidemove       = -vel.y;
    Vector oldangles       = CE_VECTOR(entity, netvar.m_angEyeAngles);
    static Vector zerov{ 0, 0, 0 };
    CE_VECTOR(entity, netvar.m_angEyeAngles) = zerov;

    CUserCmd *original_cmd = NET_VAR(ent, 4188, CUserCmd *);

    NET_VAR(ent, 4188, CUserCmd *) = &fakecmd;

    g_GlobalVars->curtime   = g_GlobalVars->interval_per_tick * NET_INT(ent, netvar.nTickBase);
    g_GlobalVars->frametime = time;

    Vector old_origin      = entity->m_vecOrigin();
    NET_VECTOR(ent, 0x354) = entity->m_vecOrigin();

    *g_PredictionRandomSeed = MD5_PseudoRandom(current_user_cmd->command_number) & 0x7FFFFFFF;
    g_IGameMovement->StartTrackPredictionErrors(reinterpret_cast<CBasePlayer *>(ent));
    oSetupMove(g_IPrediction, ent, &fakecmd, nullptr, pMoveData.get());
    g_IGameMovement->ProcessMovement(reinterpret_cast<CBasePlayer *>(ent), pMoveData.get());
    oFinishMove(g_IPrediction, ent, &fakecmd, pMoveData.get());
    g_IGameMovement->FinishTrackPredictionErrors(reinterpret_cast<CBasePlayer *>(ent));

    NET_VAR(entity, 4188, CUserCmd *) = original_cmd;

    g_GlobalVars->frametime = frameTime;
    g_GlobalVars->curtime   = curTime;

    result                                    = ent->GetAbsOrigin();
    NET_VECTOR(ent, 0x354)                    = old_origin;
    CE_VECTOR(entity, netvar.m_angEyeAngles)  = oldangles;
    const_cast<Vector &>(ent->GetAbsOrigin()) = old_origin;
    const_cast<QAngle &>(ent->GetAbsAngles()) = VectorToQAngle(oldangles);

    return result;
}

Vector ProjectilePrediction_Engine(CachedEntity *ent, int hb, float speed, float gravitymod, float entgmod /* ignored */, float proj_startvelocity)
{
    Vector origin = ent->m_vecOrigin();
    Vector hitbox;
    GetHitbox(ent, hb, hitbox);
    Vector hitbox_offset = hitbox - origin;

    if (speed == 0.0f)
        return Vector();

    static ConVar *sv_gravity = g_ICvar->FindVar("sv_gravity");

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
    int maxsteps   = 40;
    bool onground  = false;
    if (ent->m_Type() == ENTITY_PLAYER)
    {
        if (CE_INT(ent, netvar.iFlags) & FL_ONGROUND)
            onground = true;
    }
    float steplength = ((float) (2 * range) / (float) maxsteps);
    Vector ent_mins  = RAW_ENT(ent)->GetCollideable()->OBBMins();
    Vector ent_maxs  = RAW_ENT(ent)->GetCollideable()->OBBMaxs();
    for (int steps = 0; steps < maxsteps; steps++, currenttime += steplength)
    {
        ent->m_vecOrigin() = current;
        current            = EnginePrediction(ent, steplength);

        if (onground)
        {
            float toground = DistanceToGround(current, ent_mins, ent_maxs);
            current.z -= toground;
        }

        float rockettime = g_pLocalPlayer->v_Eye.DistTo(current) / speed;
        // Compensate for ping
        rockettime += g_IEngine->GetNetChannelInfo()->GetLatency(FLOW_OUTGOING) + cl_interp->GetFloat();
        if (fabs(rockettime - currenttime) < mindelta)
        {
            besttime = currenttime;
            bestpos  = current;
            mindelta = fabs(rockettime - currenttime);
        }
    }
    const_cast<Vector &>(RAW_ENT(ent)->GetAbsOrigin()) = origin;
    CE_VECTOR(ent, 0x354)                              = origin;
    // Compensate for ping
    besttime += g_IEngine->GetNetChannelInfo()->GetLatency(FLOW_OUTGOING) + cl_interp->GetFloat();
    bestpos.z += (sv_gravity->GetFloat() / 2.0f * besttime * besttime * gravitymod - proj_startvelocity * besttime);
    // S = at^2/2 ; t = sqrt(2S/a)*/
    Vector result = bestpos + hitbox_offset;
    /*logging::Info("[Pred][%d] delta: %.2f   %.2f   %.2f", result.x - origin.x,
                  result.y - origin.y, result.z - origin.z);*/
    return result;
}
Vector BuildingPrediction(CachedEntity *building, Vector vec, float speed, float gravity, float proj_startvelocity)
{
    if (!vec.z || CE_BAD(building))
        return Vector();
    Vector result = vec;
    // if (not debug_pp_extrapolate) {
    //} else {
    //        result = SimpleLatencyPrediction(ent, hb);
    //
    //}

    if (speed == 0.0f)
        return Vector();
    static ConVar *sv_gravity = g_ICvar->FindVar("sv_gravity");
    trace::filter_no_player.SetSelf(RAW_ENT(building));
    float dtg = DistanceToGround(vec, RAW_ENT(building)->GetCollideable()->OBBMins(), RAW_ENT(building)->GetCollideable()->OBBMaxs());
    // TODO ProjAim
    float medianTime  = g_pLocalPlayer->v_Eye.DistTo(result) / speed;
    float range       = 1.5f;
    float currenttime = medianTime - range;
    if (currenttime <= 0.0f)
        currenttime = 0.01f;
    float besttime = currenttime;
    float mindelta = 65536.0f;
    Vector bestpos = result;
    int maxsteps   = 300;
    for (int steps = 0; steps < maxsteps; steps++, currenttime += ((float) (2 * range) / (float) maxsteps))
    {
        Vector curpos = result;
        if (dtg > 0.0f)
        {
            if (curpos.z < result.z - dtg)
                curpos.z = result.z - dtg;
        }
        float rockettime = g_pLocalPlayer->v_Eye.DistTo(curpos) / speed;
        // Compensate for ping
        rockettime += g_IEngine->GetNetChannelInfo()->GetLatency(FLOW_OUTGOING) + cl_interp->GetFloat();
        if (fabs(rockettime - currenttime) < mindelta)
        {
            besttime = currenttime;
            bestpos  = curpos;
            mindelta = fabs(rockettime - currenttime);
        }
    }
    // Compensate for ping
    besttime += g_IEngine->GetNetChannelInfo()->GetLatency(FLOW_OUTGOING) + cl_interp->GetFloat();
    bestpos.z += (sv_gravity->GetFloat() / 2.0f * besttime * besttime * gravity - proj_startvelocity * besttime);
    // S = at^2/2 ; t = sqrt(2S/a)*/
    return bestpos;
}

Vector ProjectilePrediction(CachedEntity *ent, int hb, float speed, float gravitymod, float entgmod, float proj_startvelocity)
{
    Vector origin = ent->m_vecOrigin();
    Vector hitbox;
    GetHitbox(ent, hb, hitbox);
    Vector hitbox_offset = hitbox - origin;

    if (speed == 0.0f)
        return Vector();

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
    int maxsteps   = 40;
    bool onground  = false;
    if (ent->m_Type() == ENTITY_PLAYER)
    {
        if (CE_INT(ent, netvar.iFlags) & FL_ONGROUND)
            onground = true;
    }

    Vector velocity;
    velocity::EstimateAbsVelocity(RAW_ENT(ent), velocity);
    static ConVar *sv_gravity = g_ICvar->FindVar("sv_gravity");
    Vector acceleration       = { 0.0f, 0.0f, -(sv_gravity->GetFloat() * entgmod) };
    float steplength          = ((float) (2 * range) / (float) maxsteps);
    auto minmax               = std::make_pair(RAW_ENT(ent)->GetCollideable()->OBBMins(), RAW_ENT(ent)->GetCollideable()->OBBMaxs());

    float dist_to_ground = DistanceToGround(origin, minmax.first, minmax.second);

    Vector last = origin;

    for (int steps = 0; steps < maxsteps; steps++, currenttime += steplength)
    {
        if (steps == 0)
            last = Predict(last, velocity, acceleration, dist_to_ground, currenttime);
        else
            last = PredictStep(last, velocity, acceleration, minmax, currenttime, steplength);

        current = last;
        if (onground)
        {
            float toground = DistanceToGround(current, minmax.first, minmax.second);
            current.z -= toground;
        }

        float rockettime = g_pLocalPlayer->v_Eye.DistTo(current) / speed;
        // Compensate for ping
        rockettime += g_IEngine->GetNetChannelInfo()->GetLatency(FLOW_OUTGOING) + cl_interp->GetFloat();
        if (fabs(rockettime - currenttime) < mindelta)
        {
            besttime = currenttime;
            bestpos  = current;
            mindelta = fabs(rockettime - currenttime);
        }
    }
    // Compensate for ping
    besttime += g_IEngine->GetNetChannelInfo()->GetLatency(FLOW_OUTGOING) + cl_interp->GetFloat();
    bestpos.z += (sv_gravity->GetFloat() / 2.0f * besttime * besttime * gravitymod - proj_startvelocity * besttime);
    // S = at^2/2 ; t = sqrt(2S/a)*/
    Vector result = bestpos + hitbox_offset;
    /*logging::Info("[Pred][%d] delta: %.2f   %.2f   %.2f", result.x - origin.x,
                  result.y - origin.y, result.z - origin.z);*/
    return result;
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
    trace_t ground_trace;
    Ray_t ray;
    Vector endpos = origin;
    endpos.z -= 8192;
    ray.Init(origin, endpos, mins, maxs);
    g_ITrace->TraceRay(ray, MASK_PLAYERSOLID, &trace::filter_no_player, &ground_trace);
    return std::fabs(origin.z - ground_trace.endpos.z);
}

float DistanceToGround(Vector origin)
{
    trace_t ground_trace;
    Ray_t ray;
    Vector endpos = origin;
    endpos.z -= 8192;
    ray.Init(origin, endpos);
    g_ITrace->TraceRay(ray, MASK_PLAYERSOLID, &trace::filter_no_player, &ground_trace);
    return std::fabs(origin.z - ground_trace.endpos.z);
}
