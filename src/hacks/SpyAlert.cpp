/*
 * SpyAlert.cpp
 *
 *  Created on: Dec 5, 2016
 *      Author: nullifiedcat
 */

#include "common.hpp"

namespace hacks
{
namespace tf
{
namespace spyalert
{

static CatVar enabled(CV_SWITCH, XORSTR("spyalert_enabled"), XORSTR("0"), XORSTR("Enable"),
                      XORSTR("Master SpyAlert switch"));
static CatVar distance_warning(CV_FLOAT, XORSTR("spyalert_warning"), XORSTR("500.0"),
                               XORSTR("Warning distance"),
                               XORSTR("Distance where yellow warning shows"));
static CatVar distance_backstab(CV_FLOAT, XORSTR("spyalert_backstab"), XORSTR("200.0"),
                                XORSTR("Backstab distance"),
                                XORSTR("Distance where red warning shows"));
static CatVar sound_alerts(CV_SWITCH, XORSTR("spyalert_sound"), XORSTR("1"), XORSTR("Sound Alerts"),
                           XORSTR("Demoman yells spy when a spy is within distance"));
static CatVar sound_alert_interval(CV_FLOAT, XORSTR("spyalert_interval"), XORSTR("3"),
                                   XORSTR("Alert Interval"), XORSTR("Sound alert interval"));

bool warning_triggered  = false;
bool backstab_triggered = false;
float last_say          = 0.0f;

void Draw()
{
    CachedEntity *closest_spy, *ent;
    float closest_spy_distance, distance;
    int spy_count;

    if (!enabled)
        return;
    if (g_pLocalPlayer->life_state)
        return;
    closest_spy          = nullptr;
    closest_spy_distance = 0.0f;
    spy_count            = 0;
    if (last_say > g_GlobalVars->curtime)
        last_say = 0;
    for (int i = 0; i < HIGHEST_ENTITY && i < 32; i++)
    {
        ent = ENTITY(i);
        if (CE_BAD(ent))
            continue;
        if (CE_BYTE(ent, netvar.iLifeState))
            continue;
        if (CE_INT(ent, netvar.iClass) != tf_class::tf_spy)
            continue;
        if (CE_INT(ent, netvar.iTeamNum) == g_pLocalPlayer->team)
            continue;
        if (IsPlayerInvisible(ent))
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
        if (closest_spy_distance < (float) distance_backstab)
        {
            if (!backstab_triggered)
            {
                if (sound_alerts &&
                    (g_GlobalVars->curtime - last_say) >
                        (float) sound_alert_interval)
                {
                    g_ISurface->PlaySound(XORSTR("vo/demoman_cloakedspy03.mp3"));
                    last_say = g_GlobalVars->curtime;
                }
                backstab_triggered = true;
            }
            AddCenterString(format(XORSTR("BACKSTAB WARNING! "),
                                   (int) (closest_spy_distance / 64 * 1.22f),
                                   XORSTR("m ("), spy_count, XORSTR(")")),
                            colors::red);
        }
        else if (closest_spy_distance < (float) distance_warning)
        {
            backstab_triggered = false;
            if (!warning_triggered)
            {
                if (sound_alerts &&
                    (g_GlobalVars->curtime - last_say) >
                        (float) sound_alert_interval)
                {
                    g_ISurface->PlaySound(XORSTR("vo/demoman_cloakedspy01.mp3"));
                    last_say = g_GlobalVars->curtime;
                }
                warning_triggered = true;
            }
            AddCenterString(format(XORSTR("Incoming spy! "),
                                   (int) (closest_spy_distance / 64 * 1.22f),
                                   XORSTR("m ("), spy_count, XORSTR(")")),
                            colors::yellow);
        }
    }
    else
    {
        warning_triggered  = false;
        backstab_triggered = false;
    }
}
}
}
}
