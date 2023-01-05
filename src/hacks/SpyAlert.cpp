/*
 * SpyAlert.cpp
 *
 *  Created on: Dec 5, 2016
 *      Author: nullifiedcat
 */

#include <settings/Bool.hpp>
#include "common.hpp"
#include "PlayerTools.hpp"

namespace hacks::tf::spyalert
{
static settings::Boolean enable{ "spy-alert.enable", "false" };
static settings::Float distance_warning{ "spy-alert.distance.warning", "500" };
static settings::Float distance_alert{ "spy-alert.distance.alert", "200" };
static settings::Boolean sound_alert{ "spy-alert.sound", "true" };
static settings::Float sound_alert_interval{ "spy-alert.alert-interval", "3" };
static settings::Boolean voicemenu{ "spy-alert.voicemenu", "false" };
static settings::Boolean invisible{ "spy-alert.alert-for-invisible", "false" };

bool warning_triggered  = false;
bool backstab_triggered = false;
float last_say          = 0.0f;
Timer lastVoicemenu{};

void Draw()
{
    PROF_SECTION(DRAW_SpyAlert)
    if (!enable)
        return;
    CachedEntity *closest_spy;
    float closest_spy_distance, distance;
    int spy_count;
    if (g_pLocalPlayer->life_state)
        return;
    closest_spy          = nullptr;
    closest_spy_distance = 0.0f;
    spy_count            = 0;
    if (last_say > g_GlobalVars->curtime)
        last_say = 0;
    for (auto const &ent: entity_cache::player_cache)
    {
        if (ent->m_iClassID() != tf_spy)
            continue;
        if (!ent->m_bEnemy())
            continue;
        if (IsPlayerInvisible(ent) && !invisible)
            continue;
        if (!player_tools::shouldTarget(ent))
            continue;
        distance = ent->m_flDistance();
        if (distance < closest_spy_distance || !closest_spy_distance)
        {
            closest_spy_distance = distance;
            closest_spy          = ent;
        }
        if (distance < (float) distance_warning)
        {
            spy_count++;
        }
    }
    if (closest_spy && closest_spy_distance < (float) distance_warning)
    {
        if (closest_spy_distance < (float) distance_alert)
        {
            if (!backstab_triggered)
            {
                if (sound_alert && (g_GlobalVars->curtime - last_say) > (float) sound_alert_interval)
                {
                    g_ISurface->PlaySound("vo/demoman_cloakedspy03.mp3");
                    last_say = g_GlobalVars->curtime;
                }
                backstab_triggered = true;
            }
            if (voicemenu && lastVoicemenu.test_and_set(5000))
                g_IEngine->ClientCmd_Unrestricted("voicemenu 1 1");
            AddCenterString(format("BACKSTAB WARNING! ", (int) (closest_spy_distance / 64 * 1.22f), "m (", spy_count, ")"), colors::red_s);
        }
        else if (closest_spy_distance < (float) distance_warning)
        {
            backstab_triggered = false;
            if (!warning_triggered)
            {
                if (sound_alert && (g_GlobalVars->curtime - last_say) > (float) sound_alert_interval)
                {
                    g_ISurface->PlaySound("vo/demoman_cloakedspy01.mp3");
                    last_say = g_GlobalVars->curtime;
                }
                warning_triggered = true;
            }
            if (voicemenu && lastVoicemenu.test_and_set(5000))
                g_IEngine->ClientCmd_Unrestricted("voicemenu 1 1");
            AddCenterString(format("Incoming spy! ", (int) (closest_spy_distance / 64 * 1.22f), "m (", spy_count, ")"), colors::yellow);
        }
    }
    else
    {
        warning_triggered  = false;
        backstab_triggered = false;
    }
}
#if ENABLE_VISUALS
static InitRoutine EC([]() { EC::Register(EC::Draw, Draw, "spyalert", EC::average); });
#endif
} // namespace hacks::tf::spyalert
