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
#include <settings/Bool.hpp>
#include "PlayerTools.hpp"
#include <hacks/Backtrack.hpp>
#include "MiscTemporary.hpp"

namespace hacks::shared::backtrack
{
static settings::Boolean draw_bt{ "backtrack.draw", "false" };
static settings::Boolean draw_skeleton{ "backtrack.draw-skeleton", "false" };
static settings::Float mindistance{ "backtrack.min-distance", "60" };

static settings::Int slots{ "backtrack.slots", "0" };

settings::Boolean enable{ "backtrack.enable", "false" };
settings::Boolean backtrack_chams_glow{ "backtrack.chams_glow", "true" };
settings::Int latency{ "backtrack.latency", "0" };
settings::Boolean enable_latency_rampup{ "backtrack.latency.rampup", "true" };

void EmptyBacktrackData(BacktrackData &i);
std::pair<int, int> getBestEntBestTick();
bool shouldBacktrack();

BacktrackData headPositions[PLAYER_ARRAY_SIZE][66]{};
int lastincomingsequencenumber = 0;
bool isBacktrackEnabled        = false;
bool Vischeck_Success          = false;

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
            sequences.push_front(CIncomingSequence(instate, m_nInSequenceNr, g_GlobalVars->realtime));
        }
        if (sequences.size() > 2048)
            sequences.pop_back();
    }
}

void AddLatencyToNetchan(INetChannel *ch)
{
    if (!isBacktrackEnabled)
        return;
    for (auto &seq : sequences)
    {
        if (g_GlobalVars->realtime - seq.curtime > getLatency() / 1000.0f)
        {
            ch->m_nInReliableState = seq.inreliablestate;
            ch->m_nInSequenceNr    = seq.sequencenr;
            break;
        }
    }
}
void Init()
{
    for (int i = 0; i < PLAYER_ARRAY_SIZE; i++)
        for (int j = 0; j < 66; j++)
            headPositions[i][j] = {};

    BestTick = iBestTarget = -1;
}

int BestTick    = -1;
int iBestTarget = -1;
bool istickvalid[PLAYER_ARRAY_SIZE][66]{};
bool istickinvalid[PLAYER_ARRAY_SIZE][66]{};
static float latency_rampup = 0.0f;

static void Run()
{
    if (!shouldBacktrack())
    {
        isBacktrackEnabled = false;
        latency_rampup     = 0;
        return;
    }

    // Limit to 2 ticks
    latency_rampup += 1.0f / 66.0f;
    latency_rampup = std::min(latency_rampup, 1.0f);

    UpdateIncomingSequences();
    isBacktrackEnabled = true;

    if (CE_BAD(LOCAL_E) || !LOCAL_E->m_bAlivePlayer() || CE_BAD(LOCAL_W))
        return;
    if (g_Settings.bInvalid)
        return;
    if (!current_user_cmd)
        return;
    for (auto &a : istickvalid)
        for (auto &b : a)
            b = false;
    for (auto &a : istickinvalid)
        for (auto &b : a)
            b = false;
    CUserCmd *cmd = current_user_cmd;
    float bestFov = 99999;

    float prev_distance                 = 9999;
    std::pair<int, int> bestEntBestTick = getBestEntBestTick();
    BestTick                            = bestEntBestTick.second;
    iBestTarget                         = bestEntBestTick.first;
    // Fill backtrack data (stored in headPositions)
    {
        PROF_SECTION(cm_bt_ent_loop)
        for (int i = 1; i <= g_IEngine->GetMaxClients(); i++)
        {
            CachedEntity *pEntity = ENTITY(i);
            if (CE_BAD(pEntity) || !pEntity->m_bAlivePlayer())
            {
                for (BacktrackData &btd : headPositions[i])
                    btd.simtime = FLT_MAX;
                continue;
            }
            if (!pEntity->m_bEnemy())
                continue;
            if (pEntity->m_Type() != ENTITY_PLAYER)
                continue;
            if (!pEntity->hitboxes.GetHitbox(0))
                continue;
            if (HasCondition<TFCond_HalloweenGhostMode>(pEntity))
                continue;
            if (!*bSendPackets)
                headPositions[i][cmd->command_number % getTicks()] = {};
            else
            {
                auto &hbd         = headPositions[i][cmd->command_number % getTicks()];
                float _viewangles = CE_VECTOR(pEntity, netvar.m_angEyeAngles).y;
                hbd.viewangles    = (_viewangles > 180) ? _viewangles - 360 : _viewangles;
                hbd.simtime       = CE_FLOAT(pEntity, netvar.m_flSimulationTime);
                hbd.entorigin     = pEntity->InternalEntity()->GetAbsOrigin();
                hbd.tickcount     = cmd->tick_count;
                if (nolerp)
                {
                    static const ConVar *pUpdateRate = g_pCVar->FindVar("cl_updaterate");
                    if (!pUpdateRate)
                        pUpdateRate = g_pCVar->FindVar("cl_updaterate");
                    else
                    {

                        float interp = MAX(cl_interp->GetFloat(), cl_interp_ratio->GetFloat() / pUpdateRate->GetFloat());
                        hbd.tickcount += TIME_TO_TICKS(interp);
                        hbd.simtime += interp;
                    }
                }

                pEntity->hitboxes.InvalidateCache();
                for (size_t i = 0; i < 18; i++)
                {
                    hbd.hitboxes[i].center = pEntity->hitboxes.GetHitbox(i)->center;
                    hbd.hitboxes[i].min    = pEntity->hitboxes.GetHitbox(i)->min;
                    hbd.hitboxes[i].max    = pEntity->hitboxes.GetHitbox(i)->max;
                }
                hbd.collidable.min    = RAW_ENT(pEntity)->GetCollideable()->OBBMins() + hbd.entorigin;
                hbd.collidable.max    = RAW_ENT(pEntity)->GetCollideable()->OBBMaxs() + hbd.entorigin;
                hbd.collidable.center = (hbd.collidable.min + hbd.collidable.max) / 2;
                memcpy((void *) hbd.bones, (void *) pEntity->hitboxes.bones, sizeof(matrix3x4_t) * 128);
            }
        }
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
                Vector &angles  = NET_VECTOR(RAW_ENT(tar), netvar.m_angEyeAngles);
                float &simtime  = NET_FLOAT(RAW_ENT(tar), netvar.m_flSimulationTime);
                angles.y        = i.viewangles;
                simtime         = i.simtime;
            }
        }
    }
}
CatCommand print_bones("debug_print_bones", "debug print bone id + name", []() {
    if (CE_BAD(LOCAL_E) || !LOCAL_E->m_bAlivePlayer())
        return;
    // Get player model
    const model_t *model = RAW_ENT(LOCAL_E)->GetModel();
    if (not model)
        return;
    // Get Studio models (for bones)
    studiohdr_t *hdr = g_IModelInfo->GetStudiomodel(model);
    if (not hdr)
        return;
    // Get the name of the bones
    for (int i = 0; i < hdr->numbones; i++)
        logging::Info(format(std::string(hdr->pBone(i)->pszName()), " ", i).c_str());
});
static std::vector<int> bones_leg_r  = { 17, 16, 15 };
static std::vector<int> bones_leg_l  = { 14, 13, 12 };
static std::vector<int> bones_bottom = { 15, 1, 12 };
static std::vector<int> bones_spine  = { 1, 2, 3, 4, 5, 0 };
static std::vector<int> bones_arm_r  = { 9, 10, 11 };
static std::vector<int> bones_arm_l  = { 6, 7, 8 };
static std::vector<int> bones_up     = { 9, 5, 6 };

