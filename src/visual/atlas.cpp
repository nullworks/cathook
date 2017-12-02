/*
 * atlas.cpp
 *
 *  Created on: May 20, 2017
 *      Author: nullifiedcat
 */

#if ENABLE_VISUALS == 1

#include "common.hpp"

namespace textures {

sprite::sprite(float x, float y, float w, float h, const texture_atlas &atlas):
        nx(x / atlas.width),
        ny(y / atlas.height),
        nw(w / atlas.width),
        nh(h / atlas.height),
        atlas(atlas)
{
}

void sprite::draw(float scrx, float scry, float scrw, float scrh, const rgba_t& rgba) const
{
    draw_api::draw_rect_textured(scrx, scry, scrw, scrh, rgba, atlas.texture, nx, ny, nw, nh);
}

texture_atlas::texture_atlas(std::string filename, float width, float height):
        width(width),
        height(height)
{
    texture = draw_api::create_texture(filename.c_str());
}

sprite texture_atlas::create_sprite(float x, float y, float sx, float sy) const
{
    return sprite(x, y, sx, sy, *this);
}

texture_atlas& atlas()
{
    static texture_atlas object { DATA_PATH "/atlas.png", 1024, 512 };
    return object;
}

}

#endif
