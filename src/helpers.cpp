/*
 * helpers.cpp
 *
 *  Created on: Oct 8, 2016
 *      Author: nullifiedcat
 */

#include "common.hpp"

#include <sys/mman.h>
#include <settings/Bool.hpp>

static settings::Bool tcm{ "debug.tcm", "true" };

std::vector<ConVar *> &RegisteredVarsList()
{
    static std::vector<ConVar *> list{};
    return list;
}

std::vector<ConCommand *> &RegisteredCommandsList()
{
    static std::vector<ConCommand *> list{};
    return list;
}

void BeginConVars()
{
    logging::Info("Begin ConVars");
    if (!std::ifstream("tf/cfg/cat_autoexec.cfg"))
    {
        std::ofstream cfg_autoexec("tf/cfg/cat_autoexec.cfg",
                                   std::ios::out | std::ios::trunc);
        if (cfg_autoexec.good())
        {
            cfg_autoexec << "// Put your custom cathook settings in this "
                            "file\n// This script will be executed EACH TIME "
                            "YOU INJECT CATHOOK\n";
        }
    }
    if (!std::ifstream("tf/cfg/cat_autoexec.cfg"))
    {
        std::ofstream cfg_autoexec("tf/cfg/cat_matchexec.cfg",
                                   std::ios::out | std::ios::trunc);
        if (cfg_autoexec.good())
        {
            cfg_autoexec << "// Put your custom cathook settings in this "
                            "file\n// This script will be executed EACH TIME "
                            "YOU JOIN A MATCH\n";
        }
    }
    logging::Info(":b:");
    SetCVarInterface(g_ICvar);
}

void EndConVars()
{
    logging::Info("Registering CatCommands");
    RegisterCatCommands();
    ConVar_Register();

    std::ofstream cfg_defaults("tf/cfg/cat_defaults.cfg",
                               std::ios::out | std::ios::trunc);
    if (cfg_defaults.good())
    {
        cfg_defaults << "// This file is auto-generated and will be "
                        "overwritten each time you inject cathook\n// Do not "
                        "make edits to this file\n\n// Every registered "
                        "variable dump\n";
        for (const auto &i : RegisteredVarsList())
        {
            cfg_defaults << i->GetName() << " \"" << i->GetDefault() << "\"\n";
        }
        cfg_defaults << "\n// Every registered command dump\n";
        for (const auto &i : RegisteredCommandsList())
        {
            cfg_defaults << "// " << i->GetName() << "\n";
        }
    }
}

ConVar *CreateConVar(std::string name, std::string value, std::string help)
{
    char *namec  = new char[256];
    char *valuec = new char[256];
    char *helpc  = new char[256];
    strncpy(namec, name.c_str(), 255);
    strncpy(valuec, value.c_str(), 255);
    strncpy(helpc, help.c_str(), 255);
    // logging::Info("Creating ConVar: %s %s %s", namec, valuec, helpc);
    ConVar *ret = new ConVar(const_cast<const char *>(namec),
                             const_cast<const char *>(valuec), 0,
                             const_cast<const char *>(helpc));
    g_ICvar->RegisterConCommand(ret);
    RegisteredVarsList().push_back(ret);
    return ret;
}

// Function for when you want to goto a vector
void WalkTo(const Vector &vector)
{
    if (CE_BAD(LOCAL_E))
        return;
    // Calculate how to get to a vector
    auto result = ComputeMove(LOCAL_E->m_vecOrigin(), vector);
    // Push our move to usercmd
    current_user_cmd->forwardmove = result.first;
    current_user_cmd->sidemove    = result.second;
}

// Function to get the corner location that a vischeck to an entity is possible
// from
Vector VischeckCorner(CachedEntity *player, CachedEntity *target, float maxdist,
                      bool checkWalkable)
{
    int maxiterations = maxdist / 40;
    Vector origin     = player->m_vecOrigin();

    // if we can see an entity, we don't need to run calculations
    if (VisCheckEntFromEnt(player, target))
    {
        if (!checkWalkable)
            return origin;
        else if (canReachVector(origin, target->m_vecOrigin()))
            return origin;
    }

    for (int i = 0; i < 8; i++) // for loop for all 4 directions
    {
        // 40 * maxiterations = range in HU
        for (int j = 0; j < maxiterations; j++)
        {
            Vector virtualOrigin = origin;
            // what direction to go in
            switch (i)
            {
            case 0:
                virtualOrigin.x = virtualOrigin.x + 40 * (j + 1);
                break;
            case 1:
                virtualOrigin.x = virtualOrigin.x - 40 * (j + 1);
                break;
            case 2:
                virtualOrigin.y = virtualOrigin.y + 40 * (j + 1);
                break;
            case 3:
                virtualOrigin.y = virtualOrigin.y - 40 * (j + 1);
                break;
            case 4:
                virtualOrigin.x = virtualOrigin.x + 20 * (j + 1);
                virtualOrigin.y = virtualOrigin.y + 20 * (j + 1);
                break;
            case 5:
                virtualOrigin.x = virtualOrigin.x - 20 * (j + 1);
                virtualOrigin.y = virtualOrigin.y - 20 * (j + 1);
                break;
            case 6:
                virtualOrigin.x = virtualOrigin.x - 20 * (j + 1);
                virtualOrigin.y = virtualOrigin.y + 20 * (j + 1);
                break;
            case 7:
                virtualOrigin.x = virtualOrigin.x + 20 * (j + 1);
                virtualOrigin.y = virtualOrigin.y - 20 * (j + 1);
                break;
            }
            // check if player can see the players virtualOrigin
            if (!IsVectorVisible(origin, virtualOrigin, true))
                continue;
            // check if the virtualOrigin can see the target
            if (!VisCheckEntFromEntVector(virtualOrigin, player, target))
                continue;
            if (!checkWalkable)
                return virtualOrigin;

            // check if the location is accessible
            if (!canReachVector(origin, virtualOrigin))
                continue;
            if (canReachVector(virtualOrigin, target->m_vecOrigin()))
                return virtualOrigin;
        }
    }
    // if we didn't find anything, return an empty Vector
    return { 0, 0, 0 };
}

