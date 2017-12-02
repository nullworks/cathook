/*
 * textureapi.h
 *
 *  Created on: Nov 11, 2017
 *      Author: nullifiedcat
 */

#pragma once

typedef unsigned xoverlay_texture_handle_t;

xoverlay_texture_handle_t
xoverlay_texture_load_png_rgba(const char *path);

void
xoverlay_texture_unload(xoverlay_texture_handle_t handle);

void
xoverlay_texture_get_size(xoverlay_texture_handle_t handle, int *width, int *height);
