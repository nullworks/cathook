/*
 * fontapi.h
 *
 *  Created on: Nov 11, 2017
 *      Author: nullifiedcat
 */

#pragma once

typedef unsigned int xoverlay_font_handle_t;
#define XOVERLAY_FONT_COUNT 64
#define XOVERLAY_FONT_INVALID_HANDLE (xoverlay_font_handle_t)(-1)

xoverlay_font_handle_t
xoverlay_font_load(const char *path, float size);

void
xoverlay_font_unload(xoverlay_font_handle_t handle);

void
xoverlay_string_size(xoverlay_font_handle_t handle, const char *string, int *x, int *y);