// return Two Corners that connect perfectly to ent and local player
std::pair<Vector, Vector> VischeckWall(CachedEntity *player,
                                       CachedEntity *target, float maxdist,
                                       bool checkWalkable)
{
    int maxiterations = maxdist / 40;
    Vector origin     = player->m_vecOrigin();

    // if we can see an entity, we don't need to run calculations
    if (VisCheckEntFromEnt(player, target))
    {
        std::pair<Vector, Vector> orig(origin, target->m_vecOrigin());
        if (!checkWalkable)
            return orig;
        else if (canReachVector(origin, target->m_vecOrigin()))
            return orig;
    }

    for (int i = 0; i < 8; i++) // for loop for all 4 directions
    {
        // 40 * maxiterations = range in HU
        for (int j = 0; j < maxiterations; j++)
        {
            Vector virtualOrigin = origin;
            // what direction to go in
            switch (i)
            {
            case 0:
                virtualOrigin.x = virtualOrigin.x + 40 * (j + 1);
                break;
            case 1:
                virtualOrigin.x = virtualOrigin.x - 40 * (j + 1);
                break;
            case 2:
                virtualOrigin.y = virtualOrigin.y + 40 * (j + 1);
                break;
            case 3:
                virtualOrigin.y = virtualOrigin.y - 40 * (j + 1);
                break;
            case 4:
                virtualOrigin.x = virtualOrigin.x + 20 * (j + 1);
                virtualOrigin.y = virtualOrigin.y + 20 * (j + 1);
                break;
            case 5:
                virtualOrigin.x = virtualOrigin.x - 20 * (j + 1);
                virtualOrigin.y = virtualOrigin.y - 20 * (j + 1);
                break;
            case 6:
                virtualOrigin.x = virtualOrigin.x - 20 * (j + 1);
                virtualOrigin.y = virtualOrigin.y + 20 * (j + 1);
                break;
            case 7:
                virtualOrigin.x = virtualOrigin.x + 20 * (j + 1);
                virtualOrigin.y = virtualOrigin.y - 20 * (j + 1);
                break;
            }
            // check if player can see the players virtualOrigin
            if (!IsVectorVisible(origin, virtualOrigin, true))
                continue;
            for (int i = 0; i < 8; i++) // for loop for all 4 directions
            {
                // 40 * maxiterations = range in HU
                for (int j = 0; j < maxiterations; j++)
                {
                    Vector virtualOrigin2 = target->m_vecOrigin();
                    // what direction to go in
                    switch (i)
                    {
                    case 0:
                        virtualOrigin2.x = virtualOrigin2.x + 40 * (j + 1);
                        break;
                    case 1:
                        virtualOrigin2.x = virtualOrigin2.x - 40 * (j + 1);
                        break;
                    case 2:
                        virtualOrigin2.y = virtualOrigin2.y + 40 * (j + 1);
                        break;
                    case 3:
                        virtualOrigin2.y = virtualOrigin2.y - 40 * (j + 1);
                        break;
                    case 4:
                        virtualOrigin2.x = virtualOrigin2.x + 20 * (j + 1);
                        virtualOrigin2.y = virtualOrigin2.y + 20 * (j + 1);
                        break;
                    case 5:
                        virtualOrigin2.x = virtualOrigin2.x - 20 * (j + 1);
                        virtualOrigin2.y = virtualOrigin2.y - 20 * (j + 1);
                        break;
                    case 6:
                        virtualOrigin2.x = virtualOrigin2.x - 20 * (j + 1);
                        virtualOrigin2.y = virtualOrigin2.y + 20 * (j + 1);
                        break;
                    case 7:
                        virtualOrigin2.x = virtualOrigin2.x + 20 * (j + 1);
                        virtualOrigin2.y = virtualOrigin2.y - 20 * (j + 1);
                        break;
                    }
                    // check if the virtualOrigin2 can see the target
                    //                    if
                    //                    (!VisCheckEntFromEntVector(virtualOrigin2,
                    //                    player, target))
                    //                        continue;
                    //                    if (!IsVectorVisible(virtualOrigin,
                    //                    virtualOrigin2, true))
                    //                        continue;
                    //                    if (!IsVectorVisible(virtualOrigin2,
                    //                    target->m_vecOrigin(), true))
                    //                    	continue;
                    if (!IsVectorVisible(virtualOrigin, virtualOrigin2, true))
                        continue;
                    if (!IsVectorVisible(virtualOrigin2, target->m_vecOrigin()))
                        continue;
                    std::pair<Vector, Vector> toret(virtualOrigin,
                                                    virtualOrigin2);
                    if (!checkWalkable)
                        return toret;
                    // check if the location is accessible
                    if (!canReachVector(origin, virtualOrigin) ||
                        !canReachVector(virtualOrigin2, virtualOrigin))
                        continue;
                    if (canReachVector(virtualOrigin2, target->m_vecOrigin()))
                        return toret;
                }
            }
        }
    }
    // if we didn't find anything, return an empty Vector
    return { { 0, 0, 0 }, { 0, 0, 0 } };
}

// Returns a vectors max value. For example: {123,-150, 125} = 125
float vectorMax(Vector i)
{
    return fmaxf(fmaxf(i.x, i.y), i.z);
}

// Returns a vectors absolute value. For example {123,-150, 125} = {123,150,
// 125}
Vector vectorAbs(Vector i)
{
    Vector result = i;
    result.x      = fabsf(result.x);
    result.y      = fabsf(result.y);
    result.z      = fabsf(result.z);
    return result;
}

