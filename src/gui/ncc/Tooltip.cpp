/*
 * Tooltip.cpp
 *
 *  Created on: Mar 27, 2017
 *      Author: nullifiedcat
 */

#include "Tooltip.hpp"
#include "../CTextLabel.h"
#include "../../common.h"

namespace menu { namespace ncc {

Tooltip::Tooltip() : CTextLabel("ncc_tooltip") {
	SetZIndex(999);
	SetPadding(3, 2);
	SetMaxSize(220, -1);
	SetAutoSize(false);
	SetSize(220, -1);
	Props()->SetInt("font", font_item);
}

void Tooltip::HandleCustomEvent(KeyValues* event) {
	if (!strcmp(event->GetName(), "scale_update")) {
		SetMaxSize(Item::psize_x * (float)scale, -1);
		SetSize(Item::psize_x * (float)scale, -1);
		SetText(GetText()); // To update word wrapping.
	} else if (!strcmp(event->GetName(), "font_update")) {
		Props()->SetInt("font", font_item);
	}
}

void Tooltip::Draw(int x, int  y) {
	const auto& size = GetSize();
	int originx = x;
	int originy = y;
	if (originx + size.first > draw::width) originx -= size.first;
	if (originx + size.second > draw::height) originy -= size.second;
	static int bgcolor = colors::Create(0, 0, 0, 77); //colors::Create(70, 86, 47, 28);
	static int fgcolor = colors::Create(200, 200, 190, 255);
	draw::DrawRect(x, y, size.first, size.second, bgcolor);
	draw::OutlineRect(x, y, size.first, size.second, GUIColor());
	draw::String(font_item, x + Props()->GetInt("padding_x"), y + Props()->GetInt("padding_y"), fgcolor, 2, GetText());
}

}}
