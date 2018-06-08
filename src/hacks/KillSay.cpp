/*
 * KillSay.cpp
 *
 *  Created on: Jan 19, 2017
 *      Author: nullifiedcat
 */

#include <hacks/KillSay.hpp>
#include "common.hpp"

namespace hacks
{
namespace shared
{
namespace killsay
{

static CatEnum killsay_enum({ XORSTR("NONE"), XORSTR("CUSTOM"), XORSTR("DEFAULT"), XORSTR("NCC - OFFENSIVE"),
                              XORSTR("NCC - MLG") });
static CatVar killsay_mode(killsay_enum, XORSTR("killsay"), XORSTR("0"), XORSTR("Killsay"),
                           XORSTR("Defines source of killsay lines. CUSTOM killsay ")
                           XORSTR("file must be set in cat_killsay_file and loaded ")
                           XORSTR("with cat_killsay_reload (Use console!)"));
static CatVar
    filename(CV_STRING, XORSTR("killsay_file"), XORSTR("killsays.txt"), XORSTR("Killsay file"),
             XORSTR("Killsay file name. Should be located in cathook data folder"));
static CatCommand reload(XORSTR("killsay_reload"), XORSTR("Reload killsays"), Reload);

const std::string tf_classes_killsay[] = { XORSTR("class"),   XORSTR("scout"),   XORSTR("sniper"),
                                           XORSTR("soldier"), XORSTR("demoman"), XORSTR("medic"),
                                           XORSTR("heavy"),   XORSTR("pyro"),    XORSTR("spy"),
                                           XORSTR("engineer") };

const std::string tf_teams_killsay[] = { XORSTR("RED"), XORSTR("BLU") };

TextFile file{};

std::string ComposeKillSay(IGameEvent *event)
{
    const std::vector<std::string> *source = nullptr;
    switch ((int) killsay_mode)
    {
    case 1:
        source = &file.lines;
        break;
    case 2:
        source = &builtin_default;
        break;
    case 3:
        source = &builtin_nonecore_offensive;
        break;
    case 4:
        source = &builtin_nonecore_mlg;
        break;
    }
    if (!source || source->size() == 0)
        return XORSTR("");
    if (!event)
        return XORSTR("");
    int vid = event->GetInt(XORSTR("userid"));
    int kid = event->GetInt(XORSTR("attacker"));
    if (kid == vid)
        return XORSTR("");
    if (g_IEngine->GetPlayerForUserID(kid) != g_IEngine->GetLocalPlayer())
        return XORSTR("");
    std::string msg = source->at(rand() % source->size());
    player_info_s info;
    g_IEngine->GetPlayerInfo(g_IEngine->GetPlayerForUserID(vid), &info);
    ReplaceString(msg, XORSTR("%name%"), std::string(info.name));
    CachedEntity *ent = ENTITY(g_IEngine->GetPlayerForUserID(vid));
    int clz           = g_pPlayerResource->GetClass(ent);
    ReplaceString(msg, XORSTR("%class%"), tf_classes_killsay[clz]);
    player_info_s infok;
    g_IEngine->GetPlayerInfo(g_IEngine->GetPlayerForUserID(kid), &infok);
    ReplaceString(msg, XORSTR("%killer%"), std::string(infok.name));
    ReplaceString(msg, XORSTR("%team%"), tf_teams_killsay[ent->m_iTeam() - 2]);
    ReplaceString(msg, XORSTR("%myteam%"), tf_teams_killsay[LOCAL_E->m_iTeam() - 2]);
    ReplaceString(msg, XORSTR("%myclass%"),
                  tf_classes_killsay[g_pPlayerResource->GetClass(LOCAL_E)]);
    ReplaceString(msg, XORSTR("\\n"), XORSTR("\n"));
    return msg;
}

KillSayEventListener &getListener()
{
    static KillSayEventListener listener;
    return listener;
}

void Reload()
{
    file.Load(std::string(filename.GetString()));
}

void Init()
{
    g_IEventManager2->AddListener(&getListener(), (const char *) XORSTR("player_death"),
                                  false);
    filename.InstallChangeCallback(
        [](IConVar *var, const char *pszOV, float flOV) {
            file.TryLoad(std::string(filename.GetString()));
        });
}

void Shutdown()
{
    g_IEventManager2->RemoveListener(&getListener());
}

// Thanks HellJustFroze for linking me http://daviseford.com/shittalk/
const std::vector<std::string> builtin_default = {
    XORSTR("Don't worry guys, I'm a garbage collector. I'm used to carrying trash."),
    XORSTR("%name% is the human equivalent of a participation award."),
    XORSTR("I would insult %name%, but nature did a better job."),
    XORSTR("%name%, perhaps your strategy should include trying."),
    XORSTR("Some people get paid to suck, you do it for free, %name%."),
    XORSTR("%name%, I'd tell you to commit suicide, but then you'd have a kill."),
    XORSTR("You must really like that respawn timer, %name%."),

    XORSTR("If your main is %class%, you should give up."),
    XORSTR("Hey %name%, i see you can't play %class%. Try quitting the game.")
    XORSTR("%team% is filled with spergs"),
    XORSTR("%name%@gmail.com to vacreview@valvesoftware.com\nFOUND CHEATER"),
    XORSTR("\n☐ Not rekt\n ☑ Rekt\n ☑ Really Rekt\n ☑ Tyrannosaurus Rekt")
};

const std::vector<std::string> builtin_nonecore_offensive = {
    XORSTR("%name%, you are noob."),
    XORSTR("%name%, do you even lift?"),
    XORSTR("%name%, you're a faggot."),
    XORSTR("%name%, stop cheating."),
    XORSTR("%name%: Mom, call the police - I've got headshoted again!"),
    XORSTR("Right into your face, %name%."),
    XORSTR("Into your face, pal."),
    XORSTR("Keep crying, baby."),
    XORSTR("Faggot. Noob."),
    XORSTR("You are dead, not big surprise."),
    XORSTR("Sit down nerd."),
    XORSTR("Fuck you with a rake."),
    XORSTR("Eat a man spear, you Jamaican manure salesman."),
    XORSTR("Wallow in a river of cocks, you pathetic bitch."),
    XORSTR("I will go to heaven and you will be in prison."),
    XORSTR("Piss off, you poor, ignorant, mullet-wearing porch monkey."),
    XORSTR("Your Mom says your turn-ons consist of butthole licking and scat porn."),
    XORSTR("Shut up, you'll never be the man your mother is."),
    XORSTR("It looks like your face caught on fire and someone tried to put it out ")
    XORSTR("with a fork."),
    XORSTR("You're so ugly Hello Kitty said goodbye to you."),
    XORSTR("Don't you love nature, despite what it did to you?")

};
const std::vector<std::string> builtin_nonecore_mlg = {
    XORSTR("GET REKT U SCRUB"),     XORSTR("GET REKT M8"),       XORSTR("U GOT NOSCOPED M8"),
    XORSTR("U GOT QUICKSCOPED M8"), XORSTR("2 FAST 4 U, SCRUB"), XORSTR("U GOT REKT, M8")
};
}
}
}

void KillSayEventListener::FireGameEvent(IGameEvent *event)
{
    if (!hacks::shared::killsay::killsay_mode)
        return;
    std::string message = hacks::shared::killsay::ComposeKillSay(event);
    if (message.size())
    {
        chat_stack::Say(message, false);
    }
}
