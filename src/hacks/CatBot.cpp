/*
 * CatBot.cpp
 *
 *  Created on: Dec 30, 2017
 *      Author: nullifiedcat
 */

#include "common.hpp"
#include "hack.hpp"

namespace hacks { namespace shared { namespace catbot {

static CatVar enabled(CV_SWITCH, "cbu", "0", "CatBot Utils");
static CatVar abandon_if_bots_gte(CV_INT, "cbu_abandon_if_bots_gte", "0", "Abandon if bots >=");
static CatVar abandon_if_ipc_bots_gte(CV_INT, "cbu_abandon_if_ipc_bots_gte", "0", "Abandon if IPC bots >=");
static CatVar abandon_if_humans_lte(CV_INT, "cbu_abandon_if_humans_lte", "0", "Abandon if humans <=");
static CatVar abandon_if_players_lte(CV_INT, "cbu_abandon_if_players_lte", "0", "Abandon if players <=");
static CatVar mark_human_threshold(CV_INT, "cbu_mark_human_threshold", "2", "Mark human after N kills");
static CatVar random_votekicks(CV_SWITCH, "cbu_random_votekicks", "0", "Randomly initiate votekicks");

struct catbot_user_state
{
	int  treacherous_kills{ 0 };
};

std::unordered_map<unsigned, catbot_user_state> human_detecting_map{};

bool is_a_catbot(unsigned steamID)
{
	auto it = human_detecting_map.find(steamID);
	if (it == human_detecting_map.end())
		return false;

	//if (!(*it).second.has_bot_name)
	//	return false;

	if ((*it).second.treacherous_kills <= int(mark_human_threshold))
	{
		return true;
	}

	return false;
}

void on_killed_by(int userid)
{
	CachedEntity *player = ENTITY(g_IEngine->GetPlayerForUserID(userid));

	if (CE_BAD(player))
		return;

	unsigned steamID = player->player_info.friendsID;

	if (human_detecting_map.find(steamID) == human_detecting_map.end())
		return;

	//if (human_detecting_map[steamID].has_bot_name)
	human_detecting_map[steamID].treacherous_kills++;
	logging::Info("Treacherous kill #%d: %s [U:1:%u]", human_detecting_map[steamID].treacherous_kills, player->player_info.name, player->player_info.friendsID);
}

void do_random_votekick()
{
	std::vector<int> targets;
	for (int i = 1; i <= g_GlobalVars->maxClients; ++i)
	{
		player_info_s info;
		if (!g_IEngine->GetPlayerInfo(i, &info))
			continue;

		if (g_pPlayerResource->GetTeam(i) != g_pLocalPlayer->team)
			continue;

		if (is_a_catbot(info.friendsID))
			continue;

		targets.push_back(info.userID);
	}

	if (targets.empty())
		return;

	int target = targets[rand() % targets.size()];
	player_info_s info;
	if (!g_IEngine->GetPlayerInfo(g_IEngine->GetPlayerForUserID(target), &info))
		return;
	hack::ExecuteCommand("callvote kick " + std::to_string(target) + " cheating");
}

void update_catbot_list()
{
	for (int i = 1; i < g_GlobalVars->maxClients; ++i)
	{
		player_info_s info;
		if (!g_IEngine->GetPlayerInfo(i, &info))
			continue;

		info.name[31] = 0;
		if (strcasestr(info.name, "cat-bot") ||
			strcasestr(info.name, "just disable vac tf"))
		{
			if (human_detecting_map.find(info.friendsID) == human_detecting_map.end())
			{
				logging::Info("Found bot %s [U:1:%u]", info.name, info.friendsID);
				human_detecting_map.insert(std::make_pair(info.friendsID, catbot_user_state{ 0 }));
			}
		}
	}

}

class CatBotEventListener : public IGameEventListener2
{
    virtual void FireGameEvent(IGameEvent *event)
    {
        if (!enabled)
            return;

        int killer_id = g_IEngine->GetPlayerForUserID(event->GetInt("attacker"));
        int victim_id = g_IEngine->GetPlayerForUserID(event->GetInt("userid"));

        if (victim_id == g_IEngine->GetLocalPlayer())
        {
            on_killed_by(killer_id);
            return;
        }
    }
};

CatBotEventListener &listener()
{
    static CatBotEventListener object{};
    return object;
}

Timer timer_votekicks{};
Timer timer_catbot_list{};
Timer timer_abandon{};

int count_bots{ 0 };

bool should_ignore_player(CachedEntity *player)
{
	if (CE_BAD(player))
		return false;

	return is_a_catbot(player->player_info.friendsID);
}

void update_ipc_data(ipc::user_data_s& data)
{
	data.ingame.bot_count = count_bots;
}

void update()
{
	if (!enabled)
		return;

	if (g_Settings.bInvalid)
		return;

	if (CE_BAD(LOCAL_E))
		return;

	if (random_votekicks && timer_votekicks.test_and_set(5000))
		do_random_votekick();
	if (timer_catbot_list.test_and_set(3000))
		update_catbot_list();
	if (timer_abandon.test_and_set(2000))
	{
		count_bots = 0;
		int count_ipc = 0;
		int count_total = 0;

		for (int i = 1; i <= g_GlobalVars->maxClients; ++i)
		{
			if (g_IEntityList->GetClientEntity(i))
				++count_total;
			else
				continue;

			player_info_s info;
			if (!g_IEngine->GetPlayerInfo(i, &info))
				continue;

			if (is_a_catbot(info.friendsID))
				++count_bots;

			if (playerlist::AccessData(info.friendsID).state == playerlist::k_EState::IPC)
				++count_ipc;
		}

		if (abandon_if_bots_gte)
		{
			if (count_bots >= int(abandon_if_bots_gte))
			{
				logging::Info("Abandoning because there are %d bots in game, and abandon_if_bots_gte is %d.", count_bots, int(abandon_if_bots_gte));
				tfmm::abandon();
				return;
			}
		}
		if (abandon_if_ipc_bots_gte)
		{
			if (count_ipc >= int(abandon_if_ipc_bots_gte))
			{
				logging::Info("Abandoning because there are %d local players in game, and abandon_if_ipc_bots_gte is %d.", count_ipc, int(abandon_if_ipc_bots_gte));
				tfmm::abandon();
				return;
			}
		}
		if (abandon_if_humans_lte)
		{
			if (count_total - count_bots <= int(abandon_if_humans_lte))
			{
				logging::Info("Abandoning because there are %d non-bots in game, and abandon_if_humans_lte is %d.", count_total - count_bots, int(abandon_if_ipc_bots_gte));
				tfmm::abandon();
				return;
			}
		}
		if (abandon_if_players_lte)
		{
			if (count_total <= int(abandon_if_humans_lte))
			{
				logging::Info("Abandoning because there are %d total players in game, and abandon_if_humans_lte is %d.", count_total, int(abandon_if_humans_lte));
				tfmm::abandon();
				return;
			}
		}
	}
}

void init()
{
    g_IEventManager2->AddListener(&listener(), "player_death", false);
}

}
}
}
