// To anyone reading this and planning to add it to their own cheat,
// It would be nice if you could credit us, the Nullworks/Cathook team.
// Thanks :)

/*
* Cathook
* Copyright (C) 2020  nullworks

* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.

* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "DetourHook.hpp"
#include <regex>
#include <boost/algorithm/string.hpp>
#include "usercmd.hpp"
#include "MiscTemporary.hpp"
#include "AntiAim.hpp"
#include "WeaponData.hpp"

namespace hacks::tf2::nospread
{
static settings::Boolean projectile("nospread.projectile", "false");
/*
 * 0 Always on
 * 1 Disable if being spectated in first person
 * 2 Disable if being spectated
 */
static settings::Int specmode("nospread.spectator-mode", "1");
settings::Boolean bullet("nospread.bullet", "false");
settings::Int debug_nospread("nospread.debug", "0");
settings::Boolean center_cone{ "nospread.center-cone", "true" };
settings::Boolean draw{ "nospread.draw-info", "true" };
settings::Boolean draw_mantissa{ "nospread.draw-info.mantissa", "false" };
settings::Boolean correct_ping{ "nospread.correct-ping", "true" };
settings::Boolean use_avg_latency{ "nospread.use-average-latency", "false" };
settings::Boolean extreme_accuracy{ "nospread.use-extreme-accuracy", "false" };
bool is_syncing = false;

bool shouldNoSpread(bool _projectile)
{
    switch (*specmode)
    {
    // Always on
    default:
    case 0:
        break;
    // Disable if being spectated in first person
    case 1:
        if (g_pLocalPlayer->spectator_state == g_pLocalPlayer->FIRSTPERSON)
            return false;
        break;
    // Disable if being spectated
    case 2:
        if (g_pLocalPlayer->spectator_state != g_pLocalPlayer->NONE)
            return false;
    };
    return _projectile ? *projectile : *bullet;
}

static void CreateMove()
{
    if (CE_BAD(LOCAL_E) || CE_BAD(LOCAL_W))
        return;

    if (!shouldNoSpread(true))
        return;

    // Credits to https://www.unknowncheats.me/forum/team-fortress-2-a/139094-projectile-nospread.html

    // Set up Random Seed
    int cmd_num = current_late_user_cmd->command_number;
    RandomSeed(MD5_PseudoRandom(cmd_num) & 0x7FFFFFFF);
    SharedRandomInt(MD5_PseudoRandom(cmd_num) & 0x7FFFFFFF, "SelectWeightedSequence", 0, 0, 0);
    for (int i = 0; i < 6; ++i)
        RandomFloat();

    // Projectile/Huntsman check
    if (g_pLocalPlayer->weapon_mode != weapon_projectile && LOCAL_W->m_iClassID() != CL_CLASS(CTFCompoundBow))
        return;

    // Beggars check
    if (CE_INT(LOCAL_W, netvar.iItemDefinitionIndex) == 730)
    {
        // Player has 0 loaded rockets and reload mode is not 2 (reloading and ready to release)
        bool no_loaded_rockets = CE_INT(LOCAL_W, netvar.m_iClip1) == 0 && CE_INT(LOCAL_W, netvar.iReloadMode) != 2;
        // Player is attacking and reload is not 0 (not reloading)
        bool loading_rockets = current_late_user_cmd->buttons & IN_ATTACK && CE_INT(LOCAL_W, netvar.iReloadMode) != 0;
        if (no_loaded_rockets || loading_rockets)
            return;
    }
    // Huntsman check
    else if (LOCAL_W->m_iClassID() == CL_CLASS(CTFCompoundBow))
    {
        if (current_late_user_cmd->buttons & IN_ATTACK || CE_FLOAT(LOCAL_W, netvar.flChargeBeginTime) == 0)
            return;
    }
    // Rest of weapons
    else if (!(current_late_user_cmd->buttons & IN_ATTACK))
        return;

    switch (LOCAL_W->m_iClassID())
    {
    case CL_CLASS(CTFSyringeGun):
    {
        if (g_pLocalPlayer->v_OrigViewangles == current_late_user_cmd->viewangles)
            g_pLocalPlayer->bUseSilentAngles = true;
        float spread = 1.5f;
        current_late_user_cmd->viewangles.x -= RandomFloat(-spread, spread);
        current_late_user_cmd->viewangles.y -= RandomFloat(-spread, spread);
        fClampAngle(current_late_user_cmd->viewangles);
        break;
    }
    case CL_CLASS(CTFCompoundBow):
    {
        Vector view = re::C_BasePlayer::GetLocalEyeAngles(RAW_ENT(LOCAL_E));
        if (g_pLocalPlayer->v_OrigViewangles == current_late_user_cmd->viewangles)
            g_pLocalPlayer->bUseSilentAngles = true;

        Vector spread;
        Vector src;

        re::C_TFWeaponBase::GetProjectileFireSetupHuntsman(RAW_ENT(LOCAL_W), RAW_ENT(LOCAL_E), Vector(23.5f, -8.f, 8.f), &src, &spread, false, 2000.0f);

        spread -= view;
        current_late_user_cmd->viewangles -= spread;
        fClampAngle(current_late_user_cmd->viewangles);
        break;
    }
    default:
        Vector view = re::C_BasePlayer::GetLocalEyeAngles(RAW_ENT(LOCAL_E));
        if (g_pLocalPlayer->v_OrigViewangles == current_late_user_cmd->viewangles)
            g_pLocalPlayer->bUseSilentAngles = true;

        Vector spread = re::C_TFWeaponBase::GetSpreadAngles(RAW_ENT(LOCAL_W));

        spread -= view;
        current_late_user_cmd->viewangles -= spread;
        fClampAngle(current_late_user_cmd->viewangles);
        break;
    }
}

