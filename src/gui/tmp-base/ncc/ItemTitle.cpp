/*
 * ItemTitle.cpp
 *
 *  Created on: Mar 26, 2017
 *      Author: nullifiedcat
 */

#include <sstream>

#include "ItemTitle.hpp"
#include "Menu.hpp"

namespace menu { namespace ncc {

ItemTitle::ItemTitle(std::string title) : Item("ncc_list_title"), title(title) {}

void ItemTitle::Draw(int x, int y) {
	Item::Draw(x, y);
	// nailed it
	bool brackets3 = Props()->GetBool("brackets3");
	std::stringstream tmp;
	tmp << (brackets3 ? ">>> " : ">> ") << title << (brackets3 ? " <<<" : " <<");
	auto size = draw::GetStringLength(tmp.str().c_str(), draw::default_font, draw::default_font_size);
	draw::String(tmp.str().c_str(), x + ((Item::size_x - size.first) / 2), y, draw::default_font, draw::default_font_size, colors::white);
}

}}
