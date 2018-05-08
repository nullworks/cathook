/*
 * drawex.hpp
 *
 *  Created on: Nov 12, 2017
 *      Author: nullifiedcat
 */

#pragma once

#include <thread>

extern "C" {
#include <glez.h>
#if EXTERNAL_DRAWING
#include <xoverlay.h>
#endif
}

#define draw_api drawex::api

namespace drawex
{

namespace api
{

struct font_handle_t
{
    glez_font_t handle{ GLEZ_FONT_INVALID };
    std::string filename;
};

struct texture_handle_t
{
    glez_texture_t handle;
};

font_handle_t create_font(const char *path, float size);
texture_handle_t create_texture(const char *path);

void destroy_font(font_handle_t font);
void destroy_texture(texture_handle_t texture);

bool ready();

void initialize();

void draw_rect(float x, float y, float w, float h, const rgba_t &rgba);
void draw_rect_outlined(float x, float y, float w, float h, const rgba_t &rgba,
                        float thickness);
void draw_line(float x, float y, float dx, float dy, const rgba_t &rgba,
               float thickness);
void draw_rect_textured(float x, float y, float w, float h, const rgba_t &rgba,
                        texture_handle_t texture, float u, float v, float s,
                        float t, float a);
void draw_circle(float x, float y, float radius, const rgba_t &rgba,
                 float thickness, int steps);
void draw_string(float x, float y, const char *string, font_handle_t &font,
                 const rgba_t &rgba);
void draw_string_with_outline(float x, float y, const char *string,
                              font_handle_t &font, const rgba_t &rgba,
                              const rgba_t &rgba_outline, float thickness);
void get_string_size(const char *string, font_handle_t &font, float *x,
                     float *y);

void draw_begin();
void draw_end();
}
}