static InitRoutine init([]() { EC::Register(EC::CreateMoveLate, CreateMove, "nospread_cm", EC::very_late); });

enum nospread_sync_state
{
    NOT_SYNCED = 0,
    CORRECTING,
    SYNCED,
    DEAD_SYNC,
};

// TODO: Attempt to send clc_move over unreliable channel

static bool waiting_perf_data                = false;
static bool should_update_time               = false;
static bool waiting_for_post_SNM             = false;
static bool resync_needed                    = false;
static bool last_was_player_perf             = false;
static bool resynced_this_death              = false;
static nospread_sync_state no_spread_synced  = NOT_SYNCED; // this will be set to 0 each level init / level shutdown
static bool bad_mantissa                     = false;      // Also reset every levelinit/shutdown
static double float_time_delta               = 0.0;
static double ping_at_send                   = 0.0;
static double last_ping_at_send              = 0.0;
static double sent_client_floattime          = 0.0;
static double last_correction                = 0.0;
static double write_usercmd_correction       = 0.0;
static double last_sync_delta_time           = 0.0;
static float prediction_seed                 = 0.0;
static bool use_usercmd_seed                 = false;
static float current_weapon_spread           = 0.0;
static bool first_usercmd                    = false;
static bool called_from_sendmove             = false;
static bool should_update_usercmd_correction = false;
static CUserCmd user_cmd_backup;

static float CalculateMantissaStep(float flValue)
{
    int iRawValue = reinterpret_cast<int &>(flValue);
    int iExponent = (iRawValue >> 23) & 0xFF;
    return powf(2, iExponent - (127 + 23));
}

float GetServerCurTime()
{
    // Calculate on our own accord
    float server_time = g_GlobalVars->interval_per_tick * CE_INT(LOCAL_E, netvar.nTickBase);
    return server_time;
}

// Does the shot have any spread in general?
bool IsPerfectShot(IClientEntity *weapon, float provided_time = 0.0 /*used for optimization*/)
{
    float server_time       = provided_time == 0.0 ? GetServerCurTime() : provided_time;
    float time_since_attack = server_time - NET_FLOAT(weapon, netvar.flLastFireTime);

    int nBulletsPerShot = GetWeaponData(weapon)->m_nBulletsPerShot;
    if (nBulletsPerShot >= 1)
        nBulletsPerShot = ATTRIB_HOOK_FLOAT(nBulletsPerShot, "mult_bullets_per_shot", weapon, 0x0, true);
    else
        nBulletsPerShot = 1;
    if ((nBulletsPerShot == 1 && time_since_attack > 1.25) || (nBulletsPerShot > 1 && time_since_attack > 0.25))
        return true;
    return false;
}

