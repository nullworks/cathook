/*
 * Achievement.cpp
 *
 *  Created on: Jan 20, 2017
 *      Author: nullifiedcat
 */

#include "Achievement.h"
#include "../common.h"
#include "../sdk.h"

namespace hacks { namespace tf2 { namespace achievement {

CatVar safety(CV_SWITCH, "achievement_safety", "1", "Achievement commands safety switch");

void Lock() {
	if (safety) {
		ConColorMsg({ 255, 0, 0, 255}, "Switch " CON_PREFIX "achievement_safety to 0 before using any achievement commands!\n");
		return;
	}
	g_ISteamUserStats->RequestCurrentStats();
	for (int i = 0; i < g_IAchievementMgr->GetAchievementCount(); i++) {
		g_ISteamUserStats->ClearAchievement(g_IAchievementMgr->GetAchievementByIndex(i)->GetName());
	}
	g_ISteamUserStats->StoreStats();
	g_ISteamUserStats->RequestCurrentStats();
}

void Unlock() {
	if (safety) {
		ConColorMsg({ 255, 0, 0, 255}, "Switch " CON_PREFIX "achievement_safety to 0 before using any achievement commands!\n");
		return;
	}
	for (int i = 0; i < g_IAchievementMgr->GetAchievementCount(); i++) {
		g_IAchievementMgr->AwardAchievement(g_IAchievementMgr->GetAchievementByIndex(i)->GetAchievementID());
	}
}

CatCommand dump_achievement("achievement_dump", "Dump achievements to file (development)", []() {
	std::ofstream out("/tmp/cathook_achievements.txt", std::ios::out);
	if (out.bad()) return;
	for (int i = 0; i < g_IAchievementMgr->GetAchievementCount(); i++) {
		out << '[' << i << "] " << g_IAchievementMgr->GetAchievementByIndex(i)->GetName() << ' ' << g_IAchievementMgr->GetAchievementByIndex(i)->GetAchievementID() << "\n";
	}
	out.close();
});
CatCommand unlock_single("achievement_unlock_single", "Unlocks single achievement by ID", [](const CCommand& args) {
	char* out = nullptr;
	int id = strtol(args.Arg(1), &out, 10);
	if (out == args.Arg(1)) {
		logging::Info("NaN achievement ID!");
		return;
	}
	IAchievement* ach = reinterpret_cast<IAchievement*>(g_IAchievementMgr->GetAchievementByID(id));
	if (ach) {
		g_IAchievementMgr->AwardAchievement(id);
	}
});
// For some reason it SEGV's when I try to GetAchievementByID();
CatCommand lock_single("achievement_lock_single", "Locks single achievement by INDEX!", [](const CCommand& args) {
	char* out = nullptr;
	int index = strtol(args.Arg(1), &out, 10);
	if (out == args.Arg(1)) {
		logging::Info("NaN achievement INDEX!");
		return;
	}
	IAchievement* ach = g_IAchievementMgr->GetAchievementByIndex(index);
	if (ach) {
		g_ISteamUserStats->RequestCurrentStats();
		g_ISteamUserStats->ClearAchievement(ach->GetName());
		g_ISteamUserStats->StoreStats();
		g_ISteamUserStats->RequestCurrentStats();
	}
});
CatCommand lock("achievement_lock", "Lock all achievements", Lock);
CatCommand unlock("achievement_unlock", "Unlock all achievements", Unlock);

}}}
