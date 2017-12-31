/*
 * fidgetspinner.cpp
 *
 *  Created on: Jul 4, 2017
 *      Author: nullifiedcat
 */

#include "fidgetspinner.hpp"

#ifndef FEATURE_FIDGET_SPINNER_DISABLED

CatVar enable_spinner(CV_SWITCH, "fidgetspinner", "0", "Fidget Spinner",
                      "Part of Cathook Autism Awareness program");
std::vector<textures::AtlasTexture> spinner_states{};

float spinning_speed = 0.0f;
float angle          = 0;

// DEBUG
/*CatCommand add_spinner_speed("fidgetspinner_debug_speedup", "Add speed", []()
{ spinning_speed += 100.0f;
});*/

class SpinnerListener : public IGameEventListener
{
public:
    virtual void FireGameEvent(KeyValues *event)
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

SpinnerListener listener;

void InitSpinner()
{
    for (int i = 0; i < 4; i++)
        spinner_states.emplace_back(i * 64, textures::atlas_height - 64 * 4, 64,
                                    64);
    g_IGameEventManager->AddListener(&listener, false);
}

CatVar spinner_speed_cap(CV_FLOAT, "fidgetspinner_speed_cap", "30",
                         "Speed cap");
CatVar spinner_speed_scale(CV_FLOAT, "fidgetspinner_speed_scale", "0.03",
                           "Speed scale");
CatVar spinner_decay_speed(CV_FLOAT, "fidgetspinner_decay_speed", "0.1",
                           "Decay speed");
CatVar spinner_scale(CV_FLOAT, "fidgetspinner_scale", "32", "Spinner Size");
CatVar spinner_min_speed(CV_FLOAT, "fidgetspinner_min_speed", "2",
                         "Spinner Min Speed");

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
    ftgl::vec2 positions[4] = {
        { -size, -size }, { size, -size }, { size, size }, { -size, size }
    };
    angle += speed_scale * real_speed;
    for (int i = 0; i < 4; i++)
    {
        float x        = positions[i].x;
        float y        = positions[i].y;
        positions[i].x = x * cos(angle) - y * sin(angle);
        positions[i].y = x * sin(angle) + y * cos(angle);
        positions[i].x += draw::width / 2;
        positions[i].y += draw::height / 2;
    }
    int state = min(3, spinning_speed / 250.0f);

    // Paste from draw_api::
    using namespace drawgl;
    using namespace ftgl;

    const auto &u1v1 = spinner_states[state].tex_coords[0];
    const auto &u2v2 = spinner_states[state].tex_coords[1];
    GLuint idx       = buffer_triangles_textured->vertices->size;
    GLuint indices[] = { idx, idx + 1, idx + 2, idx, idx + 2, idx + 3 };
    vertex_v2t2c4_t vertices[] = {
        { vec2{ positions[0].x, positions[0].y }, vec2{ u1v1.x, u2v2.y },
          *reinterpret_cast<const vec4 *>(&colors::white) },
        { vec2{ positions[1].x, positions[1].y }, vec2{ u2v2.x, u2v2.y },
          *reinterpret_cast<const vec4 *>(&colors::white) },
        { vec2{ positions[2].x, positions[2].y }, vec2{ u2v2.x, u1v1.y },
          *reinterpret_cast<const vec4 *>(&colors::white) },
        { vec2{ positions[3].x, positions[3].y }, vec2{ u1v1.x, u1v1.y },
          *reinterpret_cast<const vec4 *>(&colors::white) }
    };
    vertex_buffer_push_back_indices(buffer_triangles_textured, indices, 6);
    vertex_buffer_push_back_vertices(buffer_triangles_textured, vertices, 4);
    if (angle > PI * 4)
        angle -= PI * 4;
}

#endif