// check to see if we can reach a vector or if it is too high / doesn't leave
// enough space for the player, optional second vector
bool canReachVector(Vector loc, Vector dest)
{
    if (!dest.IsZero())
    {
        Vector dist       = dest - loc;
        int maxiterations = floor(dest.DistTo(loc)) / 40;
        for (int i = 0; i < maxiterations; i++)
        {
            // math to get the next vector 40.0f in the direction of dest
            Vector vec =
                loc + dist / vectorMax(vectorAbs(dist)) * 40.0f * (i + 1);

            if (DistanceToGround({ vec.x, vec.y, vec.z + 5 }) >= 40)
                return false;

            for (int j = 0; j < 4; j++)
            {
                Vector directionalLoc = vec;
                // what direction to check
                switch (j)
                {
                case 0:
                    directionalLoc.x = directionalLoc.x + 40;
                    break;
                case 1:
                    directionalLoc.x = directionalLoc.x - 40;
                    break;
                case 2:
                    directionalLoc.y = directionalLoc.y + 40;
                    break;
                case 3:
                    directionalLoc.y = directionalLoc.y - 40;
                    break;
                }
                trace_t trace;
                Ray_t ray;
                ray.Init(vec, directionalLoc);
                g_ITrace->TraceRay(ray, 0x4200400B, &trace::filter_no_player,
                                   &trace);
                // distance of trace < than 26
                if (trace.startpos.DistTo(trace.endpos) < 26.0f)
                    return false;
            }
        }
    }
    else
    {
        // check if the vector is too high above ground
        // higher to avoid small false positives, player can jump 42 hu
        // according to
        // the tf2 wiki
        if (DistanceToGround({ loc.x, loc.y, loc.z + 5 }) >= 40)
            return false;

        // check if there is enough space arround the vector for a player to fit
        // for loop for all 4 directions
        for (int i = 0; i < 4; i++)
        {
            Vector directionalLoc = loc;
            // what direction to check
            switch (i)
            {
            case 0:
                directionalLoc.x = directionalLoc.x + 40;
                break;
            case 1:
                directionalLoc.x = directionalLoc.x - 40;
                break;
            case 2:
                directionalLoc.y = directionalLoc.y + 40;
                break;
            case 3:
                directionalLoc.y = directionalLoc.y - 40;
                break;
            }
            trace_t trace;
            Ray_t ray;
            ray.Init(loc, directionalLoc);
            g_ITrace->TraceRay(ray, 0x4200400B, &trace::filter_no_player,
                               &trace);
            // distance of trace < than 26
            if (trace.startpos.DistTo(trace.endpos) < 26.0f)
                return false;
        }
    }
    return true;
}

std::string GetLevelName()
{

    std::string name(g_IEngine->GetLevelName());
    size_t slash = name.find('/');
    if (slash == std::string::npos)
        slash = 0;
    else
        slash++;
    size_t bsp = name.find(".bsp");
    size_t length =
        (bsp == std::string::npos ? name.length() - slash : bsp - slash);
    return name.substr(slash, length);
}

std::pair<float, float> ComputeMove(const Vector &a, const Vector &b)
{
    Vector diff = (b - a);
    if (diff.Length() == 0)
        return { 0, 0 };
    const float x = diff.x;
    const float y = diff.y;
    Vector vsilent(x, y, 0);
    float speed = sqrt(vsilent.x * vsilent.x + vsilent.y * vsilent.y);
    Vector ang;
    VectorAngles(vsilent, ang);
    float yaw = DEG2RAD(ang.y - current_user_cmd->viewangles.y);
    return { cos(yaw) * 450, -sin(yaw) * 450 };
}

ConCommand *CreateConCommand(const char *name, FnCommandCallback_t callback,
                             const char *help)
{
    ConCommand *ret = new ConCommand(name, callback, help);
    g_ICvar->RegisterConCommand(ret);
    RegisteredCommandsList().push_back(ret);
    return ret;
}

const char *GetBuildingName(CachedEntity *ent)
{
    if (!ent)
        return "[NULL]";
    int classid = ent->m_iClassID();
    if (classid == CL_CLASS(CObjectSentrygun))
        return "Sentry";
    if (classid == CL_CLASS(CObjectDispenser))
        return "Dispenser";
    if (classid == CL_CLASS(CObjectTeleporter))
        return "Teleporter";
    return "[NULL]";
}

void format_internal(std::stringstream &stream)
{
    (void) (stream);
}

void ReplaceString(std::string &input, const std::string &what,
                   const std::string &with_what)
{
    size_t index;
    index = input.find(what);
    while (index != std::string::npos)
    {
        input.replace(index, what.size(), with_what);
        index = input.find(what, index + with_what.size());
    }
}

powerup_type GetPowerupOnPlayer(CachedEntity *player)
{
    if (CE_BAD(player))
        return powerup_type::not_powerup;
    //	if (!HasCondition<TFCond_HasRune>(player)) return
    // powerup_type::not_powerup;
    if (HasCondition<TFCond_RuneStrength>(player))
        return powerup_type::strength;
    if (HasCondition<TFCond_RuneHaste>(player))
        return powerup_type::haste;
    if (HasCondition<TFCond_RuneRegen>(player))
        return powerup_type::regeneration;
    if (HasCondition<TFCond_RuneResist>(player))
        return powerup_type::resistance;
    if (HasCondition<TFCond_RuneVampire>(player))
        return powerup_type::vampire;
    if (HasCondition<TFCond_RuneWarlock>(player))
        return powerup_type::reflect;
    if (HasCondition<TFCond_RunePrecision>(player))
        return powerup_type::precision;
    if (HasCondition<TFCond_RuneAgility>(player))
        return powerup_type::agility;
    if (HasCondition<TFCond_RuneKnockout>(player))
        return powerup_type::knockout;
    if (HasCondition<TFCond_KingRune>(player))
        return powerup_type::king;
    if (HasCondition<TFCond_PlagueRune>(player))
        return powerup_type::plague;
    if (HasCondition<TFCond_SupernovaRune>(player))
        return powerup_type::supernova;
    return powerup_type::not_powerup;
}

