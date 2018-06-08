/*
 * drawing.cpp
 *
 *  Created on: Oct 5, 2016
 *      Author: nullifiedcat
 */

#include "common.hpp"

#include <GL/gl.h>

std::array<std::string, 32> side_strings;
std::array<std::string, 32> center_strings;
std::array<rgba_t, 32> side_strings_colors{ colors::empty };
std::array<rgba_t, 32> center_strings_colors{ colors::empty };
size_t side_strings_count{ 0 };
size_t center_strings_count{ 0 };

void InitStrings()
{
    ResetStrings();
}

void ResetStrings()
{
    side_strings_count   = 0;
    center_strings_count = 0;
}

void AddSideString(const std::string &string, const rgba_t &color)
{
    side_strings[side_strings_count]        = string;
    side_strings_colors[side_strings_count] = color;
    ++side_strings_count;
}

void DrawStrings()
{
    int y{ 8 };
    for (size_t i = 0; i < side_strings_count; ++i)
    {
        draw_api::draw_string_with_outline(
            8, y, side_strings[i].c_str(), fonts::main_font,
            side_strings_colors[i], colors::black, 1.5f);
        y += /*((int)fonts::font_main->height)*/ 14 + 1;
    }
    y = draw::height / 2;
    for (size_t i = 0; i < center_strings_count; ++i)
    {
        float sx, sy;
        draw_api::get_string_size(center_strings[i].c_str(), fonts::main_font,
                                  &sx, &sy);
        draw_api::draw_string_with_outline(
            (draw::width - sx) / 2, y, center_strings[i].c_str(),
            fonts::main_font, center_strings_colors[i], colors::black, 1.5f);
        y += /*((int)fonts::font_main->height)*/ 14 + 1;
    }
}

void AddCenterString(const std::string &string, const rgba_t &color)
{
    center_strings[center_strings_count]        = string;
    center_strings_colors[center_strings_count] = color;
    ++center_strings_count;
}

// TODO globals
int draw::width  = 0;
int draw::height = 0;
float draw::fov  = 90.0f;
std::mutex draw::draw_mutex;

namespace fonts
{

draw_api::font_handle_t main_font;
draw_api::font_handle_t esp_font;
}

void draw::Initialize()
{
    if (!draw::width || !draw::height)
    {
        g_IEngine->GetScreenSize(draw::width, draw::height);
    }
    fonts::main_font =
        draw_api::create_font(DATA_PATH "/fonts/verasans.ttf", 14);
    fonts::esp_font =
        draw_api::create_font(DATA_PATH "/fonts/verasans.ttf", 14);
}

bool draw::EntityCenterToScreen(CachedEntity *entity, Vector &out)
{
    Vector world, min, max;
    bool succ;

    if (CE_BAD(entity))
        return false;
    RAW_ENT(entity)->GetRenderBounds(min, max);
    world = RAW_ENT(entity)->GetAbsOrigin();
    world.z += (min.z + max.z) / 2;
    succ = draw::WorldToScreen(world, out);
    return succ;
}

VMatrix draw::wts{};

void draw::UpdateWTS()
{
    memcpy(&draw::wts, &g_IEngine->WorldToScreenMatrix(), sizeof(VMatrix));
}

bool draw::WorldToScreen(const Vector &origin, Vector &screen)
{
    float w, odw;
    screen.z = 0;
    w = wts[3][0] * origin[0] + wts[3][1] * origin[1] + wts[3][2] * origin[2] +
        wts[3][3];
    if (w > 0.001)
    {
        odw      = 1.0f / w;
        screen.x = (draw::width / 2) +
                   (0.5 * ((wts[0][0] * origin[0] + wts[0][1] * origin[1] +
                            wts[0][2] * origin[2] + wts[0][3]) *
                           odw) *
                        draw::width +
                    0.5);
        screen.y = (draw::height / 2) -
                   (0.5 * ((wts[1][0] * origin[0] + wts[1][1] * origin[1] +
                            wts[1][2] * origin[2] + wts[1][3]) *
                           odw) *
                        draw::height +
                    0.5);
        return true;
    }
    return false;
}
