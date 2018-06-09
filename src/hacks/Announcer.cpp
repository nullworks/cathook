/*
 * Announcer.cpp
 *
 *  Created on: Nov 13, 2017
 *      Author: nullifiedcat
 */

#include "common.hpp"

namespace hacks
{
namespace shared
{
namespace announcer
{

static CatVar enabled(CV_SWITCH, XORSTR("announcer"), XORSTR("0"), XORSTR("Enable announcer"));

struct announcer_entry_s
{
    int count;
    const std::string sound;
};

std::vector<announcer_entry_s> announces_headshot_combo = {
    { 1, XORSTR("headshot.wav") },
    { 2, XORSTR("headshot.wav") },
    { 4, XORSTR("hattrick.wav") },
    { 6, XORSTR("headhunter.wav") }
};

std::vector<announcer_entry_s> announces_kill = {
    { 5, XORSTR("dominating.wav") },   { 7, XORSTR("rampage.wav") },
    { 9, XORSTR("killingspree.wav") }, { 11, XORSTR("monsterkill.wav") },
    { 15, XORSTR("unstoppable.wav") }, { 17, XORSTR("ultrakill.wav") },
    { 19, XORSTR("godlike.wav") },     { 21, XORSTR("wickedsick.wav") },
    { 23, XORSTR("impressive.wav") },  { 25, XORSTR("ludicrouskill.wav") },
    { 27, XORSTR("holyshit.wav") }
};

std::vector<announcer_entry_s> announces_kill_combo = { { 2, XORSTR("doublekill.wav") },
                                                        { 3, XORSTR("triplekill.wav") },
                                                        { 4, XORSTR("multikill.wav") },
                                                        { 5,
                                                          XORSTR("combowhore.wav") } };

unsigned killstreak{ 0 };
unsigned killcombo{ 0 };
unsigned headshotcombo{ 0 };
Timer last_kill{};
Timer last_headshot{};

const announcer_entry_s *
find_entry(const std::vector<announcer_entry_s> &vector, int count)
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
    snprintf(command, 128, XORSTR("aplay %s/sound/%s &"), DATA_PATH, sound.c_str());
    logging::Info(XORSTR("system(%s)"), command);
    system(command);
    // g_ISurface->PlaySound(std::string(XORSTR("announcer/") + sound).c_str());
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

    check_combos();
    last_kill.update();

    killstreak++;
    killcombo++;

    if (GetWeaponMode() == weaponmode::weapon_melee)
    {
        playsound(XORSTR("humiliation.wav"));
        return;
    }
    if (event->GetInt(XORSTR("customkill")) == 1)
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
    int userid = g_IEngine->GetPlayerForUserID(event->GetInt(XORSTR("userid")));

    if (userid == g_IEngine->GetLocalPlayer())
    {
        reset();
    }
}

class AnnouncerEventListener : public IGameEventListener2
{
    virtual void FireGameEvent(IGameEvent *event)
    {
        if (!enabled)
            return;
        if (0 == strcmp(event->GetName(), XORSTR("player_death")))
            on_kill(event);
        else if (0 == strcmp(event->GetName(), XORSTR("player_spawn")))
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
    g_IEventManager2->AddListener(&listener(), XORSTR("player_death"), false);
    g_IEventManager2->AddListener(&listener(), XORSTR("player_spawn"), false);
}

void shutdown()
{
    g_IEventManager2->RemoveListener(&listener());
}
}
}
}
