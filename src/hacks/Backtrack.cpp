/*
 * Backtrack.cpp
 *
 *  Created on: May 15, 2018
 *      Author: bencat07
 */

#include "common.hpp"
#include "hacks/Aimbot.hpp"
#include "hacks/Backtrack.hpp"
#include <boost/circular_buffer.hpp>
#include <glez/draw.hpp>
#include <settings/Bool.hpp>
#include <hacks/Backtrack.hpp>

static settings::Bool enable{ "backtrack.enable", "false" };
static settings::Bool draw_bt{ "backtrack.draw", "false" };
static settings::Int latency{ "backtrack.latency", "0" };
static settings::Float mindistance{ "backtrack.min-distance", "60" };
static settings::Int slots{ "backtrack.slots", "0" };

namespace hacks::shared::backtrack
{
void EmptyBacktrackData(BacktrackData &i);
std::pair<int, int> getBestEntBestTick();
bool shouldBacktrack();

BacktrackData headPositions[32][66]{};
int highesttick[32]{};
int lastincomingsequencenumber = 0;
bool isBacktrackEnabled        = false;

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
    if (!isBacktrackEnabled)
        return;
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
void Init()
{
    for (int i = 0; i < 32; i++)
        for (int j = 0; j < 66; j++)
            EmptyBacktrackData(headPositions[i][j]);
}

int BestTick    = 0;
int iBestTarget = -1;
bool istickvalid[32][66]{};
bool istickinvalid[32][66]{};
void Run()
{
    if (!shouldBacktrack())
    {
        isBacktrackEnabled = false;
        return;
    }
    isBacktrackEnabled = true;

    if (CE_BAD(LOCAL_E))
        return;

    for (auto &a : istickvalid)
        for (auto &b : a)
            b = false;
    for (auto &a : istickinvalid)
        for (auto &b : a)
            b = false;
    CUserCmd *cmd = current_user_cmd;
    float bestFov = 99999;

    float prev_distance = 9999;

    auto bestEntBestTick = getBestEntBestTick();
    BestTick             = bestEntBestTick.second;
    iBestTarget          = bestEntBestTick.first;

    for (int i = 1; i < g_IEngine->GetMaxClients(); i++)
    {
        CachedEntity *pEntity = ENTITY(i);

        if (CE_BAD(pEntity) || !pEntity->m_bAlivePlayer())
        {
            for (BacktrackData &btd : headPositions[i])
                EmptyBacktrackData(btd);
            continue;
        }
        if (pEntity->m_iTeam() == LOCAL_E->m_iTeam())
            continue;
        if (pEntity->m_Type() != ENTITY_PLAYER)
            continue;
        if (!pEntity->hitboxes.GetHitbox(0))
            continue;
        float _viewangles = CE_VECTOR(pEntity, netvar.m_angEyeAngles).y;
        float viewangles =
            (_viewangles > 180) ? _viewangles - 360 : _viewangles;
        float simtime = CE_FLOAT(pEntity, netvar.m_flSimulationTime);
        std::array<hitboxData, 18> hbdArray;
        for (size_t i = 0; i < hbdArray.max_size(); i++)
        {
            hbdArray.at(i).center = pEntity->hitboxes.GetHitbox(i)->center;
            hbdArray.at(i).min    = pEntity->hitboxes.GetHitbox(i)->min;
            hbdArray.at(i).max    = pEntity->hitboxes.GetHitbox(i)->max;
        }
        Vector ent_orig = pEntity->InternalEntity()->GetAbsOrigin();
        auto hdr = g_IModelInfo->GetStudiomodel(RAW_ENT(pEntity)->GetModel());
        headPositions[i][cmd->command_number % getTicks()] =
            BacktrackData{ cmd->tick_count, hbdArray,
                           viewangles,      simtime,
                           ent_orig,        cmd->command_number % getTicks() };
    }
    if (iBestTarget != -1 && CanShoot())
    {
        CachedEntity *tar = ENTITY(iBestTarget);
        if (CE_GOOD(tar))
        {
            if (cmd->buttons & IN_ATTACK)
            {
                // ok just in case
                if (CE_BAD(tar))
                    return;
                auto i          = headPositions[iBestTarget][BestTick];
                cmd->tick_count = i.tickcount;
                Vector &angles =
                    NET_VECTOR(RAW_ENT(tar), netvar.m_angEyeAngles);
                float &simtime =
                    NET_FLOAT(RAW_ENT(tar), netvar.m_flSimulationTime);
                angles.y = i.viewangles;
                simtime  = i.simtime;
            }
        }
    }
}
void Draw()
{
#if ENABLE_VISUALS
    if (!isBacktrackEnabled)
        return;
    if (!draw_bt)
        return;
    for (int i = 0; i < g_IEngine->GetMaxClients(); i++)
    {
        CachedEntity *ent = ENTITY(i);
        if (CE_BAD(ent))
            continue;
        for (int j = 0; j < getTicks(); j++)
        {
            if (!ValidTick(headPositions[i][j], ent))
                continue;
            auto hbpos = headPositions[i][j].hitboxes.at(head).center;
            auto min   = headPositions[i][j].hitboxes.at(head).min;
            auto max   = headPositions[i][j].hitboxes.at(head).max;
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

// Internal only, use isBacktrackEnabled var instead
bool shouldBacktrack()
{
    if (!*enable)
        return false;
    CachedEntity *wep = g_pLocalPlayer->weapon();
    if (CE_BAD(wep))
        return false;
    int slot = re::C_BaseCombatWeapon::GetSlot(RAW_ENT(wep));
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

float getLatency()
{
    return *latency;
}

int getTicks()
{
    return max(min(int(*latency / 200.0f * 13.0f) + 12, 65), 12);
}

bool ValidTick(BacktrackData &i, CachedEntity *ent)
{
    if (istickvalid[ent->m_IDX][i.index])
        return true;
    if (istickinvalid[ent->m_IDX][i.index])
        return false;
    if (hacks::shared::aimbot::IsBacktracking()) {
        if (IsVectorVisible(g_pLocalPlayer->v_Eye, i.hitboxes[head].center, true))
            if (fabsf(NET_FLOAT(RAW_ENT(ent), netvar.m_flSimulationTime) * 1000.0f -
                      getLatency() - i.simtime * 1000.0f) <= 200.0f) {
                istickvalid[ent->m_IDX][i.index] = true;
                return true;
            }
    }
    else if (fabsf(NET_FLOAT(RAW_ENT(ent), netvar.m_flSimulationTime) * 1000.0f -
              getLatency() - i.simtime * 1000.0f) <= 200.0f) {
        istickvalid[ent->m_IDX][i.index] = true;
        return true;
    }
    istickinvalid[ent->m_IDX][i.index] = true;
    return false;
}

void EmptyBacktrackData(BacktrackData &i)
{
    i = {};
}

// This func is internal only
std::pair<int, int> getBestEntBestTick()
{
    int bestEnt  = -1;
    int bestTick = -1;
    if (GetWeaponMode() == weapon_melee)
    {
        float bestDist = 9999.0f;
        for (int i = 0; i < g_IEngine->GetMaxClients(); i++)
        {
            CachedEntity *tar = ENTITY(i);
            if (CE_GOOD(tar))
            {
                if (tar != LOCAL_E && tar->m_bEnemy())
                {

                    for (int j = 0; j < getTicks(); j++)
                    {
                        if (ValidTick(headPositions[i][j], ENTITY(i)))
                        {
                            float dist =
                                headPositions[i][j]
                                    .hitboxes.at(spine_3)
                                    .center.DistTo(g_pLocalPlayer->v_Eye);
                            if (dist < bestDist && dist > *mindistance)
                            {
                                bestEnt  = i;
                                bestTick = j;
                                bestDist = dist;
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
        float bestFov = 180.0f;
        for (int i = 0; i < g_IEngine->GetMaxClients(); i++)
        {
            CachedEntity *tar = ENTITY(i);
            if (CE_GOOD(tar))
            {
                if (tar != LOCAL_E && tar->m_bEnemy())
                {
                    for (int j = 0; j < getTicks(); j++)
                    {
                        if (ValidTick(headPositions[i][j], tar))
                        {
                            float FOVDistance = GetFov(
                                g_pLocalPlayer->v_OrigViewangles,
                                g_pLocalPlayer->v_Eye,
                                headPositions[i][j].hitboxes.at(head).center);
                            if (bestFov > FOVDistance)
                            {
                                bestFov  = FOVDistance;
                                bestEnt  = i;
                                bestTick = j;
                            }
                        }
                    }
                }
            }
        }
    }
    return std::make_pair(bestEnt, bestTick);
}

} // namespace hacks::shared::backtrack
