/*
 * KillSay.cpp
 *
 *  Created on: Jan 19, 2017
 *      Author: nullifiedcat
 */

#include "KillSay.h"
#include "../common.h"
#include "../sdk.h"

namespace hacks { namespace shared { namespace killsay {

static CatEnum killsay_enum({"NONE", "CUSTOM", "DEFAULT", "NCC - OFFENSIVE", "NCC - MLG"});
static CatVar killsay_mode(killsay_enum, "killsay", "0", "Killsay", "Defines source of killsay lines. CUSTOM killsay file must be set in cat_killsay_file and loaded with cat_killsay_reload (Use console!)");
static CatVar filename(CV_STRING, "killsay_file", "killsays.txt", "Killsay file", "Killsay file name. Should be located in cathook data folder");
static CatCommand reload("killsay_reload", "Reload killsays", Reload);

const std::string tf_classes_killsay[] = {
	"class",
	"scout",
	"sniper",
	"soldier",
	"demoman",
	"medic",
	"heavy",
	"pyro",
	"spy",
	"engineer"
};

const std::string tf_teams_killsay[] = {
	"RED",
	"BLU"
};

TextFile file {};

std::string ComposeKillSay(IGameEvent* event) {
	const std::vector<std::string>* source = nullptr;
	switch ((int)killsay_mode) {
	case 1:
		source = &file.lines; break;
	case 2:
		source = &builtin_default; break;
	case 3:
		source = &builtin_nonecore_offensive; break;
	case 4	:
		source = &builtin_nonecore_mlg; break;
	}
	if (!source || source->size() == 0) return "";
	if (!event) return "";
	int vid = event->GetInt("userid");
	int kid = event->GetInt("attacker");
	if (kid == vid) return "";
	if (g_IEngine->GetPlayerForUserID(kid) != g_IEngine->GetLocalPlayer()) return "";
	std::string msg = source->at(rand() % source->size());
	player_info_s info;
	g_IEngine->GetPlayerInfo(g_IEngine->GetPlayerForUserID(vid), &info);
	ReplaceString(msg, "%name%", std::string(info.name));
	CachedEntity* ent = ENTITY(g_IEngine->GetPlayerForUserID(vid));
	int clz = g_pPlayerResource->GetClass(ent);
	ReplaceString(msg, "%class%", tf_classes_killsay[clz]);
	player_info_s infok;
	g_IEngine->GetPlayerInfo(g_IEngine->GetPlayerForUserID(kid), &infok);
	ReplaceString(msg, "%killer%", std::string(infok.name));
	ReplaceString(msg, "%team%", tf_teams_killsay[ent->m_iTeam - 2]);
	ReplaceString(msg, "%myteam%", tf_teams_killsay[LOCAL_E->m_iTeam - 2]);
	ReplaceString(msg, "%myclass%", tf_classes_killsay[g_pPlayerResource->GetClass(LOCAL_E)]);
	ReplaceString(msg, "\\n", "\n");
	return msg;
}

KillSayEventListener& getListener() {
	static KillSayEventListener listener;
	return listener;
}

void Reload() {
	file.Load(std::string(filename.GetString()));
}

void Init() {
	g_IEventManager2->AddListener(&getListener(), (const char*)"player_death", false);
	filename.InstallChangeCallback([](IConVar* var, const char* pszOV, float flOV) {
		file.TryLoad(std::string(filename.GetString()));
	});
}

void Shutdown() {
	g_IEventManager2->RemoveListener(&getListener());
}

// Thanks HellJustFroze for linking me http://daviseford.com/shittalk/
const std::vector<std::string> builtin_default = {
	"Don't worry guys, I'm a garbage collector. I'm used to carrying trash.",
	"%name% is the human equivalent of a participation award.",
	"I would insult %name%, but nature did a better job.",
	"%name%, perhaps your strategy should include trying.",
	"Some people get paid to suck, you do it for free, %name%.",
	"%name%, I'd tell you to commit suicide, but then you'd have a kill.",
	"You must really like that respawn timer, %name%.",

	"If your main is %class%, you should give up.",
	"Hey %name%, i see you can't play %class%. Try quitting the game."
	"%team% is filled with spergs",
	"%name%@gmail.com to vacreview@valvesoftware.com\nFOUND CHEATER",
	"\n☐ Not rekt\n ☑ Rekt\n ☑ Really Rekt\n ☑ Tyrannosaurus Rekt"
};

const std::vector<std::string> builtin_nonecore_offensive = {
		"%name%, you are noob.", "%name%, do you even lift?", "%name%, you're a faggot.", "%name%, stop cheating.",
		"%name%: Mom, call the police - I've got headshoted again!", "Right into your face, %name%.",
		"Into your face, pal.", "Keep crying, baby.", "Faggot. Noob.", "You are dead, not big surprise.",
		"Sit down nerd.", "Fuck you with a rake.", "Eat a man spear, you Jamaican manure salesman.",
		"Wallow in a river of cocks, you pathetic bitch.", "I will go to heaven and you will be in prison.",
		"Piss off, you poor, ignorant, mullet-wearing porch monkey.",
		"Your Mom says your turn-ons consist of butthole licking and scat porn.",
		"Shut up, you'll never be the man your mother is.",
		"It looks like your face caught on fire and someone tried to put it out with a fork.",
		"You're so ugly Hello Kitty said goodbye to you.",
		"Don't you love nature, despite what it did to you?"

};
const std::vector<std::string> builtin_nonecore_mlg = {
		"GET REKT U SCRUB", "GET REKT M8", "U GOT NOSCOPED M8", "U GOT QUICKSCOPED M8", "2 FAST 4 U, SCRUB", "U GOT REKT, M8"
};

}}}

void KillSayEventListener::FireGameEvent(IGameEvent* event) {
	if (!hacks::shared::killsay::killsay_mode) return;
	std::string message = hacks::shared::killsay::ComposeKillSay(event);
	if (message.size()) {
		chat_stack::Say(message);
	}
}
