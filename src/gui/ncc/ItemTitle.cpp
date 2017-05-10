/*
 * ItemTitle.cpp
 *
 *  Created on: Mar 26, 2017
 *      Author: nullifiedcat
 */

#include "ItemTitle.hpp"
#include "Menu.hpp"

#include "../../common.h"

namespace menu { namespace ncc {

ItemTitle::ItemTitle(std::string title) : Item("ncc_list_title"), title(title) {}

void ItemTitle::Draw(int x, int y) {
	Item::Draw(x, y);
	// nailed it
	bool brackets3 = Props()->GetBool("brackets3", false);
	std::string str = format(brackets3 ? ">>> " : ">> ", title, brackets3 ? " <<<" : " <<");
	const auto& size = draw::GetStringLength(font_title, str);
	draw::String(font_title, x + ((Item::size_x - size.first) / 2), y, colors::white, 2, str);
}

}}
