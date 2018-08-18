/*
 * AntiCheat.cpp
 *
 *  Created on: Jun 5, 2017
 *      Author: nullifiedcat
 */

#include <hacks/ac/aimbot.hpp>
#include <hacks/ac/antiaim.hpp>
#include <hacks/ac/bhop.hpp>
#include <settings/Bool.hpp>
#include "common.hpp"
#include "hack.hpp"

static settings::Bool enable{ "find-cheaters.enable", "0" };
static settings::Bool accuse_chat{ "find-cheaters.accuse-in-chat", "0" };
static settings::Bool autorage{ "find-cheaters.auto-rage", "0" };
static settings::Bool skip_local{ "find-cheaters.ignore-local", "1" };

namespace hacks::shared::anticheat
{

void Accuse(int eid, const std::string &hack, const std::string &details)
{
    player_info_s info;
    if (g_IEngine->GetPlayerInfo(eid, &info))
    {
        CachedEntity *ent = ENTITY(eid);
        if (accuse_chat)
        {
            hack::command_stack().push(
                format("say \"", info.name, " (",
                       classname(CE_INT(ent, netvar.iClass)), ") suspected ",
                       hack, ": ", details, "\""));
        }
        else
        {
            PrintChat("\x07%06X%s\x01 (%s) suspected \x07%06X%s\x01: %s",
                      colors::chat::team(ENTITY(eid)->m_iTeam()), info.name,
                      classname(CE_INT(ent, netvar.iClass)), 0xe05938,
                      hack.c_str(), details.c_str());
        }
    }
}

void SetRage(player_info_t info)
{
    if (autorage)
        playerlist::AccessData(info.friendsID).state =
            playerlist::k_EState::RAGE;
}

void CreateMove()
{
    if (!enable)
        return;
    angles::Update();
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
        if (!enable)
            return;
        std::string name(event->GetName());
        if (name == "player_activate")
        {
            int uid    = event->GetInt("userid");
            int entity = g_IEngine->GetPlayerForUserID(uid);
            ResetPlayer(entity);
        }
        else if (name == "player_disconnect")
        {
            int uid    = event->GetInt("userid");
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
} // namespace hacks::shared::anticheat
