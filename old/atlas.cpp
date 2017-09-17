/*
 * atlas.cpp
 *
 *  Created on: May 20, 2017
 *      Author: nullifiedcat
 */

#if ENABLE_VISUALS == 1

#include "atlas.hpp"

#include "drawing.h"
#include "drawgl.hpp"
#include "logging.h"

namespace textures {

AtlasTexture::AtlasTexture(float x, float y, float sx, float sy) {
	tex_coords[0] = ftgl::vec2{ (x + 0.5f) / atlas_width, (y + sy - 0.5f) / atlas_height };
	tex_coords[1] = ftgl::vec2{ (x - 0.5f + sx) / atlas_width, (y + 0.5f) / atlas_height };
}

void AtlasTexture::Draw(float x, float y, float sx, float sy) {
	drawgl::TexturedRect(x, y, sx, sy, tex_coords[0].x, tex_coords[0].y, tex_coords[1].x, tex_coords[1].y);
}

GLuint texture;

void Init() {
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, atlas_width, atlas_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &_binary_atlas_start);
}

}

#endif
