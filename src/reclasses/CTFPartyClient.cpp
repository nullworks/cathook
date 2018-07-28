/*
 * CTFPartyClient.cpp
 *
 *  Created on: Dec 7, 2017
 *      Author: nullifiedcat
 */

#include "common.hpp"
#include "core/e8call.hpp"

re::CTFPartyClient *re::CTFPartyClient::GTFPartyClient()
{
    typedef re::CTFPartyClient *(*GTFPartyClient_t)(void);
    static uintptr_t addr = gSignatures.GetClientSignature(
        "55 A1 ? ? ? ? 89 E5 5D C3 8D B6 00 00 00 00 A1 ? ? ? ? 85 C0");
    static GTFPartyClient_t GTFPartyClient_fn = GTFPartyClient_t(addr);

    return GTFPartyClient_fn();
}

bool re::CTFPartyClient::BInQueue(re::CTFPartyClient *this_)
{
    return *(uint8_t *) ((uint8_t *) this_ + 69);
}

int re::CTFPartyClient::SendPartyChat(re::CTFPartyClient *client,
                                      const char *message)
{
    // todo
}

bool re::CTFPartyClient::BCanQueueForStandby(re::CTFPartyClient *this_)
{
    typedef bool (*BCanQueueForStandby_t)(re::CTFPartyClient *);
    static uintptr_t addr = gSignatures.GetClientSignature(
        "55 89 E5 53 83 EC 24 8B 5D 08 80 7B 46 00 75 40 8B 4B 38 85 C9 74 39 "
        "E8 ? ? ? ? 89 04 24 E8 ? ? ? ? 84 C0 75 28");
    static BCanQueueForStandby_t BCanQueueForStandby_fn =
        BCanQueueForStandby_t(addr);

    return BCanQueueForStandby_fn(this_);
}

re::ITFGroupMatchCriteria *
re::CTFPartyClient::MutLocalGroupCriteria(re::CTFPartyClient *client)
{
    typedef re::ITFGroupMatchCriteria *(*MutLocalGroupCriteria_t)(
        re::CTFPartyClient *);
    static uintptr_t addr =
        gSignatures.GetClientSignature("55 89 E5 8B 45 ? 8B 50 ? C6 80");
    static MutLocalGroupCriteria_t MutLocalGroupCriteria_fn =
        MutLocalGroupCriteria_t(addr);

    return MutLocalGroupCriteria_fn(client);
}

int re::CTFPartyClient::LoadSavedCasualCriteria()
{
    typedef int (*LoadSavedCasualCriteria_t)(re::CTFPartyClient *);
    static uintptr_t addr =
        gSignatures.GetClientSignature(
            "55 89 E5 8B 45 ? 5D 8B 80 ? ? ? ? C3 66 90 55 89 E5 "
            "8B 45 ? 5D 0F B6 80 ? ? ? ? C3 90 55 89 E5 56") -
        0x40;
    static LoadSavedCasualCriteria_t LoadSavedCasualCriteria_fn =
        LoadSavedCasualCriteria_t(addr);

    return LoadSavedCasualCriteria_fn(this);
}

char re::CTFPartyClient::RequestQueueForMatch(int type)
{
    typedef char (*RequestQueueForMatch_t)(re::CTFPartyClient *, int);
    static uintptr_t addr = gSignatures.GetClientSignature(
        "55 89 E5 57 56 53 81 EC ? ? ? ? 8B 45 ? E8");
    static RequestQueueForMatch_t RequestQueueForMatch_fn =
        RequestQueueForMatch_t(addr);

    return RequestQueueForMatch_fn(this, type);
}
bool re::CTFPartyClient::BInQueueForMatchGroup(int type)
{
    typedef bool (*BInQueueForMatchGroup_t)(re::CTFPartyClient *, int);
    static uintptr_t addr = gSignatures.GetClientSignature(
        "55 89 E5 56 53 8B 5D ? 8B 75 ? 89 D8 E8 ? ? ? ? 84 C0 74 ? 8B 4E");
    static BInQueueForMatchGroup_t BInQueueForMatchGroup_fn =
        BInQueueForMatchGroup_t(addr);

    return BInQueueForMatchGroup_fn(this, type);
}
char re::CTFPartyClient::RequestLeaveForMatch(int type)
{
    typedef char (*RequestLeaveForMatch_t)(re::CTFPartyClient *, int);
    static uintptr_t addr = gSignatures.GetClientSignature(
        "55 89 E5 57 56 53 83 EC ? 8B 45 ? 89 44 24 ? 8B 45 ? 89 04 24 E8 ? ? "
        "? ? 84 C0 89 C6 75");
    static RequestLeaveForMatch_t RequestLeaveForMatch_fn =
        RequestLeaveForMatch_t(addr);

    return RequestLeaveForMatch_fn(this, type);
}
int re::CTFPartyClient::BInvitePlayerToParty(CSteamID steamid)
{
    typedef int (*BInvitePlayerToParty_t)(re::CTFPartyClient *, CSteamID, bool);
    static uintptr_t addr = gSignatures.GetClientSignature(
        "55 89 E5 57 56 53 81 EC ? ? ? ? 8B 45 ? 8B 5D ? 8B 55 ? 89 85 ? ? ? ? "
        "65 A1 ? ? ? ? 89 45 ? 31 C0 8B 45");
    static BInvitePlayerToParty_t BInvitePlayerToParty_fn =
        BInvitePlayerToParty_t(addr);
    return BInvitePlayerToParty_fn(this, steamid, false);
}
int re::CTFPartyClient::BRequestJoinPlayer(CSteamID steamid)
{
    typedef int (*BRequestJoinPlayer_t)(re::CTFPartyClient *, CSteamID, bool);
    static uintptr_t addr = gSignatures.GetClientSignature(
        "55 89 E5 57 56 53 81 EC ? ? ? ? 8B 45 ? 8B 5D ? 8B 55 ? 89 85 ? ? ? ? "
        "65 A1 ? ? ? ? 89 45 ? 31 C0 8B 45");
    static BRequestJoinPlayer_t BRequestJoinPlayer_fn =
        BRequestJoinPlayer_t(addr);
    return BRequestJoinPlayer_fn(this, steamid, false);
}
