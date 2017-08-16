/*
 * ipc.cpp
 *
 *  Created on: Mar 19, 2017
 *      Author: nullifiedcat
 */


#include "ipc.h"

#include "common.h"
#include "hack.h"
#include "hitrate.hpp"

#ifdef IPC_ENABLED

namespace ipc {

std::atomic<bool> thread_running(false);
pthread_t listener_thread { 0 };

void* listen(void*) {
	while (thread_running) {
		if (peer->HasCommands()) {
			peer->ProcessCommands();
		}
		usleep(10000);
	}
	return 0;
}

CatCommand fix_deadlock("ipc_fix_deadlock", "Fix deadlock", []() {
	if (peer) {
		pthread_mutex_unlock(&peer->memory->mutex);
	}
});

CatCommand connect("ipc_connect", "Connect to IPC server", []() {
	if (peer || thread_running) {
		logging::Info("Already connected!");
		return;
	}
	peer = new peer_t(std::string(server_name.GetString()), false, false);
	try {
		peer->Connect();
		logging::Info("peer count: %i", peer->memory->peer_count);
		logging::Info("magic number: 0x%08x", peer->memory->global_data.magic_number);
		logging::Info("magic number offset: 0x%08x", (uintptr_t)&peer->memory->global_data.magic_number - (uintptr_t)peer->memory);
		peer->SetCommandHandler(commands::execute_client_cmd, [](cat_ipc::command_s& command, void* payload) {
			hack::command_stack().push(std::string((const char*)&command.cmd_data));
		});
		peer->SetCommandHandler(commands::execute_client_cmd_long, [](cat_ipc::command_s& command, void* payload) {
			hack::command_stack().push(std::string((const char*)payload));
		});
		hacks::shared::followbot::AddMessageHandlers(peer);
		user_data_s& data = peer->memory->peer_user_data[peer->client_id];
		// Preserve total score
		int o_total_score = data.total_score;
		memset(&data, 0, sizeof(data));
		data.total_score = o_total_score;
		StoreClientData();
		Heartbeat();
		thread_running = true;
		pthread_create(&listener_thread, nullptr, listen, nullptr);
	} catch (std::exception& error) {
		logging::Info("Runtime error: %s", error.what());
		delete peer;
		peer = nullptr;
	}

});
CatCommand lobby("ipc_lobby", "Join a lobby", [](const CCommand& args) {
	std::string input(args.ArgS());
	std::size_t lobby_id_start = input.find("[L:1:");
	if (lobby_id_start == std::string::npos) {
		logging::Info("couldn't find lobby ID!");
		return;
	}
	input = input.substr(lobby_id_start + 5);
	unsigned long lobby32 = strtoul(input.c_str(), nullptr, 10);
	unsigned long long lobby64 = ((25559040ull << 32) | lobby32);
	logging::Info("lobby64 ID: %llu", lobby64);
	peer->SendMessage(format("connect_lobby ", lobby64).c_str(), 0, ipc::commands::execute_client_cmd, 0, 0);
});
CatCommand disconnect("ipc_disconnect", "Disconnect from IPC server", []() {
	thread_running = false;
	pthread_join(listener_thread, nullptr);
	if (peer) delete peer;
	listener_thread = 0;
	peer = nullptr;
});
CatCommand exec("ipc_exec", "Execute command (first argument = bot ID)", [](const CCommand& args) {
	char* endptr = nullptr;
	unsigned target_id = strtol(args.Arg(1), &endptr, 10);
	if (endptr == args.Arg(1)) {
		logging::Info("Target id is NaN!");
		return;
	}
	if (target_id == 0 || target_id > 31) {
		logging::Info("Invalid target id: %u", target_id);
		return;
	}
	{
		if (peer->memory->peer_data[target_id].free) {
			logging::Info("Trying to send command to a dead peer");
			return;
		}
	}
	std::string command = std::string(args.ArgS());
	command = command.substr(command.find(' ', 0) + 1);
	ReplaceString(command, " && ", " ; ");
	if (command.length() >= 63) {
		peer->SendMessage(0, (1 << target_id), ipc::commands::execute_client_cmd_long, command.c_str(), command.length() + 1);
	} else {
		peer->SendMessage(command.c_str(), (1 << target_id), ipc::commands::execute_client_cmd, 0, 0);
	}
});
CatCommand exec_all("ipc_exec_all", "Execute command (on every peer)", [](const CCommand& args) {
	std::string command = args.ArgS();
	ReplaceString(command, " && ", " ; ");
	if (command.length() >= 63) {
		peer->SendMessage(0, 0, ipc::commands::execute_client_cmd_long, command.c_str(), command.length() + 1);
	} else {
		peer->SendMessage(command.c_str(), 0, ipc::commands::execute_client_cmd, 0, 0);
	}
});
CatVar server_name(CV_STRING, "ipc_server", "cathook_followbot_server", "IPC server name");

peer_t* peer { nullptr };

CatCommand debug_get_ingame_ipc("ipc_debug_dump_server", "Show other bots on server", []() {
	std::vector<unsigned> players {};
	for (int j = 1; j < 32; j++) {
		player_info_s info;
		if (g_IEngine->GetPlayerInfo(j, &info)) {
			if (info.friendsID)
				players.push_back(info.friendsID);
		}
	}
	int count = 0;
	unsigned highest = 0;
	std::vector<unsigned> botlist {};
	for (unsigned i = 1; 0 < cat_ipc::max_peers; i++) {
		if (!ipc::peer->memory->peer_data[i].free) {
			for (auto& k : players) {
				if (ipc::peer->memory->peer_user_data[i].friendid && k == ipc::peer->memory->peer_user_data[i].friendid) {
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

void UpdateServerAddress(bool shutdown) {
	if (not peer)
		return;
	const char* s_addr = "0.0.0.0";
	if (not shutdown and g_IEngine->GetNetChannelInfo()) {
		s_addr = g_IEngine->GetNetChannelInfo()->GetAddress();
	}

	peer_t::MutexLock lock(peer);
	user_data_s& data = peer->memory->peer_user_data[peer->client_id];
	data.friendid = g_ISteamUser->GetSteamID().GetAccountID();
	strncpy(data.server, s_addr, sizeof(data.server));
}

void UpdateTemporaryData() {
	user_data_s& data = peer->memory->peer_user_data[peer->client_id];
	data.connected = g_IEngine->IsInGame();
	data.shots = hitrate::count_shots;
	data.hits = hitrate::count_hits;
	data.headshots = hitrate::count_hits_head;
	if (data.connected) {
		IClientEntity* player = g_IEntityList->GetClientEntity(g_IEngine->GetLocalPlayer());
		if (player) {
			data.good = true;
			data.health = NET_INT(player, netvar.iHealth);
			data.health_max = g_pPlayerResource->GetMaxHealth(LOCAL_E);
			data.clazz = g_pPlayerResource->GetClass(LOCAL_E);
			data.life_state = NET_BYTE(player, netvar.iLifeState);
			data.score = g_pPlayerResource->GetScore(g_IEngine->GetLocalPlayer());
			if (data.last_score != data.score) {
				if (data.last_score > data.score) {
					data.total_score += data.score;
				} else {
					data.total_score += (data.score - data.last_score);
				}
				data.last_score = data.score;
			}
			data.team = g_pPlayerResource->GetTeam(g_IEngine->GetLocalPlayer());
			data.x = g_pLocalPlayer->v_Origin.x;
			data.y = g_pLocalPlayer->v_Origin.y;
			data.z = g_pLocalPlayer->v_Origin.z;
		} else {
			data.good = false;
		}
	}
}

void StoreClientData() {
	UpdateServerAddress();
	user_data_s& data = peer->memory->peer_user_data[peer->client_id];
	data.friendid = g_ISteamUser->GetSteamID().GetAccountID();
	data.ts_injected = time_injected;
	strncpy(data.name, GetFriendPersonaName_hook(g_ISteamFriends, g_ISteamUser->GetSteamID()), sizeof(data.name));
}


void Heartbeat() {
	user_data_s& data = peer->memory->peer_user_data[peer->client_id];
	data.heartbeat = time(nullptr);
}

static CatVar ipc_update_list(CV_SWITCH, "ipc_update_list", "1", "IPC Auto-Ignore", "Automaticly assign playerstates for bots");
void UpdatePlayerlist() {
	if (peer && ipc_update_list) {
		for (unsigned i = 0; i < cat_ipc::max_peers; i++) {
			if (!peer->memory->peer_data[i].free) {
				playerlist::userdata& info = playerlist::AccessData(peer->memory->peer_user_data[i].friendid);
				if (info.state == playerlist::k_EState::DEFAULT)
					info.state = playerlist::k_EState::IPC;
			}
		}
	}
}

}

#endif
