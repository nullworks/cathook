/*
 * AutoJoin.cpp
 *
 *  Created on: Jul 28, 2017
 *      Author: nullifiedcat
 */

#include "../common.h"
#include "../hack.h"
#include "../copypasted/CSignature.h"

namespace hacks { namespace shared { namespace autojoin {

/*
 * Credits to Blackfire for helping me with auto-requeue!
 */

CatEnum classes_enum({ "DISABLED", "SCOUT", "SNIPER", "SOLDIER", "DEMOMAN", "MEDIC", "HEAVY", "PYRO", "SPY", "ENGINEER" });
CatVar autojoin_team(CV_SWITCH, "autojoin_team", "0", "AutoJoin", "Automatically joins a team");
CatVar preferred_class(classes_enum, "autojoin_class", "0", "AutoJoin class", "You will pick a class automatically");

CatVar auto_queue(CV_SWITCH, "autoqueue", "0", "AutoQueue", "Automatically queue in casual matches");

const std::string classnames[] = {
	"scout", "sniper", "soldier", "demoman", "medic", "heavyweapons", "pyro", "spy", "engineer"
};

CatCommand debug_startsearch("debug_startsearch", "DEBUG StartSearch", []() {
	logging::Info("%d", g_TFGCClientSystem->RequestSelectWizardStep(4));
});
CatCommand debug_casual("debug_casual", "DEBUG Casual", []() {
	g_IEngine->ExecuteClientCmd("OpenMatchmakingLobby casual");
	g_TFGCClientSystem->LoadSearchCriteria();
	//logging::Info("%d", g_TFGCClientSystem->RequestSelectWizardStep(6));
});

CatCommand debug_readytosearch("debug_gcstate", "DEBUG GCState", []() {
	logging::Info("%d", g_TFGCClientSystem->GetState());
});
CatCommand debug_abandon("debug_abandon", "DEBUG Abandon", []() {
	g_TFGCClientSystem->SendExitMatchmaking(true);
});
bool UnassignedTeam() {
	return !g_pLocalPlayer->team or (g_pLocalPlayer->team == TEAM_SPEC);
}

bool UnassignedClass() {
	return g_pLocalPlayer->clazz != int(preferred_class);
}

void UpdateSearch() {
	if (!auto_queue) return;
	if (g_IEngine->IsInGame()) return;
	static auto last_check = std::chrono::system_clock::now();
	auto s = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - last_check).count();
	if (s < 4) return;

	if (g_TFGCClientSystem->GetState() == 6) {
		logging::Info("Sending MM request");
		g_TFGCClientSystem->RequestSelectWizardStep(4);
	} else if (g_TFGCClientSystem->GetState() == 5) {
		g_IEngine->ExecuteClientCmd("OpenMatchmakingLobby casual");
		g_TFGCClientSystem->LoadSearchCriteria();
		//logging::Info("%d", g_TFGCClientSystem->RequestSelectWizardStep(6));
	}

	last_check = std::chrono::system_clock::now();
}

void Update() {
	static auto last_check = std::chrono::system_clock::now();
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - last_check).count();

	if (ms < 500) {
		return;
	}

	if (autojoin_team and UnassignedTeam()) {
		hack::ExecuteCommand("jointeam auto");
	} else if (preferred_class and UnassignedClass()) {
		if (int(preferred_class) < 10)
		g_IEngine->ExecuteClientCmd(format("join_class ", classnames[int(preferred_class) - 1]).c_str());
	}

	last_check = std::chrono::system_clock::now();
}

}}}
