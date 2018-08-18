/*
 * atlas.hpp
 *
 *  Created on: May 20, 2017
 *      Author: nullifiedcat
 */

#pragma once

#include <glez/texture.hpp>
#include "common.hpp"

namespace textures
{

class texture_atlas;
class sprite;

class sprite
{
public:
    sprite(float x, float y, float w, float h, texture_atlas &atlas);

public:
    void draw(float scrx, float scry, float scrw, float scrh,
              const rgba_t &rgba);

public:
    float nx;
    float ny;
    float nw;
    float nh;

    texture_atlas &atlas;
};

class texture_atlas
{
public:
    texture_atlas(std::string filename, float width, float height);

public:
    sprite create_sprite(float x, float y, float sx, float sy);

public:
    const float width;
    const float height;

    glez::texture texture;
};

texture_atlas &atlas();
} // namespace textures
