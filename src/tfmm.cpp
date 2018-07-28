/*
 * tfmm.cpp
 *
 *  Created on: Dec 7, 2017
 *      Author: nullifiedcat
 */

#include "common.hpp"
#include "hacks/AutoJoin.hpp"

CatCommand cmd_queue_start("mm_queue_casual", "Start casual queue",
                           []() { tfmm::queue_start(); });

CatCommand cmd_abandon("mm_abandon", "Abandon match",
                       []() { tfmm::abandon(); });

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
CatEnum queue_mode({ "MvmPractice", "MvmMannup", "LadderMatch6v6",
                     "LadderMatch9v9", "LadderMatch12v12", "CasualMatch6v6",
                     "CasualMatch9v9", "CasualMatch12v12",
                     "CompetitiveEventMatch12v12" });
CatVar queue(queue_mode, "autoqueue_mode", "7", "Autoqueue for this mode", "");
void queue_start()
{
    re::CTFPartyClient *client = re::CTFPartyClient::GTFPartyClient();
    if (client)
    {
        if (queue == 7)
            client->LoadSavedCasualCriteria();
        client->RequestQueueForMatch((int) queue);
        hacks::shared::autojoin::queuetime.update();
    }
    else
        logging::Info("queue_start: CTFPartyClient == null!");
}
void queue_leave()
{
    re::CTFPartyClient *client = re::CTFPartyClient::GTFPartyClient();
    if (client)
        client->RequestLeaveForMatch((int) queue);
    else
        logging::Info("queue_start: CTFPartyClient == null!");
}
Timer abandont{};

void dcandabandon()
{
    re::CTFPartyClient *client = re::CTFPartyClient::GTFPartyClient();
    re::CTFGCClientSystem *gc  = re::CTFGCClientSystem::GTFGCClientSystem();
    if (client)
        abandon();
    else
    {
        logging::Info("your party client is gay!");
        if (gc)
            queue_leave();
        else
            logging::Info("your gc is gay!");
    }
    if (gc && client)
        while (1)
            if (abandont.test_and_set(4000))
            {
                queue_leave();
                break;
            }
}
CatCommand abandoncmd("disconnect_and_abandon", "Disconnect and abandon",
                      []() { dcandabandon(); });

void abandon()
{
    re::CTFGCClientSystem *gc = re::CTFGCClientSystem::GTFGCClientSystem();
    if (gc != nullptr && gc->BConnectedToMatchServer(false))
        gc->AbandonCurrentMatch();
    else
        logging::Info("abandon: CTFGCClientSystem == null!");
}
}
