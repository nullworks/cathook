/*
 * ipc.cpp
 *
 *  Created on: Mar 19, 2017
 *      Author: nullifiedcat
 */

#include "ipc.hpp"

#include "common.hpp"
#include "hack.hpp"
#include "hitrate.hpp"

#if ENABLE_IPC == 1

namespace ipc
{

CatCommand fix_deadlock("ipc_fix_deadlock", "Fix deadlock", []() {
    if (peer)
    {
        pthread_mutex_unlock(&peer->memory->mutex);
    }
});

CatCommand connect("ipc_connect", "Connect to IPC server", []() {
    if (peer)
    {
        logging::Info("Already connected!");
        return;
    }
    peer = new peer_t(std::string(server_name.GetString()), false, false);
    try
    {
        peer->Connect();
        logging::Info("peer count: %i", peer->memory->peer_count);
        logging::Info("magic number: 0x%08x",
                      peer->memory->global_data.magic_number);
        logging::Info("magic number offset: 0x%08x",
                      (uintptr_t) &peer->memory->global_data.magic_number -
                          (uintptr_t) peer->memory);
        peer->SetCommandHandler(commands::execute_client_cmd,
                                [](cat_ipc::command_s &command, void *payload) {
                                    hack::command_stack().push(std::string(
                                        (const char *) &command.cmd_data));
                                });
        peer->SetCommandHandler(commands::execute_client_cmd_long,
                                [](cat_ipc::command_s &command, void *payload) {
                                    hack::command_stack().push(
                                        std::string((const char *) payload));
                                });
        hacks::shared::followbot::AddMessageHandlers(peer);
        user_data_s &data = peer->memory->peer_user_data[peer->client_id];

        // Preserve accumulated data
        ipc::user_data_s::accumulated_t accumulated;
        memcpy(&accumulated, &data.accumulated, sizeof(accumulated));
        memset(&data, 0, sizeof(data));
        memcpy(&data.accumulated, &accumulated, sizeof(accumulated));

        StoreClientData();
        Heartbeat();
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
CatCommand
    exec("ipc_exec", "Execute command (first argument = bot ID)",
         [](const CCommand &args) {
             char *endptr       = nullptr;
             unsigned target_id = strtol(args.Arg(1), &endptr, 10);
             if (endptr == args.Arg(1))
             {
                 logging::Info("Target id is NaN!");
                 return;
             }
             if (target_id == 0 || target_id > 31)
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
                 peer->SendMessage(0, (1 << target_id),
                                   ipc::commands::execute_client_cmd_long,
                                   command.c_str(), command.length() + 1);
             }
             else
             {
                 peer->SendMessage(command.c_str(), (1 << target_id),
                                   ipc::commands::execute_client_cmd, 0, 0);
             }
         });
CatCommand exec_all("ipc_exec_all", "Execute command (on every peer)",
                    [](const CCommand &args) {
                        std::string command = args.ArgS();
                        ReplaceString(command, " && ", " ; ");
                        if (command.length() >= 63)
                        {
                            peer->SendMessage(
                                0, 0, ipc::commands::execute_client_cmd_long,
                                command.c_str(), command.length() + 1);
                        }
                        else
                        {
                            peer->SendMessage(command.c_str(), 0,
                                              ipc::commands::execute_client_cmd,
                                              0, 0);
                        }
                    });
CatVar server_name(CV_STRING, "ipc_server", "cathook_followbot_server",
                   "IPC server name");

peer_t *peer{ nullptr };

CatCommand debug_get_ingame_ipc(
    "ipc_debug_dump_server", "Show other bots on server", []() {
        std::vector<unsigned> players{};
        for (int j = 1; j < 32; j++)
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
        for (unsigned i = 1; 0 < cat_ipc::max_peers; i++)
        {
            if (!ipc::peer->memory->peer_data[i].free)
            {
                for (auto &k : players)
                {
                    if (ipc::peer->memory->peer_user_data[i].friendid &&
                        k == ipc::peer->memory->peer_user_data[i].friendid)
                    {
                        botlist.push_back(i);
                        logging::Info(
                            "-> %u (%u)", i,
                            ipc::peer->memory->peer_user_data[i].friendid);
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

void UpdateTemporaryData()
{
    user_data_s &data = peer->memory->peer_user_data[peer->client_id];

    data.connected    = g_IEngine->IsInGame();
    // TODO kills, deaths
    data.accumulated.shots = hitrate::count_shots;
    data.accumulated.hits = hitrate::count_hits;
    data.accumulated.headshots = hitrate::count_hits_head;

    if (data.connected)
    {
        IClientEntity *player =
            g_IEntityList->GetClientEntity(g_IEngine->GetLocalPlayer());
        if (player)
        {
            data.ingame.good = true;
            // TODO kills, deaths, shots, hits, headshots

            int score_saved = data.ingame.score;

            data.ingame.score =
                g_pPlayerResource->GetScore(g_IEngine->GetLocalPlayer());
            data.ingame.team = g_pPlayerResource->GetTeam(g_IEngine->GetLocalPlayer());
            data.ingame.role = g_pPlayerResource->GetClass(LOCAL_E);
            data.ingame.life_state = NET_BYTE(player, netvar.iLifeState);
            data.ingame.health = NET_INT(player, netvar.iHealth);
            data.ingame.health_max = g_pPlayerResource->GetMaxHealth(LOCAL_E);

            if (score_saved > data.ingame.score)
            	score_saved = 0;

            data.accumulated.score += data.ingame.score - score_saved;

            data.ingame.x = g_pLocalPlayer->v_Origin.x;
            data.ingame.y = g_pLocalPlayer->v_Origin.y;
            data.ingame.z = g_pLocalPlayer->v_Origin.z;

            int players = 0;

    		for (int i = 1; i <= g_GlobalVars->maxClients; ++i)
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
    UpdateServerAddress();
    user_data_s &data = peer->memory->peer_user_data[peer->client_id];
    data.friendid     = g_ISteamUser->GetSteamID().GetAccountID();
    data.ts_injected  = time_injected;
    strncpy(data.name, GetFriendPersonaName_hook(g_ISteamFriends,
                                                 g_ISteamUser->GetSteamID()),
            sizeof(data.name));
}

void Heartbeat()
{
    user_data_s &data = peer->memory->peer_user_data[peer->client_id];
    data.heartbeat    = time(nullptr);
}

static CatVar ipc_update_list(CV_SWITCH, "ipc_update_list", "1",
                              "IPC Auto-Ignore",
                              "Automaticly assign playerstates for bots");
void UpdatePlayerlist()
{
    if (peer && ipc_update_list)
    {
        for (unsigned i = 0; i < cat_ipc::max_peers; i++)
        {
            if (!peer->memory->peer_data[i].free)
            {
                playerlist::userdata &info = playerlist::AccessData(
                    peer->memory->peer_user_data[i].friendid);
                if (info.state == playerlist::k_EState::DEFAULT)
                    info.state = playerlist::k_EState::IPC;
            }
        }
    }
}
}

#endif
