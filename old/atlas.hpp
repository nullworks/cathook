/*
 * atlas.hpp
 *
 *  Created on: May 20, 2017
 *      Author: nullifiedcat
 */

#ifndef ATLAS_HPP_
#define ATLAS_HPP_

#include <GL/glew.h>
#include <GL/gl.h>

extern "C" {
#include <vec234.h>
}

extern char _binary_atlas_start;

namespace textures {

constexpr float atlas_width = 1024.0f;
constexpr float atlas_height = 512.0f;

class AtlasTexture {
public:
	AtlasTexture(float x, float y, float sx, float sy);
	void Draw(float x, float y, float sx, float sy);
public:
	ftgl::vec2 tex_coords[2];
};

extern GLuint texture;

void Init();
}

#endif /* ATLAS_HPP_ */
