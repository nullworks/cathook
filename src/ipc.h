/*
 * ipc.h
 *
 *  Created on: Mar 19, 2017
 *      Author: nullifiedcat
 */

#ifdef IPC_ENABLED

#ifndef IPC_H_
#define IPC_H_

#include "beforecheaders.h"
#include "ipcb.hpp"
#include "pthread.h"
#include <time.h>
#include "aftercheaders.h"

class CatCommand;
class CatVar;

namespace ipc {

namespace commands {

constexpr unsigned execute_client_cmd = 1;
constexpr unsigned set_follow_steamid = 2;
constexpr unsigned execute_client_cmd_long = 3;
constexpr unsigned move_to_vector = 4;
constexpr unsigned stop_moving = 5;
constexpr unsigned start_moving = 6;

}

extern CatCommand connect;
extern CatCommand disconnect;
extern CatCommand exec;
extern CatCommand exec_all;
extern CatCommand lobby;
extern CatVar server_name;

extern pthread_t listener_thread;
constexpr unsigned cathook_magic_number = 0x0DEADCA7;

struct server_data_s {
	unsigned magic_number;
};

struct user_data_s {
	char name[32];
	char server[22];
	unsigned friendid;
	bool connected;
	bool good;
	int health;
	int health_max;
	char life_state;
	int team;
	int clazz;
	int score;
	int last_score;
	int total_score;
	time_t heartbeat;
	float x;
	float y;
	float z;
	time_t ts_injected;
	time_t ts_connected;
	time_t ts_disconnected;
	int shots;
	int hits;
	int headshots;
};

using peer_t = cat_ipc::Peer<server_data_s, user_data_s>;

extern peer_t* peer;

void Heartbeat();
void UpdateTemporaryData();
void UpdateServerAddress(bool shutdown = false);
void StoreClientData();
void UpdatePlayerlist();

}

#endif /* IPC_H_ */

#endif
