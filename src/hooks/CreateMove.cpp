/*
 * CreateMove.cpp
 *
 *  Created on: Jan 8, 2017
 *      Author: nullifiedcat
 */

#include "common.hpp"
#include "hack.hpp"
#include "MiscTemporary.hpp"
#include "SeedPrediction.hpp"
#include <link.h>
#include <hacks/hacklist.hpp>
#include <settings/Bool.hpp>
#include <hacks/AntiAntiAim.hpp>
#include "NavBot.hpp"

#include "HookedMethods.hpp"

static settings::Bool minigun_jump{ "misc.minigun-jump-tf2c", "false" };
static settings::Bool roll_speedhack{ "misc.roll-speedhack", "false" };
static settings::Bool engine_pred{ "misc.engine-prediction", "false" };
static settings::Bool debug_projectiles{ "debug.projectiles", "false" };
static settings::Int semiauto{ "misc.semi-auto", "0" };
static settings::Int fakelag_amount{ "misc.fakelag", "0" };

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
        MD5_PseudoRandom(current_user_cmd->command_number) & 0x7FFFFFFF;
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
} // namespace engine_prediction
#if not LAGBOT_MODE
#define antikick_time 35
#else
#define antikick_time 90
#endif
const char *cmds[7] = { "use",         "voicecommand", "spec_next", "spec_prev",
                        "spec_player", "invprev",      "invnext" };

static int attackticks = 0;

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
        hacks::tf2::NavBot::Init();
        firstcm = false;
    }
    tickcount++;
    current_user_cmd = cmd;
#if !LAGBOT_MODE
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

    if (!isHackActive())
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

    if (current_user_cmd && current_user_cmd->command_number)
        last_cmd_number = current_user_cmd->command_number;

    /**bSendPackets = true;
    if (hacks::shared::lagexploit::ExploitActive()) {
        *bSendPackets = ((current_user_cmd->command_number % 4) == 0);
        //logging::Info("%d", *bSendPackets);
    }*/

    // logging::Info("canpacket: %i", ch->CanPacket());
    // if (!cmd) return ret;

    time_replaced = false;
    curtime_old   = g_GlobalVars->curtime;

    if (nolerp)
    {
        // current_user_cmd->tick_count += 1;
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

    hacks::shared::autojoin::update();

    {
        PROF_SECTION(CM_AAA);
        hacks::shared::anti_anti_aim::createMove();
    }

#if ENABLE_IPC
#if !LAGBOT_MODE
    if (hacks::shared::followbot::isEnabled())
    {
        hacks::shared::followbot::WorldTick();
    }
#endif
#endif
    if (CE_GOOD(g_pLocalPlayer->entity))
    {
#if !LAGBOT_MODE
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
#if !LAGBOT_MODE
            {
                PROF_SECTION(CM_walkbot);
                hacks::shared::walkbot::Move();
            }
            {
                PROF_SECTION(CM_navbot);
                nav::CreateMove();
            }
            {
                PROF_SECTION(CM_NavParse);
                hacks::tf2::NavBot::CreateMove();
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
                                                       current_user_cmd);
            {
                PROF_SECTION(CM_backtracc);
                hacks::shared::backtrack::Run();
            }
            {
                PROF_SECTION(CM_aimbot);
                hacks::shared::aimbot::CreateMove();
            }
            if (current_user_cmd->buttons & IN_ATTACK)
                ++attackticks;
            else
                attackticks = 0;
            if (semiauto)
                if (current_user_cmd->buttons & IN_ATTACK)
                    if (attackticks % *semiauto < *semiauto - 1)
                        current_user_cmd->buttons &= ~IN_ATTACK;
            static int fakelag_queue = 0;
            if (CE_GOOD(LOCAL_E))
                if (fakelag_amount)
                {
                    *bSendPackets = *fakelag_amount == fakelag_queue;
                    fakelag_queue++;
                    if (fakelag_queue > *fakelag_amount)
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
#if !LAGBOT_MODE
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
            hacks::shared::spam::createMove();
        }
#if !LAGBOT_MODE
        {
            PROF_SECTION(CM_AC);
            hacks::shared::anticheat::CreateMove();
        }
#endif
    }
    if (time_replaced)
        g_GlobalVars->curtime = curtime_old;
    g_Settings.bInvalid = false;
#if !LAGBOT_MODE
    {
        PROF_SECTION(CM_chat_stack);
        chat_stack::OnCreateMove();
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
#if !LAGBOT_MODE
    hacks::shared::backtrack::UpdateIncomingSequences();
    if (CE_GOOD(g_pLocalPlayer->entity))
    {
        speedapplied = false;
        if (roll_speedhack && cmd->buttons & IN_DUCK &&
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
        if (cmd && (cmd->buttons & IN_ATTACK ||
                    !(hacks::shared::antiaim::isEnabled() && !*bSendPackets)))
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

    //	PROF_END("CreateMove");
    if (!(cmd->buttons & IN_ATTACK))
    {
        // LoadSavedState();
    }
    g_pLocalPlayer->bAttackLastTick = (cmd->buttons & IN_ATTACK);
    g_Settings.is_create_move       = false;
    return ret;
}
} // namespace hooked_methods

/*float o_curtime;
float o_frametime;

void Start() {
    g_IGameMovement->StartTrackPredictionErrors((CBasePlayer*)(RAW_ENT(LOCAL_E)));

    IClientEntity* player = RAW_ENT(LOCAL_E);
    // CPredictableId::ResetInstanceCounters();
    *(reinterpret_cast<CUserCmd*>(reinterpret_cast<uintptr_t>(player) + 1047)) =
current_user_cmd; o_curtime = g_GlobalVars->curtime; o_frametime =
g_GlobalVars->frametime; *g_PredictionRandomSeed =
MD5_PseudoRandom(current_user_cmd->command_number) & 0x7FFFFFFF;
g_GlobalVars->curtime
= CE_INT(LOCAL_E, netvar.nTickBase) * g_GlobalVars->interval_per_tick;
    g_GlobalVars->frametime = g_GlobalVars->interval_per_tick;

    CMoveData data;

}

void End() {
    *g_PredictionRandomSeed = -1;
    g_GlobalVars->curtime = o_curtime;
    g_GlobalVars->frametime = o_frametime;
}*/
