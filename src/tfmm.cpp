/*
 * tfmm.cpp
 *
 *  Created on: Dec 7, 2017
 *      Author: nullifiedcat
 */

#include "common.hpp"

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

void queue_start()
{
    re::CTFPartyClient *client = re::CTFPartyClient::GTFPartyClient();
    if (client)
    {
        re::ITFGroupMatchCriteria::SetMatchGroup(
            re::CTFPartyClient::MutLocalGroupCriteria(client),
            re::ITFGroupMatchCriteria::group::CASUAL);
        re::CTFPartyClient::LoadSavedCasualCriteria(client);
        re::CTFPartyClient::RequestQueueForMatch(client);
    }
    else
    {
    	logging::Info("queue_start: CTFPartyClient == null!");
    }
}

void abandon()
{
    re::CTFGCClientSystem *gc = re::CTFGCClientSystem::GTFGCClientSystem();
    if (gc != nullptr)
        gc->AbandonCurrentMatch();
    else
    	logging::Info("abandon: CTFGCClientSystem == null!");
}
}
