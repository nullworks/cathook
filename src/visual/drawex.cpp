/*
 * drawex.cpp
 *
 *  Created on: Nov 12, 2017
 *      Author: nullifiedcat
 */

#include "common.hpp"
#include "hack.hpp"

extern "C"
{
#include "overlay.h"
}

namespace drawex
{

namespace api
{

bool ready_state = false;
bool init = false;

font_handle_t    create_font(const char *path, float size)
{
    logging::Info("Creating font '%s':%f", path, size);
    font_handle_t result;
    result.filename = std::string(path);
    result.size = size;
    result.handle = 0;
    return result;
}

texture_handle_t create_texture(const char *path)
{
    return texture_handle_t { xoverlay_texture_load_png_rgba(path) };
}

void destroy_font(font_handle_t font)
{
    xoverlay_font_unload(font.handle);
}

void destroy_texture(texture_handle_t texture)
{
    xoverlay_texture_unload(texture.handle);
}

bool ready()
{
    return ready_state && init;
}

void initialize()
{
    int status = xoverlay_init();
    if (status < 0)
    {
        logging::Info("ERROR: could not initialize Xoverlay: %d", status);
    }
    else
    {
        logging::Info("Xoverlay initialized");
    }
    xoverlay_show();
}

void draw_rect(float x, float y, float w, float h, const rgba_t& rgba)
{
    xoverlay_draw_rect(x, y, w, h, *reinterpret_cast<const xoverlay_rgba_t *>(&rgba));
}

void draw_rect_outlined(float x, float y, float w, float h, const rgba_t& rgba, float thickness)
{
    xoverlay_draw_rect_outline(x, y, w, h, *reinterpret_cast<const xoverlay_rgba_t *>(&rgba), thickness);
}

void draw_line(float x, float y, float dx, float dy, const rgba_t& rgba, float thickness)
{
    xoverlay_draw_line(x, y, dx, dy, *reinterpret_cast<const xoverlay_rgba_t *>(&rgba), thickness);
}

void draw_rect_textured(float x, float y, float w, float h, const rgba_t& rgba, texture_handle_t texture, float u, float v, float s, float t)
{
    xoverlay_draw_rect_textured(x, y, w, h, *reinterpret_cast<const xoverlay_rgba_t *>(&rgba), texture.handle, u, v, s, t);
}

void draw_circle(float x, float y, float radius, const rgba_t& rgba, float thickness, int steps)
{
    xoverlay_draw_circle(x, y, radius, *reinterpret_cast<const xoverlay_rgba_t *>(&rgba), thickness, steps);
}

void draw_string(float x, float y, const char *string, font_handle_t& font, const rgba_t& rgba)
{
    if (!font.handle)
        font.handle = xoverlay_font_load(font.filename.c_str(), font.size);
    xoverlay_draw_string(x, y, string, font.handle, *reinterpret_cast<const xoverlay_rgba_t *>(&rgba), nullptr, nullptr);
}

void draw_string_with_outline(float x, float y, const char *string, font_handle_t& font, const rgba_t& rgba, const rgba_t& rgba_outline, float thickness)
{
    if (!font.handle)
        font.handle = xoverlay_font_load(font.filename.c_str(), font.size);
    xoverlay_draw_string_with_outline(x, y, string, font.handle, *reinterpret_cast<const xoverlay_rgba_t *>(&rgba), *reinterpret_cast<const xoverlay_rgba_t *>(&rgba_outline), thickness, 1, nullptr, nullptr);
    //xoverlay_draw_string(x, y, string, font.handle, *reinterpret_cast<const xoverlay_rgba_t *>(&rgba), nullptr, nullptr);
}

void get_string_size(const char *string, font_handle_t& font, float *x, float *y)
{
    if (!font.handle)
        font.handle = xoverlay_font_load(font.filename.c_str(), font.size);
    xoverlay_get_string_size(string, font.handle, x, y);
}

void draw_begin()
{
    PROF_SECTION(DRAWEX_draw_begin);
    xoverlay_draw_begin();
}

void draw_end()
{
    PROF_SECTION(DRAWEX_draw_end);
    xoverlay_draw_end();
}

}}


