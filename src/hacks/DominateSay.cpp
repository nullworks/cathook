/*
 * DominateSay.cpp
 *
 *	Created on: October 30, 2018
 */

#include <settings/Int.hpp>
#include "common.hpp"

namespace hacks::shared::dominatesay
{
static settings::Int dominatesay_mode{ "dominatesay.mode", "0" };
static settings::String filename{ "dominatesay.file", "dominatesay.txt" };

//	a much better default dominatesay would be appreciated.
const std::vector<std::string> builtin_default = {
    "dominating %name%! (%dominum% dominations)",
    "%name%, nice skill, you sell?",
    "%name%, umad?",
    "smh i have %dominum% dominations",
    "Well, off to vist %name%'s mother!",
    "Cleanup on asile 3, %name% left their skill!",
    "Congrats %name%! You just won 50 precent off another asswhoopin! Call now and get a third absolutely free!",
};
const std::string tf_classes_dominatesay[] = { "class", "scout", "sniper", "soldier", "demoman", "medic", "heavy", "pyro", "spy", "engineer" };
const std::string tf_teams_dominatesay[]   = { "RED", "BLU" };

static std::string lastmsg{};

TextFile file{};

std::string ComposeDominateSay(IGameEvent *event)
{
    const std::vector<std::string> *source = nullptr;
    switch (*dominatesay_mode)
    {
    case 1:
        source = &file.lines;
        break;
    case 2:
        source = &builtin_default;
        break;
    default:
        break;
    }
    if (!source || source->empty())
        return "";
    if (!event)
        return "";
    int vid  = event->GetInt("dominated");
    int kid  = event->GetInt("dominator");
    int dnum = event->GetInt("dominations");

    //	this is actually impossible but just in case.
    if (g_IEngine->GetPlayerForUserID(kid) != g_IEngine->GetLocalPlayer())
        return "";

    std::string msg = source->at(rand() % source->size());

    while (msg == lastmsg && source->size() > 1)
        msg = source->at(rand() % source->size());
    lastmsg = msg;
    player_info_s info{};

    g_IEngine->GetPlayerInfo(g_IEngine->GetPlayerForUserID(vid), &info);
    ReplaceSpecials(msg);

    CachedEntity *ent = ENTITY(g_IEngine->GetPlayerForUserID(vid));
    int clz           = g_pPlayerResource->GetClass(ent);

    ReplaceString(msg, "%class%", tf_classes_dominatesay[clz]);
    player_info_s infok{};
    g_IEngine->GetPlayerInfo(g_IEngine->GetPlayerForUserID(kid), &infok);

    ReplaceString(msg, "%dominum%", std::to_string(dnum));
    ReplaceString(msg, "%killer%", std::string(infok.name));
    ReplaceString(msg, "%team%", tf_teams_dominatesay[ent->m_iTeam() - 2]);
    ReplaceString(msg, "%myteam%", tf_teams_dominatesay[LOCAL_E->m_iTeam() - 2]);
    ReplaceString(msg, "%myclass%", tf_classes_dominatesay[g_pPlayerResource->GetClass(LOCAL_E)]);
    ReplaceString(msg, "%name%", std::string(info.name));
    return msg;
}

class DominateSayEventListener : public IGameEventListener2
{
    void FireGameEvent(IGameEvent *event) override
    {
        if (!dominatesay_mode)
            return;
        std::string message = hacks::shared::dominatesay::ComposeDominateSay(event);
        if (!message.empty())
            chat_stack::Say(message, false);
    }
};

static DominateSayEventListener listener{};

void reload()
{
    file.Load(*filename);
}

void init()
{
    g_IEventManager2->AddListener(&listener, (const char *) "player_domination", false);
}

void shutdown()
{
    g_IEventManager2->RemoveListener(&listener);
}

static CatCommand reload_command("dominatesay_reload", "Reload dominatesays", []() { reload(); });

static InitRoutine EC([]() {
    EC::Register(EC::Shutdown, shutdown, "shutdown_dominatesay", EC::average);
    init();
});

} // namespace hacks::shared::dominatesay