// Applies nospread
void ApplySpreadCorrection(Vector &angles, int seed, float spread)
{
    if (CE_BAD(LOCAL_E) || !LOCAL_E->m_bAlivePlayer() || CE_BAD(LOCAL_W))
        return;
    IClientEntity *weapon = RAW_ENT(LOCAL_W);

    bool is_first_shot_perfect = IsPerfectShot(weapon);

    // Size of one WeaponMode_t is 0x40, 0x6fc is the offset to bullets per shot
    int nBulletsPerShot = GetWeaponData(weapon)->m_nBulletsPerShot;
    if (nBulletsPerShot >= 1)
        nBulletsPerShot = ATTRIB_HOOK_FLOAT(nBulletsPerShot, "mult_bullets_per_shot", RAW_ENT(LOCAL_W), 0x0, true);
    else
        nBulletsPerShot = 1;

    // We only have one shot or we do not want to center the cone or it's perfect, so no need to adjust
    if ((nBulletsPerShot == 1 || !center_cone) && is_first_shot_perfect)
        return;

    // We only correct one bullet in this case
    if (!center_cone)
        nBulletsPerShot = 1;

    std::vector<Vector> bullet_corrections;
    Vector average_spread(0.0f);

    for (int i = 0; i < nBulletsPerShot; i++)
    {
        RandomSeed(seed + i);
        float flX = RandomFloat(-0.5, 0.5) + RandomFloat(-0.5, 0.5);
        float flY = RandomFloat(-0.5, 0.5) + RandomFloat(-0.5, 0.5);

        // This is our perfect shot, do not adjust
        if (is_first_shot_perfect && !i)
        {
            flX = 0.0f;
            flY = 0.0f;
        }
        fClampAngle(angles);
        Vector vShootForward, vShootRight, vShootUp;
        AngleVectors3(VectorToQAngle(angles), &vShootForward, &vShootRight, &vShootUp);

        Vector fixed_spread = vShootForward + (vShootRight * flX * spread) + (vShootUp * flY * spread);
        fixed_spread.NormalizeInPlace();
        // Add to the average
        average_spread += fixed_spread;
        // Add to Bullet spread vector
        bullet_corrections.push_back(fixed_spread);
    }
    // Turn it into an actual average
    average_spread /= nBulletsPerShot;

    // What we set our Vector to, start with FLT_MAX as deviation
    Vector fixed_spread(FLT_MAX);

    // Get the bullet closest to the average
    for (auto &spread : bullet_corrections)
    {
        // Is it closer to the average spread? if yes, use it
        if (spread.DistTo(average_spread) < fixed_spread.DistTo(average_spread))
            fixed_spread = spread;
    }

    Vector fixed_angles;
    VectorAngles(fixed_spread, fixed_angles);
    Vector vCorrection = (angles - fixed_angles);
    angles += vCorrection;
    fClampAngle(angles);
}

CatCommand debug_mantissa("test_mantissa", "For debug purposes",
                          [](const CCommand &rCmd)
                          {
                              if (rCmd.ArgC() < 2)
                              {
                                  g_ICvar->ConsoleColorPrintf(MENU_COLOR, "You must provide float to test.\n");
                                  return;
                              }

                              try
                              {
                                  float float_value   = atof(rCmd.Arg(1));
                                  float mantissa_step = CalculateMantissaStep(float_value);

                                  g_ICvar->ConsoleColorPrintf(MENU_COLOR, "Mantissa step for %.3f: %.10f\n", float_value, mantissa_step);
                              }
                              catch (const std::invalid_argument &)
                              {
                                  g_ICvar->ConsoleColorPrintf(MENU_COLOR, "Invalid float.\n");
                              }
                              return;
                          });

static CatCommand nospread_sync("nospread_sync", "Try to sync client and server time",
                                []()
                                {
                                    if (!bullet)
                                    {
                                        g_ICvar->ConsoleColorPrintf(MENU_COLOR, "Set nospread.enable to true first.\n");
                                        return;
                                    }
                                    should_update_time = true;
                                    no_spread_synced   = NOT_SYNCED;
                                    g_ICvar->ConsoleColorPrintf(MENU_COLOR, "Trying to sync Seed...\n");
                                });

static CatCommand nospread_resync("nospread_resync", "Try to sync client and server time",
                                  []()
                                  {
                                      if (!bullet)
                                      {
                                          g_ICvar->ConsoleColorPrintf(MENU_COLOR, "Set nospread.enable to true first.\n");
                                          return;
                                      }
                                      if (no_spread_synced == CORRECTING)
                                      {
                                          g_ICvar->ConsoleColorPrintf(MENU_COLOR, "Already syncing.");
                                          return;
                                      }
                                      if (no_spread_synced != SYNCED)
                                      {
                                          g_ICvar->ConsoleColorPrintf(MENU_COLOR, "Can't resync when not previously synced! Use cat_nospread_sync\n");
                                          return;
                                      }
                                      no_spread_synced = CORRECTING;
                                      g_ICvar->ConsoleColorPrintf(MENU_COLOR, "Trying to resync Seed...\n");
                                  });

// Our Detour hooks
DetourHook cl_writeusercmd_detour;
typedef void (*WriteUserCmd_t)(bf_write *, CUserCmd *, CUserCmd *);
DetourHook fx_firebullets_detour;
typedef void (*FX_FireBullets_t)(IClientEntity *, int, Vector *, Vector *, int, int, int, float, float, bool);
// DetourHook net_sendpacket_detour;
// typedef int (*NET_SendPacket_t)(INetChannel *, int, const netadr_t &, const unsigned char *, int, bf_write *, bool);

