/*
 * AutoJoin.cpp
 *
 *  Created on: Jul 28, 2017
 *      Author: nullifiedcat
 */

#include "common.hpp"
#include "hack.hpp"

namespace hacks
{
namespace shared
{
namespace autojoin
{

/*
 * Credits to Blackfire for helping me with auto-requeue!
 */

static CatEnum classes_enum({ XORSTR("DISABLED"), XORSTR("SCOUT"), XORSTR("SNIPER"), XORSTR("SOLDIER"),
                              XORSTR("DEMOMAN"), XORSTR("MEDIC"), XORSTR("HEAVY"), XORSTR("PYRO"), XORSTR("SPY"),
                              XORSTR("ENGINEER") });
static CatVar autojoin_team(CV_SWITCH, XORSTR("autojoin_team"), XORSTR("0"), XORSTR("AutoJoin"),
                            XORSTR("Automatically joins a team"));
static CatVar preferred_class(classes_enum, XORSTR("autojoin_class"), XORSTR("0"),
                              XORSTR("AutoJoin class"),
                              XORSTR("You will pick a class automatically"));

CatVar auto_queue(CV_SWITCH, XORSTR("autoqueue"), XORSTR("0"), XORSTR("AutoQueue"),
                  XORSTR("Automatically queue in casual matches"));

const std::string classnames[] = { XORSTR("scout"),   XORSTR("sniper"), XORSTR("soldier"),
                                   XORSTR("demoman"), XORSTR("medic"),  XORSTR("heavyweapons"),
                                   XORSTR("pyro"),    XORSTR("spy"),    XORSTR("engineer") };

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
/*CatVar party_bypass(CV_SWITCH, XORSTR("party_bypass"), XORSTR("0"), XORSTR("Party Bypass"),
                    XORSTR("Bypass Party restrictions"));*/
void UpdateSearch()
{
    // segfaults for no reason
    /*static bool calld = false;
    if (party_bypass && !calld)
    {
        static unsigned char patch[] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
        uintptr_t party_addr1        = gSignatures.GetClientSignature(
            XORSTR("55 89 E5 57 56 53 81 EC ? ? ? ? 8B 45 ? 8B 5D ? 8B 55 ? 89 85 ? ? ")
            XORSTR("? ? 65 A1 ? ? ? ? 89 45 ? 31 C0 8B 45"));
        static unsigned char patch2[] = { 0x90, 0xE9 };
        uintptr_t party_addr2         = gSignatures.GetClientSignature(
            XORSTR("55 89 E5 57 56 53 81 EC ? ? ? ? C7 85 ? ? ? ? ? ? ? ? C7 85 ? ? ? ")
            XORSTR("? ? ? ? ? 8B 45 ? 89 85 ? ? ? ? 65 A1 ? ? ? ? 89 45 ? 31 C0 A1 ? ")
            XORSTR("? ? ? 85 C0 0F 84"));
        static unsigned char patch3[] = { 0x90, 0x90 };
        uintptr_t party_addr3         = gSignatures.GetClientSignature(
            XORSTR("8D 65 ? 5B 5E 5F 5D C3 31 F6 8B 87"));
        static unsigned char patch4[] = { 0x90, 0xE9 };
        static unsigned char patch5[] = { 0x90, 0x90, 0x90, 0x90 };
        uintptr_t party_addr4         = gSignatures.GetClientSignature(
            XORSTR("55 89 E5 57 56 53 83 EC ? 8B 5D ? 8B 75 ? 8B 7D ? 89 1C 24 89 74 24
    ? 89 7C 24 ? E8 ? ? ? ? 84 C0 74 ? 83 C4XORSTR(");
        if (!party_addr1 || !party_addr2 || !party_addr3 || !party_addr4)
            logging::Info(XORSTR("Invalid Party signature"));
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
    if (g_pUserCmd && gc && gc->BConnectedToMatchServer(false))
        tfmm::queue_leave();
    if (autoqueue_timer.test_and_set(60000))
    {
        if (!gc->BConnectedToMatchServer(false) &&
            queuetime.test_and_set(10 * 1000 * 60))
            tfmm::queue_leave();
        if (gc && !gc->BConnectedToMatchServer(false))
        {
            logging::Info(XORSTR("Starting queue"));
            tfmm::queue_start();
        }
    }
    if (req_timer.test_and_set(1800000))
    {
        logging::Info(XORSTR("Starting queue"));
        tfmm::queue_start();
    }
}

Timer timer{};
void Update()
{
#if not LAGBOT_MODE
    if (timer.test_and_set(500))
    {
        if (autojoin_team and UnassignedTeam())
        {
            hack::ExecuteCommand(XORSTR("jointeam auto"));
        }
        else if (preferred_class and UnassignedClass())
        {
            if (int(preferred_class) < 10)
                g_IEngine->ExecuteClientCmd(
                    format(XORSTR("join_class "), classnames[int(preferred_class) - 1])
                        .c_str());
        }
    }
#endif
}
}
}
}