#if ENABLE_VISUALS
void DrawBone(std::vector<int> hitbox, std::array<hitboxData, 18> hitboxes)
{
    for (int i = 0; i < hitbox.size() - 1; i++)
    {
        Vector bone1 = hitboxes.at(hitbox.at(i)).center;
        Vector bone2 = hitboxes.at(hitbox.at(i + 1)).center;
        Vector draw_position1, draw_position2;
        if (draw::WorldToScreen(bone1, draw_position1) && draw::WorldToScreen(bone2, draw_position2))
            draw::Line(draw_position1.x, draw_position1.y, draw_position2.x - draw_position1.x, draw_position2.y - draw_position1.y, colors::white, 1.0f);
    }
}
#endif
static void Draw()
{
#if ENABLE_VISUALS
    if (!isBacktrackEnabled)
        return;
    // :b:ones for non drawable ents
    if (draw_skeleton)
        for (int i = 0; i <= g_IEngine->GetMaxClients(); i++)
        {
            CachedEntity *ent = ENTITY(i);
            if (CE_BAD(ent) || !ent->m_bAlivePlayer() || i == g_IEngine->GetLocalPlayer())
                continue;
            auto head_pos = headPositions[i];
            // Usable vector instead of ptr to c style array, also used to filter valid and invalid ticks
            std::vector<BacktrackData> usable;
            for (int i = 0; i < 66; i++)
            {
                if (ValidTick(head_pos[i], ent))
                    usable.push_back(head_pos[i]);
            }
            // Crash much?
            if (usable.size())
            {
                DrawBone(bones_leg_l, usable[0].hitboxes);
                DrawBone(bones_leg_r, usable[0].hitboxes);
                DrawBone(bones_bottom, usable[0].hitboxes);
                DrawBone(bones_spine, usable[0].hitboxes);
                DrawBone(bones_arm_l, usable[0].hitboxes);
                DrawBone(bones_arm_r, usable[0].hitboxes);
                DrawBone(bones_up, usable[0].hitboxes);
            }
        }
    if (!draw_bt)
        return;
    for (int i = 0; i <= g_IEngine->GetMaxClients(); i++)
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
                    draw::Rectangle(out.x, out.y, size / 2, size / 2, colors::red);
                else
                    draw::Rectangle(out.x, out.y, size / 4, size / 4, colors::green);
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
    if (*slots == 0)
        return true;
    int slot = re::C_BaseCombatWeapon::GetSlot(RAW_ENT(wep));
    switch ((int) slots)
    {
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
float getRealLatency()
{
    auto ch = (INetChannel *) g_IEngine->GetNetChannelInfo();
    if (!ch)
        return 0.0f;
    float Latency             = ch->GetLatency(FLOW_OUTGOING);
    static auto cl_updaterate = g_ICvar->FindVar("cl_updaterate");
    if (cl_updaterate && cl_updaterate->GetFloat() > 0.001f)
    {
        Latency += -0.5f / cl_updaterate->GetFloat();
        if (nolerp)
        {
            float interp = MAX(cl_interp->GetFloat(), cl_interp_ratio->GetFloat() / cl_updaterate->GetFloat());
            Latency += interp;
        }
    }
    else if (!cl_updaterate)
        cl_updaterate = g_ICvar->FindVar("cl_updaterate");
    return MAX(0.0f, Latency) * 1000.f;
}

float getLatency()
{
    auto ch = (INetChannel *) g_IEngine->GetNetChannelInfo();
    if (!ch)
        return 0;
    float Latency = *latency;
    Latency       = std::min(Latency, 800.0f);
    Latency -= getRealLatency();
    Latency = std::max(Latency, 0.0f);
    if (enable_latency_rampup)
        Latency = Latency * latency_rampup;
    Latency = ROUND_TO_TICKS(Latency / 1000.0f) * 1000.0f;
    return Latency;
}

int getTicks()
{
    // Get Latency in seconds
    float latency = getLatency() / 1000.0f;
    // Latency in ticks
    int ticks = TIME_TO_TICKS(latency);
    // We can backtrack 200ms into the future of the latency
    ticks += TIME_TO_TICKS(0.2f);
    // Clamp between 0.2s and 1s in ticks (12-66)
    return clamp(ticks, TIME_TO_TICKS(0.2f), TIME_TO_TICKS(1.0f));
}

bool ValidTick(BacktrackData &i, CachedEntity *ent)
{
    if (!(fabsf(NET_FLOAT(RAW_ENT(ent), netvar.m_flSimulationTime) * 1000.0f - getLatency() - i.simtime * 1000.0f) < ROUND_TO_TICKS(200.0f)))
        return false;
    return true;
}

void EmptyBacktrackData(BacktrackData &i)
{
    i = {};
}

// This func is internal only
std::pair<int, int> getBestEntBestTick()
{
    int bestEnt            = -1;
    int bestTick           = -1;
    bool vischeck_priority = false;
    Vischeck_Success       = false;
    if (GetWeaponMode() == weapon_melee)
    {
        float bestDist = 9999.0f;
        for (int i = 0; i <= g_IEngine->GetMaxClients(); i++)
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
                            float dist = headPositions[i][j].hitboxes.at(spine_3).center.DistTo(g_pLocalPlayer->v_Eye);
                            if (dist < bestDist)
                            {
                                bestEnt           = i;
                                bestTick          = j;
                                bestDist          = dist;
                                Vischeck_Success  = true;
                                vischeck_priority = true;
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
        for (int i = 0; i <= g_IEngine->GetMaxClients(); i++)
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
                            float FOVDistance = GetFov(g_pLocalPlayer->v_OrigViewangles, g_pLocalPlayer->v_Eye, headPositions[i][j].hitboxes.at(head).center);
                            if (FOVDistance > bestFov && vischeck_priority)
                                continue;
                            bool Vischeck_suceeded = IsVectorVisible(g_pLocalPlayer->v_Eye, headPositions[i][j].hitboxes.at(0).center, true);
                            if (FOVDistance < bestFov || (Vischeck_suceeded && !vischeck_priority))
                            {
                                bestEnt  = i;
                                bestTick = j;
                                bestFov  = FOVDistance;
                                if (Vischeck_suceeded)
                                {
                                    Vischeck_Success  = true;
                                    vischeck_priority = true;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return std::make_pair(bestEnt, bestTick);
}
static InitRoutine EC([]() {
    EC::Register(EC::LevelInit, Init, "INIT_Backtrack", EC::average);
    EC::Register(EC::CreateMove, Run, "CM_Backtrack", EC::early);
#if ENABLE_VISUALS
    EC::Register(EC::Draw, Draw, "DRAW_Backtrack", EC::average);
#endif
});
static CatCommand debug_flowout("debug_flowout", "test", []() {
    auto ch = (INetChannel *) g_IEngine->GetNetChannelInfo();
    logging::Info("Out Avg: %f In Avg: %f Out current: %f In Current: %f", 1000.0f * ch->GetAvgLatency(FLOW_OUTGOING), 1000.0f * ch->GetAvgLatency(FLOW_INCOMING), 1000.0f * ch->GetLatency(FLOW_OUTGOING), 1000.0f * ch->GetLatency(FLOW_INCOMING));
});
static CatCommand debug_richpresence("debug_presence", "Debug stuff", []() {
    g_ISteamFriends->SetRichPresence("steam_display", "#TF_RichPresence_State_PlayingGeneric");
    g_ISteamFriends->SetRichPresence("currentmap", "Cathooking");
});
} // namespace hacks::shared::backtrack
