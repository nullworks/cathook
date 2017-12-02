/*
 * atlas.hpp
 *
 *  Created on: May 20, 2017
 *      Author: nullifiedcat
 */

#ifndef ATLAS_HPP_
#define ATLAS_HPP_

#include "common.hpp"
#include "visual/drawex.hpp"

namespace textures
{

class texture_atlas;
class sprite;

class sprite
{
public:
    sprite(float x, float y, float w, float h, const texture_atlas &atlas);
public:
    void draw(float scrx, float scry, float scrw, float scrh, const rgba_t& rgba) const;
public:
    const float nx;
    const float ny;
    const float nw;
    const float nh;

    const texture_atlas &atlas;
};

class texture_atlas
{
public:
    texture_atlas(std::string filename, float width, float height);
public:
    sprite create_sprite(float x, float y, float sx, float sy) const;
public:
    const float width;
    const float height;

    draw_api::texture_handle_t texture;
};

texture_atlas& atlas();

}

#endif /* ATLAS_HPP_ */
