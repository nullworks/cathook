/*
 * AutoJoin.cpp
 *
 *  Created on: Jul 28, 2017
 *      Author: nullifiedcat
 */

#include "common.hpp"
#include "hack.hpp"

namespace hacks::shared::autojoin
{

/*
 * Credits to Blackfire for helping me with auto-requeue!
 */

static CatEnum classes_enum({ "DISABLED", "SCOUT", "SNIPER", "SOLDIER",
                              "DEMOMAN", "MEDIC", "HEAVY", "PYRO", "SPY",
                              "ENGINEER" });
static CatVar autojoin_team(CV_SWITCH, "autojoin_team", "0", "AutoJoin",
                            "Automatically joins a team");
static CatVar preferred_class(classes_enum, "autojoin_class", "0",
                              "AutoJoin class",
                              "You will pick a class automatically");

CatVar auto_queue(CV_SWITCH, "autoqueue", "0", "AutoQueue",
                  "Automatically queue in casual matches");

const std::string classnames[] = { "scout",   "sniper", "soldier",
                                   "demoman", "medic",  "heavyweapons",
                                   "pyro",    "spy",    "engineer" };

bool UnassignedTeam()
{
    return !g_pLocalPlayer->team or (g_pLocalPlayer->team == TEAM_SPEC);
}

bool UnassignedClass()
{
    return g_pLocalPlayer->clazz != int(preferred_class);
}

Timer autoqueue_timer{};
Timer queuetime{};
Timer req_timer{};
/*CatVar party_bypass(CV_SWITCH, "party_bypass", "0", "Party Bypass",
                    "Bypass Party restrictions");*/
void UpdateSearch()
{
    // segfaults for no reason
    /*static bool calld = false;
    if (party_bypass && !calld)
    {
        static unsigned char patch[] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
        uintptr_t party_addr1        = gSignatures.GetClientSignature(
            "55 89 E5 57 56 53 81 EC ? ? ? ? 8B 45 ? 8B 5D ? 8B 55 ? 89 85 ? ? "
            "? ? 65 A1 ? ? ? ? 89 45 ? 31 C0 8B 45");
        static unsigned char patch2[] = { 0x90, 0xE9 };
        uintptr_t party_addr2         = gSignatures.GetClientSignature(
            "55 89 E5 57 56 53 81 EC ? ? ? ? C7 85 ? ? ? ? ? ? ? ? C7 85 ? ? ? "
            "? ? ? ? ? 8B 45 ? 89 85 ? ? ? ? 65 A1 ? ? ? ? 89 45 ? 31 C0 A1 ? "
            "? ? ? 85 C0 0F 84");
        static unsigned char patch3[] = { 0x90, 0x90 };
        uintptr_t party_addr3         = gSignatures.GetClientSignature(
            "8D 65 ? 5B 5E 5F 5D C3 31 F6 8B 87");
        static unsigned char patch4[] = { 0x90, 0xE9 };
        static unsigned char patch5[] = { 0x90, 0x90, 0x90, 0x90 };
        uintptr_t party_addr4         = gSignatures.GetClientSignature(
            "55 89 E5 57 56 53 83 EC ? 8B 5D ? 8B 75 ? 8B 7D ? 89 1C 24 89 74 24
    ? 89 7C 24 ? E8 ? ? ? ? 84 C0 74 ? 83 C4");
        if (!party_addr1 || !party_addr2 || !party_addr3 || !party_addr4)
            logging::Info("Invalid Party signature");
        else
        {
            Patch((void *) (party_addr1 + 0x41), (void *) patch, sizeof(patch));
            Patch((void *) (party_addr2 + 0x9FA), (void *) patch2,
                  sizeof(patch2));
            Patch((void *) (party_addr3 + 0xC5), (void *) patch3,
                  sizeof(patch3));
            Patch((void *) (party_addr3 + 0xF0), (void *) patch4,
                  sizeof(patch4));
            Patch((void *) (party_addr4 + 0x7F), (void *) patch5,
                  sizeof(patch5));
            calld = true;
        }
    }*/
    if (!auto_queue)
        return;
    if (g_IEngine->IsInGame())
        return;

    re::CTFGCClientSystem *gc = re::CTFGCClientSystem::GTFGCClientSystem();
    re::CTFPartyClient *pc    = re::CTFPartyClient::GTFPartyClient();
    if (g_pUserCmd && gc && gc->BConnectedToMatchServer(false) &&
        gc->BHaveLiveMatch())
        tfmm::queue_leave();
    if (gc && !gc->BConnectedToMatchServer(false) &&
        queuetime.test_and_set(10 * 1000 * 60) && !gc->BHaveLiveMatch())
        tfmm::queue_leave();
    if (gc && !gc->BConnectedToMatchServer(false) && !gc->BHaveLiveMatch())
        if (!(pc && pc->BInQueueForMatchGroup(int(tfmm::queue))))
        {
            logging::Info("Starting queue");
            tfmm::queue_start();
        }
#if LAGBOT_MODE
    if (req_timer.test_and_set(1800000))
    {
        logging::Info("Stuck in queue, segfaulting");
        *(int *) nullptr;
        exit(1);
    }
#endif
}

Timer timer{};
void Update()
{
#if not LAGBOT_MODE
    if (timer.test_and_set(500))
    {
        if (autojoin_team and UnassignedTeam())
        {
            hack::ExecuteCommand("autoteam");
        }
        else if (preferred_class and UnassignedClass())
        {
            if (int(preferred_class) < 10)
                g_IEngine->ExecuteClientCmd(
                    format("join_class ", classnames[int(preferred_class) - 1])
                        .c_str());
        }
    }
#endif
}
}
