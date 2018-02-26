/*
 * Tooltip.cpp
 *
 *  Created on: Mar 27, 2017
 *      Author: nullifiedcat
 */

#include "Tooltip.hpp"
#include "../CTextLabel.h"
#include "../../../framework/drawing.hpp"
#include "../GUI.h"
namespace menu { namespace ncc {

Tooltip::Tooltip() : CTextLabel("ncc_tooltip") {
	SetZIndex(999);
	SetPadding(3, 2);
	SetMaxSize(220, -1);
	SetAutoSize(false);
	SetSize(220, -1);
	Props()->SetInt("font", draw::default_font);
}

void Tooltip::HandleCustomEvent(KeyValues* event) {
	/*//if (!strcmp(event->GetName(), "scale_update")) {
		SetMaxSize(Item::psize_x * (float)scale, -1);
		SetSize(Item::psize_x * (float)scale, -1);
		SetText(GetText()); // To update word wrapping.
	} else if (!strcmp(event->GetName(), "font_update")) {
		Props()->SetInt("font", font_item);
	}*/
}

void Tooltip::Draw(int x, int  y) {
	const auto& size = GetSize();
	int originx = x;
	int originy = y;
	if (originx + size.first > input::bounds.first) originx -= size.first;
	if (originx + size.second > input::bounds.second) originy -= size.second;
	auto bgcolor = Color(0, 0, 0, 77); //colors::Create(70, 86, 47, 28);
	auto fgcolor = Color(200, 200, 190, 255);
	draw::RectFilled(x, y, size.first, size.second, bgcolor);
	draw::Rect(x, y, size.first, size.second, GUIColor());
	draw::String(GetText().c_str(), x + Props()->GetInt("padding_x"), y + Props()->GetInt("padding_y"), draw::default_font, draw::default_font_size, fgcolor);
}

}}
