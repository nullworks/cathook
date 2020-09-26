/*
 * fidgetspinner.cpp
 *
 *  Created on: Jul 4, 2017
 *      Author: nullifiedcat
 */

#include "common.hpp"

#include <math.h>
#include <settings/Bool.hpp>

#ifndef FEATURE_FIDGET_SPINNER_ENABLED

static settings::Boolean enable_spinner{ "visual.fidget-spinner.enable", "false" };
static settings::Float spinner_speed_cap{ "visual.fidget-spinner.speed-cap", "30" };
static settings::Float spinner_speed_scale{ "visual.fidget-spinner.speed-scale", "0.03" };
static settings::Float spinner_decay_speed{ "visual.fidget-spinner.decay-speed", "0.1" };
static settings::Float spinner_scale{ "visual.fidget-spinner.scale", "32" };
static settings::Float spinner_min_speed{ "visual.fidget-spinner.min-speed", "2" };

static float spinning_speed = 0.0f;
static float angle          = 0;

// DEBUG
/*CatCommand add_spinner_speed("fidgetspinner_debug_speedup", "Add speed", []()
{ spinning_speed += 100.0f;
});*/

class SpinnerListener : public IGameEventListener
{
public:
    void FireGameEvent(KeyValues *event) override
    {
        std::string name(event->GetName());
        if (name == "player_death")
        {
            int attacker = event->GetInt("attacker");
            int eid      = g_IEngine->GetPlayerForUserID(attacker);
            if (eid == g_IEngine->GetLocalPlayer())
            {
                spinning_speed += 300.0f;
                // logging::Info("Spinning %.2f", spinning_speed);
            }
        }
    }
};

static SpinnerListener spinner_listener;

void InitSpinner()
{
    g_IGameEventManager->AddListener(&spinner_listener, false);
}

static Timer retrytimer{};

void DrawSpinner()
{
    if (not enable_spinner)
        return;
    spinning_speed -= (spinning_speed > 150.0f) ? float(spinner_decay_speed) : float(spinner_decay_speed) / 2.0f;
    if (spinning_speed < float(spinner_min_speed))
        spinning_speed = float(spinner_min_speed);
    if (spinning_speed > 1000)
        spinning_speed = 1000;
    float real_speed = 0;
    const float speed_cap(spinner_speed_cap);
    if (spinning_speed < 250)
        real_speed = speed_cap * (spinning_speed / 250.0f);
    else if (spinning_speed < 500)
        real_speed = speed_cap - (speed_cap - 10) * ((spinning_speed - 250.0f) / 250.0f);
    else if (spinning_speed < 750)
        real_speed = 10 + (speed_cap - 20) * ((spinning_speed - 500.0f) / 250.0f);
    else
        real_speed = (speed_cap - 10) + 10 * ((spinning_speed - 750.0f) / 250.0f);
    const float speed_scale(spinner_speed_scale);
    const float size(spinner_scale);
    angle += speed_scale * real_speed;
    int state = min(3, int(spinning_speed / 250));

    draw::RectangleTextured(draw::width / 2 - size * 0.5f, draw::height / 2 - size * 0.5f, size, size, colors::white, textures::atlas().texture, 64 * state, 4 * 64, 64, 64, angle);
    if (angle > PI * 4)
        angle -= PI * 4;
}

static InitRoutine init([]() {
    InitSpinner();
    EC::Register(EC::Draw, DrawSpinner, "spinner");
    EC::Register(
        EC::Shutdown, []() { g_IGameEventManager->RemoveListener(&spinner_listener); }, "shutdown_spinner");
});

#endif
