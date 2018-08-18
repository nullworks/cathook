/*
 * hitrate.cpp
 *
 *  Created on: Aug 16, 2017
 *      Author: nullifiedcat
 */

#include "common.hpp"
#include <hacks/Aimbot.hpp>
#include <settings/Bool.hpp>
#include "MiscTemporary.hpp"
#include "init.hpp"

static settings::Bool hitrate_check{ "hitrate.enable", "true" };

namespace hitrate
{

int lastweapon{ 0 };
int lastammo{ 0 };

int count_shots{ 0 };
int count_hits{ 0 };
int count_hits_head{ 0 };

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

CatCommand debug_hitrate("debug_hitrate", "Debug hitrate", []() {
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
    logging::Info("%d / %d (%d%%)", count_hits, count_shots, p1);
    logging::Info("%d / %d (%d%%)", count_hits_head, count_hits, p2);
});

CatCommand debug_ammo("debug_ammo", "Debug ammo", []() {
    for (int i = 0; i < 4; i++)
    {
        logging::Info("%d %d", i, CE_INT(LOCAL_E, netvar.m_iAmmo + i * 4));
    }
});
bool brutesoon[32];
int lasthits = 0;
std::array<Timer, 32> xd{};
void Update()
{
    CachedEntity *weapon = LOCAL_W;
    if (CE_GOOD(weapon))
    {
        if (LOCAL_W->m_iClassID() == CL_CLASS(CTFSniperRifle) ||
            LOCAL_W->m_iClassID() == CL_CLASS(CTFSniperRifleDecap))
        {
            /*INetChannel *ch = (INetChannel *)g_IEngine->GetNetChannelInfo();
            static int prevhits = count_hits;
            int latency = ch->GetAvgLatency(MAX_FLOWS) * 1000 + 0.5f;
            if (hacks::shared::aimbot::target_eid != -1 &&
            !timers[hacks::shared::aimbot::target_eid].check(latency))
            {
                if (count_hits > prevhits)
                {
                    prevhits = count_hits;
                    timers[hacks::shared::aimbot::target_eid].update();
                }
            }*/
            // ONLY tracks primary ammo
            int ammo = CE_INT(LOCAL_E, netvar.m_iAmmo + 4);

            INetChannel *ch = (INetChannel *) g_IEngine->GetNetChannelInfo();
            static bool firstcall = true;
            for (int i = 0; i < 32; i++)
            {
                if (firstcall)
                    xd[i].update();
                firstcall = false;
                if (ch &&
                    xd[i].check(ch->GetLatency(MAX_FLOWS) * 1000.0f + 100.0f) &&
                    brutesoon[i])
                {
                    if (lasthits == count_hits)
                    {
                        logging::Info("Increased Brutenum of ent %d", i);
                        g_Settings.brute.brutenum[i]++;
                    }
                    brutesoon[i] = false;
                    lasthits     = count_hits;
                }
            }
            if (lastweapon)
            {

                if (ammo < lastammo)
                {
                    if (hacks::shared::aimbot::target_eid > -1)
                    {
                        if (ch &&
                            xd[hacks::shared::aimbot::target_eid].check(
                                ch->GetLatency(MAX_FLOWS) * 1000.0f + 110.0f))
                        {
                            xd[hacks::shared::aimbot::target_eid].update();
                            brutesoon[hacks::shared::aimbot::target_eid] = true;
                        }
                    }
                    // for (auto i : entstocheck)
                    //{
                    OnShot();
                    /*static int prevent = 0;

                    if (hacks::shared::aimbot::target_eid != prevent)
                    {
                        entstocheck.push_back(hacks::shared::aimbot::target_eid);
                        prevent = hacks::shared::aimbot::target_eid;
                        timers[hacks::shared::aimbot::target_eid].update();
                    }
                    if (i != -1)
                        {
                            if (timers[i].test_and_set(latency))
                            {
                                bruteint[i]++;
                                entstocheck[];
                            }
                        }
                    }
                }*/
                }
                /*else if
                (timers[hacks::shared::aimbot::target_eid].check(latency / 2))
                {

                }*/
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
        if (strcmp("player_hurt", event->GetName()) &&
            strcmp("player_death", event->GetName()))
            return;
        if (g_IEngine->GetPlayerForUserID(event->GetInt("attacker")) ==
            g_IEngine->GetLocalPlayer())
        {
            if (CE_GOOD(LOCAL_W) &&
                (LOCAL_W->m_iClassID() == CL_CLASS(CTFSniperRifle) ||
                 LOCAL_W->m_iClassID() == CL_CLASS(CTFSniperRifleDecap)))
                OnHit(strcmp("player_death", event->GetName())
                          ? event->GetBool("crit")
                          : true);
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
} // namespace hitrate