// A function to tell if a player is using a specific weapon
bool HasWeapon(CachedEntity *ent, int wantedId)
{
    if (CE_BAD(ent))
        return false;
    // Create a var to store the handle
    int *hWeapons;
    // Grab the handle and store it into the var
    hWeapons = (int *) ((unsigned) (RAW_ENT(ent) + netvar.hMyWeapons));
    // Go through the handle array and search for the item
    for (int i = 0; hWeapons[i]; i++)
    {
        // Get the weapon id from the handle array
        IClientEntity *weapon =
            g_IEntityList->GetClientEntityFromHandle(hWeapons[i]);
        // if weapon is what we are looking for, return true
        if (weapon && NET_INT(weapon, netvar.iItemDefinitionIndex) == wantedId)
            return true;
    }
    // We didnt find the weapon we needed, return false
    return false;
}

void VectorTransform(const float *in1, const matrix3x4_t &in2, float *out)
{
    out[0] = (in1[0] * in2[0][0] + in1[1] * in2[0][1] + in1[2] * in2[0][2]) +
             in2[0][3];
    out[1] = (in1[0] * in2[1][0] + in1[1] * in2[1][1] + in1[2] * in2[1][2]) +
             in2[1][3];
    out[2] = (in1[0] * in2[2][0] + in1[1] * in2[2][1] + in1[2] * in2[2][2]) +
             in2[2][3];
}

bool GetHitbox(CachedEntity *entity, int hb, Vector &out)
{
    hitbox_cache::CachedHitbox *box;

    if (CE_BAD(entity))
        return false;
    box = entity->hitboxes.GetHitbox(hb);
    if (!box)
        out = entity->m_vecOrigin();
    else
        out = box->center;
    return true;
}

void VectorAngles(Vector &forward, Vector &angles)
{
    float tmp, yaw, pitch;

    if (forward[1] == 0 && forward[0] == 0)
    {
        yaw = 0;
        if (forward[2] > 0)
            pitch = 270;
        else
            pitch = 90;
    }
    else
    {
        yaw = (atan2(forward[1], forward[0]) * 180 / PI);
        if (yaw < 0)
            yaw += 360;

        tmp   = sqrt((forward[0] * forward[0] + forward[1] * forward[1]));
        pitch = (atan2(-forward[2], tmp) * 180 / PI);
        if (pitch < 0)
            pitch += 360;
    }

    angles[0] = pitch;
    angles[1] = yaw;
    angles[2] = 0;
}

char GetUpperChar(ButtonCode_t button)
{
    switch (button)
    {
    case KEY_0:
        return ')';
    case KEY_1:
        return '!';
    case KEY_2:
        return '@';
    case KEY_3:
        return '#';
    case KEY_4:
        return '$';
    case KEY_5:
        return '%';
    case KEY_6:
        return '^';
    case KEY_7:
        return '&';
    case KEY_8:
        return '*';
    case KEY_9:
        return '(';
    case KEY_LBRACKET:
        return '{';
    case KEY_RBRACKET:
        return '}';
    case KEY_SEMICOLON:
        return ':';
    case KEY_BACKQUOTE:
        return '~';
    case KEY_APOSTROPHE:
        return '"';
    case KEY_COMMA:
        return '<';
    case KEY_PERIOD:
        return '>';
    case KEY_SLASH:
        return '?';
    case KEY_BACKSLASH:
        return '|';
    case KEY_MINUS:
        return '_';
    case KEY_EQUAL:
        return '+';
    default:
        if (strlen(g_IInputSystem->ButtonCodeToString(button)) != 1)
            return 0;
        return toupper(*g_IInputSystem->ButtonCodeToString(button));
    }
}

char GetChar(ButtonCode_t button)
{
    switch (button)
    {
    case KEY_PAD_DIVIDE:
        return '/';
    case KEY_PAD_MULTIPLY:
        return '*';
    case KEY_PAD_MINUS:
        return '-';
    case KEY_PAD_PLUS:
        return '+';
    case KEY_SEMICOLON:
        return ';';
    default:
        if (button >= KEY_PAD_0 && button <= KEY_PAD_9)
        {
            return button - KEY_PAD_0 + '0';
        }
        if (strlen(g_IInputSystem->ButtonCodeToString(button)) != 1)
            return 0;
        return *g_IInputSystem->ButtonCodeToString(button);
    }
}

void FixMovement(CUserCmd &cmd, Vector &viewangles)
{
    Vector movement, ang;
    float speed, yaw;
    movement.x = cmd.forwardmove;
    movement.y = cmd.sidemove;
    movement.z = cmd.upmove;
    speed      = sqrt(movement.x * movement.x + movement.y * movement.y);
    VectorAngles(movement, ang);
    yaw             = DEG2RAD(ang.y - viewangles.y + cmd.viewangles.y);
    cmd.forwardmove = cos(yaw) * speed;
    cmd.sidemove    = sin(yaw) * speed;
}

bool AmbassadorCanHeadshot()
{
    if (IsAmbassador(g_pLocalPlayer->weapon()))
    {
        if ((g_GlobalVars->curtime -
             CE_FLOAT(g_pLocalPlayer->weapon(), netvar.flLastFireTime)) <= 1.0)
        {
            return false;
        }
    }
    return true;
}

float RandFloatRange(float min, float max)
{
    return (min + 1) +
           (((float) rand()) / (float) RAND_MAX) * (max - (min + 1));
}

bool IsEntityVisible(CachedEntity *entity, int hb)
{
    if (g_Settings.bInvalid)
        return false;
    if (entity == g_pLocalPlayer->entity)
        return true;
    if (hb == -1)
    {
        return IsEntityVectorVisible(entity, entity->m_vecOrigin());
    }
    else
    {
        return entity->hitboxes.VisibilityCheck(hb);
    }
}

