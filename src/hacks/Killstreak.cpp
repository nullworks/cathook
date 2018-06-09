/*
 * Killstreak.cpp
 *
 *  Created on: Nov 13, 2017
 *      Author: nullifiedcat
 */

#include "common.hpp"
#include "hooks.hpp"

namespace hacks
{
namespace tf2
{
namespace killstreak
{

static CatVar enabled(CV_SWITCH, XORSTR("killstreak"), XORSTR("0"),
                      XORSTR("Enable killstreaks on all weapons"));

int killstreak{ 0 };

void reset()
{
    killstreak = 0;
}

int current_streak()
{
    return killstreak;
}

void apply_killstreaks()
{
    if (!enabled)
        return;

    IClientEntity *ent =
        g_IEntityList->GetClientEntity(g_IEngine->GetLocalPlayer());

    IClientEntity *resource =
        g_IEntityList->GetClientEntity(g_pPlayerResource->entity);
    if (!ent || ent->GetClientClass()->m_ClassID != RCC_PLAYERRESOURCE)
        return;

    if (!(ent && resource))
    {
        logging::Info(XORSTR("1"));
    }

    int *streaks_resource =
        (int *) ((unsigned) resource + netvar.m_nStreaks_Resource +
                 4 * g_IEngine->GetLocalPlayer());
    if (*streaks_resource != current_streak())
    {
        logging::Info(XORSTR("Adjusting %d -> %d"), *streaks_resource,
                      current_streak());
        *streaks_resource = current_streak();
    }
    int *streaks_player = (int *) ent + netvar.m_nStreaks_Player;
    // logging::Info(XORSTR("P0: %d"), streaks_player[0]);
    streaks_player[0] = current_streak();
    streaks_player[1] = current_streak();
    streaks_player[2] = current_streak();
    streaks_player[3] = current_streak();
}

void on_kill(IGameEvent *event)
{
    int killer_id = g_IEngine->GetPlayerForUserID(event->GetInt(XORSTR("attacker")));
    int victim_id = g_IEngine->GetPlayerForUserID(event->GetInt(XORSTR("userid")));

    if (victim_id == g_IEngine->GetLocalPlayer())
    {
        reset();
        return;
    }

    if (killer_id != g_IEngine->GetLocalPlayer())
        return;
    if (killer_id == victim_id)
        return;

    killstreak++;

    //    if (event->GetInt(XORSTR("kill_streak_total")) == 0)
    {
        logging::Info(XORSTR("Manipulating KS %d"), killstreak);
        event->SetInt(XORSTR("kill_streak_total"), current_streak());
        event->SetInt(XORSTR("kill_streak_wep"), current_streak());
    }
    apply_killstreaks();
}

void on_spawn(IGameEvent *event)
{
    int userid = g_IEngine->GetPlayerForUserID(event->GetInt(XORSTR("userid")));

    if (userid == g_IEngine->GetLocalPlayer())
    {
        reset();
    }
    apply_killstreaks();
}

void fire_event(IGameEvent *event)
{
    if (enabled)
    {
        if (0 == strcmp(event->GetName(), XORSTR("player_death")))
            on_kill(event);
        else if (0 == strcmp(event->GetName(), XORSTR("player_spawn")))
            on_spawn(event);
    }
}

hooks::VMTHook hook;

typedef bool (*FireEvent_t)(IGameEventManager2 *, IGameEvent *, bool);
bool FireEvent_hook(IGameEventManager2 *manager, IGameEvent *event,
                    bool bDontBroadcast)
{
    static FireEvent_t original =
        (FireEvent_t) hook.GetMethod(offsets::FireEvent());
    fire_event(event);
    return original(manager, event, bDontBroadcast);
}

typedef bool (*FireEventClientSide_t)(IGameEventManager2 *, IGameEvent *);
bool FireEventClientSide(IGameEventManager2 *manager, IGameEvent *event)
{
    static FireEventClientSide_t original =
        (FireEventClientSide_t) hook.GetMethod(offsets::FireEventClientSide());
    fire_event(event);
    return original(manager, event);
}

void init()
{
}
}
}
}
