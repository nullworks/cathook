/*
 * KillSay.cpp
 *
 *  Created on: Jan 19, 2017
 *      Author: nullifiedcat
 */

#include <settings/Int.hpp>
#include "common.hpp"

namespace hacks::shared::killsay
{
static settings::Int killsay_mode{ "killsay.mode", "0" };
static settings::String filename{ "killsay.file", "killsay.txt" };
static settings::Int delay{ "killsay.delay", "100" };

struct KillsayStorage
{
    Timer timer{};
    unsigned delay{};
    std::string message{};
};

static std::unordered_map<int, KillsayStorage> killsay_storage{};

// Thanks HellJustFroze for linking me http://daviseford.com/shittalk/
const std::vector<std::string> builtin_default = { "Don't worry guys, I'm a garbage collector. I'm used to carrying trash.",
                                                   "%name% is the human equivalent of a participation award.",
                                                   "I would insult %name%, but nature did a better job.",
                                                   "%name%, perhaps your strategy should include trying.",
                                                   "Some people get paid to suck, you do it for free, %name%.",
                                                   "%name%, I'd tell you to commit suicide, but then you'd have a kill.",
                                                   "You must really like that respawn timer, %name%.",

                                                   "If your main is %class%, you should give up.",
                                                   "Hey %name%, i see you can't play %class%. Try quitting the game.",
                                                   "%team% is filled with spergs",
                                                   "%name%@gmail.com to vacreview@valvesoftware.com\nFOUND CHEATER",
                                                   "\n☐ Not rekt\n ☑ Rekt\n ☑ Really Rekt\n ☑ Tyrannosaurus Rekt" };

const std::vector<std::string> builtin_nonecore_offensive = { "%name%, you are noob.",
                                                              "%name%, do you even lift?",
                                                              "%name%, you're a faggot.",
                                                              "%name%, stop cheating.",
                                                              "%name%: Mom, call the police - I've got headshoted again!",
                                                              "Right into your face, %name%.",
                                                              "Into your face, pal.",
                                                              "Keep crying, baby.",
                                                              "Faggot. Noob.",
                                                              "You are dead, not big surprise.",
                                                              "Sit down nerd.",
                                                              "Fuck you with a rake.",
                                                              "Eat a man spear, you Jamaican manure salesman.",
                                                              "Wallow in a river of cocks, you pathetic bitch.",
                                                              "I will go to heaven and you will be in prison.",
                                                              "Piss off, you poor, ignorant, mullet-wearing porch monkey.",
                                                              "Your Mom says your turn-ons consist of butthole licking and scat porn.",
                                                              "Shut up, you'll never be the man your mother is.",
                                                              "It looks like your face caught on fire and someone tried to put it out "
                                                              "with a fork.",
                                                              "You're so ugly Hello Kitty said goodbye to you.",
                                                              "Don't you love nature, despite what it did to you?"

};
const std::vector<std::string> builtin_nonecore_mlg = { "GET REKT U SCRUB", "GET REKT M8", "U GOT NOSCOPED M8", "U GOT QUICKSCOPED M8", "2 FAST 4 U, SCRUB", "U GOT REKT, M8" };
const std::string tf_classes_killsay[]              = { "class", "scout", "sniper", "soldier", "demoman", "medic", "heavy", "pyro", "spy", "engineer" };
const std::string tf_teams_killsay[]                = { "RED", "BLU" };
static std::string lastmsg{};

TextFile file{};

std::string ComposeKillSay(IGameEvent *event)
{
    const std::vector<std::string> *source = nullptr;
    switch (*killsay_mode)
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
    default:
        break;
    }
    if (!source || source->empty())
        return "";
    if (!event)
        return "";
    int vid = event->GetInt("userid");
    int kid = event->GetInt("attacker");
    if (kid == vid)
        return "";
    if (g_IEngine->GetPlayerForUserID(kid) != g_IEngine->GetLocalPlayer())
        return "";
    std::string msg = source->at(rand() % source->size());
    //	checks if the killsay.txt file is not 1 line. 100% sure it's going
    // to crash if it is.
    while (msg == lastmsg && source->size() > 1)
        msg = source->at(rand() % source->size());
    lastmsg = msg;
    player_info_s info{};
    g_IEngine->GetPlayerInfo(g_IEngine->GetPlayerForUserID(vid), &info);

    ReplaceSpecials(msg);
    CachedEntity *ent = ENTITY(g_IEngine->GetPlayerForUserID(vid));
    int clz           = g_pPlayerResource->GetClass(ent);
    ReplaceString(msg, "%class%", tf_classes_killsay[clz]);
    player_info_s infok{};
    g_IEngine->GetPlayerInfo(g_IEngine->GetPlayerForUserID(kid), &infok);
    ReplaceString(msg, "%killer%", std::string(infok.name));
    ReplaceString(msg, "%team%", tf_teams_killsay[ent->m_iTeam() - 2]);
    ReplaceString(msg, "%myteam%", tf_teams_killsay[LOCAL_E->m_iTeam() - 2]);
    ReplaceString(msg, "%myclass%", tf_classes_killsay[g_pPlayerResource->GetClass(LOCAL_E)]);
    ReplaceString(msg, "%name%", std::string(info.name));
    return msg;
}

class KillSayEventListener : public IGameEventListener2
{
    void FireGameEvent(IGameEvent *event) override
    {
        if (!killsay_mode)
            return;
        std::string message = hacks::shared::killsay::ComposeKillSay(event);
        if (!message.empty())
        {
            int vid                    = event->GetInt("userid");
            killsay_storage[vid].delay = *delay;
            killsay_storage[vid].timer.update();
            killsay_storage[vid].message = message;
        }
    }
};

static void ProcessKillsay()
{
    if (killsay_storage.empty())
        return;
    for (auto &i : killsay_storage)
    {
        if (i.second.message.empty())
            continue;
        if (i.second.timer.test_and_set(i.second.delay))
        {
            chat_stack::Say(i.second.message, false);
            i.second = {};
        }
    }
}

static KillSayEventListener listener{};

void reload()
{
    file.Load(*filename);
}

static CatCommand reload_command("killsay_reload", "Reload killsays", []() { reload(); });

void init()
{
    reload();
    g_IEventManager2->AddListener(&listener, (const char *) "player_death", false);
}

void shutdown()
{
    g_IEventManager2->RemoveListener(&listener);
}

static InitRoutine runinit([]() {
    EC::Register(EC::Paint, ProcessKillsay, "paint_killsay", EC::average);
    EC::Register(EC::Shutdown, shutdown, "shutdown_killsay", EC::average);
    init();
});

} // namespace hacks::shared::killsay