// false == unchanged, true == Force reliable
// We force the clc_move as a reliable message here to ensure its arrival before we call the original
bool SendNetMessage(INetMessage *data)
{
    if (!bullet)
        return false;

    // if we send clc_move with playerperf command or corrected angles, we must ensure it will be sent via reliable stream
    if (should_update_time)
    {
        if (data->GetType() != clc_Move)
            return false;

        // and wait for post call
        waiting_for_post_SNM = true;

        // Force reliable
        return true;
    }
    else if (no_spread_synced)
    {
        if (data->GetType() != clc_Move)
            return false;
    }
    return false;
}

static Timer wait_perf{};

// We send the playerperf in here to be (mostly) sure that it's sent along with the clc_move.
void SendNetMessagePost()
{
    if (!waiting_for_post_SNM || !bullet || (waiting_perf_data && !wait_perf.test_and_set(1000)))
        return;

    waiting_for_post_SNM = false;

    // Create playerperf
    NET_StringCmd sCmd("playerperf");

    // And send it along with our clc_move. Yes, we are calling SendNetMsg from inside SendNetMsg
    ((INetChannel *) (g_IEngine->GetNetChannelInfo()))->SendNetMsg(sCmd, true);

    // remember client float time
    should_update_time = false;
    // Only set when not syncing
    if (no_spread_synced == NOT_SYNCED)
        sent_client_floattime = Plat_FloatTime();

    // force transmit now
    ((INetChannel *) (g_IEngine->GetNetChannelInfo()))->Transmit();

    if (use_avg_latency)
        ping_at_send = ((INetChannel *) (g_IEngine->GetNetChannelInfo()))->GetAvgLatency(FLOW_OUTGOING);
    else
        ping_at_send = ((INetChannel *) (g_IEngine->GetNetChannelInfo()))->GetLatency(FLOW_OUTGOING);

    waiting_perf_data = true;
    wait_perf.update();
}

CatCommand debug_flows("debug_flows", "debug", []() { logging::Info("Incoming: %f\n Outgoing: %f", ((INetChannel *) g_IEngine->GetNetChannelInfo())->GetLatency(FLOW_INCOMING), ((INetChannel *) g_IEngine->GetNetChannelInfo())->GetLatency(FLOW_OUTGOING)); });

