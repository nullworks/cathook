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
    uintptr_t addr = gSignatures.GetClientSignature(
        XORSTR("55 A1 ? ? ? ? 89 E5 5D C3 8D B6 00 00 00 00 A1 ? ? ? ? 85 C0"));
    GTFPartyClient_t GTFPartyClient_fn = GTFPartyClient_t(addr);

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
        XORSTR("55 89 E5 53 83 EC 24 8B 5D 08 80 7B 46 00 75 40 8B 4B 38 85 C9 74 39 "
        "E8 ? ? ? ? 89 04 24 E8 ? ? ? ? 84 C0 75 28"));
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
        gSignatures.GetClientSignature(XORSTR("55 89 E5 8B 45 ? 8B 50 ? C6 80"));
    static MutLocalGroupCriteria_t MutLocalGroupCriteria_fn =
        MutLocalGroupCriteria_t(addr);

    return MutLocalGroupCriteria_fn(client);
}

int re::CTFPartyClient::LoadSavedCasualCriteria()
{
    typedef int (*LoadSavedCasualCriteria_t)(re::CTFPartyClient *);
    uintptr_t addr = gSignatures.GetClientSignature(
        XORSTR("55 89 E5 83 EC ? 8B 45 ? 8B 50 ? C6 80"));
    LoadSavedCasualCriteria_t LoadSavedCasualCriteria_fn =
        LoadSavedCasualCriteria_t(addr);

    return LoadSavedCasualCriteria_fn(this);
}

char re::CTFPartyClient::RequestQueueForMatch(int type)
{
    typedef char (*RequestQueueForMatch_t)(re::CTFPartyClient *, int);
    uintptr_t addr = gSignatures.GetClientSignature(
        XORSTR("55 89 E5 57 56 53 81 EC ? ? ? ? 8B 45 ? E8"));
    RequestQueueForMatch_t RequestQueueForMatch_fn =
        RequestQueueForMatch_t(addr);

    return RequestQueueForMatch_fn(this, type);
}
char re::CTFPartyClient::RequestLeaveForMatch(int type)
{
    typedef char (*RequestLeaveForMatch_t)(re::CTFPartyClient *, int);
    uintptr_t addr = gSignatures.GetClientSignature(
        XORSTR("55 89 E5 57 56 53 83 EC ? 8B 45 ? 89 44 24 ? 8B 45 ? 89 04 24 E8 ? ? "
        "? ? 84 C0 89 C6 75"));
    RequestLeaveForMatch_t RequestLeaveForMatch_fn =
        RequestLeaveForMatch_t(addr);

    return RequestLeaveForMatch_fn(this, type);
}
