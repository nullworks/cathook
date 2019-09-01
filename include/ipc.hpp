/*
 * ipc.h
 *
 *  Created on: Mar 19, 2017
 *      Author: nullifiedcat
 */

#pragma once

#include "config.h"

#if ENABLE_IPC

#include <SimpleIPC/ipcb.hpp>
#include "pthread.h"
#include <time.h>

class CatCommand;

namespace ipc
{

namespace commands
{

constexpr unsigned execute_client_cmd      = 1;
constexpr unsigned set_follow_steamid      = 2;
constexpr unsigned execute_client_cmd_long = 3;
constexpr unsigned move_to_vector          = 4;
constexpr unsigned stop_moving             = 5;
constexpr unsigned start_moving            = 6;
} // namespace commands

constexpr unsigned cathook_magic_number = 0x0DEADCA7;

struct server_data_s
{
    unsigned magic_number;
};

struct user_data_s
{
    char name[32];
    unsigned friendid;
    bool textmode;

    bool connected;

    time_t heartbeat;

    time_t ts_injected;
    time_t ts_connected;
    time_t ts_disconnected;

    struct accumulated_t
    {
        int kills;  // TODO
        int deaths; // TODO
        int score;

        int shots;
        int hits;
        int headshots;
    } accumulated;

    struct
    {
        bool good;

        int kills;  // TODO
        int deaths; // TODO
        int score;

        int shots;     // TODO
        int hits;      // TODO
        int headshots; // TODO

        int team;
        int role; // class
        char life_state;
        int health;
        int health_max;

        float x;
        float y;
        float z;

        int player_count;
        int bot_count;

        char server[24];
        char mapname[32];
    } ingame;
};

using peer_t = cat_ipc::Peer<server_data_s, user_data_s>;

extern peer_t *peer;

void update_mapname();

void Heartbeat();
void UpdateTemporaryData();
void UpdateServerAddress(bool shutdown = false);
void StoreClientData();
void UpdatePlayerlist();
} // namespace ipc

#endif
