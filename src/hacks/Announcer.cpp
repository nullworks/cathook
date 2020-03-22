/*
 * Announcer.cpp
 *
 *  Created on: Nov 13, 2017
 *      Author: nullifiedcat
 */

#include <settings/Registered.hpp>
#include "common.hpp"

namespace hacks::shared::announcer
{
static settings::Boolean enable{ "announcer", "false" };

struct announcer_entry_s
{
    int count;
    const std::string sound;
};

std::vector<announcer_entry_s> announces_headshot_combo = { { 1, "headshot.wav" }, { 2, "headshot.wav" }, { 4, "hattrick.wav" }, { 6, "headhunter.wav" } };

std::vector<announcer_entry_s> announces_kill = { { 5, "dominating.wav" }, { 7, "rampage.wav" }, { 9, "killingspree.wav" }, { 11, "monsterkill.wav" }, { 15, "unstoppable.wav" }, { 17, "ultrakill.wav" }, { 19, "godlike.wav" }, { 21, "wickedsick.wav" }, { 23, "impressive.wav" }, { 25, "ludicrouskill.wav" }, { 27, "holyshit.wav" } };

std::vector<announcer_entry_s> announces_kill_combo = { { 2, "doublekill.wav" }, { 3, "triplekill.wav" }, { 4, "multikill.wav" }, { 5, "combowhore.wav" } };

unsigned killstreak{ 0 };
unsigned killcombo{ 0 };
unsigned headshotcombo{ 0 };
Timer last_kill{};
Timer last_headshot{};

const announcer_entry_s *find_entry(const std::vector<announcer_entry_s> &vector, int count)
{
    for (auto it = vector.rbegin(); it != vector.rend(); ++it)
    {
        if ((*it).count <= count)
            return &*it;
    }
    return nullptr;
}

void playsound(const std::string &sound)
{
    // yes
    char command[128];
    snprintf(command, 128, "aplay %s/sound/%s &", paths::getDataPath().c_str(), sound.c_str());
    logging::Info("system(%s)", command);
    system(command);
    // g_ISurface->PlaySound(std::string("announcer/" + sound).c_str());
}

void reset()
{
    killstreak    = 0;
    killcombo     = 0;
    headshotcombo = 0;
}

void check_combos()
{
    if (last_kill.check(5000))
    {
        killcombo = 0;
    }
    if (last_headshot.check(5000))
    {
        headshotcombo = 0;
    }
}

void on_kill(IGameEvent *event)
{
    int killer_id = g_IEngine->GetPlayerForUserID(event->GetInt("attacker"));
    int victim_id = g_IEngine->GetPlayerForUserID(event->GetInt("userid"));

    if (victim_id == g_IEngine->GetLocalPlayer())
    {
        reset();
        return;
    }

    if (killer_id != g_IEngine->GetLocalPlayer())
        return;
    if (killer_id == victim_id)
        return;

    check_combos();
    last_kill.update();

    killstreak++;
    killcombo++;

    if (GetWeaponMode() == weaponmode::weapon_melee)
    {
        playsound("humiliation.wav");
        return;
    }
    if (event->GetInt("customkill") == 1)
    {
        headshotcombo++;
        last_headshot.update();
        const auto sound = find_entry(announces_headshot_combo, headshotcombo);
        if (sound)
        {
            playsound(sound->sound);
            return;
        }
    }
    auto entry = find_entry(announces_kill_combo, killcombo);
    if (entry)
    {
        playsound(entry->sound);
        return;
    }
    entry = find_entry(announces_kill, killstreak);
    if (entry)
    {
        playsound(entry->sound);
    }
}

void on_spawn(IGameEvent *event)
{
    int userid = g_IEngine->GetPlayerForUserID(event->GetInt("userid"));

    if (userid == g_IEngine->GetLocalPlayer())
    {
        reset();
    }
}

class AnnouncerEventListener : public IGameEventListener2
{
    virtual void FireGameEvent(IGameEvent *event)
    {
        if (!enable)
            return;
        if (0 == strcmp(event->GetName(), "player_death"))
            on_kill(event);
        else if (0 == strcmp(event->GetName(), "player_spawn"))
            on_spawn(event);
    }
};

AnnouncerEventListener &listener()
{
    static AnnouncerEventListener object{};
    return object;
}

void init()
{
    g_IEventManager2->AddListener(&listener(), "player_death", false);
    g_IEventManager2->AddListener(&listener(), "player_spawn", false);
}

void shutdown()
{
    g_IEventManager2->RemoveListener(&listener());
}

static InitRoutine EC([]() {
    EC::Register(EC::Shutdown, shutdown, "shutdown_announcer", EC::average);
    init();
});
} // namespace hacks::shared::announcer
