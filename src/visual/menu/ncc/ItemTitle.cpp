/*
 * ItemTitle.cpp
 *
 *  Created on: Mar 26, 2017
 *      Author: nullifiedcat
 */

#include "menu/ncc/ItemTitle.hpp"
#include "menu/ncc/Menu.hpp"

#include "common.hpp"
namespace menu
{
namespace ncc
{

ItemTitle::ItemTitle(std::string title) : Item(XORSTR("ncc_list_title")), title(title)
{
}

void ItemTitle::Draw(int x, int y)
{
    Item::Draw(x, y);
    // nailed it
    bool brackets3 = Props()->GetBool(XORSTR("brackets3"), false);
    std::string str =
        format(brackets3 ? XORSTR(">>> ") : XORSTR(">> "), title, brackets3 ? XORSTR(" <<<") : XORSTR(" <<"));
    const auto &size = draw::GetStringLength(font_title, str);
    draw::String(font_title, x + ((Item::size_x - size.first) / 2), y,
                 colorsint::white, 2, str);
}
}
}
