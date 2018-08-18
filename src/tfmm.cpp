/*
 * tfmm.cpp
 *
 *  Created on: Dec 7, 2017
 *      Author: nullifiedcat
 */

#include <settings/Int.hpp>
#include "common.hpp"
#include "hacks/AutoJoin.hpp"

settings::Int queue{ "autoqueue.mode", "7" };

CatCommand cmd_queue_start("mm_queue_casual", "Start casual queue",
                           []() { tfmm::startQueue(); });

CatCommand cmd_abandon("mm_abandon", "Abandon match",
                       []() { tfmm::abandon(); });

CatCommand abandoncmd("disconnect_and_abandon", "Disconnect and abandon",
                      []() { tfmm::disconnectAndAbandon(); });

CatCommand get_state("mm_state", "Get party state", []() {
    re::CTFParty *party = re::CTFParty::GetParty();
    if (!party)
    {
        logging::Info("Party == NULL");
        return;
    }
    logging::Info("State: %d", re::CTFParty::state_(party));
});

namespace tfmm
{

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
        hacks::shared::autojoin::resetQueueTimer();
    }
    else
        logging::Info("queue_start: CTFPartyClient == null!");
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
    re::CTFPartyClient *client = re::CTFPartyClient::GTFPartyClient();
    re::CTFGCClientSystem *gc  = re::CTFGCClientSystem::GTFGCClientSystem();
    if (client)
        abandon();
    else
    {
        logging::Info("your party client is gay!");
        if (gc)
            leaveQueue();
        else
            logging::Info("your gc is gay!");
    }
    if (gc && client)
        leaveQueue();
}

void abandon()
{
    re::CTFGCClientSystem *gc = re::CTFGCClientSystem::GTFGCClientSystem();
    if (gc != nullptr && gc->BConnectedToMatchServer(false))
        gc->AbandonCurrentMatch();
    else
        logging::Info("abandon: CTFGCClientSystem == null!");
}
} // namespace tfmm