std::mutex trace_lock;
bool IsEntityVectorVisible(CachedEntity *entity, Vector endpos)
{
    trace_t trace_object;
    Ray_t ray;

    if (g_Settings.bInvalid)
        return false;
    if (entity == g_pLocalPlayer->entity)
        return true;
    if (CE_BAD(g_pLocalPlayer->entity))
        return false;
    if (CE_BAD(entity))
        return false;
    trace::filter_default.SetSelf(RAW_ENT(g_pLocalPlayer->entity));
    ray.Init(g_pLocalPlayer->v_Eye, endpos);
    {
        PROF_SECTION(IEVV_TraceRay);
        std::lock_guard<std::mutex> lock(trace_lock);
        if (!tcm || g_Settings.is_create_move)
            g_ITrace->TraceRay(ray, MASK_SHOT_HULL, &trace::filter_default,
                               &trace_object);
    }
    return (((IClientEntity *) trace_object.m_pEnt) == RAW_ENT(entity) ||
            trace_object.fraction >= 0.99f);
}

// For when you need to vis check something that isnt the local player
bool VisCheckEntFromEnt(CachedEntity *startEnt, CachedEntity *endEnt)
{
    // We setSelf as the starting ent as we dont want to hit it, we want the
    // other ent
    trace_t trace;
    trace::filter_default.SetSelf(RAW_ENT(startEnt));

    // Setup the trace starting with the origin of the starting ent attemting to
    // hit the origin of the end ent
    Ray_t ray;
    ray.Init(startEnt->m_vecOrigin(), endEnt->m_vecOrigin());
    {
        PROF_SECTION(IEVV_TraceRay);
        g_ITrace->TraceRay(ray, MASK_SHOT_HULL, &trace::filter_default, &trace);
    }
    // Is the entity that we hit our target ent? if so, the vis check passes
    if (trace.m_pEnt)
    {
        if ((((IClientEntity *) trace.m_pEnt)) == RAW_ENT(endEnt))
            return true;
    }
    // Since we didnt hit our target ent, the vis check failed so return false
    return false;
}

// Use when you need to vis check something but its not the ent origin that you
// use, so we check from the vector to the ent, ignoring the first just in case
bool VisCheckEntFromEntVector(Vector startVector, CachedEntity *startEnt,
                              CachedEntity *endEnt)
{
    // We setSelf as the starting ent as we dont want to hit it, we want the
    // other ent
    trace_t trace;
    trace::filter_default.SetSelf(RAW_ENT(startEnt));

    // Setup the trace starting with the origin of the starting ent attemting to
    // hit the origin of the end ent
    Ray_t ray;
    ray.Init(startVector, endEnt->m_vecOrigin());
    {
        PROF_SECTION(IEVV_TraceRay);
        g_ITrace->TraceRay(ray, MASK_SHOT_HULL, &trace::filter_default, &trace);
    }
    // Is the entity that we hit our target ent? if so, the vis check passes
    if (trace.m_pEnt)
    {
        if ((((IClientEntity *) trace.m_pEnt)) == RAW_ENT(endEnt))
            return true;
    }
    // Since we didnt hit our target ent, the vis check failed so return false
    return false;
}

Vector GetBuildingPosition(CachedEntity *ent)
{
    Vector res;
    res         = ent->m_vecOrigin();
    int classid = ent->m_iClassID();
    if (classid == CL_CLASS(CObjectDispenser))
        res.z += 30;
    if (classid == CL_CLASS(CObjectTeleporter))
        res.z += 8;
    if (classid == CL_CLASS(CObjectSentrygun))
    {
        switch (CE_INT(ent, netvar.iUpgradeLevel))
        {
        case 1:
            res.z += 30;
            break;
        case 2:
            res.z += 50;
            break;
        case 3:
            res.z += 60;
            break;
        }
    }
    return res;
}

bool IsBuildingVisible(CachedEntity *ent)
{
    return IsEntityVectorVisible(ent, GetBuildingPosition(ent));
}

void fClampAngle(Vector &qaAng)
{
    while (qaAng[0] > 89)
        qaAng[0] -= 180;

    while (qaAng[0] < -89)
        qaAng[0] += 180;

    while (qaAng[1] > 180)
        qaAng[1] -= 360;

    while (qaAng[1] < -180)
        qaAng[1] += 360;

    qaAng.z = 0;
}

float DistToSqr(CachedEntity *entity)
{
    if (CE_BAD(entity))
        return 0.0f;
    return g_pLocalPlayer->v_Origin.DistToSqr(entity->m_vecOrigin());
}

void Patch(void *address, void *patch, size_t length)
{
    void *page = (void *) ((uintptr_t) address & ~0xFFF);
    logging::Info("mprotect: %d",
                  mprotect(page, 0xFFF, PROT_READ | PROT_WRITE | PROT_EXEC));
    memcpy(address, patch, length);
    logging::Info("mprotect reverse: %d", mprotect(page, 0xFFF, PROT_EXEC));
}

bool IsProjectileCrit(CachedEntity *ent)
{
    if (ent->m_bGrenadeProjectile())
        return CE_BYTE(ent, netvar.Grenade_bCritical);
    return CE_BYTE(ent, netvar.Rocket_bCritical);
}

