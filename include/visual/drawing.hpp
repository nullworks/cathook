/*
 * drawing.h
 *
 *  Created on: Oct 5, 2016
 *      Author: nullifiedcat
 */

#pragma once

#include <glez/font.hpp>
#include "common.hpp"

class CachedEntity;
class Vector;
class IClientEntity;
class CatEnum;
class VMatrix;

namespace fonts
{

extern std::unique_ptr<glez::font> esp;
extern std::unique_ptr<glez::font> menu;
} // namespace fonts

namespace fonts
{

// FIXME add menu fonts
extern unsigned long ESP;
extern unsigned long MENU;
extern unsigned long MENU_BIG;

void Update();

extern const std::vector<std::string> fonts;
extern CatEnum family_enum;
} // namespace fonts

constexpr rgba_t GUIColor()
{
    return colors::white;
}

void InitStrings();
void ResetStrings();
void AddCenterString(const std::string &string,
                     const rgba_t &color = colors::white);
void AddSideString(const std::string &string,
                   const rgba_t &color = colors::white);
void DrawStrings();

namespace draw
{

extern std::mutex draw_mutex;
extern VMatrix wts;

extern int width;
extern int height;
extern float fov;

void Initialize();

void UpdateWTS();
bool WorldToScreen(const Vector &origin, Vector &screen);
bool EntityCenterToScreen(CachedEntity *entity, Vector &out);

void InitGL();
void BeginGL();
void EndGL();
} // namespace draw