// false == don't call original, true == call original
// This function is used to parse the playerperf data
bool DispatchUserMessage(bf_read *buf, int type)
{
    bool should_call_original = true;
    if ((!waiting_perf_data && !last_was_player_perf) || !bullet)
        return should_call_original;

    // We are looking for TextMsg
    if (type != 5)
        return should_call_original;

    int message_dest = buf->ReadByte();

    // Not send to us
    if (message_dest != 2)
    {
        buf->Seek(0);
        return should_call_original;
    }

    double start_time = Plat_FloatTime();

    char msg_str[256];
    buf->ReadString(msg_str, sizeof(msg_str));
    buf->Seek(0);

    std::vector<std::string> lines;
    boost::split(lines, msg_str, boost::is_any_of("\n"), boost::token_compress_on);

    // Regex to find the playerperf data we want/need
    static std::regex primary_regex("^(([0-9]+\\.[0-9]+) ([0-9]{1,2}) ([0-9]{1,2}))$");

    std::vector<double> vData;

    for (auto sStr : lines)
    {
        std::smatch sMatch;

        if (!std::regex_match(sStr, sMatch, primary_regex) || sMatch.size() != 5)
        {
            static std::regex backup_regex("^(([0-9]+\\.[0-9]+) ([0-9]{1,2}) ([0-9]{1,2}) ([0-9]+\\.[0-9]+) ([0-9]+\\.[0-9]+) vel ([0-9]+\\.[0-9]+))$");
            std::smatch sMatch2;
            if (std::regex_match(sStr, sMatch2, backup_regex) && sMatch2.size() > 5)
            {
                last_was_player_perf = true;
                should_call_original = false;
            }
            continue;
        }

        std::string server_time = sMatch[2].str();

        char *tmp;
        try
        {
            vData.push_back(std::strtod(server_time.c_str(), &tmp));
        }
        // Shouldn't happen
        catch (const std::invalid_argument &)
        {
        }
    }

    if (vData.size() < 2)
    {
        if (!vData.size())
            last_was_player_perf = false;
        // Still do not call original, we don't want the playerperf spewing everywhere
        else
            return false;
        return should_call_original; // not our case, just return
    }

    // Less than 1 in step size is literally impossible to predict, although 1 is already pushing it
    if (CalculateMantissaStep(vData[0] * 1000.0) < 1.0)
    {
        if (waiting_perf_data)
            g_ICvar->ConsoleColorPrintf(MENU_COLOR, "Couldn't sync nospread: server uptime too low.\n");
        waiting_perf_data    = false;
        last_was_player_perf = true;
        should_call_original = false;
        bad_mantissa         = true;
        no_spread_synced     = NOT_SYNCED;
        return should_call_original;
    }

    bad_mantissa         = false;
    last_was_player_perf = true;
    should_call_original = false;

    // Process time!

    // ...or not
    if (!waiting_perf_data)
        return should_call_original;

    if (no_spread_synced == NOT_SYNCED)
    {
        // first, compensate procession time and latency between us and server
        double client_time   = Plat_FloatTime();
        double total_latency = (client_time - (client_time - start_time)) - sent_client_floattime;

        // Second compensate latency and get delta time (this might be negative, so be careful!)
        float_time_delta = vData[0] - sent_client_floattime;

        // We got time with latency included, but only outgoing, so compensate
        float_time_delta -= (total_latency / 2.0);

        if (debug_nospread)
            g_ICvar->ConsoleColorPrintf(MENU_COLOR, "Assumed delta time: %.10f calculated based on %i entries.\n", float_time_delta, (int) vData.size() - 1);

        // we need only first output which is latest
        waiting_perf_data = false;

        // and now collect history
        no_spread_synced = CORRECTING;
        is_syncing       = true;
    }
    else if (no_spread_synced != SYNCED)
    {
        // Now sync and Correct
        double time_difference = sent_client_floattime - vData[0];

        double mantissa_step = CalculateMantissaStep(sent_client_floattime * 1000.0);
        // Apply correction
        // We must try to be super accurate if the rvar is set, else base on mantissa step
        double correction_threshhold = extreme_accuracy ? 0.001 : (mantissa_step / 1000.0 / 2.0);

        // Check if we are not precise enough or snapped too hard for it to actually be synced
        if (abs(time_difference) > correction_threshhold || abs(last_correction) > mantissa_step / 1000.0)
        {
            float_time_delta -= time_difference;
            // it will auto resync it
            resync_needed = true;
            // Print debug if desired
            if (debug_nospread)
                g_ICvar->ConsoleColorPrintf(MENU_COLOR, "Applied correction: %.10f\n", time_difference);
        }
        // We synced successfully
        else
        {
            if (debug_nospread)
                g_ICvar->ConsoleColorPrintf(MENU_COLOR, "Nospread successfully synced. Possible precision loss: %.10f Mantissa step: %.2f\n", time_difference, mantissa_step);
            else
                g_ICvar->ConsoleColorPrintf(MENU_COLOR, "Nospread successfully synced. Mantissa step: %.2f\n", mantissa_step);
            resync_needed = false;
        }
        last_correction = time_difference;

        // We need only first output which is latest
        waiting_perf_data = false;

        // We are synced.
        if (!resync_needed)
            no_spread_synced = SYNCED;
    }
    // May happen when dead
    else
    {
        resync_needed     = false;
        waiting_perf_data = false;
    }
    return should_call_original;
};

