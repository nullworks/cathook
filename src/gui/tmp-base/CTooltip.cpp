/*
 * CTooltip.cpp
 *
 *  Created on: Jan 28, 2017
 *      Author: nullifiedcat
 */

#include <limits>

#include "../../framework/drawing.hpp"
#include "GUI.h"

#include "CTooltip.h"

CTooltip::CTooltip(IWidget* parent) : CTextLabel("tooltip", parent) {
	SetZIndex(999);
	SetPadding(4, 4);
	SetMaxSize(100, -1);
	SetAutoSize(false);
	SetSize(100, -1);
}

void CTooltip::Draw(int x, int y) {
	auto size = GetSize();
	draw::RectFilled(x, y, size.first, size.second, Color(0, 0, 0, 230));
	draw::Rect(x, y, size.first, size.second, GUIColor());
	draw::String(GetText().c_str(), x + Props()->GetInt("padding_x"), y + Props()->GetInt("padding_y"), draw::default_font, draw::default_font_size, GUIColor());
}
