/*
 * drawex.cpp
 *
 *  Created on: Nov 12, 2017
 *      Author: nullifiedcat
 */

#include "common.hpp"
#include "hack.hpp"

#include <SDL2/SDL.h>

#include <GL/glew.h>
#include <GL/gl.h>
#include <visual/SDLHooks.hpp>

#if EXTERNAL_DRAWING
extern XORSTR("C") {
#include <xoverlay.h>
}
#endif

SDL_GLContext context = nullptr;

namespace drawex
{

namespace api
{

bool ready_state = false;
bool init        = false;

font_handle_t create_font(const char *path, float size)
{
    logging::Info(XORSTR("Creating font '%s':%f"), path, size);
    font_handle_t result;
    result.filename = std::string(path);
    result.size     = size;
    result.handle   = GLEZ_FONT_INVALID;
    return result;
}

texture_handle_t create_texture(const char *path)
{
    return texture_handle_t{ glez_texture_load_png_rgba(path) };
}

void destroy_font(font_handle_t font)
{
    glez_font_unload(font.handle);
}

void destroy_texture(texture_handle_t texture)
{
    glez_texture_unload(texture.handle);
}

bool ready()
{
    return ready_state && init;
}

void initialize()
{
#if EXTERNAL_DRAWING
    int status = xoverlay_init();
    xoverlay_draw_begin();
    glez_init(xoverlay_library.width, xoverlay_library.height);
    xoverlay_draw_end();
    if (status < 0)
    {
        logging::Info(XORSTR("ERROR: could not initialize Xoverlay: %d"), status);
    }
    else
    {
        logging::Info(XORSTR("Xoverlay initialized"));
    }
    xoverlay_show();
    context = SDL_GL_CreateContext(sdl_hooks::window);
#else
    glClearColor(1.0, 0.0, 0.0, 0.5);
    glewExperimental = GL_TRUE;
    glewInit();
    glez_init(draw::width, draw::height);
#endif
}

void draw_rect(float x, float y, float w, float h, const rgba_t &rgba)
{
    glez_rect(x, y, w, h, *reinterpret_cast<const glez_rgba_t *>(&rgba));
}

void draw_rect_outlined(float x, float y, float w, float h, const rgba_t &rgba,
                        float thickness)
{
    glez_rect_outline(x, y, w, h, *reinterpret_cast<const glez_rgba_t *>(&rgba),
                      thickness);
}

void draw_line(float x, float y, float dx, float dy, const rgba_t &rgba,
               float thickness)
{
    glez_line(x, y, dx, dy, *reinterpret_cast<const glez_rgba_t *>(&rgba),
              thickness);
}

void draw_rect_textured(float x, float y, float w, float h, const rgba_t &rgba,
                        texture_handle_t texture, float u, float v, float s,
                        float t, float a)
{
    glez_rect_textured(x, y, w, h,
                       *reinterpret_cast<const glez_rgba_t *>(&rgba),
                       texture.handle, u, v, s, t, a);
}

void draw_circle(float x, float y, float radius, const rgba_t &rgba,
                 float thickness, int steps)
{
    glez_circle(x, y, radius, *reinterpret_cast<const glez_rgba_t *>(&rgba),
                thickness, steps);
}

void draw_string(float x, float y, const char *string, font_handle_t &font,
                 const rgba_t &rgba)
{
    if (font.handle == GLEZ_FONT_INVALID)
        font.handle = glez_font_load(font.filename.c_str(), font.size);
    glez_string(x, y, string, font.handle,
                *reinterpret_cast<const glez_rgba_t *>(&rgba), nullptr,
                nullptr);
}

void draw_string_with_outline(float x, float y, const char *string,
                              font_handle_t &font, const rgba_t &rgba,
                              const rgba_t &rgba_outline, float thickness)
{
    if (font.handle == GLEZ_FONT_INVALID)
        font.handle = glez_font_load(font.filename.c_str(), font.size);
    glez_string_with_outline(
        x, y, string, font.handle,
        *reinterpret_cast<const glez_rgba_t *>(&rgba),
        *reinterpret_cast<const glez_rgba_t *>(&rgba_outline), thickness, 1,
        nullptr, nullptr);
    // xoverlay_draw_string(x, y, string, font.handle, *reinterpret_cast<const
    // xoverlay_rgba_t *>(&rgba), nullptr, nullptr);
}

void get_string_size(const char *string, font_handle_t &font, float *x,
                     float *y)
{
    if (font.handle == GLEZ_FONT_INVALID)
        font.handle = glez_font_load(font.filename.c_str(), font.size);
    glez_font_string_size(font.handle, string, x, y);
}

void draw_begin()
{
    glColor3f(1, 1, 1);
    PROF_SECTION(DRAWEX_draw_begin);
#if EXTERNAL_DRAWING
    xoverlay_draw_begin();
    {
        PROF_SECTION(draw_begin__SDL_GL_MakeCurrent);
        // SDL_GL_MakeCurrent(sdl_hooks::window, context);
    }
#endif
    {
        glActiveTexture(GL_TEXTURE0);
        PROF_SECTION(draw_begin__glez_end)
        glez_begin();
    }
}

void draw_end()
{
    PROF_SECTION(DRAWEX_draw_end);
    {
        PROF_SECTION(draw_end__glez_end);
        glez_end();
    }
#if EXTERNAL_DRAWING
    xoverlay_draw_end();
    {
        PROF_SECTION(draw_end__SDL_GL_MakeCurrent);
        SDL_GL_MakeCurrent(sdl_hooks::window, nullptr);
    }
#endif
}
}
}
