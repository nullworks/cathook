/*
 * tfmm.cpp
 *
 *  Created on: Dec 7, 2017
 *      Author: nullifiedcat
 */

#include <settings/Int.hpp>
#include "common.hpp"
#include "hacks/AutoJoin.hpp"
#include "hack.hpp"

static settings::Boolean auto_party{ "player-tools.set-party-state", "true" };
settings::Int queue{ "autoqueue.mode", "7" };

CatCommand cmd_queue_start("mm_queue_casual", "Start casual queue", []() { tfmm::startQueue(); });
CatCommand queue_party("mm_queue_party", "Queue for Party", []() {
    re::CTFPartyClient *client = re::CTFPartyClient::GTFPartyClient();
    client->RequestQueueForStandby();
});
CatCommand cmd_abandon("mm_abandon", "Abandon match", []() { tfmm::abandon(); });

CatCommand abandoncmd("disconnect_and_abandon", "Disconnect and abandon", []() { tfmm::disconnectAndAbandon(); });

CatCommand get_state("mm_state", "Get party state", []() {
    re::CTFParty *party = re::CTFParty::GetParty();
    if (!party)
    {
        logging::Info("Party == NULL");
        return;
    }
    logging::Info("State: %d", re::CTFParty::state_(party));
});

static CatCommand mm_stop_queue("mm_stop_queue", "Stop current TF2 MM queue", []() { tfmm::leaveQueue(); });
static CatCommand mm_debug_leader("mm_debug_leader", "Get party's leader", []() {
    CSteamID id;
    bool success = re::CTFPartyClient::GTFPartyClient()->GetCurrentPartyLeader(id);
    if (success)
        logging::Info("%u", id.GetAccountID());
    else
        logging::Info("Failed to get party leader");
});
static CatCommand mm_debug_promote("mm_debug_promote", "Promote player to leader", [](const CCommand &args) {
    if (args.ArgC() < 2)
        return;

    uint32_t id32 = std::strtoul(args.Arg(1), nullptr, 10);
    CSteamID id(id32, EUniverse::k_EUniversePublic, EAccountType::k_EAccountTypeIndividual);
    logging::Info("Attempting to promote %u", id);
    int succ = re::CTFPartyClient::GTFPartyClient()->PromotePlayerToLeader(id);
    logging::Info("Success ? %d", succ);
});
static CatCommand mm_debug_kick("mm_debug_kick", "Kick player from party", [](const CCommand &args) {
    if (args.ArgC() < 2)
        return;

    uint32_t id32 = std::strtoul(args.Arg(1), nullptr, 10);
    CSteamID id(id32, EUniverse::k_EUniversePublic, EAccountType::k_EAccountTypeIndividual);
    logging::Info("Attempting to kick %u", id);
    int succ = re::CTFPartyClient::GTFPartyClient()->KickPlayer(id);
    logging::Info("Success ? %d", succ);
});
static CatCommand mm_debug_chat("mm_debug_chat", "Debug party chat", [](const CCommand &args) {
    if (args.ArgC() <= 1)
        return;

    re::CTFPartyClient::GTFPartyClient()->SendPartyChat(args.ArgS());
});

namespace tfmm
{
int queuecount = 0;

static bool old_isMMBanned()
{
    auto client = re::CTFPartyClient::GTFPartyClient();
    if (!client || ((client->BInQueueForMatchGroup(7) || client->BInQueueForStandby()) && queuecount < 10))
    {
        queuecount = 0;
        return false;
    }
    return true;
}
static bool getMMBanData(int type, int *time, int *time2)
{
    typedef bool (*GetMMBanData_t)(int, int *, int *);
    static uintptr_t addr                 = gSignatures.GetClientSignature("55 89 E5 57 56 53 83 EC ? 8B 5D 08 8B 75 0C 8B 7D 10 83 FB FF");
    static GetMMBanData_t GetMMBanData_fn = GetMMBanData_t(addr);

    if (!addr)
    {
        *time  = -1;
        *time2 = -1;
        logging::Info("GetMMBanData sig is broken");
        return old_isMMBanned();
    }
    return GetMMBanData_fn(type, time, time2);
}

static CatCommand mm_debug_banned("mm_debug_banned", "Prints if your are MM banned and extra data if you are banned", []() {
    int i, time, left, banned;
    for (i = 0; i < 1; ++i)
    {
        time = left = 0;
        banned      = getMMBanData(0, &time, &left);
        logging::Info("%d: banned %d, time %d, left %d", banned, time, left);
    }
});

bool isMMBanned()
{
    /* Competitive only bans are not interesting */
    return getMMBanData(0, nullptr, nullptr);
}
int getQueue()
{
    return *queue;
}

void startQueue()
{
    re::CTFPartyClient *client = re::CTFPartyClient::GTFPartyClient();
    if (client)
    {
        if (*queue == 7)
            client->LoadSavedCasualCriteria();
        client->RequestQueueForMatch((int) queue);
        // client->RequestQueueForStandby();
        queuecount++;
    }
    else
        logging::Info("queue_start: CTFPartyClient == null!");
}
void startQueueStandby()
{
    re::CTFPartyClient *client = re::CTFPartyClient::GTFPartyClient();
    if (client)
    {
        client->RequestQueueForStandby();
    }
}
void leaveQueue()
{
    re::CTFPartyClient *client = re::CTFPartyClient::GTFPartyClient();
    if (client)
        client->RequestLeaveForMatch((int) queue);
    else
        logging::Info("queue_start: CTFPartyClient == null!");
}

void disconnectAndAbandon()
{
    abandon();
    leaveQueue();
}

void abandon()
{
    re::CTFGCClientSystem *gc = re::CTFGCClientSystem::GTFGCClientSystem();
    if (gc && gc->BConnectedToMatchServer(false))
        gc->AbandonCurrentMatch();
    else if (!gc)
        logging::Info("abandon: CTFGCClientSystem == null!");
    else
    {
        logging::Info("Not connected to a Match server. Disconnecting normally");
        hack::ExecuteCommand("disconnect");
    }
}

static Timer friend_party_t{};
void friend_party()
{
    if (auto_party && friend_party_t.test_and_set(10000))
    {
        re::CTFPartyClient *pc = re::CTFPartyClient::GTFPartyClient();
        if (pc)
        {
            unsigned steamid_local                = g_ISteamUser->GetSteamID().GetAccountID();
            std::vector<unsigned> valid_steam_ids = pc->GetPartySteamIDs();
            for (auto steamid : valid_steam_ids)
                if (steamid && steamid != steamid_local)
                    playerlist::ChangeState(steamid, playerlist::k_EState::PARTY);
        }
    }
}

static InitRoutine init([]() { EC::Register(EC::Paint, friend_party, "paint_friendparty"); });

} // namespace tfmm