weaponmode GetWeaponMode()
{
    int weapon_handle, slot;
    CachedEntity *weapon;

    if (CE_BAD(LOCAL_E))
        return weapon_invalid;
    weapon_handle = CE_INT(LOCAL_E, netvar.hActiveWeapon);
    if (IDX_BAD((weapon_handle & 0xFFF)))
    {
        // logging::Info("IDX_BAD: %i", weapon_handle & 0xFFF);
        return weaponmode::weapon_invalid;
    }
    weapon = (ENTITY(weapon_handle & 0xFFF));
    if (CE_BAD(weapon))
        return weaponmode::weapon_invalid;
    int classid = weapon->m_iClassID();
    slot        = re::C_BaseCombatWeapon::GetSlot(RAW_ENT(weapon));
    if (slot == 2)
        return weaponmode::weapon_melee;
    if (slot > 2)
    {
        return weaponmode::weapon_pda;
    }
    else if (classid == CL_CLASS(CTFLunchBox) ||
             classid == CL_CLASS(CTFLunchBox_Drink) ||
             classid == CL_CLASS(CTFBuffItem))
    {
        return weaponmode::weapon_consumable;
    }
    else if (classid == CL_CLASS(CTFRocketLauncher_DirectHit) ||
             classid == CL_CLASS(CTFRocketLauncher) ||
             classid == CL_CLASS(CTFGrenadeLauncher) ||
             classid == CL_CLASS(CTFPipebombLauncher) ||
             classid == CL_CLASS(CTFCompoundBow) ||
             classid == CL_CLASS(CTFBat_Wood) ||
             classid == CL_CLASS(CTFBat_Giftwrap) ||
             classid == CL_CLASS(CTFFlareGun) ||
             classid == CL_CLASS(CTFFlareGun_Revenge) ||
             classid == CL_CLASS(CTFSyringeGun) ||
             classid == CL_CLASS(CTFCrossbow) ||
             classid == CL_CLASS(CTFShotgunBuildingRescue) ||
             classid == CL_CLASS(CTFDRGPomson) ||
             classid == CL_CLASS(CTFWeaponFlameBall) ||
             classid == CL_CLASS(CTFRaygun) ||
             classid == CL_CLASS(CTFGrapplingHook))
    {
        return weaponmode::weapon_projectile;
    }
    else if (classid == CL_CLASS(CTFJar) || classid == CL_CLASS(CTFJarMilk))
    {
        return weaponmode::weapon_throwable;
    }
    else if (classid == CL_CLASS(CWeaponMedigun))
    {
        return weaponmode::weapon_medigun;
    }
    return weaponmode::weapon_hitscan;
}

bool LineIntersectsBox(Vector &bmin, Vector &bmax, Vector &lmin, Vector &lmax)
{
    if (lmax.x < bmin.x && lmin.x < bmin.x)
        return false;
    if (lmax.y < bmin.y && lmin.y < bmin.y)
        return false;
    if (lmax.z < bmin.z && lmin.z < bmin.z)
        return false;
    if (lmax.x > bmax.x && lmin.x > bmax.x)
        return false;
    if (lmax.y > bmax.y && lmin.y > bmax.y)
        return false;
    if (lmax.z > bmax.z && lmin.z > bmax.z)
        return false;
    return true;
}

bool GetProjectileData(CachedEntity *weapon, float &speed, float &gravity)
{
    float rspeed, rgrav;

    IF_GAME(!IsTF()) return false;

    if (CE_BAD(weapon))
        return false;
    rspeed = 0.0f;
    rgrav  = 0.0f;
    typedef float(GetProjectileData)(IClientEntity *);

    switch (weapon->m_iClassID())
    {
    case CL_CLASS(CTFRocketLauncher_DirectHit):
    {
        rspeed = 1980.0f;
        break;
    }
    case CL_CLASS(CTFRocketLauncher):
    {
        rspeed = 1100.0f;
        break;
    }
    case CL_CLASS(CTFGrenadeLauncher):
    {
        IF_GAME(IsTF2())
        {
            rspeed = 1200.0f;
            rgrav  = 0.4f;
        }
        else IF_GAME(IsTF2C())
        {
            rspeed = 1100.0f;
            rgrav  = 0.5f;
        }
        break;
    }
    case CL_CLASS(CTFPipebombLauncher):
    {
        float chargebegin = *((float *) ((unsigned) RAW_ENT(LOCAL_W) + 3152));
        float chargetime  = g_GlobalVars->curtime - chargebegin;
        rspeed =
            (fminf(fmaxf(chargetime / 4.0f, 0.0f), 1.0f) * 1500.0f) + 900.0f;
        rgrav =
            (fminf(fmaxf(chargetime / 4.0f, 0.0f), 1.0f) * -0.70000001f) + 0.5f;
        break;
    }
    case CL_CLASS(CTFCompoundBow):
    {
        float chargetime =
            g_GlobalVars->curtime - CE_FLOAT(weapon, netvar.flChargeBeginTime);
        rspeed = (float) ((float) (fminf(fmaxf(chargetime, 0.0), 1.0) * 800.0) +
                          1800.0);
        rgrav  = (float) ((float) (fminf(fmaxf(chargetime, 0.0), 1.0) *
                                  -0.40000001) +
                         0.5);
        break;
    }
    case CL_CLASS(CTFBat_Wood):
    {
        rspeed = 3000.0f;
        rgrav  = 0.5f;
        break;
    }
    case CL_CLASS(CTFFlareGun):
    {
        rspeed = 2000.0f;
        rgrav  = 0.25f;
        break;
    }
    case CL_CLASS(CTFSyringeGun):
    {
        rgrav  = 0.2f;
        rspeed = 990.0f;
        break;
    }
    case CL_CLASS(CTFCrossbow):
    {
        rgrav  = 0.2f;
        rspeed = 2400.0f;
        break;
    }
    case CL_CLASS(CTFShotgunBuildingRescue):
    {
        rgrav  = 0.2f;
        rspeed = 2400.0f;
        break;
    }
    case CL_CLASS(CTFDRGPomson):
    {
        rspeed = 1200.0f;
        break;
    }
    case CL_CLASS(CTFWeaponFlameBall):
    {
        rspeed = 3000.0f;
        break;
    }
    case CL_CLASS(CTFRaygun):
    {
        rspeed = 1200.0f;
        break;
    }
    case CL_CLASS(CTFGrapplingHook):
    {
        rspeed = 1500.0f;
        break;
    }
    }

    speed   = rspeed;
    gravity = rgrav;
    return (rspeed || rgrav);
}

