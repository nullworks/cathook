/*
 * AntiCheat.cpp
 *
 *  Created on: Jun 5, 2017
 *      Author: nullifiedcat
 */

#include "AntiCheat.hpp"
#include "../common.h"

#include "../hack.h"

#include "ac/aimbot.hpp"
#include "ac/antiaim.hpp"
#include "ac/removecond.hpp"
#include "ac/bhop.hpp"

namespace hacks { namespace shared { namespace anticheat {

static CatVar enabled(CV_SWITCH, "ac_enabled", "0", "Enable AC");
static CatVar accuse_chat(CV_SWITCH, "ac_chat", "0", "Accuse in chat");

void Accuse(int eid, const std::string& hack, const std::string& details) {
	player_info_s info;
	if (g_IEngine->GetPlayerInfo(eid, &info)) {
		CachedEntity* ent = ENTITY(eid);
		if (accuse_chat) {
			hack::command_stack().push(format("say \"", info.name, " (", classname(CE_INT(ent, netvar.iClass)), ") suspected ", hack, ": ", details,"\""));
		} else {
			PrintChat("\x07%06X%s\x01 (%s) suspected \x07%06X%s\x01: %s", colors::chat::team(ENTITY(eid)->m_iTeam), info.name, classname(CE_INT(ent, netvar.iClass)), 0xe05938, hack.c_str(), details.c_str());
		}
	}
}

static CatVar skip_local(CV_SWITCH, "ac_ignore_local", "1", "Ignore Local");

void CreateMove() {
	if (!enabled) return;
	for (int i = 1; i < 33; i++) {
		if (skip_local && (i == g_IEngine->GetLocalPlayer())) continue;
		CachedEntity* ent = ENTITY(i);
		if (CE_GOOD(ent)) {
			if ((CE_BYTE(ent, netvar.iLifeState) == 0)) {
				ac::aimbot::Update(ent);
				ac::antiaim::Update(ent);
				ac::bhop::Update(ent);
			}
		}
		ac::removecond::Update(ent);
	}
}

void ResetPlayer(int index) {
	ac::aimbot::ResetPlayer(index);
	ac::antiaim::ResetPlayer(index);
	ac::bhop::ResetPlayer(index);
	ac::removecond::ResetPlayer(index);
}

void ResetEverything() {
	ac::aimbot::ResetEverything();
	ac::antiaim::ResetEverything();
	ac::bhop::ResetEverything();
	ac::removecond::ResetEverything();
}

class ACListener : public IGameEventListener {
public:
	virtual void FireGameEvent(KeyValues* event) {
		if (!enabled) return;
		std::string name(event->GetName());
		if (name == "player_activate") {
			int uid = event->GetInt("userid");
			int entity = g_IEngine->GetPlayerForUserID(uid);
			ResetPlayer(entity);
		} else if (name == "player_disconnect") {
			int uid = event->GetInt("userid");
			int entity = g_IEngine->GetPlayerForUserID(uid);
			ResetPlayer(entity);
		}

		ac::aimbot::Event(event);
	}
};

ACListener listener;

void Init() {
	// FIXME free listener
	g_IGameEventManager->AddListener(&listener, false);
}

}}}