void CL_SendMove_hook()
{
    first_usercmd        = true;
    called_from_sendmove = false;

    if (!no_spread_synced || !shouldNoSpread(false))
    {
        CL_SendMove_t original = (CL_SendMove_t) cl_nospread_sendmovedetour.GetOriginalFunc();
        original();
        cl_nospread_sendmovedetour.RestorePatch();
        return;
    }
    // Here we need to calculate process average process time, predict spread and get weapon spread

    current_weapon_spread = 0.0;

    // first try to get the player and check if he is valid
    if (!RAW_ENT(LOCAL_E) || HasCondition<TFCond_HalloweenGhostMode>(LOCAL_E))
    {
        // don't set called_from_sendmove here cuz we don't care
        CL_SendMove_t original = (CL_SendMove_t) cl_nospread_sendmovedetour.GetOriginalFunc();
        original();
        cl_nospread_sendmovedetour.RestorePatch();
        return;
    }

    int *choked_packets = nullptr;
    if (!choked_packets)
        choked_packets = (int *) ((uintptr_t) g_IBaseClientState + offsets::lastoutgoingcommand() + 4);

    int new_packets = 1 + *choked_packets;

    auto RecheckIfresync_needed = [&new_packets](double asumed_time) -> void
    {
        static Timer s_NextCheck;
        // we use it as 1 sec delay
        if (s_NextCheck.check(1000) && new_packets == 1 && (no_spread_synced != SYNCED || !LOCAL_E->m_bAlivePlayer()) && !waiting_perf_data)
        {
            s_NextCheck.update();
            // request playerperf once again here, and this time we don't care if it will be sent with clc_move
            // reuse this variable, this time we store here predicted time here
            sent_client_floattime = asumed_time;
            // set it to true to send playerperf with this cmd
            should_update_time = true;

            // Incase we are dead sync too, unless we already are doing so
            if (!LOCAL_E->m_bAlivePlayer() && no_spread_synced != CORRECTING)
            {
                // Backup data
                last_sync_delta_time = float_time_delta;
                last_ping_at_send    = ping_at_send;
                // We now start syncing
                no_spread_synced = DEAD_SYNC;
            }
            // We always set this, and change it later in CL_SendMove if we aren't dead
            resynced_this_death = true;
        }
    };

    // try to predict server's float time
    // please notice that time delta calculated relative to this code possition
    // this means we don't care about procession time of the code below
    // but we care about procession time of dynamic code in WriteUserCmd hook
    double asumed_real_time = Plat_FloatTime() + float_time_delta;
    double predicted_time   = asumed_real_time;

    predicted_time += write_usercmd_correction * new_packets;
    double ping = ((INetChannel *) (g_IEngine->GetNetChannelInfo()))->GetLatency(FLOW_OUTGOING);
    if (use_avg_latency)
        ping = ((INetChannel *) (g_IEngine->GetNetChannelInfo()))->GetAvgLatency(FLOW_OUTGOING);

    if (correct_ping)
        // Ping changed, adjust (Provided we are not fakelagging)
        if (!(fakelag_amount || (hacks::shared::antiaim::isEnabled() && hacks::shared::antiaim::force_fakelag)) && (int) (ping * 1000.0) != (int) (ping_at_send * 1000.0))
            predicted_time += ping - ping_at_send;

    // Check if we need to sync
    RecheckIfresync_needed(asumed_real_time);

    // If we're dead just return original
    if (!LOCAL_E->m_bAlivePlayer())
    {
        CL_SendMove_t original = (CL_SendMove_t) cl_nospread_sendmovedetour.GetOriginalFunc();
        original();
        cl_nospread_sendmovedetour.RestorePatch();
        return;
    }
    else
    {
        // We are alive
        resynced_this_death = false;

        // We should cancel the dead sync process
        if (no_spread_synced == DEAD_SYNC)
        {
            // Restore delta time
            float_time_delta = last_sync_delta_time;
            ping_at_send     = last_ping_at_send;
            // Mark as synced
            no_spread_synced   = SYNCED;
            should_update_time = false;
        }
    }

    // Bad weapon
    if ((g_pLocalPlayer->weapon_mode != weapon_hitscan && LOCAL_W->m_iClassID() != CL_CLASS(CTFCompoundBow)))
    {
        CL_SendMove_t original = (CL_SendMove_t) cl_nospread_sendmovedetour.GetOriginalFunc();
        original();
        cl_nospread_sendmovedetour.RestorePatch();
        return;
    }

    float current_time = GetServerCurTime();

    // Check if we are attacking, if not then no point in adjusting
    if (!current_user_cmd || !(current_user_cmd->buttons & IN_ATTACK))
    {
        CL_SendMove_t original = (CL_SendMove_t) cl_nospread_sendmovedetour.GetOriginalFunc();
        original();
        cl_nospread_sendmovedetour.RestorePatch();
        return;
    }

    // If we have a perfect shot and we don#t want to center the whole cone, returne too
    if (IsPerfectShot(RAW_ENT(LOCAL_W), current_time) && !center_cone)
    {
        CL_SendMove_t original = (CL_SendMove_t) cl_nospread_sendmovedetour.GetOriginalFunc();
        original();
        cl_nospread_sendmovedetour.RestorePatch();
        return;
    }

    current_weapon_spread = re::C_TFWeaponBaseGun::GetWeaponSpread(RAW_ENT(LOCAL_W));

    // Bad spread
    if (!IsFinite(current_weapon_spread))
    {
        CL_SendMove_t original = (CL_SendMove_t) cl_nospread_sendmovedetour.GetOriginalFunc();
        original();
        cl_nospread_sendmovedetour.RestorePatch();
        return;
    }

    if (*debug_nospread >= 2)
        g_ICvar->ConsoleColorPrintf(MENU_COLOR, "Predicted: %.6f Assumed: %.6f Correction: %.6f Commands: %i\n", predicted_time, asumed_real_time, write_usercmd_correction, new_packets);
    // Try to predict seed now
    // The important thing to understand this: server_random_seed set as soon as ProcessUsercmds called. And it's called in clc_move process function, so as soon as server accepts our packet, not when it actually processed.
    // This means every usercmd in 1 clc_move will have almost same random seed - if process time less than mantissa step for random seed number, then same "random" number will be set for each usercmd in this packet

    // Only adjust if not using usercmd seed
    if (!use_usercmd_seed)
        prediction_seed = (float) (predicted_time * 1000.0);

    // Call original
    called_from_sendmove = true;
    double time_start    = Plat_FloatTime();

    CL_SendMove_t original = (CL_SendMove_t) cl_nospread_sendmovedetour.GetOriginalFunc();
    original();
    cl_nospread_sendmovedetour.RestorePatch();

    double time_end      = Plat_FloatTime();
    called_from_sendmove = false;

    // Update the processing time if we actually processed stuff
    if (should_update_usercmd_correction)
    {
        // How long it took for us to process each cmd? We will add this number next time we will process usercommands
        write_usercmd_correction         = (time_end - time_start) / new_packets;
        should_update_usercmd_correction = false;
    }
}

