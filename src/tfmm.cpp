/*
 * tfmm.cpp
 *
 *  Created on: Dec 7, 2017
 *      Author: nullifiedcat
 */

#include "common.hpp"
#include "AutoJoin.hpp"

CatCommand cmd_queue_start(XORSTR("mm_queue_casual"), XORSTR("Start casual queue"),
                           []() { tfmm::queue_start(); });

CatCommand cmd_abandon(XORSTR("mm_abandon"), XORSTR("Abandon match"),
                       []() { tfmm::abandon(); });
static CatEnum queue_mode({ XORSTR("MvmPractice"), XORSTR("MvmMannup"), XORSTR("LadderMatch6v6"),
                            XORSTR("LadderMatch9v9"), XORSTR("LadderMatch12v12"),
                            XORSTR("CasualMatch6v6"), XORSTR("CasualMatch9v9"),
                            XORSTR("CasualMatch12v12"), XORSTR("CompetitiveEventMatch12v12") });
static CatVar queue(queue_mode, XORSTR("autoqueue_mode"), XORSTR("7"),
                    XORSTR("Autoqueue for this mode"), XORSTR(""));

CatCommand get_state(XORSTR("mm_state"), XORSTR("Get party state"), []() {
    re::CTFParty *party = re::CTFParty::GetParty();
    if (!party)
    {
        logging::Info(XORSTR("Party == NULL"));
        return;
    }
    logging::Info(XORSTR("State: %d"), re::CTFParty::state_(party));
});

namespace tfmm
{

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
        logging::Info(XORSTR("queue_start: CTFPartyClient == null!"));
}
void queue_leave()
{
    re::CTFPartyClient *client = re::CTFPartyClient::GTFPartyClient();
    if (client)
        client->RequestLeaveForMatch((int) queue);
    else
        logging::Info(XORSTR("queue_start: CTFPartyClient == null!"));
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
        logging::Info(XORSTR("your party client is gay!"));
        if (gc)
            queue_leave();
        else
            logging::Info(XORSTR("your gc is gay!"));
    }
    if (gc && client)
        while (1)
            if (abandont.test_and_set(4000))
            {
                queue_leave();
                break;
            }
}
CatCommand abandoncmd(XORSTR("disconnect_and_abandon"), XORSTR("Disconnect and abandon"),
                      []() { dcandabandon(); });

void abandon()
{
    re::CTFGCClientSystem *gc = re::CTFGCClientSystem::GTFGCClientSystem();
    if (gc != nullptr && gc->BConnectedToMatchServer(false))
        gc->AbandonCurrentMatch();
    else
        logging::Info(XORSTR("abandon: CTFGCClientSystem == null!"));
}
}
