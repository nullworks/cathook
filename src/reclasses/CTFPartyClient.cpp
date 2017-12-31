/*
 * CTFPartyClient.cpp
 *
 *  Created on: Dec 7, 2017
 *      Author: nullifiedcat
 */

#include "common.hpp"
#include "e8call.hpp"

re::CTFPartyClient *re::CTFPartyClient::GTFPartyClient()
{
    typedef re::CTFPartyClient *(*GTFPartyClient_t)(void);
    static uintptr_t addr = gSignatures.GetClientSignature(
                                "83 04 02 00 00 00 00 00 00 ? 83 08 02 00 00 "
                                "01 E8 ? ? ? ? 89 04 24 E8 ? ? ? ?") +
                            17;
    static GTFPartyClient_t GTFPartyClient_fn =
        GTFPartyClient_t(e8call((void *) addr));

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
    static uintptr_t addr = gSignatures.GetClientSignature(
        "55 89 E5 8B 45 08 8B 50 38 C6 80 BC 01 00 00 01 85 D2 74 06 80 78 44 "
        "00 74 07");
    static MutLocalGroupCriteria_t MutLocalGroupCriteria_fn =
        MutLocalGroupCriteria_t(addr);

    return MutLocalGroupCriteria_fn(client);
}

int re::CTFPartyClient::LoadSavedCasualCriteria(re::CTFPartyClient *client)
{
    typedef int (*LoadSavedCasualCriteria_t)(re::CTFPartyClient *);
    static uintptr_t addr = gSignatures.GetClientSignature(
                                "83 04 02 00 00 00 00 00 00 ? 83 08 02 00 00 "
                                "01 E8 ? ? ? ? 89 04 24 E8 ? ? ? ?") +
                            25;
    static LoadSavedCasualCriteria_t LoadSavedCasualCriteria_fn =
        LoadSavedCasualCriteria_t(e8call((void *) addr));

    return LoadSavedCasualCriteria_fn(client);
}

void re::CTFPartyClient::RequestQueueForMatch(re::CTFPartyClient *client)
{
    typedef void (*RequestQueueForMatch_t)(re::CTFPartyClient *);
    static uintptr_t addr = gSignatures.GetClientSignature(
        "55 89 E5 57 56 53 81 EC 8C 00 00 00 8B 7D 08 80 BF C1 01 00 00 00 0F "
        "85 4F 04 00 00 80 7F 45 00 0F 85 45 04 00 00");
    static RequestQueueForMatch_t RequestQueueForMatch_fn =
        RequestQueueForMatch_t(addr);

    return RequestQueueForMatch_fn(client);
}