void WriteUserCmd_hook(bf_write *buf, CUserCmd *to, CUserCmd *from)
{
    // Called by a demo recorder or we shouldn't compensate it.
    if ((no_spread_synced != SYNCED && !resync_needed) || !shouldNoSpread(false) || current_weapon_spread == 0.0)
    {
        WriteUserCmd_t original = (WriteUserCmd_t) cl_writeusercmd_detour.GetOriginalFunc();
        original(buf, to, from);
        cl_writeusercmd_detour.RestorePatch();
        return;
    }

    // Save the original from
    CUserCmd backup_from = *from;

    if (!(to->buttons & IN_ATTACK))
    {
        user_cmd_backup         = *to;
        first_usercmd           = false;
        WriteUserCmd_t original = (WriteUserCmd_t) cl_writeusercmd_detour.GetOriginalFunc();
        original(buf, to, from);
        cl_writeusercmd_detour.RestorePatch();
        return;
    }

    // Save the original to
    CUserCmd backup_to = *to;

    // If nospread fully synced, the only thing we need to do is to use already known float time with our random seed

    ApplySpreadCorrection(to->viewangles, reinterpret_cast<int &>(prediction_seed) & 0xFF, current_weapon_spread);

    // We are processing, so we should update it
    should_update_usercmd_correction = true;

    user_cmd_backup = *to;
    first_usercmd   = false;

    WriteUserCmd_t original = (WriteUserCmd_t) cl_writeusercmd_detour.GetOriginalFunc();
    original(buf, to, from);
    cl_writeusercmd_detour.RestorePatch();

    // Restore the original from
    *from = backup_from;
    // Restore the original to
    *to = backup_to;
}

void FX_FireBullets_hook(IClientEntity *weapon, int player, Vector *origin, Vector *angles, int weapon_idx, int bullet_mode, int seed, float spread, float damage, bool is_critical)
{
    // Not synced/weapon bad
    if (!weapon || (no_spread_synced != SYNCED && !resync_needed) || !bullet || (IsPerfectShot(weapon) && !center_cone))
    {
        FX_FireBullets_t original = (FX_FireBullets_t) fx_firebullets_detour.GetOriginalFunc();
        original(weapon, player, origin, angles, weapon_idx, bullet_mode, seed, spread, damage, is_critical);
        fx_firebullets_detour.RestorePatch();
        return;
    }

    Vector corrected_angles = *angles;
    ApplySpreadCorrection(corrected_angles, seed, spread);

    FX_FireBullets_t original = (FX_FireBullets_t) fx_firebullets_detour.GetOriginalFunc();
    original(weapon, player, origin, &corrected_angles, weapon_idx, bullet_mode, seed, spread, damage, is_critical);
    fx_firebullets_detour.RestorePatch();
}

/*int NET_SendPacket_hook(INetChannel *chan, int sock, const netadr_t &to, const unsigned char *data, int length, bf_write *pVoicePayload, bool bUseCompression)
{
    logging::Info("Packet size: %d", length);
    NET_SendPacket_t original = (NET_SendPacket_t) net_sendpacket_detour.GetOriginalFunc();
    auto ret_val              = original(chan, sock, to, data, length, pVoicePayload, bUseCompression);
    net_sendpacket_detour.RestorePatch();
    return ret_val;
}*/

