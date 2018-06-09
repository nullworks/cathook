/*
 * AntiCheat.cpp
 *
 *  Created on: Jun 5, 2017
 *      Author: nullifiedcat
 */

#include <hacks/ac/aimbot.hpp>
#include <hacks/ac/antiaim.hpp>
#include <hacks/ac/bhop.hpp>
#include "common.hpp"
#include "hack.hpp"

namespace hacks
{
namespace shared
{
namespace anticheat
{
static CatVar enabled(CV_SWITCH, XORSTR("ac_enabled"), XORSTR("0"), XORSTR("Enable AC"));
static CatVar accuse_chat(CV_SWITCH, XORSTR("ac_chat"), XORSTR("0"), XORSTR("Accuse in chat"));
static CatVar autorage(CV_SWITCH, XORSTR("ac_autorage"), XORSTR("0"), XORSTR("Auto Rage"));

void Accuse(int eid, const std::string &hack, const std::string &details)
{
    player_info_s info;
    if (g_IEngine->GetPlayerInfo(eid, &info))
    {
        CachedEntity *ent = ENTITY(eid);
        if (accuse_chat)
        {
            hack::command_stack().push(
                format(XORSTR("say \""), info.name, XORSTR(" ("),
                       classname(CE_INT(ent, netvar.iClass)), XORSTR(") suspected "),
                       hack, XORSTR(": "), details, XORSTR("\"")));
        }
        else
        {
            PrintChat(XORSTR("\x07%06X%s\x01 (%s) suspected \x07%06X%s\x01: %s"),
                      colors::chat::team(ENTITY(eid)->m_iTeam()), info.name,
                      classname(CE_INT(ent, netvar.iClass)), 0xe05938,
                      hack.c_str(), details.c_str());
        }
    }
}

static CatVar skip_local(CV_SWITCH, XORSTR("ac_ignore_local"), XORSTR("1"), XORSTR("Ignore Local"));

void SetRage(player_info_t info)
{
    if (autorage)
        playerlist::AccessData(info.friendsID).state =
            playerlist::k_EState::RAGE;
}

void CreateMove()
{
    if (!enabled)
        return;
    for (int i = 1; i < 33; i++)
    {
        if (skip_local && (i == g_IEngine->GetLocalPlayer()))
            continue;
        CachedEntity *ent = ENTITY(i);
        if (CE_GOOD(ent))
        {
            if ((CE_BYTE(ent, netvar.iLifeState) == 0))
            {
                ac::aimbot::Update(ent);
                ac::antiaim::Update(ent);
                ac::bhop::Update(ent);
            }
        }
    }
}

void ResetPlayer(int index)
{
    ac::aimbot::ResetPlayer(index);
    ac::antiaim::ResetPlayer(index);
    ac::bhop::ResetPlayer(index);
}

void ResetEverything()
{
    ac::aimbot::ResetEverything();
    ac::antiaim::ResetEverything();
    ac::bhop::ResetEverything();
}

class ACListener : public IGameEventListener
{
public:
    virtual void FireGameEvent(KeyValues *event)
    {
        if (!enabled)
            return;
        std::string name(event->GetName());
        if (name == XORSTR("player_activate"))
        {
            int uid    = event->GetInt(XORSTR("userid"));
            int entity = g_IEngine->GetPlayerForUserID(uid);
            ResetPlayer(entity);
        }
        else if (name == XORSTR("player_disconnect"))
        {
            int uid    = event->GetInt(XORSTR("userid"));
            int entity = g_IEngine->GetPlayerForUserID(uid);
            ResetPlayer(entity);
        }

        ac::aimbot::Event(event);
    }
};

ACListener listener;

void Init()
{
    // FIXME free listener
    g_IGameEventManager->AddListener(&listener, false);
}
}
}
}
