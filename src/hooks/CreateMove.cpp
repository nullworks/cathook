/*
 * CreateMove.cpp
 *
 *  Created on: Jan 8, 2017
 *      Author: nullifiedcat
 */

#include "common.hpp"
#include "hack.hpp"
#include "MiscTemporary.hpp"
#include <link.h>
#include <hacks/hacklist.hpp>

#include "HookedMethods.hpp"

class CMoveData;
#if LAGBOT_MODE
CatCommand set_value("set", "Set value", [](const CCommand &args) {
    if (args.ArgC() < 2)
        return;
    ConVar *var = g_ICvar->FindVar(args.Arg(1));
    if (!var)
        return;
    std::string value(args.Arg(2));
    ReplaceString(value, "\\n", "\n");
    var->SetValue(value.c_str());
    logging::Info("Set '%s' to '%s'", args.Arg(1), value.c_str());
});
#endif
namespace engine_prediction
{

void RunEnginePrediction(IClientEntity *ent, CUserCmd *ucmd)
{
    if (!ent)
        return;

    typedef void (*SetupMoveFn)(IPrediction *, IClientEntity *, CUserCmd *,
                                class IMoveHelper *, CMoveData *);
    typedef void (*FinishMoveFn)(IPrediction *, IClientEntity *, CUserCmd *,
                                 CMoveData *);

    void **predictionVtable = *((void ***) g_IPrediction);
    SetupMoveFn oSetupMove =
        (SetupMoveFn)(*(unsigned *) (predictionVtable + 19));
    FinishMoveFn oFinishMove =
        (FinishMoveFn)(*(unsigned *) (predictionVtable + 20));

    // CMoveData *pMoveData = (CMoveData*)(sharedobj::client->lmap->l_addr +
    // 0x1F69C0C);  CMoveData movedata {};
    char object[165];
    CMoveData *pMoveData = (CMoveData *) object;

    float frameTime = g_GlobalVars->frametime;
    float curTime   = g_GlobalVars->curtime;
    CUserCmd defaultCmd;
    if (ucmd == NULL)
    {
        ucmd = &defaultCmd;
    }

    NET_VAR(ent, 4188, CUserCmd *) = ucmd;

    g_GlobalVars->curtime =
        g_GlobalVars->interval_per_tick * NET_INT(ent, netvar.nTickBase);
    g_GlobalVars->frametime = g_GlobalVars->interval_per_tick;

    *g_PredictionRandomSeed =
        MD5_PseudoRandom(g_pUserCmd->command_number) & 0x7FFFFFFF;
    g_IGameMovement->StartTrackPredictionErrors(
        reinterpret_cast<CBasePlayer *>(ent));
    oSetupMove(g_IPrediction, ent, ucmd, NULL, pMoveData);
    g_IGameMovement->ProcessMovement(reinterpret_cast<CBasePlayer *>(ent),
                                     pMoveData);
    oFinishMove(g_IPrediction, ent, ucmd, pMoveData);
    g_IGameMovement->FinishTrackPredictionErrors(
        reinterpret_cast<CBasePlayer *>(ent));

    NET_VAR(ent, 4188, CUserCmd *) = nullptr;

    g_GlobalVars->frametime = frameTime;
    g_GlobalVars->curtime   = curTime;

    return;
}
}
#if not LAGBOT_MODE
#define antikick_time 35
#else
#define antikick_time 90
#endif
const char *cmds[7] = { "use",         "voicecommand", "spec_next", "spec_prev",
                        "spec_player", "invprev",      "invnext" };
