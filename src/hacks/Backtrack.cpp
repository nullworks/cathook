/*
 * Backtrack.cpp
 *
 *  Created on: May 15, 2018
 *      Author: bencat07
 */

#include "common.hpp"
#include "Backtrack.hpp"
#include <boost/circular_buffer.hpp>

namespace hacks
{
namespace shared
{
namespace backtrack
{
CatVar enable(CV_SWITCH, XORSTR("backtrack"), XORSTR("0"), XORSTR("Enable backtrack"),
              XORSTR("For legit play only as of right now."));
CatVar draw_bt(CV_SWITCH, XORSTR("backtrack_draw"), XORSTR("0"), XORSTR("Draw"),
               XORSTR("Draw backtrack ticks"));
CatVar latency(CV_FLOAT, XORSTR("backtrack_latency"), XORSTR("0"), XORSTR("fake lantency"),
               XORSTR("Set fake latency to this many ms"));
CatVar mindistance(CV_FLOAT, XORSTR("mindistance"), XORSTR("60"), XORSTR("mindistance"));

BacktrackData headPositions[32][66];
BestTickData sorted_ticks[66];
int highesttick[32]{};
int lastincomingsequencenumber = 0;

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
        for (int j              = 0; j < 66; j++)
            headPositions[i][j] = BacktrackData{
                0, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }
            };
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

    CUserCmd *cmd       = g_pUserCmd;
    float bestFov       = 99999;
    BestTick            = 0;
    iBestTarget         = -1;
    bool IsMelee        = GetWeaponMode() == weapon_melee;
    float prev_distance = 9999;

    for (int i = 1; i < g_IEngine->GetMaxClients(); i++)
    {
        CachedEntity *pEntity = ENTITY(i);

        if (CE_BAD(pEntity) || !pEntity->m_bAlivePlayer())
        {
            for (BacktrackData &btd : headPositions[i])
                btd = BacktrackData{ 0,           { 0, 0, 0 }, { 0, 0, 0 },
                                     { 0, 0, 0 }, { 0, 0, 0 }, 0 };
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
        Vector hitbox_spine = pEntity->hitboxes.GetHitbox(3)->center;
        headPositions[i][cmd->command_number % ticks] =
            BacktrackData{ cmd->tick_count, hitboxpos, min, max,
                           hitbox_spine,    viewangles };
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
        float bestFOV = 40.0f;
        float distance, prev_distance_ticks = 9999;

        for (int t = 0; t < ticks; ++t)
            sorted_ticks[t] =
                BestTickData{ headPositions[iBestTarget][t].tickcount, t };
        std::sort(sorted_ticks, sorted_ticks + ticks);
        for (int t = 0; t < ticks; ++t)
        {
            bool good_tick = false;
            for (int i = 0; i < 12; ++i)
                if (t == sorted_ticks[i].tick)
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
            cmd->tick_count = headPositions[iBestTarget][bestTick].tickcount;
    }
}
void Draw()
{
#if ENABLE_VISUALS
    if (!enable)
        return;
    if (!draw_bt)
        return;
    for (int i = 0; i < g_IEngine->GetMaxClients(); i++)
    {
        for (int j = 0; j < ticks; j++)
        {
            bool good_tick = false;
            for (int i = 0; i < 12; ++i)
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
                    draw_api::draw_rect(out.x, out.y, size / 2, size / 2,
                                        colors::red);
                else
                    draw_api::draw_rect(out.x, out.y, size / 4, size / 4,
                                        colors::green);
            }
        }
    }
#endif
}
}
}
}