constexpr unsigned developer_list[] = { 306902159 };

bool Developer(CachedEntity *ent)
{
    /*
    for (int i = 0; i < sizeof(developer_list) / sizeof(unsigned); i++) {
        if (developer_list[i] == ent->player_info.friendsID) return true;
    }
    */
    return false;
}

/*const char* MakeInfoString(IClientEntity* player) {
    char* buf = new char[256]();
    player_info_t info;
    if (!engineClient->GetPlayerInfo(player->entindex(), &info)) return (const
char*)0; logging::Info("a"); int hWeapon = NET_INT(player,
netvar.hActiveWeapon); if (NET_BYTE(player, netvar.iLifeState)) { sprintf(buf,
"%s is dead %s", info.name, tfclasses[NET_INT(player, netvar.iClass)]); return
buf;
    }
    if (hWeapon) {
        IClientEntity* weapon = ENTITY(hWeapon & 0xFFF);
        sprintf(buf, "%s is %s with %i health using %s", info.name,
tfclasses[NET_INT(player, netvar.iClass)], NET_INT(player, netvar.iHealth),
weapon->GetClientClass()->GetName()); } else { sprintf(buf, "%s is %s with %i
health", info.name, tfclasses[NET_INT(player, netvar.iClass)], NET_INT(player,
netvar.iHealth));
    }
    logging::Info("Result: %s", buf);
    return buf;
}*/

bool IsVectorVisible(Vector origin, Vector target, bool enviroment_only)
{
    trace_t trace_visible;
    Ray_t ray;

    trace::filter_no_player.SetSelf(RAW_ENT(g_pLocalPlayer->entity));
    ray.Init(origin, target);
    if (!enviroment_only)
        g_ITrace->TraceRay(ray, MASK_SHOT_HULL, &trace::filter_no_player,
                           &trace_visible);
    else
        g_ITrace->TraceRay(ray, 0x4200400B, &trace::filter_no_player,
                           &trace_visible);

    return (trace_visible.fraction == 1.0f);
}

void WhatIAmLookingAt(int *result_eindex, Vector *result_pos)
{
    Ray_t ray;
    Vector forward;
    float sp, sy, cp, cy;
    QAngle angle;
    trace_t trace;

    trace::filter_default.SetSelf(RAW_ENT(g_pLocalPlayer->entity));
    g_IEngine->GetViewAngles(angle);
    sy        = sinf(DEG2RAD(angle[1]));
    cy        = cosf(DEG2RAD(angle[1]));
    sp        = sinf(DEG2RAD(angle[0]));
    cp        = cosf(DEG2RAD(angle[0]));
    forward.x = cp * cy;
    forward.y = cp * sy;
    forward.z = -sp;
    forward   = forward * 8192.0f + g_pLocalPlayer->v_Eye;
    ray.Init(g_pLocalPlayer->v_Eye, forward);
    g_ITrace->TraceRay(ray, 0x4200400B, &trace::filter_default, &trace);
    if (result_pos)
        *result_pos = trace.endpos;
    if (result_eindex)
    {
        *result_eindex = 0;
    }
    if (trace.m_pEnt && result_eindex)
    {
        *result_eindex = ((IClientEntity *) (trace.m_pEnt))->entindex();
    }
}

bool IsSentryBuster(CachedEntity *entity)
{
    return (entity->m_Type() == EntityType::ENTITY_PLAYER &&
            CE_INT(entity, netvar.iClass) == tf_class::tf_demoman &&
            g_pPlayerResource->GetMaxHealth(entity) == 2500);
}

bool IsAmbassador(CachedEntity *entity)
{
    IF_GAME(!IsTF2()) return false;
    if (entity->m_iClassID() != CL_CLASS(CTFRevolver))
        return false;
    const int &defidx = CE_INT(entity, netvar.iItemDefinitionIndex);
    return (defidx == 61 || defidx == 1006);
}

bool IsPlayerInvulnerable(CachedEntity *player)
{
    return HasConditionMask<
        KInvulnerabilityMask.cond_0, KInvulnerabilityMask.cond_1,
        KInvulnerabilityMask.cond_2, KInvulnerabilityMask.cond_3>(player);
}

bool IsPlayerCritBoosted(CachedEntity *player)
{
    return HasConditionMask<KCritBoostMask.cond_0, KCritBoostMask.cond_1,
                            KCritBoostMask.cond_2, KCritBoostMask.cond_3>(
        player);
}

bool IsPlayerInvisible(CachedEntity *player)
{
    return HasConditionMask<KInvisibilityMask.cond_0, KInvisibilityMask.cond_1,
                            KInvisibilityMask.cond_2, KInvisibilityMask.cond_3>(
        player);
}

bool IsPlayerDisguised(CachedEntity *player)
{
    return HasConditionMask<KDisguisedMask.cond_0, KDisguisedMask.cond_1,
                            KDisguisedMask.cond_2, KDisguisedMask.cond_3>(
        player);
}

// F1 c&p
Vector CalcAngle(Vector src, Vector dst)
{
    Vector AimAngles, delta;
    float hyp;
    delta = src - dst;
    hyp   = sqrtf(
        (delta.x * delta.x) +
        (delta.y * delta.y)); // SUPER SECRET IMPROVEMENT CODE NAME DONUT STEEL
    AimAngles.x = atanf(delta.z / hyp) * RADPI;
    AimAngles.y = atanf(delta.y / delta.x) * RADPI;
    AimAngles.z = 0.0f;
    if (delta.x >= 0.0)
        AimAngles.y += 180.0f;
    return AimAngles;
}