static Timer update_nospread_timer{};
static void CreateMove2()
{
    if (bullet)
    {
        static auto sv_usercmd_custom_random_seed = g_ICvar->FindVar("sv_usercmd_custom_random_seed");
        if (!sv_usercmd_custom_random_seed)
            sv_usercmd_custom_random_seed = g_ICvar->FindVar("sv_usercmd_custom_random_seed");

        // Server owner decided it would be a great idea to give the user control over the random seed
        else if (!sv_usercmd_custom_random_seed->GetBool())
        {
            auto seed        = MD5_PseudoRandom(current_user_cmd->command_number) & 0x7FFFFFFF;
            prediction_seed  = *reinterpret_cast<float *>(&seed);
            use_usercmd_seed = true;
        }
        // Normal server
        else
            use_usercmd_seed = false;

        // Synced, mark as such to other modules
        if (no_spread_synced == SYNCED)
            is_syncing = false;
        // Not synced currently, try to sync
        if (no_spread_synced == NOT_SYNCED && !bad_mantissa)
        {
            is_syncing         = true;
            should_update_time = true;
            update_nospread_timer.update();
        }
        // Else if mantissa bad, update every 10 mins
        else if (no_spread_synced == NOT_SYNCED && update_nospread_timer.test_and_set(10 * 60 * 1000))
            no_spread_synced = CORRECTING;
    }
}

static InitRoutine init_bulletnospread(
    []()
    {
        // Get our detour hooks running
        static auto writeusercmd_addr = gSignatures.GetClientSignature("55 89 E5 57 56 53 83 EC 2C 8B 45 ? 8B 7D ? 8B 5D ? 89 45 ? 8B 40");
        cl_writeusercmd_detour.Init(writeusercmd_addr, (void *) WriteUserCmd_hook);
        static auto fx_firebullets_addr = gSignatures.GetClientSignature("55 89 E5 57 56 53 81 EC 0C 01 00 00 8B 45 ? 8B 7D ? 89 85");
        fx_firebullets_detour.Init(fx_firebullets_addr, (void *) FX_FireBullets_hook);
        /*static auto net_sendpacket_addr = gSignatures.GetEngineSignature("55 89 E5 57 56 53 81 EC EC 20 00 00 C7 85 ? ? ? ? 00 00 00 00 8B 45");
        net_sendpacket_detour.Init(net_sendpacket_addr, (void *) NET_SendPacket_hook);*/

        // Register Event callbacks
        EC::Register(EC::CreateMove, CreateMove2, "nospread_createmove2");
        EC::Register(EC::CreateMoveWarp, CreateMove2, "nospread_createmove2w");

        bullet.installChangeCallback(
            [](settings::VariableBase<bool> &, bool after)
            {
                if (!after)
                {
                    is_syncing       = false;
                    no_spread_synced = NOT_SYNCED;
                }
            });
#if ENABLE_VISUALS
        EC::Register(
            EC::Draw,
            []()
            {
                if (bullet && (draw || draw_mantissa) && CE_GOOD(LOCAL_E) && LOCAL_E->m_bAlivePlayer())
                {
                    std::string draw_string = "";
                    rgba_t draw_color       = colors::white;
                    switch (no_spread_synced)
                    {
                    case NOT_SYNCED:
                    {
                        if (bad_mantissa)
                        {
                            draw_color  = colors::red_s;
                            draw_string = "Server uptime too Low!";
                        }
                        else
                        {
                            draw_color  = colors::orange;
                            draw_string = "Not Syncing";
                        }
                        break;
                    }
                    case CORRECTING:
                    case DEAD_SYNC:
                    {
                        draw_color  = colors::yellow;
                        draw_string = "Syncing...";
                        break;
                    }
                    case SYNCED:
                    {
                        draw_color  = colors::green;
                        draw_string = "Synced.";
                        break;
                    }
                    default:
                        break;
                    }
                    if (draw)
                        AddCenterString(draw_string, draw_color);
                    if (draw_mantissa && no_spread_synced != NOT_SYNCED)
                        AddCenterString("Mantissa step size: " + std::to_string((int) CalculateMantissaStep(1000.0 * (Plat_FloatTime() + float_time_delta))), draw_color);
                }
            },
            "nospread_draw");
#endif
        EC::Register(
            EC::LevelInit,
            []()
            {
                no_spread_synced     = NOT_SYNCED;
                last_was_player_perf = false;
                bad_mantissa         = false;
                waiting_perf_data    = false;
            },
            "nospread_levelinit");

        EC::Register(
            EC::LevelShutdown,
            []()
            {
                no_spread_synced     = NOT_SYNCED;
                last_was_player_perf = false;
                bad_mantissa         = false;
                waiting_perf_data    = false;
            },
            "nospread_levelshutdown");
        EC::Register(
            EC::Shutdown,
            []()
            {
                cl_writeusercmd_detour.Shutdown();
                fx_firebullets_detour.Shutdown();
                // net_sendpacket_detour.Shutdown();
            },
            "nospread_shutdown");
    });

} // namespace hacks::tf2::nospread
