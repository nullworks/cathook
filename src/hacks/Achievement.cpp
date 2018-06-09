/*
 * Achievement.cpp
 *
 *  Created on: Jan 20, 2017
 *      Author: nullifiedcat
 */

#include "common.hpp"

namespace hacks::tf2::achievement
{

static CatVar safety(CV_SWITCH, XORSTR("achievement_safety"), XORSTR("1"),
                     XORSTR("Achievement commands safety switch"));

void Lock()
{
    if (safety)
    {
        ConColorMsg(
            { 255, 0, 0, 255 },
            "Switch " CON_PREFIX
            "achievement_safety to 0 before using any achievement commands!\n");
        return;
    }
    g_ISteamUserStats->RequestCurrentStats();
    for (int i = 0; i < g_IAchievementMgr->GetAchievementCount(); i++)
    {
        g_ISteamUserStats->ClearAchievement(
            g_IAchievementMgr->GetAchievementByIndex(i)->GetName());
    }
    g_ISteamUserStats->StoreStats();
    g_ISteamUserStats->RequestCurrentStats();
}

void Unlock()
{
    /*auto Invmng = re::CTFInventoryManager::GTFInventoryManager();
    auto Inv = re::CTFPlayerInventory::GTFPlayerInventory();
    auto Item = Inv->GetFirstItemOfItemDef(59);
    Invmng->EquipItemInLoadout(0, 0, (unsigned long long
    int)Item->uniqueid());*/
    if (safety)
    {
        ConColorMsg(
            { 255, 0, 0, 255 },
            "Switch " CON_PREFIX
            "achievement_safety to 0 before using any achievement commands!\n");
        return;
    }
    for (int i = 0; i < g_IAchievementMgr->GetAchievementCount(); i++)
    {
        g_IAchievementMgr->AwardAchievement(
            g_IAchievementMgr->GetAchievementByIndex(i)->GetAchievementID());
    }
}

CatCommand dump_achievement(
    XORSTR("achievement_dump"), XORSTR("Dump achievements to file (development)"), []() {
        std::ofstream out(XORSTR("/tmp/cathook_achievements.txt"), std::ios::out);
        if (out.bad())
            return;
        for (int i = 0; i < g_IAchievementMgr->GetAchievementCount(); i++)
        {
            out << '[' << i << XORSTR("] ")
                << g_IAchievementMgr->GetAchievementByIndex(i)->GetName() << ' '
                << g_IAchievementMgr->GetAchievementByIndex(i)
                       ->GetAchievementID()
                << XORSTR("\n");
        }
        out.close();
    });
CatCommand unlock_single(XORSTR("achievement_unlock_single"),
                         XORSTR("Unlocks single achievement by ID"),
                         [](const CCommand &args) {
                             char *out = nullptr;
                             int id    = strtol(args.Arg(1), &out, 10);
                             if (out == args.Arg(1))
                             {
                                 logging::Info(XORSTR("NaN achievement ID!"));
                                 return;
                             }
                             IAchievement *ach =
                                 reinterpret_cast<IAchievement *>(
                                     g_IAchievementMgr->GetAchievementByID(id));
                             if (ach)
                             {
                                 g_IAchievementMgr->AwardAchievement(id);
                             }
                         });
// For some reason it SEGV's when I try to GetAchievementByID();
CatCommand
    lock_single(XORSTR("achievement_lock_single"), XORSTR("Locks single achievement by INDEX!"),
                [](const CCommand &args) {
                    char *out = nullptr;
                    int index = strtol(args.Arg(1), &out, 10);
                    if (out == args.Arg(1))
                    {
                        logging::Info(XORSTR("NaN achievement INDEX!"));
                        return;
                    }
                    IAchievement *ach =
                        g_IAchievementMgr->GetAchievementByIndex(index);
                    if (ach)
                    {
                        g_ISteamUserStats->RequestCurrentStats();
                        g_ISteamUserStats->ClearAchievement(ach->GetName());
                        g_ISteamUserStats->StoreStats();
                        g_ISteamUserStats->RequestCurrentStats();
                    }
                });
CatCommand lock(XORSTR("achievement_lock"), XORSTR("Lock all achievements"), Lock);
CatCommand unlock(XORSTR("achievement_unlock"), XORSTR("Unlock all achievements"), Unlock);
}