void MakeVector(Vector angle, Vector &vector)
{
    float pitch, yaw, tmp;
    pitch     = float(angle[0] * PI / 180);
    yaw       = float(angle[1] * PI / 180);
    tmp       = float(cos(pitch));
    vector[0] = float(-tmp * -cos(yaw));
    vector[1] = float(sin(yaw) * tmp);
    vector[2] = float(-sin(pitch));
}

float GetFov(Vector angle, Vector src, Vector dst)
{
    Vector ang, aim;
    float mag, u_dot_v;
    ang = CalcAngle(src, dst);

    MakeVector(angle, aim);
    MakeVector(ang, ang);

    mag     = sqrtf(pow(aim.x, 2) + pow(aim.y, 2) + pow(aim.z, 2));
    u_dot_v = aim.Dot(ang);

    return RAD2DEG(acos(u_dot_v / (pow(mag, 2))));
}

bool CanHeadshot()
{
    return (g_pLocalPlayer->flZoomBegin > 0.0f &&
            (g_GlobalVars->curtime - g_pLocalPlayer->flZoomBegin > 0.2f));
}

bool CanShoot()
{

    float servertime, nextattack;

    servertime = (float) (CE_INT(g_pLocalPlayer->entity, netvar.nTickBase)) *
                 g_GlobalVars->interval_per_tick;
    nextattack = CE_FLOAT(g_pLocalPlayer->weapon(), netvar.flNextPrimaryAttack);
    return nextattack <= servertime;
}

QAngle VectorToQAngle(Vector in)
{
    return *(QAngle *) &in;
}

Vector QAngleToVector(QAngle in)
{
    return *(Vector *) &in;
}

void AimAt(Vector origin, Vector target, CUserCmd *cmd)
{
    Vector angles, tr;
    tr = (target - origin);
    VectorAngles(tr, angles);
    fClampAngle(angles);
    cmd->viewangles = angles;
}

void AimAtHitbox(CachedEntity *ent, int hitbox, CUserCmd *cmd)
{
    Vector r;
    r = ent->m_vecOrigin();
    GetHitbox(ent, hitbox, r);
    AimAt(g_pLocalPlayer->v_Eye, r, cmd);
}

bool IsEntityVisiblePenetration(CachedEntity *entity, int hb)
{
    trace_t trace_visible;
    Ray_t ray;
    Vector hit;
    int ret;
    bool correct_entity;
    IClientEntity *ent;
    trace::filter_penetration.SetSelf(RAW_ENT(g_pLocalPlayer->entity));
    trace::filter_penetration.Reset();
    ret = GetHitbox(entity, hb, hit);
    if (ret)
    {
        return false;
    }
    ray.Init(g_pLocalPlayer->v_Origin + g_pLocalPlayer->v_ViewOffset, hit);
    g_ITrace->TraceRay(ray, MASK_SHOT_HULL, &trace::filter_penetration,
                       &trace_visible);
    correct_entity = false;
    if (trace_visible.m_pEnt)
    {
        correct_entity =
            ((IClientEntity *) trace_visible.m_pEnt) == RAW_ENT(entity);
    }
    if (!correct_entity)
        return false;
    g_ITrace->TraceRay(ray, 0x4200400B, &trace::filter_default, &trace_visible);
    if (trace_visible.m_pEnt)
    {
        ent = (IClientEntity *) trace_visible.m_pEnt;
        if (ent)
        {
            if (ent->GetClientClass()->m_ClassID == RCC_PLAYER)
            {
                if (ent == RAW_ENT(entity))
                    return false;
                if (trace_visible.hitbox >= 0)
                {
                    return true;
                }
            }
        }
    }
    return false;
}

// Used for getting class names
CatCommand print_classnames(
    "debug_print_classnames", "Lists classnames currently available in console",
    []() {
        // Create a tmp ent for the loop
        CachedEntity *ent;

        // Go through all the entities
        for (int i = 0; i < HIGHEST_ENTITY; i++)
        {

            // Get an entity
            ent = ENTITY(i);
            // Check for null/dormant
            if (CE_BAD(ent))
                continue;

            // Print in console, the class name of the ent
            logging::Info(
                format(RAW_ENT(ent)->GetClientClass()->m_pNetworkName).c_str());
        }
    });

void PrintChat(const char *fmt, ...)
{
#if not ENABLE_VISUALS
    return;
#endif
    CHudBaseChat *chat = (CHudBaseChat *) g_CHUD->FindElement("CHudChat");
    if (chat)
    {
        std::unique_ptr<char[]> buf(new char[1024]);
        va_list list;
        va_start(list, fmt);
        vsprintf(buf.get(), fmt, list);
        va_end(list);
        std::unique_ptr<char[]> str =
            std::move(strfmt("\x07%06X[\x07%06XCAT\x07%06X]\x01 %s", 0x5e3252,
                             0xba3d9a, 0x5e3252, buf.get()));
        // FIXME DEBUG LOG
        logging::Info("%s", str.get());
        chat->Printf(str.get());
    }
    else
    {
    }
}

// You shouldn't delete[] this unique_ptr since it
// does it on its own
std::unique_ptr<char[]> strfmt(const char *fmt, ...)
{
    // char *buf = new char[1024];
    auto buf = std::make_unique<char[]>(1024);
    va_list list;
    va_start(list, fmt);
    vsprintf(buf.get(), fmt, list);
    va_end(list);
    return buf;
}

const char *powerups[] = { "STRENGTH", "RESISTANCE",   "VAMPIRE",   "REFLECT",
                           "HASTE",    "REGENERATION", "PRECISION", "AGILITY",
                           "KNOCKOUT", "KING",         "PLAGUE",    "SUPERNOVA",
                           "CRITS" };

const std::string classes[] = { "Scout",   "Sniper", "Soldier",
                                "Demoman", "Medic",  "Heavy",
                                "Pyro",    "Spy",    "Engineer" };
