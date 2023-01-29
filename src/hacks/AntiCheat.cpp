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
#include "PlayerTools.hpp"
#include "hack.hpp"
#include "angles.hpp"

namespace hacks::shared::anticheat
{
static settings::Boolean enable{ "find-cheaters.enable", "0" };
static settings::Boolean accuse_chat{ "find-cheaters.accuse-in-chat", "0" };
static settings::Boolean autorage{ "find-cheaters.auto-rage", "0" };
static settings::Boolean skip_local{ "find-cheaters.ignore-local", "1" };

void Accuse(int eid, const std::string &hack, const std::string &details)
{
    player_info_s info;
    if (GetPlayerInfo(eid, &info))
    {
        CachedEntity *ent = ENTITY(eid);
        if (accuse_chat)
        {
            hack::command_stack().push(format("say \"", info.name, " (", classname(CE_INT(ent, netvar.iClass)), ") suspected ", hack, ": ", details, "\""));
        }
        else
        {
#if ENABLE_VISUALS
            PrintChat("\x07%06X%s\x01 (%s) suspected \x07%06X%s\x01: %s", colors::chat::team(ENTITY(eid)->m_iTeam()), info.name, classname(CE_INT(ent, netvar.iClass)), 0xe05938, hack.c_str(), details.c_str());
#endif
        }
    }
}

void SetRage(player_info_t info)
{
    if (autorage)
        playerlist::ChangeState(info.friendsID, playerlist::k_EState::RAGE);
}

static void CreateMove()
{
    if (!enable)
        return;
    angles::Update();
    ac::aimbot::player_orgs().clear();
    for (auto const &ent : entity_cache::player_cache)
    {
        if (skip_local && ent == LOCAL_E)
            continue;

        if (CE_GOOD(ent))
        {

            if (player_tools::shouldTarget(ent) || ent == LOCAL_E)
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
            int entity = GetPlayerForUserID(uid);
            ResetPlayer(entity);
        }
        else if (name == "player_disconnect")
        {
            int uid    = event->GetInt("userid");
            int entity = GetPlayerForUserID(uid);
            ResetPlayer(entity);
        }

        ac::aimbot::Event(event);
    }
};

ACListener listener;

void Init()
{
    g_IGameEventManager->AddListener(&listener, false);
}

void Shutdown()
{
    g_IGameEventManager->RemoveListener(&listener);
}

static InitRoutine EC(
    []()
    {
        EC::Register(EC::CreateMove, CreateMove, "cm_AntiCheat", EC::average);
        EC::Register(EC::LevelInit, ResetEverything, "init_AntiCheat", EC::average);
        EC::Register(EC::LevelShutdown, ResetEverything, "reset_AntiCheat", EC::average);
        EC::Register(EC::Shutdown, Shutdown, "shutdown_AntiCheat", EC::average);
        Init();
    });
} // namespace hacks::shared::anticheat
