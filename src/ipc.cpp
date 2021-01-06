/*
 * ipc.cpp
 *
 *  Created on: Mar 19, 2017
 *      Author: nullifiedcat
 */

#include <hacks/CatBot.hpp>
#include <settings/Bool.hpp>
#include "ipc.hpp"

#include "common.hpp"
#include "hack.hpp"
#include "hitrate.hpp"
#include "MiscTemporary.hpp"
#include "GetFriendPersonaName.hpp"

#if ENABLE_IPC

static settings::Boolean ipc_update_list{ "ipc.update-player-list", "true" };
static settings::Int bot_chunks("ipc.bot-chunks", "1");

namespace ipc
{

static settings::String server_name{ "ipc.server", "cathook_followbot_server" };

CatCommand fix_deadlock("ipc_fix_deadlock", "Fix deadlock", []() {
    if (peer)
    {
        pthread_mutex_unlock(&peer->memory->mutex);
    }
});
CatCommand id("ipc_id", "Echo ipc id", []() { logging::Info("%d", ipc::peer->client_id); });
CatCommand connect("ipc_connect", "Connect to IPC server", []() {
    if (peer)
    {
        logging::Info("Already connected!");
        return;
    }
    peer = new peer_t(*server_name, false, false);
    try
    {
        peer->Connect();
        logging::Info("peer count: %i", peer->memory->peer_count);
        logging::Info("magic number: 0x%08x", peer->memory->global_data.magic_number);
        logging::Info("magic number offset: 0x%08x", (uintptr_t) &peer->memory->global_data.magic_number - (uintptr_t) peer->memory);
        peer->SetCommandHandler(commands::execute_client_cmd, [](cat_ipc::command_s &command, void *payload) { hack::command_stack().push(std::string((const char *) &command.cmd_data)); });
        peer->SetCommandHandler(commands::execute_client_cmd_long, [](cat_ipc::command_s &command, void *payload) { hack::command_stack().push(std::string((const char *) payload)); });
        user_data_s &data = peer->memory->peer_user_data[peer->client_id];

        // Preserve accumulated data
        ipc::user_data_s::accumulated_t accumulated;
        memcpy(&accumulated, &data.accumulated, sizeof(accumulated));
        memset(&data, 0, sizeof(data));
        memcpy(&data.accumulated, &accumulated, sizeof(accumulated));

        StoreClientData();
        Heartbeat();
        // Load a config depending on id
        hack::command_stack().push("exec cat_autoexec_ipc_" + std::to_string(peer->client_id % std::max(1, *bot_chunks)));
    }
    catch (std::exception &error)
    {
        logging::Info("Runtime error: %s", error.what());
        delete peer;
        peer = nullptr;
    }
});
CatCommand disconnect("ipc_disconnect", "Disconnect from IPC server", []() {
    if (peer)
        delete peer;
    peer = nullptr;
});
CatCommand exec("ipc_exec", "Execute command (first argument = bot ID)", [](const CCommand &args) {
    char *endptr       = nullptr;
    unsigned target_id = strtol(args.Arg(1), &endptr, 10);
    if (endptr == args.Arg(1))
    {
        logging::Info("Target id is NaN!");
        return;
    }
    if (target_id < 0 || target_id > 255)
    {
        logging::Info("Invalid target id: %u", target_id);
        return;
    }
    {
        if (peer->memory->peer_data[target_id].free)
        {
            logging::Info("Trying to send command to a dead peer");
            return;
        }
    }
    std::string command = std::string(args.ArgS());
    command             = command.substr(command.find(' ', 0) + 1);
    ReplaceString(command, " && ", " ; ");
    if (command.length() >= 63)
    {
        peer->SendMessage(0, target_id, ipc::commands::execute_client_cmd_long, command.c_str(), command.length() + 1);
    }
    else
    {
        peer->SendMessage(command.c_str(), target_id, ipc::commands::execute_client_cmd, 0, 0);
    }
});
CatCommand exec_all("ipc_exec_all", "Execute command (on every peer)", [](const CCommand &args) {
    std::string command = args.ArgS();
    ReplaceString(command, " && ", " ; ");
    if (command.length() >= 63)
    {
        peer->SendMessage(0, -1, ipc::commands::execute_client_cmd_long, command.c_str(), command.length() + 1);
    }
    else
    {
        peer->SendMessage(command.c_str(), -1, ipc::commands::execute_client_cmd, 0, 0);
    }
});

peer_t *peer{ nullptr };

CatCommand debug_get_ingame_ipc("ipc_debug_dump_server", "Show other bots on server", []() {
    std::vector<unsigned> players{};
    for (int j = 1; j <= MAX_PLAYERS; j++)
    {
        player_info_s info;
        if (g_IEngine->GetPlayerInfo(j, &info))
        {
            if (info.friendsID)
                players.push_back(info.friendsID);
        }
    }
    int count        = 0;
    unsigned highest = 0;
    std::vector<unsigned> botlist{};
    for (unsigned i = 0; i < cat_ipc::max_peers; i++)
    {
        if (!ipc::peer->memory->peer_data[i].free)
        {
            for (auto &k : players)
            {
                if (ipc::peer->memory->peer_user_data[i].friendid && k == ipc::peer->memory->peer_user_data[i].friendid)
                {
                    botlist.push_back(i);
                    logging::Info("-> %u (%u)", i, ipc::peer->memory->peer_user_data[i].friendid);
                    count++;
                    highest = i;
                }
            }
        }
    }
    logging::Info("%d other IPC players on server", count);
});

void UpdateServerAddress(bool shutdown)
{
    if (not peer)
        return;
    const char *s_addr = "0.0.0.0";
    if (not shutdown and g_IEngine->GetNetChannelInfo())
    {
        s_addr = g_IEngine->GetNetChannelInfo()->GetAddress();
    }

    user_data_s &data = peer->memory->peer_user_data[peer->client_id];
    data.friendid     = g_ISteamUser->GetSteamID().GetAccountID();
    strncpy(data.ingame.server, s_addr, sizeof(data.ingame.server));
}

void update_mapname()
{
    if (not peer)
        return;

    user_data_s &data = peer->memory->peer_user_data[peer->client_id];
    strncpy(data.ingame.mapname, GetLevelName().c_str(), sizeof(data.ingame.mapname));
}
float framerate = 0.0f;
void UpdateTemporaryData()
{
    user_data_s &data = peer->memory->peer_user_data[peer->client_id];

    data.connected = g_IEngine->IsInGame();
    // TODO kills, deaths
    data.accumulated.shots     = hitrate::count_shots;
    data.accumulated.hits      = hitrate::count_hits;
    data.accumulated.headshots = hitrate::count_hits_head;

    if (data.connected)
    {
        IClientEntity *player = g_IEntityList->GetClientEntity(g_IEngine->GetLocalPlayer());
        if (player)
        {
            data.ingame.good = true;
            // TODO kills, deaths, shots, hits, headshots

            int score_saved = data.ingame.score;

            data.ingame.score      = g_pPlayerResource->GetScore(g_IEngine->GetLocalPlayer());
            data.ingame.team       = g_pPlayerResource->GetTeam(g_IEngine->GetLocalPlayer());
            data.ingame.role       = g_pPlayerResource->GetClass(LOCAL_E);
            data.ingame.life_state = NET_BYTE(player, netvar.iLifeState);
            data.ingame.health     = NET_INT(player, netvar.iHealth);
            data.ingame.health_max = g_pPlayerResource->GetMaxHealth(LOCAL_E);

            if (score_saved > data.ingame.score)
                score_saved = 0;

            data.accumulated.score += data.ingame.score - score_saved;

            data.ingame.x = g_pLocalPlayer->v_Origin.x;
            data.ingame.y = g_pLocalPlayer->v_Origin.y;
            data.ingame.z = g_pLocalPlayer->v_Origin.z;

            int players = 0;

            for (int i = 1; i < g_GlobalVars->maxClients; ++i)
            {
                if (g_IEntityList->GetClientEntity(i))
                    ++players;
                else
                    continue;
            }

            data.ingame.player_count = players;
            hacks::shared::catbot::update_ipc_data(data);
        }
        else
        {
            data.ingame.good = false;
        }
        if (g_IEngine->GetLevelName())
            update_mapname();
    }
}

void StoreClientData()
{
    if (!peer)
        return;

    UpdateServerAddress();
    user_data_s &data = peer->memory->peer_user_data[peer->client_id];
    data.friendid     = g_ISteamUser->GetSteamID().GetAccountID();
    data.ts_injected  = time_injected;
    data.textmode     = ENABLE_TEXTMODE;
    if (g_ISteamUser)
    {
        strncpy(data.name, GetNamestealName(g_ISteamUser->GetSteamID()).c_str(), sizeof(data.name));
    }
}

void Heartbeat()
{
    user_data_s &data = peer->memory->peer_user_data[peer->client_id];
    data.heartbeat    = time(nullptr);
}

void UpdatePlayerlist()
{
    if (peer && ipc_update_list)
    {
        for (unsigned i = 0; i < cat_ipc::max_peers; i++)
        {
            if (!peer->memory->peer_data[i].free)
            {
                playerlist::ChangeState(peer->memory->peer_user_data[i].friendid, peer->memory->peer_user_data[i].textmode ? playerlist::k_EState::TEXTMODE : playerlist::k_EState::IPC);
            }
        }
    }
}
} // namespace ipc

#endif
