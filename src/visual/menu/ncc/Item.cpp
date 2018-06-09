/*
 * Item.cpp
 *
 *  Created on: Mar 26, 2017
 *      Author: nullifiedcat
 */

#include "menu/ncc/Item.hpp"
#include "menu/ncc/Menu.hpp"

#include "common.hpp"
namespace menu
{
namespace ncc
{

int Item::size_x = 0;
int Item::size_y = 0;

Item::Item(std::string name) : CBaseWidget(name, nullptr)
{
    SetSize(psize_x, psize_y);
    SetMaxSize(psize_x, psize_y);
}

void Item::Draw(int x, int y)
{
    const auto &size = GetSize();
    // draw::DrawRect(x, y, size.first, size.second, colorsint::red);
    draw::DrawRect(x, y, size.first, size.second,
                   colorsint::Create(0, 0, 0, gui_color_a));
    if (IsHovered())
    {
        draw::DrawRect(x, y, size.first, size.second,
                       colorsint::Transparent(NCGUIColor(), 0.32f));
    }
}

void Item::HandleCustomEvent(KeyValues *event)
{
    if (!strcmp(event->GetName(), XORSTR("scale_update")))
    {
        size_x = psize_x * (float) scale;
        size_y = psize_y * (float) scale;
        SetSize(size_x, size_y);
        SetMaxSize(size_x, size_y);
    }
}
}
}
