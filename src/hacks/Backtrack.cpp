/*
 * Backtrack.cpp
 *
 *  Created on: May 15, 2018
 *      Author: bencat07
 */

#include "common.hpp"
#include "hacks/Backtrack.hpp"
#include <boost/circular_buffer.hpp>
#include <glez/draw.hpp>
namespace hacks::shared::backtrack
{
CatVar enable(CV_SWITCH, "backtrack", "0", "Enable backtrack",
              "For legit play only as of right now.");
static CatVar draw_bt(CV_SWITCH, "backtrack_draw", "0", "Draw",
                      "Draw backtrack ticks");
CatVar latency(CV_FLOAT, "backtrack_latency", "0", "fake lantency",
               "Set fake latency to this many ms");
static CatVar mindistance(CV_FLOAT, "mindistance", "60", "mindistance");
static CatEnum slots_enum({ "All", "Primary", "Secondary", "Melee",
                            "Primary Secondary", "Primary Melee",
                            "Secondary Melee" });
static CatVar slots(slots_enum, "backtrack_slots", "0", "Enabled Slots",
                    "Select what slots backtrack should be enabled on.");

BacktrackData headPositions[32][66]{};
BestTickData sorted_ticks[66]{};
int highesttick[32]{};
int lastincomingsequencenumber = 0;
static bool shouldDrawBt;

circular_buf sequences{ 2048 };
void UpdateIncomingSequences()
{
    INetChannel *ch = (INetChannel *) g_IEngine->GetNetChannelInfo();
    if (ch)
    {
        int m_nInSequenceNr = ch->m_nInSequenceNr;
        int instate         = ch->m_nInReliableState;
        if (m_nInSequenceNr > lastincomingsequencenumber)
        {
            lastincomingsequencenumber = m_nInSequenceNr;
            sequences.push_front(CIncomingSequence(instate, m_nInSequenceNr,
                                                   g_GlobalVars->realtime));
        }

        if (sequences.size() > 2048)
            sequences.pop_back();
    }
}
void AddLatencyToNetchan(INetChannel *ch, float Latency)
{
    if (Latency > 200.0f)
        Latency -= ch->GetLatency(MAX_FLOWS);
    for (auto &seq : sequences)
    {
        if (g_GlobalVars->realtime - seq.curtime > Latency / 1000.0f)
        {
            ch->m_nInReliableState = seq.inreliablestate;
            ch->m_nInSequenceNr    = seq.sequencenr;
            break;
        }
    }
}
bool installed = false;
int ticks      = 12;
void Init()
{
    for (int i = 0; i < 32; i++)
        for (int j = 0; j < 66; j++)
            headPositions[i][j] =
                BacktrackData{ 0,           { 0, 0, 0 }, { 0, 0, 0 },
                               { 0, 0, 0 }, { 0, 0, 0 }, 0,
                               0,           { 0, 0, 0 } };
    if (!installed)
    {
        latency.InstallChangeCallback(
            [](IConVar *var, const char *pszOldValue, float flOldValue) {
                ticks = max(min(int((float) latency) / 15, 65), 12);
            });
        installed = true;
    }
}

int BestTick    = 0;
int iBestTarget = -1;
void Run()
{
    if (!enable)
        return;

    if (CE_BAD(LOCAL_E))
        return;

    if (!shouldBacktrack())
    {
        shouldDrawBt = false;
        return;
    }
    shouldDrawBt = true;

    CUserCmd *cmd = g_pUserCmd;
    float bestFov = 99999;
    BestTick      = 0;
    iBestTarget   = -1;
    bool IsMelee  = GetWeaponMode() == weapon_melee;

    float prev_distance = 9999;

    for (int i = 1; i < g_IEngine->GetMaxClients(); i++)
    {
        CachedEntity *pEntity = ENTITY(i);

        if (CE_BAD(pEntity) || !pEntity->m_bAlivePlayer())
        {
            for (BacktrackData &btd : headPositions[i])
                btd = BacktrackData{ 0,           { 0, 0, 0 }, { 0, 0, 0 },
                                     { 0, 0, 0 }, { 0, 0, 0 }, 0,
                                     0,           { 0, 0, 0 } };
            continue;
        }
        if (pEntity->m_iTeam() == LOCAL_E->m_iTeam())
            continue;
        if (pEntity->m_Type() != ENTITY_PLAYER)
            continue;
        if (!pEntity->hitboxes.GetHitbox(0))
            continue;
        Vector hitboxpos = pEntity->hitboxes.GetHitbox(0)->center;
        Vector min       = pEntity->hitboxes.GetHitbox(0)->min;
        Vector max       = pEntity->hitboxes.GetHitbox(0)->max;
        float _viewangles =
            NET_VECTOR(RAW_ENT(pEntity), netvar.m_angEyeAngles).y;
        float viewangles =
            (_viewangles > 180) ? _viewangles - 360 : _viewangles;
        float simtime       = CE_FLOAT(pEntity, netvar.m_flSimulationTime);
        Vector hitbox_spine = pEntity->hitboxes.GetHitbox(3)->center;
        Vector ent_orig     = pEntity->InternalEntity()->GetAbsOrigin();
        auto hdr = g_IModelInfo->GetStudiomodel(RAW_ENT(pEntity)->GetModel());
        headPositions[i][cmd->command_number % ticks] =
            BacktrackData{ cmd->tick_count, hitboxpos,  min,     max,
                           hitbox_spine,    viewangles, simtime, ent_orig };
        float FOVDistance = GetFov(g_pLocalPlayer->v_OrigViewangles,
                                   g_pLocalPlayer->v_Eye, hitboxpos);
        float distance = g_pLocalPlayer->v_Eye.DistTo(hitbox_spine);
        if (!IsMelee && bestFov > FOVDistance && FOVDistance < 60.0f)
        {
            bestFov     = FOVDistance;
            iBestTarget = i;
        }
        if (IsMelee && distance < prev_distance)
        {
            prev_distance = distance;
            iBestTarget   = i;
        }
    }
    if (iBestTarget != -1 && CanShoot())
    {
        int bestTick  = 0;
        float tempFOV = 9999;
        float bestFOV = 180.0f;
        float distance, prev_distance_ticks = 9999;

        for (int i          = 0; i < ticks; ++i)
            sorted_ticks[i] = BestTickData{ INT_MAX, i };
        for (int t = 0; t < ticks; ++t)
        {
            if (headPositions[iBestTarget][t].tickcount)
                sorted_ticks[t] =
                    BestTickData{ headPositions[iBestTarget][t].tickcount, t };
        }
        std::sort(sorted_ticks, sorted_ticks + ticks);
        int tickus = (float(latency) > 800.0f || float(latency) < 200.0f) ? 12 : 24;
        for (int t = 0; t < ticks; ++t)
        {
            bool good_tick = false;

            for (int i = 0; i < tickus; ++i)
                if (t == sorted_ticks[i].tick &&
                    sorted_ticks[i].tickcount != INT_MAX &&
                    sorted_ticks[i].tickcount)
                    good_tick = true;
            if (!good_tick)
                continue;
            tempFOV =
                GetFov(g_pLocalPlayer->v_OrigViewangles, g_pLocalPlayer->v_Eye,
                       headPositions[iBestTarget][t].hitboxpos);
            if (IsMelee)
            {
                distance = g_pLocalPlayer->v_Eye.DistTo(
                    headPositions[iBestTarget][t].origin);
                if (distance < (float) mindistance)
                    continue;
                if (distance < prev_distance_ticks && tempFOV < 90.0f)
                    prev_distance_ticks = distance, bestTick = t;
            }
            else
            {
                if (bestFOV > tempFOV)
                    bestTick = t, bestFOV = tempFOV;
            }
        }

        BestTick = bestTick;
        if (cmd->buttons & IN_ATTACK)
        {
            CachedEntity *tar = ENTITY(iBestTarget);
            // ok just in case
            if (CE_BAD(tar))
                return;
            auto i          = headPositions[iBestTarget][bestTick];
            cmd->tick_count = i.tickcount;
            Vector &angles  = NET_VECTOR(RAW_ENT(tar), netvar.m_angEyeAngles);
            float &simtime = NET_FLOAT(RAW_ENT(tar), netvar.m_flSimulationTime);
            angles.y       = i.viewangles;
            simtime        = i.simtime;
        }
    }
}
void Draw()
{
#if ENABLE_VISUALS
    if (!enable)
        return;
    if (!draw_bt)
        return;
    if (!shouldDrawBt)
        return;
    int tickus = (float(latency) > 800.0f || float(latency) < 200.0f) ? 12 : 24;
    for (int i = 0; i < g_IEngine->GetMaxClients(); i++)
    {
        for (int j = 0; j < ticks; j++)
        {
            bool good_tick = false;

            for (int i = 0; i < tickus; ++i)
                if (j == sorted_ticks[i].tick)
                    good_tick = true;
            if (!good_tick)
                continue;
            auto hbpos    = headPositions[i][j].hitboxpos;
            auto tickount = headPositions[i][j].tickcount;
            auto min      = headPositions[i][j].min;
            auto max      = headPositions[i][j].max;
            if (!hbpos.x && !hbpos.y && !hbpos.z)
                continue;
            Vector out;
            if (draw::WorldToScreen(hbpos, out))
            {
                float size = 0.0f;
                if (abs(max.x - min.x) > abs(max.y - min.y))
                    size = abs(max.x - min.x);
                else
                    size = abs(max.y - min.y);

                if (i == iBestTarget && j == BestTick)
                    glez::draw::rect(out.x, out.y, size / 2, size / 2,
                                     colors::red);
                else
                    glez::draw::rect(out.x, out.y, size / 4, size / 4,
                                     colors::green);
            }
        }
    }
#endif
}

bool shouldBacktrack()
{
    int slot =
        re::C_BaseCombatWeapon::GetSlot(RAW_ENT(g_pLocalPlayer->weapon()));
    switch ((int) slots)
    {
    case 0:
        return true;
        break;
    case 1:
        if (slot == 0)
            return true;
        break;
    case 2:
        if (slot == 1)
            return true;
        break;
    case 3:
        if (slot == 2)
            return true;
        break;
    case 4:
        if (slot == 0 || slot == 1)
            return true;
        break;
    case 5:
        if (slot == 0 || slot == 2)
            return true;
        break;
    case 6:
        if (slot == 1 || slot == 2)
            return true;
        break;
    }
    return false;
}
}
