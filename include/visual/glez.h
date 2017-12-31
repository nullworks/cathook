/*
 * glez.h
 *
 *  Created on: Dec 7, 2017
 *      Author: nullifiedcat
 */

#pragma once

/* Types */

typedef struct glez_vec4_s
{
    union {
        float data[4];
        struct
        {
            float r;
            float g;
            float b;
            float a;
        };
        struct
        {
            float x;
            float y;
            float z;
            float w;
        };
    };
} glez_vec4_t, glez_rgba_t;

typedef unsigned int glez_texture_t;
typedef unsigned int glez_font_t;

/* State functions */

void glez_init(int width, int height);

void glez_shutdown();

void glez_begin();

void glez_end();

void glez_resize(int width, int height);

/* Helper functions */

static inline glez_rgba_t
glez_rgba(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    glez_rgba_t result;
    result.r = (float) r / 255.0f;
    result.g = (float) g / 255.0f;
    result.b = (float) b / 255.0f;
    result.a = (float) a / 255.0f;
    return result;
}

/* Font-related functions */

#define GLEZ_FONT_COUNT 64
#define GLEZ_FONT_INVALID ((glez_font_t) 0xFFFFFFFF)

glez_font_t glez_font_load(const char *path, float size);

void glez_font_unload(glez_font_t handle);

void glez_font_string_size(glez_font_t font, const char *string, float *out_x,
                           float *out_y);

/* Texture-related functions */

#define GLEZ_TEXTURE_COUNT 64
#define GLEZ_TEXTURE_INVALID ((glez_texture_t) 0xFFFFFFFF)

glez_texture_t glez_texture_load_png_rgba(const char *path);

void glez_texture_unload(glez_texture_t handle);

void glez_texture_size(glez_texture_t handle, int *width, int *height);

/* Drawing functions */

void glez_line(float x, float y, float dx, float dy, glez_rgba_t color,
               float thickness);

void glez_rect(float x, float y, float w, float h, glez_rgba_t color);

void glez_rect_outline(float x, float y, float w, float h, glez_rgba_t color,
                       float thickness);

void glez_rect_textured(float x, float y, float w, float h, glez_rgba_t color,
                        glez_texture_t texture, float tx, float ty, float tw,
                        float th);

void glez_string(float x, float y, const char *string, glez_font_t font,
                 glez_rgba_t color, float *out_x, float *out_y);

void glez_string_with_outline(float x, float y, const char *string,
                              glez_font_t font, glez_rgba_t color,
                              glez_rgba_t outline_color, float outline_width,
                              int adjust_outline_alpha, float *out_x,
                              float *out_y);

void glez_circle(float x, float y, float radius, glez_rgba_t color,
                 float thickness, int steps);
