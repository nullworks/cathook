/*
 * Tooltip.cpp
 *
 *  Created on: Mar 27, 2017
 *      Author: nullifiedcat
 */

#include "menu/ncc/Tooltip.hpp"
#include "menu/CTextLabel.h"
#include "common.hpp"
#include "GUI.h"
namespace menu
{
namespace ncc
{

Tooltip::Tooltip() : CTextLabel(XORSTR("ncc_tooltip"))
{
    SetZIndex(999);
    SetPadding(3, 2);
    SetMaxSize(220, -1);
    SetAutoSize(false);
    SetSize(220, -1);
    Props()->SetInt(XORSTR("font"), font_item);
}

void Tooltip::HandleCustomEvent(KeyValues *event)
{
    if (!strcmp(event->GetName(), XORSTR("scale_update")))
    {
        SetMaxSize(Item::psize_x * (float) scale, -1);
        SetSize(Item::psize_x * (float) scale, -1);
        SetText(GetText()); // To update word wrapping.
    }
    else if (!strcmp(event->GetName(), XORSTR("font_update")))
    {
        Props()->SetInt(XORSTR("font"), font_item);
    }
}

void Tooltip::Draw(int x, int y)
{
    const auto &size = GetSize();
    int originx      = x;
    int originy      = y;
    if (originx + size.first > draw::width)
        originx -= size.first;
    if (originx + size.second > draw::height)
        originy -= size.second;
    draw::DrawRect(x, y, size.first, size.second,
                   colorsint::Create(0, 0, 0, 130));
    draw::OutlineRect(x, y, size.first, size.second, NCGUIColor());
    draw::String(font_item, x + Props()->GetInt(XORSTR("padding_x")),
                 y + Props()->GetInt(XORSTR("padding_y")), colorsint::white, 2,
                 GetText());
}
}
}
