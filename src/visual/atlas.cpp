/*
 * atlas.cpp
 *
 *  Created on: May 20, 2017
 *      Author: nullifiedcat
 */

#include "common.hpp"

namespace textures
{

sprite::sprite(float x, float y, float w, float h, texture_atlas &atlas)
    : nx(x / atlas.width), ny(y / atlas.height), nw(w / atlas.width),
      nh(h / atlas.height), atlas(atlas)
{
}
void sprite::setsprite(float x, float y, float w, float h)
{
    nx = x;
    ny = y;
    nw = w;
    nh = h;
}
void sprite::draw(float scrx, float scry, float scrw, float scrh,
                  const rgba_t &rgba)
{
    draw_api::draw_rect_textured(scrx, scry, scrw, scrh, rgba, atlas.texture,
                                 nx, ny, nw, nh, 0);
}

texture_atlas::texture_atlas(std::string filename, float width, float height)
    : width(width), height(height)
{
    texture = draw_api::create_texture(filename.c_str());
}

sprite texture_atlas::create_sprite(float x, float y, float sx, float sy)
{
    return sprite(x, y, sx, sy, *this);
}

texture_atlas &atlas()
{
    static texture_atlas object{ DATA_PATH "/res/atlas.png", 1024, 512 };
    return object;
}
}
