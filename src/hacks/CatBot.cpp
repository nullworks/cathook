/*
 * CatBot.cpp
 *
 *  Created on: Dec 30, 2017
 *      Author: nullifiedcat
 */

#include "common.hpp"
#include "hack.hpp"

namespace hacks
{
namespace shared
{
namespace catbot
{

static CatVar enabled(CV_SWITCH, XORSTR("cbu"), XORSTR("0"), XORSTR("CatBot Utils"));
static CatVar abandon_if_bots_gte(CV_INT, XORSTR("cbu_abandon_if_bots_gte"), XORSTR("0"),
                                  XORSTR("Abandon if bots >="));
static CatVar abandon_if_ipc_bots_gte(CV_INT, XORSTR("cbu_abandon_if_ipc_bots_gte"),
                                      XORSTR("0"), XORSTR("Abandon if IPC bots >="));
static CatVar abandon_if_humans_lte(CV_INT, XORSTR("cbu_abandon_if_humans_lte"), XORSTR("0"),
                                    XORSTR("Abandon if humans <="));
static CatVar abandon_if_players_lte(CV_INT, XORSTR("cbu_abandon_if_players_lte"), XORSTR("0"),
                                     XORSTR("Abandon if players <="));
static CatVar mark_human_threshold(CV_INT, XORSTR("cbu_mark_human_threshold"), XORSTR("2"),
                                   XORSTR("Mark human after N kills"));
static CatVar random_votekicks(CV_SWITCH, XORSTR("cbu_random_votekicks"), XORSTR("0"),
                               XORSTR("Randomly initiate votekicks"));
static CatVar micspam(CV_SWITCH, XORSTR("cbu_micspam"), XORSTR("0"), XORSTR("Micspam helper"));
static CatVar micspam_on(CV_INT, XORSTR("cbu_micspam_on_interval"), XORSTR("3"),
                         XORSTR("+voicerecord interval"));
static CatVar micspam_off(CV_INT, XORSTR("cbu_micspam_off_interval"), XORSTR("60"),
                          XORSTR("-voicerecord interval"));

struct catbot_user_state
{
    int treacherous_kills{ 0 };
};

std::unordered_map<unsigned, catbot_user_state> human_detecting_map{};

bool is_a_catbot(unsigned steamID)
{
    auto it = human_detecting_map.find(steamID);
    if (it == human_detecting_map.end())
        return false;

    // if (!(*it).second.has_bot_name)
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

    // if (human_detecting_map[steamID].has_bot_name)
    human_detecting_map[steamID].treacherous_kills++;
    logging::Info(XORSTR("Treacherous kill #%d: %s [U:1:%u]"),
                  human_detecting_map[steamID].treacherous_kills,
                  player->player_info.name, player->player_info.friendsID);
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
    hack::ExecuteCommand(XORSTR("callvote kick ") + std::to_string(target) +
                         XORSTR(" cheating"));
}

void update_catbot_list()
{
    for (int i = 1; i < g_GlobalVars->maxClients; ++i)
    {
        player_info_s info;
        if (!g_IEngine->GetPlayerInfo(i, &info))
            continue;

        info.name[31] = 0;
        if (strcasestr(info.name, XORSTR("cat-bot")) ||
            strcasestr(info.name, XORSTR("just disable vac tf")) ||
            strcasestr(info.name, XORSTR("raul.garcia")) ||
            strcasestr(info.name, XORSTR("zCat")) ||
            strcasestr(info.name, XORSTR("lagger bot")) ||
            strcasestr(info.name, XORSTR("zLag-bot")) ||
            strcasestr(info.name, XORSTR("crash-bot")))
        {
            if (human_detecting_map.find(info.friendsID) ==
                human_detecting_map.end())
            {
                logging::Info(XORSTR("Found bot %s [U:1:%u]"), info.name,
                              info.friendsID);
                human_detecting_map.insert(
                    std::make_pair(info.friendsID, catbot_user_state{ 0 }));
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

        int killer_id =
            g_IEngine->GetPlayerForUserID(event->GetInt(XORSTR("attacker")));
        int victim_id = g_IEngine->GetPlayerForUserID(event->GetInt(XORSTR("userid")));

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

#if ENABLE_IPC
void update_ipc_data(ipc::user_data_s &data)
{
    data.ingame.bot_count = count_bots;
}
#endif

Timer level_init_timer{};

Timer micspam_on_timer{};
Timer micspam_off_timer{};

void reportall()
{
    typedef uint64_t (*ReportPlayer_t)(uint64_t, int);
    static uintptr_t addr2 = gSignatures.GetClientSignature(
        XORSTR("55 89 E5 57 56 53 81 EC ? ? ? ? 8B 5D ? 8B 7D ? 89 D8"));
    ReportPlayer_t ReportPlayer_fn = ReportPlayer_t(addr2);
    if (!addr2)
        return;
    player_info_s local;
    g_IEngine->GetPlayerInfo(g_pLocalPlayer->entity_idx, &local);
    for (int i = 1; i < g_IEngine->GetMaxClients(); i++)
    {
        CachedEntity *ent = ENTITY(i);
        // We only want a nullptr check since dormant entities are still on the
        // server
        if (!ent)
            continue;
        player_info_s info;
        if (g_IEngine->GetPlayerInfo(i, &info))
        {
            if (info.friendsID == local.friendsID ||
                playerlist::AccessData(info.friendsID).state ==
                    playerlist::k_EState::FRIEND ||
                playerlist::AccessData(info.friendsID).state ==
                    playerlist::k_EState::IPC)
                continue;
            CSteamID id(info.friendsID, EUniverse::k_EUniversePublic,
                        EAccountType::k_EAccountTypeIndividual);
            ReportPlayer_fn(id.ConvertToUint64(), 1);
        }
    }
}
CatCommand report(XORSTR("report_debug"), XORSTR("debug"), []() { reportall(); });
void update()
{
    if (!enabled)
        return;

    if (g_Settings.bInvalid)
        return;

    if (CE_BAD(LOCAL_E))
        return;

    if (micspam)
    {
        if (micspam_on && micspam_on_timer.test_and_set(int(micspam_on) * 1000))
            g_IEngine->ExecuteClientCmd(XORSTR("+voicerecord"));
        if (micspam_off &&
            micspam_off_timer.test_and_set(int(micspam_off) * 1000))
            g_IEngine->ExecuteClientCmd(XORSTR("-voicerecord"));
    }

    if (random_votekicks && timer_votekicks.test_and_set(5000))
        do_random_votekick();
    if (timer_catbot_list.test_and_set(3000))
        update_catbot_list();
    if (timer_abandon.test_and_set(2000) && level_init_timer.check(13000))
    {
        count_bots      = 0;
        int count_ipc   = 0;
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

            if (playerlist::AccessData(info.friendsID).state ==
                playerlist::k_EState::IPC)
                ++count_ipc;
        }

        if (abandon_if_bots_gte)
        {
            if (count_bots >= int(abandon_if_bots_gte))
            {
                logging::Info(XORSTR("Abandoning because there are %d bots in game, ")
                              XORSTR("and abandon_if_bots_gte is %d."),
                              count_bots, int(abandon_if_bots_gte));
                tfmm::abandon();
                return;
            }
        }
        if (abandon_if_ipc_bots_gte)
        {
            if (count_ipc >= int(abandon_if_ipc_bots_gte))
            {
                logging::Info(XORSTR("Abandoning because there are %d local players ")
                              XORSTR("in game, and abandon_if_ipc_bots_gte is %d."),
                              count_ipc, int(abandon_if_ipc_bots_gte));
                tfmm::abandon();
                return;
            }
        }
        if (abandon_if_humans_lte)
        {
            if (count_total - count_bots <= int(abandon_if_humans_lte))
            {
                logging::Info(XORSTR("Abandoning because there are %d non-bots in ")
                              XORSTR("game, and abandon_if_humans_lte is %d."),
                              count_total - count_bots,
                              int(abandon_if_humans_lte));
                tfmm::abandon();
                return;
            }
        }
        if (abandon_if_players_lte)
        {
            if (count_total <= int(abandon_if_players_lte))
            {
                logging::Info(XORSTR("Abandoning because there are %d total players ")
                              XORSTR("in game, and abandon_if_players_lte is %d."),
                              count_total, int(abandon_if_players_lte));
                tfmm::abandon();
                return;
            }
        }
    }
}

void init()
{
    g_IEventManager2->AddListener(&listener(), XORSTR("player_death"), false);
}

void level_init()
{
    level_init_timer.update();
}
}
}
}
