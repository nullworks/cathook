/*
 * Menu.hpp
 *
 *  Created on: Mar 26, 2017
 *      Author: nullifiedcat
 */

#ifndef MENU_HPP_
#define MENU_HPP_

#include "../../../framework/drawing.hpp"

#include "../guicommon.h"

#include "List.hpp"
#include "Item.hpp"
#include "ItemSublist.hpp"
#include "ItemTitle.hpp"
#include "ItemVariable.hpp"
#include "List.hpp"
#include "Root.hpp"
#include "Tooltip.hpp"

namespace menu { namespace ncc {

class List;
class Tooltip;

extern CatVarFloat scale;

constexpr int psize_font_item = 12;
constexpr int psize_font_title = 14;

extern unsigned long font_title; // Verdana Bold 10px?
extern unsigned long font_item;  // Verdana 	 10px?

extern Tooltip* tooltip;
extern Root* root;

void RefreshFonts();
void ShowTooltip(const std::string& text);

void Init();

}}

#endif /* MENU_HPP_ */
