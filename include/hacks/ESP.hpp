/*
 * HEsp.h
 *
 *  Created on: Oct 6, 2016
 *      Author: nullifiedcat
 */

#pragma once

#include "common.hpp"

namespace hacks::shared::esp
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
    bool transparent{ false };
};

// Init
void Init();
extern std::array<ESPData, 2048> data;

void CreateMove();
void Draw();

// Entity Processing
void __attribute__((fastcall)) ProcessEntity(CachedEntity *ent);
void __attribute__((fastcall)) ProcessEntityPT(CachedEntity *ent);
void __attribute__((fastcall)) emoji(CachedEntity *ent);

// helper funcs
void __attribute__((fastcall)) Draw3DBox(CachedEntity *ent, const rgba_t &clr);
void __attribute__((fastcall)) DrawBox(CachedEntity *ent, const rgba_t &clr);
void BoxCorners(int minx, int miny, int maxx, int maxy, const rgba_t &color,
                bool transparent);
bool GetCollide(CachedEntity *ent);

// Strings
void AddEntityString(CachedEntity *entity, const std::string &string,
                     const rgba_t &color = colors::empty);
void SetEntityColor(CachedEntity *entity, const rgba_t &color);
void ResetEntityStrings();
} // namespace hacks::shared::esp