namespace hooked_methods
{

DEFINE_HOOKED_METHOD(CreateMove, bool, void *this_, float input_sample_time,
                     CUserCmd *cmd)
{
#define TICK_INTERVAL (g_GlobalVars->interval_per_tick)
#define TIME_TO_TICKS(dt) ((int) (0.5f + (float) (dt) / TICK_INTERVAL))
#define TICKS_TO_TIME(t) (TICK_INTERVAL * (t))
#define ROUND_TO_TICKS(t) (TICK_INTERVAL * TIME_TO_TICKS(t))
    uintptr_t **fp;
    __asm__("mov %%ebp, %0" : "=r"(fp));
    bSendPackets = reinterpret_cast<bool *>(**fp - 8);

    g_Settings.is_create_move = true;
    bool time_replaced, ret, speedapplied;
    float curtime_old, servertime, speed, yaw;
    Vector vsilent, ang;

    if (firstcm)
    {
        DelayTimer.update();
        firstcm = false;
    }
    tickcount++;
    g_pUserCmd = cmd;
#if not LAGBOT_MODE
    IF_GAME(IsTF2C())
    {
        if (CE_GOOD(LOCAL_W) && minigun_jump &&
            LOCAL_W->m_iClassID() == CL_CLASS(CTFMinigun))
        {
            CE_INT(LOCAL_W, netvar.iWeaponState) = 0;
        }
    }
#endif
    ret = original::CreateMove(this_, input_sample_time, cmd);

    PROF_SECTION(CreateMove);

    if (!cmd)
    {
        g_Settings.is_create_move = false;
        return ret;
    }

    if (!cathook)
    {
        g_Settings.is_create_move = false;
        return ret;
    }

    if (!g_IEngine->IsInGame())
    {
        g_Settings.bInvalid       = true;
        g_Settings.is_create_move = false;
        return true;
    }

    //	PROF_BEGIN();

    if (g_pUserCmd && g_pUserCmd->command_number)
        last_cmd_number = g_pUserCmd->command_number;

    /**bSendPackets = true;
    if (hacks::shared::lagexploit::ExploitActive()) {
        *bSendPackets = ((g_pUserCmd->command_number % 4) == 0);
        //logging::Info("%d", *bSendPackets);
    }*/

    // logging::Info("canpacket: %i", ch->CanPacket());
    // if (!cmd) return ret;

    time_replaced = false;
    curtime_old   = g_GlobalVars->curtime;

    hacks::tf2::global::runcfg();
    static IClientEntity *enti;
    if (resolver && cathook && CE_GOOD(LOCAL_E))
    {
        for (int i = 0; i < g_IEngine->GetMaxClients(); i++)
        {
            if (i == g_IEngine->GetLocalPlayer())
                continue;
            enti = g_IEntityList->GetClientEntity(i);
            if (enti && !enti->IsDormant() &&
                !NET_BYTE(enti, netvar.iLifeState))
            {
                float quotat = 0;
                float quotaf = 0;
                if (!g_Settings.brute.choke[i].empty())
                    for (auto it : g_Settings.brute.choke[i])
                    {
                        if (it)
                            quotat++;
                        else
                            quotaf++;
                    }
                float quota    = quotat / quotaf;
                Vector &angles = NET_VECTOR(enti, netvar.m_angEyeAngles);
                static bool brutepitch = false;
                if (g_Settings.brute.brutenum[i] > 5)
                {
                    g_Settings.brute.brutenum[i] = 0;
                    brutepitch                   = !brutepitch;
                }
                angles.y = fmod(angles.y + 180.0f, 360.0f);
                if (angles.y < 0)
                    angles.y += 360.0f;
                angles.y -= 180.0f;
                if (quota < 0.8f)
                    switch (g_Settings.brute.brutenum[i])
                    {
                    case 0:
                        break;
                    case 1:
                        angles.y += 180.0f;
                        break;
                    case 2:
                        angles.y -= 90.0f;
                        break;
                    case 3:
                        angles.y += 90.0f;
                        break;
                    case 4:
                        angles.y -= 180.0f;
                        break;
                    case 5:
                        angles.y = 0.0f;
                        break;
                    }
                if (brutepitch || quota < 0.8f)
                    switch (g_Settings.brute.brutenum[i] % 4)
                    {
                    case 0:
                        break;
                    case 1:
                        angles.x = -89.0f;
                        break;
                    case 2:
                        angles.x = 89.0f;
                        break;
                    case 3:
                        angles.x = 0.0f;
                        break;
                    }
            }
        }
    }
    if (nolerp)
    {
        // g_pUserCmd->tick_count += 1;
        if (sv_client_min_interp_ratio->GetInt() != -1)
        {
            // sv_client_min_interp_ratio->m_nFlags = 0;
            sv_client_min_interp_ratio->SetValue(-1);
        }
        if (cl_interp->m_fValue != 0)
        {
            cl_interp->SetValue(0);
            cl_interp->m_fValue = 0.0f;
            cl_interp->m_nValue = 0;
        }
        if (cl_interp_ratio->GetInt() != 0)
            cl_interp_ratio->SetValue(0);
        // if (cl_interpolate->GetInt() != 0) cl_interpolate->SetValue(0);
    }

    if (!g_Settings.bInvalid && CE_GOOD(g_pLocalPlayer->entity))
    {
        servertime = (float) CE_INT(g_pLocalPlayer->entity, netvar.nTickBase) *
                     g_GlobalVars->interval_per_tick;
        g_GlobalVars->curtime = servertime;
        time_replaced         = true;
    }
    if (g_Settings.bInvalid)
    {
        entity_cache::Invalidate();
    }

    // Disabled because this causes EXTREME aimbot inaccuracy
    // if (!cmd->command_number) return ret;
    //	PROF_BEGIN();
    {
        PROF_SECTION(EntityCache);
        entity_cache::Update();
    }
    //	PROF_END("Entity Cache updating");
    {
        PROF_SECTION(CM_PlayerResource);
        g_pPlayerResource->Update();
    }
    {
        PROF_SECTION(CM_LocalPlayer);
        g_pLocalPlayer->Update();
    }
    g_Settings.bInvalid = false;

    hacks::shared::autojoin::Update();

#if ENABLE_IPC
#if not LAGBOT_MODE
    static int team_joining_state  = 0;
    static float last_jointeam_try = 0;
    CachedEntity *found_entity, *ent;

    if (hacks::shared::followbot::followbot)
    {
        hacks::shared::followbot::WorldTick();
        if (g_GlobalVars->curtime < last_jointeam_try)
        {
            team_joining_state = 0;
            last_jointeam_try  = 0.0f;
        }

        if (!g_pLocalPlayer->team || (g_pLocalPlayer->team == TEAM_SPEC))
        {
            // if (!team_joining_state) logging::Info("Bad team, trying to
            // join...");
            team_joining_state = 1;
        }
        else
        {
            if (team_joining_state)
            {
                logging::Info("Trying to change CLASS");
                g_IEngine->ExecuteClientCmd(
                    format("join_class ", joinclass.GetString()).c_str());
            }
            team_joining_state = 0;
        }

        if (team_joining_state)
        {
            found_entity = nullptr;
            for (int i = 1; i < 32 && i < HIGHEST_ENTITY; i++)
            {
                ent = ENTITY(i);
                if (CE_BAD(ent))
                    continue;
                if (ent->player_info.friendsID ==
                    hacks::shared::followbot::steamid)
                {
                    found_entity = ent;
                    break;
                }
            }

            if (found_entity && CE_GOOD(found_entity))
            {
                if (jointeam &&
                    (g_GlobalVars->curtime - last_jointeam_try) > 1.0f)
                {
                    last_jointeam_try = g_GlobalVars->curtime;
                    switch (CE_INT(found_entity, netvar.iTeamNum))
                    {
                    case TEAM_RED:
                        logging::Info("Trying to join team RED");
                        g_IEngine->ExecuteClientCmd("jointeam red");
                        break;
                    case TEAM_BLU:
                        logging::Info("Trying to join team BLUE");
                        g_IEngine->ExecuteClientCmd("jointeam blue");
                        break;
                    }
                }
            }
        }
    }
#endif
#endif
    if (CE_GOOD(g_pLocalPlayer->entity))
    {
#if not LAGBOT_MODE
        IF_GAME(IsTF2())
        {
            UpdateHoovyList();
        }
        g_pLocalPlayer->v_OrigViewangles = cmd->viewangles;
#if ENABLE_VISUALS
        {
            PROF_SECTION(CM_esp);
            hacks::shared::esp::CreateMove();
        }
        {
            PROF_SECTION(CM_lightesp);
            hacks::shared::lightesp::run();
        }
#endif
#endif
        if (!g_pLocalPlayer->life_state && CE_GOOD(g_pLocalPlayer->weapon()))
        {
#if not LAGBOT_MODE
            {
                PROF_SECTION(CM_walkbot);
                hacks::shared::walkbot::Move();
            }
            // Walkbot can leave game.
            if (!g_IEngine->IsInGame())
            {
                g_Settings.is_create_move = false;
                return ret;
            }
            IF_GAME(IsTF())
            {
                PROF_SECTION(CM_uberspam);
                hacks::tf::uberspam::CreateMove();
            }
            IF_GAME(IsTF2())
            {
                PROF_SECTION(CM_noisemaker);
                hacks::tf2::noisemaker::CreateMove();
            }
            {
                PROF_SECTION(CM_deadringer);
                hacks::shared::deadringer::CreateMove();
            }
            {
                PROF_SECTION(CM_bunnyhop);
                hacks::shared::bunnyhop::CreateMove();
            }
            if (engine_pred)
                engine_prediction::RunEnginePrediction(RAW_ENT(LOCAL_E),
                                                       g_pUserCmd);
            {
                PROF_SECTION(CM_backtracc);
                hacks::shared::backtrack::Run();
            }
            {
                PROF_SECTION(CM_aimbot);
                hacks::shared::aimbot::CreateMove();
            }
            static int attackticks = 0;
            if (g_pUserCmd->buttons & IN_ATTACK)
                ++attackticks;
            else
                attackticks = 0;
            if (semiauto)
                if (g_pUserCmd->buttons & IN_ATTACK)
                    if (attackticks % int(semiauto) < int(semiauto) - 1)
                        g_pUserCmd->buttons &= ~IN_ATTACK;
            static int fakelag_queue = 0;
            if (CE_GOOD(LOCAL_E))
                if (fakelag_amount)
                {
                    *bSendPackets = int(fakelag_amount) == fakelag_queue;
                    fakelag_queue++;
                    if (fakelag_queue > int(fakelag_amount))
                        fakelag_queue = 0;
                }
            {
                PROF_SECTION(CM_antiaim);
                hacks::shared::antiaim::ProcessUserCmd(cmd);
            }
            IF_GAME(IsTF())
            {
                PROF_SECTION(CM_autosticky);
                hacks::tf::autosticky::CreateMove();
            }
            IF_GAME(IsTF())
            {
                PROF_SECTION(CM_autodetonator);
                hacks::tf::autodetonator::CreateMove();
            }
            IF_GAME(IsTF())
            {
                PROF_SECTION(CM_autoreflect);
                hacks::tf::autoreflect::CreateMove();
            }
            {
                PROF_SECTION(CM_triggerbot);
                hacks::shared::triggerbot::CreateMove();
            }
            IF_GAME(IsTF())
            {
                PROF_SECTION(CM_autoheal);
                hacks::tf::autoheal::CreateMove();
            }
            IF_GAME(IsTF2())
            {
                PROF_SECTION(CM_antibackstab);
                hacks::tf2::antibackstab::CreateMove();
            }
            IF_GAME(IsTF2())
            {
                PROF_SECTION(CM_autobackstab);
                hacks::tf2::autobackstab::CreateMove();
            }
            IF_GAME(IsTF2())
            {
                PROF_SECTION(CM_autodeadringer);
                hacks::shared::deadringer::CreateMove();
            }
            if (debug_projectiles)
                projectile_logging::Update();
            Prediction_CreateMove();
#endif
        }
#if not LAGBOT_MODE
        {
            PROF_SECTION(CM_misc);
            hacks::shared::misc::CreateMove();
        }
        {
            PROF_SECTION(CM_crits);
            criticals::create_move();
        }
#endif
        {
            PROF_SECTION(CM_spam);
            hacks::shared::spam::CreateMove();
        }
#if not LAGBOT_MODE
        {
            PROF_SECTION(CM_AC);
            hacks::shared::anticheat::CreateMove();
        }
#endif
    }
    if (time_replaced)
        g_GlobalVars->curtime = curtime_old;
    g_Settings.bInvalid       = false;
#if not LAGBOT_MODE
    {
        PROF_SECTION(CM_chat_stack);
        chat_stack::OnCreateMove();
    }
    {
        PROF_SECTION(CM_healarrow);
        hacks::tf2::healarrow::CreateMove();
    }
    {
        PROF_SECTION(CM_lagexploit);
        hacks::shared::lagexploit::CreateMove();
    }
#endif

// TODO Auto Steam Friend

#if ENABLE_IPC
    {
        PROF_SECTION(CM_playerlist);
        static Timer ipc_update_timer{};
        //	playerlist::DoNotKillMe();
        if (ipc_update_timer.test_and_set(1000 * 10))
        {
            ipc::UpdatePlayerlist();
        }
    }
#endif
#if not LAGBOT_MODE
    hacks::shared::backtrack::UpdateIncomingSequences();
    if (CE_GOOD(g_pLocalPlayer->entity))
    {
        speedapplied = false;
        if (roll_speedhack &&
            g_IInputSystem->IsButtonDown(
                (ButtonCode_t)((int) roll_speedhack)) &&
            !(cmd->buttons & IN_ATTACK))
        {
            speed = cmd->forwardmove;
            if (fabs(speed) > 0.0f)
            {
                cmd->forwardmove  = -speed;
                cmd->sidemove     = 0.0f;
                cmd->viewangles.y = g_pLocalPlayer->v_OrigViewangles.y;
                cmd->viewangles.y -= 180.0f;
                if (cmd->viewangles.y < -180.0f)
                    cmd->viewangles.y += 360.0f;
                cmd->viewangles.z                = 90.0f;
                g_pLocalPlayer->bUseSilentAngles = true;
                speedapplied                     = true;
            }
        }

        if (g_pLocalPlayer->bUseSilentAngles)
        {
            if (!speedapplied)
            {
                vsilent.x = cmd->forwardmove;
                vsilent.y = cmd->sidemove;
                vsilent.z = cmd->upmove;
                speed     = sqrt(vsilent.x * vsilent.x + vsilent.y * vsilent.y);
                VectorAngles(vsilent, ang);
                yaw = DEG2RAD(ang.y - g_pLocalPlayer->v_OrigViewangles.y +
                              cmd->viewangles.y);
                cmd->forwardmove = cos(yaw) * speed;
                cmd->sidemove    = sin(yaw) * speed;
                if (cmd->viewangles.x >= 90 && cmd->viewangles.x <= 270)
                    cmd->forwardmove = -cmd->forwardmove;
            }

            ret = false;
        }
        if (cmd &&
            (cmd->buttons & IN_ATTACK ||
             !(hacks::shared::antiaim::enabled &&
               float(hacks::shared::antiaim::yaw_mode) >= 9 && !*bSendPackets)))
            g_Settings.brute.last_angles[LOCAL_E->m_IDX] = cmd->viewangles;
        for (int i = 0; i < g_IEngine->GetMaxClients(); i++)
        {

            CachedEntity *ent = ENTITY(i);
            if (CE_GOOD(LOCAL_E))
                if (ent == LOCAL_E)
                    continue;
            if (CE_BAD(ent) || !ent->m_bAlivePlayer())
                continue;
            INetChannel *ch = (INetChannel *) g_IEngine->GetNetChannelInfo();
            if (NET_FLOAT(RAW_ENT(ent), netvar.m_flSimulationTime) <= 1.5f)
                continue;
            float latency = ch->GetAvgLatency(MAX_FLOWS);
            g_Settings.brute.choke[i].push_back(
                NET_FLOAT(RAW_ENT(ent), netvar.m_flSimulationTime) ==
                g_Settings.brute.lastsimtime);
            g_Settings.brute.last_angles[ent->m_IDX] =
                NET_VECTOR(RAW_ENT(ent), netvar.m_angEyeAngles);
            if (!g_Settings.brute.choke[i].empty() &&
                g_Settings.brute.choke[i].size() > 20)
                g_Settings.brute.choke[i].pop_front();
        }
    }
#endif
    int nextdata = 0;
    NET_StringCmd senddata(cmds[nextdata]);
    INetChannel *ch = (INetChannel *) g_IEngine->GetNetChannelInfo();

    senddata.SetNetChannel(ch);
    senddata.SetReliable(false);
    if (servercrash && DelayTimer.check((int) delay * 1000))
    {
        for (int i = 0; i < 7800; i += sizeof(cmds[nextdata]))
        {
            senddata.m_szCommand = cmds[nextdata];
            ch->SendNetMsg(senddata);
            if (nextdata == 6)
                nextdata = 0;
            else
                nextdata++;
        }
        ch->Transmit();
    }
    if (serverlag_amount || (votelogger::active &&
                             !votelogger::antikick.check(antikick_time * 1000)))
    {
        if (adjust && !votelogger::active)
        {
            if ((int) serverlag_amount == 1)
                serverlag_amount = (int) serverlag_amount + 10;
            if (ch->GetAvgData(FLOW_INCOMING) == prevflow)
            {
                if (prevflowticks > 66 * (int) adjust)
                    serverlag_amount = (int) serverlag_amount - 1;
                prevflowticks++;
            }
            if (ch->GetAvgData(FLOW_INCOMING) != prevflow)
            {
                if (prevflowticks < 66 * (int) adjust)
                    serverlag_amount = (int) serverlag_amount + 4;
                prevflowticks        = 0;
            }
        }
        if (votelogger::active &&
            !votelogger::antikick.check(antikick_time * 1000))
        {
            static int additionallag = 1;
            if (ch->GetAvgData(FLOW_INCOMING) == prevflow)
            {
                prevflowticks++;
            }
            else
                prevflowticks = 0;
            if (prevflowticks <= 10)
                additionallag *= 0.1f;
            for (int i = 0; i < 7800 + additionallag;
                 i += sizeof(cmds[nextdata]))
            {
                senddata.m_szCommand = cmds[nextdata];
                ch->SendNetMsg(senddata, false);
                if (nextdata == 6)
                    nextdata = 0;
                else
                    nextdata++;
            }
            ch->Transmit();
        }
        else if (!votelogger::active && serverlag_amount &&
                 DelayTimer.check((int) delay * 1000))
        {
            for (int i = 0; i < (int) serverlag_amount; i++)
            {
                senddata.m_szCommand = cmds[nextdata];
                if (nextdata == 6)
                    nextdata = 0;
                else
                    nextdata++;
                ch->SendNetMsg(senddata, false);
            }
            ch->Transmit();
        }
        prevflow = ch->GetAvgData(FLOW_INCOMING);
    }
    else if (votelogger::active &&
             votelogger::antikick.test_and_set(antikick_time * 1000))
        votelogger::active = false;
    //	PROF_END("CreateMove");
    if (!(cmd->buttons & IN_ATTACK))
    {
        // LoadSavedState();
    }
    g_pLocalPlayer->bAttackLastTick = (cmd->buttons & IN_ATTACK);
    g_Settings.is_create_move       = false;
    return ret;
}
}

/*float o_curtime;
float o_frametime;

void Start() {
    g_IGameMovement->StartTrackPredictionErrors((CBasePlayer*)(RAW_ENT(LOCAL_E)));

    IClientEntity* player = RAW_ENT(LOCAL_E);
    // CPredictableId::ResetInstanceCounters();
    *(reinterpret_cast<CUserCmd*>(reinterpret_cast<uintptr_t>(player) + 1047)) =
g_pUserCmd; o_curtime = g_GlobalVars->curtime; o_frametime =
g_GlobalVars->frametime; *g_PredictionRandomSeed =
MD5_PseudoRandom(g_pUserCmd->command_number) & 0x7FFFFFFF; g_GlobalVars->curtime
= CE_INT(LOCAL_E, netvar.nTickBase) * g_GlobalVars->interval_per_tick;
    g_GlobalVars->frametime = g_GlobalVars->interval_per_tick;

    CMoveData data;

}

void End() {
    *g_PredictionRandomSeed = -1;
    g_GlobalVars->curtime = o_curtime;
    g_GlobalVars->frametime = o_frametime;
}*/
