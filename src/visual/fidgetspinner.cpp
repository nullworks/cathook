/*
 * fidgetspinner.cpp
 *
 *  Created on: Jul 4, 2017
 *      Author: nullifiedcat
 */

#include "visual/fidgetspinner.hpp"
#include "common.hpp"

#include <math.h>

#ifndef FEATURE_FIDGET_SPINNER_ENABLED

static CatVar enable_spinner(CV_SWITCH, XORSTR("fidgetspinner"), XORSTR("0"), XORSTR("Fidget Spinner"),
                             XORSTR("Part of Cathook Autism Awareness program"));
CatVar v9mode(CV_SWITCH, XORSTR("nullcore_mode"), XORSTR("0"), XORSTR("Nullcore mode"),
              XORSTR("Part of Cathook Autism Awareness program"));

float spinning_speed = 0.0f;
float angle          = 0;

// DEBUG
/*CatCommand add_spinner_speed(XORSTR("fidgetspinner_debug_speedup"), XORSTR("Add speed"), []()
{ spinning_speed += 100.0f;
});*/

class SpinnerListener : public IGameEventListener
{
public:
    virtual void FireGameEvent(KeyValues *event)
    {
        std::string name(event->GetName());
        if (name == XORSTR("player_death"))
        {
            int attacker = event->GetInt(XORSTR("attacker"));
            int eid      = g_IEngine->GetPlayerForUserID(attacker);
            if (eid == g_IEngine->GetLocalPlayer())
            {
                spinning_speed += 300.0f;
                // logging::Info(XORSTR("Spinning %.2f"), spinning_speed);
            }
        }
    }
};

SpinnerListener listener;

void InitSpinner()
{
    g_IGameEventManager->AddListener(&listener, false);
}

static CatVar spinner_speed_cap(CV_FLOAT, XORSTR("fidgetspinner_speed_cap"), XORSTR("30"),
                                XORSTR("Speed cap"));
static CatVar spinner_speed_scale(CV_FLOAT, XORSTR("fidgetspinner_speed_scale"), XORSTR("0.03"),
                                  XORSTR("Speed scale"));
static CatVar spinner_decay_speed(CV_FLOAT, XORSTR("fidgetspinner_decay_speed"), XORSTR("0.1"),
                                  XORSTR("Decay speed"));
static CatVar spinner_scale(CV_FLOAT, XORSTR("fidgetspinner_scale"), XORSTR("32"),
                            XORSTR("Spinner Size"));
static CatVar spinner_min_speed(CV_FLOAT, XORSTR("fidgetspinner_min_speed"), XORSTR("2"),
                                XORSTR("Spinner Min Speed"));

draw_api::texture_handle_t text{ GLEZ_TEXTURE_INVALID };

void DrawSpinner()
{
    if (not enable_spinner)
        return;
    spinning_speed -= (spinning_speed > 150.0f)
                          ? float(spinner_decay_speed)
                          : float(spinner_decay_speed) / 2.0f;
    if (spinning_speed < float(spinner_min_speed))
        spinning_speed = float(spinner_min_speed);
    if (spinning_speed > 1000)
        spinning_speed = 1000;
    float real_speed   = 0;
    const float speed_cap(spinner_speed_cap);
    if (spinning_speed < 250)
        real_speed = speed_cap * (spinning_speed / 250.0f);
    else if (spinning_speed < 500)
        real_speed =
            speed_cap - (speed_cap - 10) * ((spinning_speed - 250.0f) / 250.0f);
    else if (spinning_speed < 750)
        real_speed =
            10 + (speed_cap - 20) * ((spinning_speed - 500.0f) / 250.0f);
    else
        real_speed =
            (speed_cap - 10) + 10 * ((spinning_speed - 750.0f) / 250.0f);
    const float speed_scale(spinner_speed_scale);
    const float size(spinner_scale);
    angle += speed_scale * real_speed;
    int state = min(3, int(spinning_speed / 250));

    const glez_rgba_t color = glez_rgba(255, 255, 255, 255);

    if (text.handle == GLEZ_TEXTURE_INVALID)
        text = draw_api::create_texture(DATA_PATH "/textures/atlas.png");

    draw_api::draw_rect_textured(draw::width / 2, draw::height / 2, size, size,
                                 colors::white, text, 0 + 64 * state,
                                 (3 + (v9mode ? 1 : 0)) * 64, 64, 64, angle);
    if (angle > PI * 4)
        angle -= PI * 4;
}

#endif
