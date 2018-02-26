/*
 * Menu.cpp
 *
 *  Created on: Mar 26, 2017
 *      Author: nullifiedcat
 */

#include "Menu.hpp"
#include "List.hpp"
#include "ItemSublist.hpp"
#include "Tooltip.hpp"

namespace menu { namespace ncc {

CatEnum menu_menu_menu_menu({"Visuals", "Menu"}); // MENU MENU MENU MENU
CatVarFloat scale(menu_menu_menu_menu, "gui_ncc_scale", 1, "NCC GUI Scale", "Defines scale of NCC gui", 0.5f, 4.0f);

Tooltip* tooltip = nullptr;;
Root* root = nullptr;

void ShowTooltip(const std::string& text) {
	tooltip->Show();
	tooltip->SetText(text);
}

bool init_done = false;

void Init() {
	root = new Root();
	root->Setup();
	init_done = true;
}

}}
