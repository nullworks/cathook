/*
 * drawing.cpp
 *
 *  Created on: Oct 5, 2016
 *      Author: nullifiedcat
 */

#include "common.hpp"

#include <glez/glez.hpp>
#include <glez/draw.hpp>
#include <GL/glew.h>
#include <SDL2/SDL_video.h>
#include <SDLHooks.hpp>
#include <menu/GuiInterface.hpp>

#if EXTERNAL_DRAWING
#include "xoverlay.h"
#endif

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
        glez::draw::outlined_string(8, y, side_strings[i], *fonts::menu,
                                    side_strings_colors[i], colors::black,
                                    nullptr, nullptr);
        y += fonts::menu->size + 1;
    }
    y = draw::height / 2;
    for (size_t i = 0; i < center_strings_count; ++i)
    {
        float sx, sy;
        fonts::menu->stringSize(center_strings[i], &sx, &sy);
        glez::draw::outlined_string(
            (draw::width - sx) / 2, y, center_strings[i].c_str(), *fonts::menu,
            center_strings_colors[i], colors::black, nullptr, nullptr);
        y += fonts::menu->size + 1;
    }
}

void AddCenterString(const std::string &string, const rgba_t &color)
{
    center_strings[center_strings_count]        = string;
    center_strings_colors[center_strings_count] = color;
    ++center_strings_count;
}

int draw::width  = 0;
int draw::height = 0;
float draw::fov  = 90.0f;
std::mutex draw::draw_mutex;

namespace fonts
{

std::unique_ptr<glez::font> menu{ nullptr };
std::unique_ptr<glez::font> esp{ nullptr };
}

void draw::Initialize()
{
    if (!draw::width || !draw::height)
    {
        g_IEngine->GetScreenSize(draw::width, draw::height);
    }
    glez::preInit();
    fonts::menu.reset(new glez::font(DATA_PATH "/fonts/verasans.ttf", 14));
    fonts::esp.reset(new glez::font(DATA_PATH "/fonts/verasans.ttf", 14));
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

SDL_GLContext context = nullptr;

void draw::InitGL()
{
    logging::Info("InitGL: %d, %d", draw::width, draw::height);
#if EXTERNAL_DRAWING
    int status = xoverlay_init();
    xoverlay_draw_begin();
    glez::init(xoverlay_library.width, xoverlay_library.height);
    xoverlay_draw_end();
    if (status < 0)
    {
        logging::Info("ERROR: could not initialize Xoverlay: %d", status);
    }
    else
    {
        logging::Info("Xoverlay initialized");
    }
    xoverlay_show();
    context = SDL_GL_CreateContext(sdl_hooks::window);
#else
    glClearColor(1.0, 0.0, 0.0, 0.5);
    glewExperimental = GL_TRUE;
    glewInit();
    glez::init(draw::width, draw::height);
#endif

#if ENABLE_GUI
    gui::init();
#endif
}

void draw::BeginGL()
{
    glColor3f(1, 1, 1);
#if EXTERNAL_DRAWING
    xoverlay_draw_begin();
    {
        PROF_SECTION(draw_begin__SDL_GL_MakeCurrent);
        // SDL_GL_MakeCurrent(sdl_hooks::window, context);
    }
#endif
    {
        glActiveTexture(GL_TEXTURE0);
        PROF_SECTION(draw_begin__glez_begin);
        glez::begin();
        glDisable(GL_FRAMEBUFFER_SRGB);
        PROF_SECTION(DRAWEX_draw_begin);
    }
}

void draw::EndGL()
{
    PROF_SECTION(DRAWEX_draw_end);
    {
        PROF_SECTION(draw_end__glez_end);
        glEnable(GL_FRAMEBUFFER_SRGB);
        glez::end();
    }
#if EXTERNAL_DRAWING
    xoverlay_draw_end();
    {
        PROF_SECTION(draw_end__SDL_GL_MakeCurrent);
        SDL_GL_MakeCurrent(sdl_hooks::window, nullptr);
    }
#endif
}
