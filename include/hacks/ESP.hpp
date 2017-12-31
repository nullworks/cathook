/*
 * HEsp.h
 *
 *  Created on: Oct 6, 2016
 *      Author: nullifiedcat
 */

#ifndef HESP_H_
#define HESP_H_

#include "common.hpp"

namespace hacks
{
namespace shared
{
namespace esp
{

// Strings
class ESPString
{
public:
    std::string data{ "" };
    rgba_t color{ colors::empty };
};

// Cached data
class ESPData
{
public:
    int string_count{ 0 };
    std::array<ESPString, 16> strings{};
    rgba_t color{ colors::empty };
    bool needs_paint{ false };
    bool has_collide{ false };
    Vector collide_max{ 0, 0, 0 };
    Vector collide_min{ 0, 0, 0 };
};

//
extern std::array<ESPData, 2048> data;

void CreateMove();
void Draw();

// Entity Processing
void __attribute__((fastcall)) ProcessEntity(CachedEntity *ent);
void __attribute__((fastcall)) ProcessEntityPT(CachedEntity *ent);

// helper funcs
void __attribute__((fastcall)) DrawBox(CachedEntity *ent, const rgba_t &clr);
void BoxCorners(int minx, int miny, int maxx, int maxy, const rgba_t &color,
                bool transparent);
bool GetCollide(CachedEntity *ent);

// Strings
void AddEntityString(CachedEntity *entity, const std::string &string,
                     const rgba_t &color = colors::empty);
void SetEntityColor(CachedEntity *entity, const rgba_t &color);
void ResetEntityStrings();
}
}
}

#endif /* HESP_H_ */
