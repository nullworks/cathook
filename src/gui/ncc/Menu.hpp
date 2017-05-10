/*
 * Menu.hpp
 *
 *  Created on: Mar 26, 2017
 *      Author: nullifiedcat
 */

#ifndef MENU_HPP_
#define MENU_HPP_

#include "../../drawing.h"

#include "../guicommon.h"

#include "List.hpp"
#include "Item.hpp"
#include "ItemSublist.hpp"
#include "ItemTitle.hpp"
#include "ItemVariable.hpp"
#include "List.hpp"
#include "PlayerList.hpp"
#include "PlayerListEntry.hpp"
#include "Radar.hpp"
#include "Root.hpp"
#include "Tooltip.hpp"
#include "Background.hpp"
#include "Logo.hpp"

namespace menu { namespace ncc {

class List;
class Tooltip;

extern CatVar scale;
extern CatVar font_family;
extern CatVar font_title_family;

constexpr int psize_font_item = 12;
constexpr int psize_font_title = 14;

extern unsigned long font_title; // Verdana Bold 10px?
extern unsigned long font_item;  // Verdana 	 10px?

extern Tooltip* tooltip;
extern Root* root;

void RefreshFonts();
void ShowTooltip(const std::string& text);

void Init();
List& MainList();

}}

#endif /* MENU_HPP_ */
