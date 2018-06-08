/*
 * hitrate.cpp
 *
 *  Created on: Aug 16, 2017
 *      Author: nullifiedcat
 */

#include "common.hpp"
#include "init.hpp"

namespace hitrate
{

int lastweapon{ 0 };
int lastammo{ 0 };

int count_shots{ 0 };
int count_hits{ 0 };
int count_hits_head{ 0 };

CatVar hitrate_check(CV_SWITCH, XORSTR("hitrate"), XORSTR("1"), XORSTR("Monitor hitrate"));

std::vector<std::chrono::time_point<std::chrono::high_resolution_clock>>
    shots{};

void OnShot()
{
    ++count_shots;
}

void OnHit(bool crit)
{
    count_hits++;
    if (crit)
    {
        count_hits_head++;
    }
}

CatCommand debug_hitrate(XORSTR("debug_hitrate"), XORSTR("Debug hitrate"), []() {
    int p1 = 0;
    int p2 = 0;
    if (count_shots)
    {
        p1 = float(count_hits) / float(count_shots) * 100.0f;
    }
    if (count_hits)
    {
        p2 = float(count_hits_head) / float(count_hits) * 100.0f;
    }
    logging::Info(XORSTR("%d / %d (%d%%)"), count_hits, count_shots, p1);
    logging::Info(XORSTR("%d / %d (%d%%)"), count_hits_head, count_hits, p2);
});

CatCommand debug_ammo(XORSTR("debug_ammo"), XORSTR("Debug ammo"), []() {
    for (int i = 0; i < 4; i++)
    {
        logging::Info(XORSTR("%d %d"), i, CE_INT(LOCAL_E, netvar.m_iAmmo + i * 4));
    }
});

void Update()
{
    CachedEntity *weapon = LOCAL_W;
    if (CE_GOOD(weapon))
    {
        if (LOCAL_W->m_iClassID() == CL_CLASS(CTFSniperRifle) ||
            LOCAL_W->m_iClassID() == CL_CLASS(CTFSniperRifleDecap))
        {
            // ONLY tracks primary ammo
            int ammo = CE_INT(LOCAL_E, netvar.m_iAmmo + 4);
            if (lastweapon)
            {
                if (ammo < lastammo)
                {
                    OnShot();
                }
            }
            lastweapon = weapon->m_IDX;
            lastammo   = ammo;
        }
    }
    else
    {
        lastweapon = 0;
    }
}

class HurtListener : public IGameEventListener
{
public:
    virtual void FireGameEvent(KeyValues *event)
    {
        if (strcmp(XORSTR("player_hurt"), event->GetName()))
            return;
        if (g_IEngine->GetPlayerForUserID(event->GetInt(XORSTR("attacker"))) ==
            g_IEngine->GetLocalPlayer())
        {
            if (CE_GOOD(LOCAL_W) &&
                (LOCAL_W->m_iClassID() == CL_CLASS(CTFSniperRifle) ||
                 LOCAL_W->m_iClassID() == CL_CLASS(CTFSniperRifleDecap)))
                OnHit(event->GetBool(XORSTR("crit")));
        }
    }
};

HurtListener &listener()
{
    static HurtListener l{};
    return l;
}

InitRoutine init([]() {
    g_IGameEventManager->AddListener(&listener(), false);
});
}
