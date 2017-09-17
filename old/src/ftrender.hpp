/*
 * ftrender.hpp
 *
 *  Created on: May 20, 2017
 *      Author: nullifiedcat
 */

#ifndef FTRENDER_HPP_
#define FTRENDER_HPP_

#include "common.h"

extern "C" {
#include <texture-font.h>
}

extern ftgl::texture_atlas_t* atlas;

void FTGL_PreInit();
void FTGL_Init();
void FTGL_Draw(const std::string& text, int x, int y, ftgl::texture_font_t* font, const rgba_t& color = colors::white, int *size_x = nullptr, int *size_y = nullptr);
void FTGL_NewFrame();
void FTGL_Render();
void FTGL_StringLength(const std::string& text, ftgl::texture_font_t* font, int *size_x = nullptr, int *size_y = nullptr);
void FTGL_ChangeFont(ftgl::texture_font_t** font, const char* newfont);

#endif /* FTRENDER_HPP_ */
